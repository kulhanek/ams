// =============================================================================
// AMS
// -----------------------------------------------------------------------------
//    Copyright (C) 2011      Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2001-2008 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include "CacheCmd.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <AMSGlobalConfig.hpp>
#include <Cache.hpp>
#include <Utils.hpp>
#include <PrintEngine.hpp>
#include <AmsUUID.hpp>
#include <DirectoryEnum.hpp>
#include <Site.hpp>
#include <ErrorSystem.hpp>
#include <XMLParser.hpp>
#include <FileSystem.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//------------------------------------------------------------------------------

MAIN_ENTRY(CCacheCmd)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CCacheCmd::CCacheCmd(void)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CCacheCmd::Init(int argc, char* argv[])
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
    vout << "# ams-cache (AMS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    vout << low;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CCacheCmd::Run(void)
{
    // overwrite active site setup
    if( Options.IsOptSiteSet() == true ) {

        CSmallString site_sid;

        if( (Options.GetOptSite().GetLength() > 0) && (Options.GetOptSite()[0] == '{') ) {
            site_sid = Options.GetOptSite();
        } else {
            site_sid = CUtils::GetSiteID(Options.GetOptSite());
        }

        // find site id
        if( site_sid == NULL ) {
            CSmallString error;
            error << "unable to get ID of site '" << Options.GetOptSite() << "'";
            ES_ERROR(error);
            return(false);
        }
        AMSGlobalConfig.SetActiveSiteID(site_sid);
    }

    if( (Options.GetArgAction() != "rebuildall") && (Options.GetArgAction() != "syntaxall") ) {
        if( AMSGlobalConfig.GetActiveSiteID() == NULL ) {
            ES_ERROR("no site is active");
            return(false);
        }
    }

    //-----------------------------------------------
    if( Options.GetArgAction() == "rebuild" ) {
        vout << "  # full cache" << endl;
        // rebuild cache
        if( Cache.RebuildCache(vout,true) == false) return(false);
        // save cache
        if( Cache.SaveCache(true) == false) return(false);

        vout << "  # compressed cache" << endl;
        // rebuild cache
        if( Cache.RebuildCache(vout,false) == false) return(false);
        // remove doc
        Cache.RemoveDocumentation();
        // save cache
        if( Cache.SaveCache(false) == false) return(false);
        return(true);
    }
    //-----------------------------------------------
    else if( Options.GetArgAction() == "split" ) {
        // initialze AMS cache
        if( Cache.LoadCache() == false) {
            ES_ERROR("unable to load AMS cache");
            return(false);
        }
        // split cache
        if( Cache.SplitCache(vout) == false) return(false);
        return(true);
    }
    //-----------------------------------------------
    else if( Options.GetArgAction() == "splitmore" ) {
        // initialze AMS cache
        if( Cache.LoadCache() == false) {
            ES_ERROR("unable to load AMS cache");
            return(false);
        }
        // split cache
        if( Cache.SplitMoreCache(vout) == false) return(false);
        return(true);
    }
    //-----------------------------------------------
    else if( Options.GetArgAction() == "syntax" ) {
        vout << endl;

        // rebuild cache
        vout << "#  Building the AMS cache for the site : " << AMSGlobalConfig.GetActiveSiteName() << endl;
        if( Cache.RebuildCache(vout,false) == false) return(false);
        vout << endl;

        // check syntax
        vout << "#  Checking the cache syntax for the site : " << AMSGlobalConfig.GetActiveSiteName() << endl;
        if( Cache.CheckCacheSyntax(vout) == false) return(false);
        vout << endl;

        return(true);
    }

    //-----------------------------------------------
    else if( Options.GetArgAction() == "deps" ) {
        // initialze AMS cache
        if( Cache.LoadCache() == false) {
            ES_ERROR("unable to load AMS cache");
            return(false);
        }

        // initialize AMS print engine
        if( PrintEngine.LoadConfig() == false) {
            ES_ERROR("unable to load print config");
            return(false);
        }

        // set output stream
        PrintEngine.SetOutputStream(vout);

        // print deps
        PrintEngine.PrintRawDependencies();
        return(true);
    }
    //-----------------------------------------------
    else if( Options.GetArgAction() == "allmods" ) {
        // initialze AMS cache
        if( Cache.LoadCache() == false) {
            ES_ERROR("unable to load AMS cache");
            return(false);
        }
        // initialize AMS print engine
        if( PrintEngine.LoadConfig() == false) {
            ES_ERROR("unable to load print config");
            return(false);
        }

        // set output stream
        PrintEngine.SetOutputStream(vout);

        // print all builds
        PrintEngine.PrintRawAllModules();
        return(true);
    }
    //-----------------------------------------------
    else if( Options.GetArgAction() == "allbuilds" ) {
        // initialze AMS cache
        if( Cache.LoadCache() == false) {
            ES_ERROR("unable to load AMS cache");
            return(false);
        }
        // initialize AMS print engine
        if( PrintEngine.LoadConfig() == false) {
            ES_ERROR("unable to load print config");
            return(false);
        }

        // set output stream
        PrintEngine.SetOutputStream(vout);

        // print all builds
        PrintEngine.PrintRawAllBuilds();
        return(true);
    }
    //-----------------------------------------------
    else if( Options.GetArgAction() == "getbuilds" ) {
        // initialze AMS cache
        if( Cache.LoadCache() == false) {
            ES_ERROR("unable to load AMS cache");
            return(false);
        }
        // initialize AMS print engine
        if( PrintEngine.LoadConfig() == false) {
            ES_ERROR("unable to load print config");
            return(false);
        }

        // set output stream
        PrintEngine.SetOutputStream(vout);

        // print all builds
        PrintEngine.PrintRawBuilds(Options.GetProgArg(1));
        return(true);
    }
    //-----------------------------------------------
    else if( Options.GetArgAction() == "getvariable" ) {
        // initialze AMS cache
        if( Cache.LoadCache() == false) {
            ES_ERROR("unable to load AMS cache");
            return(false);
        }
        // set output stream
        PrintEngine.SetOutputStream(vout);

        return( PrintEngine.PrintRawVariable(Options.GetProgArg(1),Options.GetProgArg(2)) );
    }
    //-----------------------------------------------
    else if( Options.GetArgAction() == "archs" ) {
        // initialze AMS cache
        if( Cache.LoadCache() == false) {
            ES_ERROR("unable to load AMS cache");
            return(false);
        }

        // set output stream
        PrintEngine.SetOutputStream(vout);

        // print data
        PrintEngine.PrintRawArchs();
        return(true);
    }
    //-----------------------------------------------
    else if( Options.GetArgAction() == "modes" ) {
        // initialze AMS cache
        if( Cache.LoadCache() == false) {
            ES_ERROR("unable to load AMS cache");
            return(false);
        }
        // set output stream
        PrintEngine.SetOutputStream(vout);

        // print data
        PrintEngine.PrintRawModes();
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "rebuildall" ) {

        if( LoadSiteAliases() == false ) return(false);
        GetSites(Options.GetOptSites());

        list<string>::iterator it = SiteList.begin();
        list<string>::iterator ie = SiteList.end();

        vout << endl;

        while( it != ie ){
            CAmsUUID     site_id = CUtils::GetSiteID(*it);
            CSmallString site_sid = site_id.GetFullStringForm();

            if( ! site_id.IsValidUUID() ){
                CSmallString error;
                error << "site '" << site_sid << "' does not exist";
                ES_ERROR(error);
                return(false);
            }

            AMSGlobalConfig.SetActiveSiteID(site_id);

            if( Site.LoadConfig() == false ) {
                CSmallString error;
                error << "unable to read site configuration " << site_sid;
                ES_ERROR(error);
                return(false);
            }

            vout << "# " << Site.GetName() << " site ";
            for(unsigned int i=0; i < 80 - 8 - Site.GetName().GetLength(); i++) vout << "=";
            vout << endl;
            vout << "  # full cache" << endl;
            if( Cache.RebuildCache(vout,true) == false ) {
                CSmallString error;
                error << "unable to rebuild site module cache " << site_sid;
                ES_ERROR(error);
                vout << endl;
                return(false);
            }
            if( Cache.SaveCache(true) == false ) {
                CSmallString error;
                error << "unable to save module cache for site " << site_sid;
                ES_ERROR(error);
                vout << endl;
                return(false);
            }
            vout << "  # compressed cache" << endl;
            if( Cache.RebuildCache(vout,false) == false ) {
                CSmallString error;
                error << "unable to rebuild site module cache " << site_sid;
                ES_ERROR(error);
                vout << endl;
                return(false);
            }
            // remove doc
            Cache.RemoveDocumentation();
            if( Cache.SaveCache(false) == false ) {
                CSmallString error;
                error << "unable to save module cache for site " << site_sid;
                ES_ERROR(error);
                vout << endl;
                return(false);
            }
            vout << endl;
            it++;
        }

        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "syntaxall" ) {

        if( LoadSiteAliases() == false ) return(false);
        GetSites(Options.GetOptSites());

        list<string>::iterator it = SiteList.begin();
        list<string>::iterator ie = SiteList.end();

        vout << endl;

        while( it != ie ){
            CAmsUUID     site_id = CUtils::GetSiteID(*it);
            CSmallString site_sid = site_id.GetFullStringForm();

            if( ! site_id.IsValidUUID() ){
                CSmallString error;
                error << "site '" << site_sid << "' does not exist";
                ES_ERROR(error);
                return(false);
            }

            AMSGlobalConfig.SetActiveSiteID(site_sid);

            if( Site.LoadConfig() == false ) {
                CSmallString error;
                error << "unable to read site configuration " << site_sid;
                ES_ERROR(error);
                return(false);
            }

            // rebuild cache
            vout << "#  Building the AMS cache for the site : " << Site.GetName() << endl;
            if( Cache.RebuildCache(vout,false) == false) return(false);
            vout << endl;

            // check syntax
            vout << "#  Checking the cache syntax for the site : " << Site.GetName() << endl;
            if( Cache.CheckCacheSyntax(vout) == false) return(false);
            vout << endl;
            it++;
        }
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "debdepsall" ) {

        if( LoadSiteAliases() == false ) return(false);
        GetSites(Options.GetOptSites());

        list<string>::iterator it = SiteList.begin();
        list<string>::iterator ie = SiteList.end();

        std::set<CSmallString> debdeps;

        while( it != ie ){
            CAmsUUID     site_id = CUtils::GetSiteID(*it);
            CSmallString site_sid = site_id.GetFullStringForm();

            if( ! site_id.IsValidUUID() ){
                CSmallString error;
                error << "site '" << site_sid << "' does not exist";
                ES_ERROR(error);
                return(false);
            }

            AMSGlobalConfig.SetActiveSiteID(site_sid);

            if( Site.LoadConfig() == false ) {
                CSmallString error;
                error << "unable to read site configuration " << site_sid;
                ES_ERROR(error);
                return(false);
            }

            // rebuild cache
            vout << high;
            vout << "#  Building the AMS cache for the site : " << Site.GetName() << endl;
            if( Cache.RebuildCache(vout,false) == false) return(false);
            vout << endl;

            Cache.GetDebDependencies(Options.GetOptRelease(),debdeps);
            it++;
        }

        std::set<CSmallString>::iterator dit = debdeps.begin();
        std::set<CSmallString>::iterator die = debdeps.end();

        vout << low;
        while( dit != die ){
            vout << *dit << endl;
            dit++;
        }

        return(true);
    }
    //-----------------------------------------------
    else {
        CSmallString error;
        error << "action '" << Options.GetArgAction() << "' is not supported!";
        ES_ERROR(error);
        return(false);
    }
}

//------------------------------------------------------------------------------

void CCacheCmd::Finalize(void)
{
    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << "# ==============================================================================" << endl;
    vout << "# ams-cache (AMS utility) terminated at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    if( ErrorSystem.IsError() || (ErrorSystem.IsAnyRecord() && Options.GetOptVerbose()) ){
        vout << low;
        ErrorSystem.PrintErrors(vout);
    }

    vout << endl;
}

//------------------------------------------------------------------------------

bool CCacheCmd::LoadSiteAliases(void)
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

void CCacheCmd::GetSites(const CSmallString& sites)
{
    // split sites into individual words
    string ssites = string(sites);

    list<string> arg_list;
    split(arg_list,ssites,is_any_of(","));

    list<string>::iterator    it = arg_list.begin();
    list<string>::iterator    ie = arg_list.end();

    while( it != ie ){
        // is it alias?
        list<string> alias_list = SiteAliases[*it];
        if( alias_list.size() > 0 ){
            SiteList.insert(SiteList.end(),alias_list.begin(),alias_list.end());
        } else{
            SiteList.push_back(*it);
        }
        it++;
    }

    // sort list and keep only unique items
    SiteList.sort();
    SiteList.unique();
}

//------------------------------------------------------------------------------

void CCacheCmd::GetAllSites(std::list<std::string>& sites)
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

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


