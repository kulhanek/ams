// =============================================================================
// AMS - Advanced Module System
// -----------------------------------------------------------------------------
//    Copyright (C) 2012      Petr Kulhanek, kulhanek@chemi.muni.cz
//
//     This program is free software; you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation; either version 2 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License along
//     with this program; if not, write to the Free Software Foundation, Inc.,
//     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// =============================================================================

#include "ListDirsCmd.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <AMSGlobalConfig.hpp>
#include <Cache.hpp>
#include <AmsUUID.hpp>
#include <DirectoryEnum.hpp>
#include <prefix.h>
#include <Site.hpp>
#include <ErrorSystem.hpp>
#include <Shell.hpp>
#include <Utils.hpp>
#include <XMLIterator.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//------------------------------------------------------------------------------

MAIN_ENTRY(CListDirsCmd)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CListDirsCmd::CListDirsCmd(void)
{
    DirectoryTree = CNodeItemPtr(new CNodeItem);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CListDirsCmd::Init(int argc, char* argv[])
{
    // encode program options
    int result = Options.ParseCmdLine(argc,argv);

    // should we exit or was it error?
    if( result != SO_CONTINUE ) return(result);

    // attach verbose stream to terminal stream and set desired verbosity level
    vout.Attach(Console);
    if( Options.GetOptVerbose() ) {
        vout.Verbosity(CVerboseStr::high);
    } else {
        vout.Verbosity(CVerboseStr::low);
    }

    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << endl;
    vout << "# ==============================================================================" << endl;
    vout << "# ams-list-dirs (AMS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    vout << low;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CListDirsCmd::Run(void)
{
    // scan software repository
    CFileName softrepo = CShell::GetSystemVariable("SOFTREPO");
    if( softrepo == NULL ){
        ES_ERROR("SOFTREPO variable is not defined");
        return(false);
    }
    DirectoryTree->ScanSoftRepoTree(softrepo,4);

    // scan all sites and add nodes by build AMS_PACKAGE_DIR
    if( Options.IsOptSitesSet() == false ){
        // all sites
        ScanAllSiteBuilds();
    } else {
        if( ScanUserSiteBuilds(Options.GetOptSites()) == false ) return(false);
    }

    if( Options.GetArgType() == "missing" ){
        // print missing records
        DirectoryTree->PrintMissing(vout,softrepo);
    }

    if( Options.GetArgType() == "orphans" ){
        // print missing records
        DirectoryTree->CountOrphanedChilds();
        DirectoryTree->PrintOrphans(vout,softrepo);
    }

    if( Options.GetArgType() == "existing" ){
        // print missing records
        DirectoryTree->PrintExisting(vout,softrepo);
    }

    if( Options.GetArgType() == "all" ){
        // print all records
        DirectoryTree->PrintTree(vout,softrepo,0);
    }

    return(true);
}

//------------------------------------------------------------------------------

void CListDirsCmd::ScanAllSiteBuilds(void)
{
    CFileName site_dir = CFileName(ETCDIR) / "sites";
    CDirectoryEnum dir_enum(site_dir);

    dir_enum.StartFindFile("*");
    CFileName site_sid;
    while( dir_enum.FindFile(site_sid) ) {
        if( site_sid == "." ) continue;
        if( site_sid == ".." ) continue;
        CAmsUUID    site_id;
        CSite       site;
        if( site_id.LoadFromString(site_sid) == false ) continue;
        if( site.LoadConfig(site_sid) == false ) continue;
        ScanSiteBuilds(site);
    }
    dir_enum.EndFindFile();
}

//------------------------------------------------------------------------------

bool CListDirsCmd::ScanUserSiteBuilds(const CSmallString& sites)
{
    list<string> site_list;
    string       ssites(sites);
    split(site_list,ssites,is_any_of(","));

    list<string>::iterator it = site_list.begin();
    list<string>::iterator ie = site_list.end();

    while( it != ie ){
        CSite       site;
        CAmsUUID    site_id = CUtils::GetSiteID(*it);
        if( site.LoadConfig(site_id.GetFullStringForm()) == false ) return(false);
        ScanSiteBuilds(site);
        it++;
    }

    return(true);
}

//------------------------------------------------------------------------------

void CListDirsCmd::ScanSiteBuilds(const CSite& site)
{
    GlobalConfig.SetActiveSiteID(site.GetID());
    Cache.LoadCache(false);

    CXMLElement* p_mele = Cache.GetRootElementOfCache();
    if( p_mele == NULL ){
        RUNTIME_ERROR("unable to find root cache element");
    }

    CXMLElement*     p_ele;
    CXMLElement*     p_sele;

    CXMLIterator    I(p_mele);

    while( (p_ele = I.GetNextChildElement("module")) != NULL ) {
        CSmallString name;
        p_ele->GetAttribute("name",name);
        CXMLElement* p_list = p_ele->GetFirstChildElement("builds");
        CXMLIterator    J(p_list);
        while( (p_sele = J.GetNextChildElement("build")) != NULL ) {
            CSmallString ver,arch,mode;
            p_sele->GetAttribute("ver",ver);
            p_sele->GetAttribute("arch",arch);
            p_sele->GetAttribute("mode",mode);
            CFileName package_dir;
            package_dir = Cache.GetVariableValue(p_sele,"AMS_PACKAGE_DIR");
            if( package_dir != NULL ){
                CSmallString build_name;
                build_name << name << ":" << ver << ":" << arch << ":" << mode;
                DirectoryTree->AddPackageDir(package_dir,build_name);
            }
        }
    }
}

//------------------------------------------------------------------------------

void CListDirsCmd::Finalize(void)
{
    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << "# ==============================================================================" << endl;
    vout << "# ams-list-dirs (AMS utility) terminated at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    if( ErrorSystem.IsError() || (ErrorSystem.IsAnyRecord() && Options.GetOptVerbose()) ){
        vout << low;
        ErrorSystem.PrintErrors(vout);
    }

    vout << endl;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


