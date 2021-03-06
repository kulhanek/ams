// =============================================================================
// AMS - Advanced Module System
// -----------------------------------------------------------------------------
//    Copyright (C) 2016      Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include "RepoIndexCreateAMS.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <AMSGlobalConfig.hpp>
#include <Cache.hpp>
#include <AmsUUID.hpp>
#include <DirectoryEnum.hpp>
#include <Site.hpp>
#include <ErrorSystem.hpp>
#include <Shell.hpp>
#include <Utils.hpp>
#include <XMLIterator.hpp>
#include <XMLParser.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <vector>
#include <list>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <FSIndex.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//------------------------------------------------------------------------------

MAIN_ENTRY(CRepoIndexCreateAMS)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRepoIndexCreateAMS::CRepoIndexCreateAMS(void)
{
    NumOfAllBuilds = 0;
    NumOfNonSoftRepoBuilds = 0;
    NumOfSharedBuilds = 0;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CRepoIndexCreateAMS::Init(int argc, char* argv[])
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
    vout << "# ams-repoindex-create (AMS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    vout << high;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CRepoIndexCreateAMS::Run(void)
{
    // scan software repository
    SoftRepo = CShell::GetSystemVariable("SOFTREPO");
    if( SoftRepo == NULL ){
        ES_ERROR("SOFTREPO variable is not defined");
        return(false);
    }

    // load site aliases
    if( LoadSiteAliases() == false ) return(false);

    // create list of builds
    vout << endl;
    vout << "# Assembling list of builds ..." << endl;
    if( ListSites(Options.GetOptSites()) == false ) return(false);

    vout << "  = Sites: " << FinalListOfSites << endl;
    vout << "  > Number of module builds                              = " << NumOfAllBuilds << endl;
    vout << "  > Number of unique builds                              = " << NumOfUniqueBuilds << endl;
    vout << "  > Number of builds (no AMS_PACKAGE_DIR)                = " << NumOfNonSoftRepoBuilds << endl;
    vout << "  > Number of shared builds (the same AMS_PACKAGE_DIR)   = " << NumOfSharedBuilds << endl;
    vout << "  > Number of builds for index (AMS_PACKAGE_DIR and dir) = " << BuildPaths.size() << endl;

    // calculate index
    vout << endl;
    vout << "# Calculating index ..." << endl;

    map<CBuildId,CFileName>::iterator it = BuildPaths.begin();
    map<CBuildId,CFileName>::iterator ie = BuildPaths.end();

    CFSIndex index;
    index.RootDir = SoftRepo;
    index.PersonalSite = Options.GetOptPersonalSite();

    while( it != ie ){
        CBuildId  build_id = it->first;
        CFileName build_path(it->second);
        string sha1 = index.CalculateBuildHash(build_path);
        BuildIndexes[build_id] = sha1;
        vout << sha1 << " " << build_id.Prefix << "/" << build_id.Name << endl;
        it++;
    }

    vout << endl;
    vout << "# Statistics ..." << endl;
    vout << "  > Number of stat objects  = " << index.NumOfStats << endl;

    vout << endl;
    vout << "# Saving index ..." << endl;

    ofstream ofs(Options.GetArgOutputFile());
    if( ! ofs ){
        ES_ERROR("Unable to open the index file for writing!");
        return(false);
    }

    it = BuildPaths.begin();

    while( it != ie ){
        CBuildId  build_id = it->first;
        string sha1 = BuildIndexes[build_id];
        ofs << "* " << sha1 << " " << build_id.Prefix << "/" << build_id.Name << " " << it->second << endl;
        it++;
    }

    if( ! ofs ){
        ES_ERROR("The index was not saved due to error!");
        return(false);
    }

    ofs.close();

    return(true);
}

//------------------------------------------------------------------------------

bool CRepoIndexCreateAMS::ListSites(const CSmallString& sites)
{
    // split sites into individual words
    string ssites = string(sites);

    list<string> arg_list;
    split(arg_list,ssites,is_any_of(","));

    list<string>::iterator    it = arg_list.begin();
    list<string>::iterator    ie = arg_list.end();

    list<string> site_list;

    while( it != ie ){
        // is it alias?
        list<string> alias_list = SiteAliases[*it];
        if( alias_list.size() > 0 ){
            site_list.insert(site_list.end(),alias_list.begin(),alias_list.end());
        } else{
            site_list.push_back(*it);
        }
        it++;
    }

    // sort list and keep only unique items
    site_list.sort();
    site_list.unique();

    FinalListOfSites = join(site_list,",");

    it = site_list.begin();
    ie = site_list.end();
    while( it != ie ){
        if( ListSiteBuilds(*it) == false ) return(false);
        it++;
    }
    return(true);
}

//------------------------------------------------------------------------------

bool CRepoIndexCreateAMS::ListSiteBuilds(const CSmallString& site_name)
{
    CAmsUUID    site_id = CUtils::GetSiteID(site_name);

    if( ! site_id.IsValidUUID() ){
        CSmallString error;
        error << "site '" << site_name << "' does not exist";
        ES_ERROR(error);
        return(false);
    }

    AMSGlobalConfig.SetActiveSiteID(site_id);
    if( Cache.LoadCache(false) == false ){
        CSmallString error;
        error << "unable to load cache for " << site_name;
        ES_ERROR(error);
        return(false);
    }

    CXMLElement* p_mele = Cache.GetRootElementOfCache();
    if( p_mele == NULL ){
        CSmallString error;
        error << "unable to find root cache element for " << site_name;
        ES_ERROR(error);
        return(false);
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

            NumOfAllBuilds++;

            CSmallString ver,arch,mode,prefix;
            p_sele->GetAttribute("ver",ver);
            p_sele->GetAttribute("arch",arch);
            p_sele->GetAttribute("mode",mode);
            p_sele->GetAttribute("prefix",prefix);

            // default values
            if( prefix == "" ) prefix = "common";


            // register build
            CBuildId build_id;
            build_id.Prefix = prefix;
            stringstream str;
            str << name << ":" << ver << ":" << arch << ":" << mode;
            build_id.Name = str.str();

            if( UniqueBuilds.count(build_id) == 1 ) continue;
            UniqueBuilds.insert(build_id);
            NumOfUniqueBuilds++;

            CFileName package_dir;
            package_dir = Cache.GetVariableValue(p_sele,"AMS_PACKAGE_DIR");
            if( package_dir == NULL ){
                NumOfNonSoftRepoBuilds++;
                continue;
            }

            CFileName path;
            if( package_dir[0] == '/' ){
                path = package_dir;
            } else {
                path = SoftRepo / package_dir;
            }

            if( Options.GetOptPersonalSite() == false ){
                // ignore this test for personal site as the build might not be synchronized yet
                if( CFileSystem::IsDirectory(path) == false ){
                    CSmallString error;
                    error << build_id.Name << " -> AMS_PACKAGE_DIR: " << package_dir << " does not exist!";
                    ES_ERROR(error);
                    return(false);
                }
            }

            // register build
            if( UniqueBuildPaths.count(string(path)) == 1 ){
                // already registered
                NumOfSharedBuilds++;
                continue;
            }
            UniqueBuildPaths.insert(string(path));

            // register build for index
            BuildPaths[build_id] = package_dir;
        }
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CRepoIndexCreateAMS::LoadSiteAliases(void)
{
    CFileName aliases_name = AMSGlobalConfig.GetETCDIR() / "map" / "aliases.xml";

    // generate all site alias
    list<string> all_sites;
    GetAllSites(all_sites);
    SiteAliases["all"] = all_sites;

    if( CFileSystem::IsFile(aliases_name) == false ){
        return(true); // no aliases found
    }

    CXMLDocument    xml_doc;
    CXMLParser      xml_parser;
    xml_parser.SetOutputXMLNode(&xml_doc);

    if( xml_parser.Parse(aliases_name) == false ) {
        CSmallString error;
        error <<  "unable to parse aliases file (" << aliases_name << ")";
        ES_ERROR(error);
        return(false);
    }

    CXMLElement* p_alias = xml_doc.GetChildElementByPath("aliases/alias");
    while( p_alias != NULL ){
        string name;
        string sites;
        p_alias->GetAttribute("name",name);
        p_alias->GetAttribute("sites",sites);

        list<string> site_list;
        split(site_list,sites,is_any_of(","));
        site_list.sort();
        site_list.unique();

        SiteAliases[name] = site_list;
        p_alias = p_alias->GetNextSiblingElement();
    }
    return(true);
}

//------------------------------------------------------------------------------

void CRepoIndexCreateAMS::GetAllSites(std::list<std::string>& sites)
{
    sites.clear();

    CFileName site_dir = AMSGlobalConfig.GetETCDIR() / "sites";
    CDirectoryEnum dir_enum(site_dir);

    dir_enum.StartFindFile("*");
    CFileName site_sid;
    while( dir_enum.FindFile(site_sid) ) {
        CAmsUUID    site_id;
        CSite       site;
        if( site_id.LoadFromString(site_sid) == false ) continue;
        if( site.LoadConfig(site_sid) == false ) continue;

        sites.push_back(string(site.GetName()));
    }
    dir_enum.EndFindFile();

    // sort list
    sites.sort();
}

//------------------------------------------------------------------------------

void CRepoIndexCreateAMS::Finalize(void)
{
    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << endl;
    vout << "# ==============================================================================" << endl;
    vout << "# ams-repoindex-create (AMS utility) terminated at " << dt.GetSDateAndTime() << endl;
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

bool CBuildId::operator < (const CBuildId& bp_id) const
{
    if (Prefix < bp_id.Prefix)  return(true);
    if (Prefix > bp_id.Prefix)  return(false);
    // Otherwise a are equal
    if (Name < bp_id.Name)  return(true);
    if (Name > bp_id.Name)  return(false);
    // Otherwise both are equal
    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


