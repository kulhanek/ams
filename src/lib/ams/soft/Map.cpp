// =============================================================================
//  AMS - Advanced Module System
// -----------------------------------------------------------------------------
//     Copyright (C) 2012 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2011 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2004,2005,2008,2010 Petr Kulhanek (kulhanek@chemi.muni.cz)
//
//     This library is free software; you can redistribute it and/or
//     modify it under the terms of the GNU Lesser General Public
//     License as published by the Free Software Foundation; either
//     version 2.1 of the License, or (at your option) any later version.
//
//     This library is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//     Lesser General Public License for more details.
//
//     You should have received a copy of the GNU Lesser General Public
//     License along with this library; if not, write to the Free Software
//     Foundation, Inc., 51 Franklin Street, Fifth Floor,
//     Boston, MA  02110-1301  USA
// =============================================================================

#include <Map.hpp>
#include <ErrorSystem.hpp>
#include <FileName.hpp>
#include <FileSystem.hpp>
#include <DirectoryEnum.hpp>
#include <Site.hpp>
#include <AmsUUID.hpp>
#include <XMLParser.hpp>
#include <XMLElement.hpp>
#include <XMLDocument.hpp>
#include <Utils.hpp>
#include <XMLPrinter.hpp>
#include <XMLComment.hpp>
#include <Terminal.hpp>
#include <string.h>
#include <XMLIterator.hpp>
#include <AMSGlobalConfig.hpp>
#include <Cache.hpp>
#include <iomanip>
#include <sstream>
#include <list>
#include <set>
#include <fnmatch.h>
#include <fstream>
#include <sstream>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CMap::CMap(void)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CMap::LoadMap(void)
{
    CFileName map_name = AMSGlobalConfig.GetETCDIR() / "map" / "map.xml";

    if( CFileSystem::IsFile(map_name) == false ){
        // create header
        MapDoc.CreateChildDeclaration();
        MapDoc.CreateChildComment("AMS Map File");

        CSmallString warning;
        warning <<  "map file (" << map_name << ") does not exist";
        ES_WARNING(warning);
        return(true);
    }

    CXMLParser      xml_parser;
    xml_parser.SetOutputXMLNode(&MapDoc);

    if( xml_parser.Parse(map_name) == false ) {
        CSmallString error;
        error <<  "unable to parse map file (" << map_name << ")";
        ES_ERROR(error);
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CMap::SaveMap(void)
{
    // backup map
    if( BackupMap() == false ){
        ES_ERROR("unable to backup AMS map");
        return(false);
    }

    // save map
    CFileName map_name = AMSGlobalConfig.GetETCDIR() / "map" / "map.xml";

    CXMLPrinter      xml_printer;
    xml_printer.SetPrintedXMLNode(&MapDoc);

    if( xml_printer.Print(map_name) == false ) {
        CSmallString error;
        error <<  "unable to print map file (" << map_name << ")";
        ES_ERROR(error);
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CMap::LoadAutoPrefixes(const CSmallString& user_prefix)
{
    AutoPrefixes.clear();

    if( user_prefix != NULL ){
        AutoPrefixes.push_back(string(user_prefix));
    }

    // load prefixes
    CFileName prefixes_name = AMSGlobalConfig.GetETCDIR() / "map" / "prefixes";
    ifstream ifs(prefixes_name);

    if( ! ifs ){
        CSmallString warning;
        warning <<  "unable to load prefixes file (" << prefixes_name << ")";
        ES_WARNING(warning);
        return(true);
    }

    string line,prefix;
    while( getline(ifs,line) ){
        stringstream str(line);
        str >> prefix;
        if( ! prefix.empty() ) AutoPrefixes.push_back(prefix);
    }

    ifs.close();

    return(true);
}

//------------------------------------------------------------------------------

bool CMap::BackupMap(void)
{
    CFileName map_dir = AMSGlobalConfig.GetETCDIR() / "map";

    CFileName map5 = map_dir / "map5.xml";
    CFileName map4 = map_dir / "map4.xml";
    CFileName map3 = map_dir / "map3.xml";
    CFileName map2 = map_dir / "map2.xml";
    CFileName map1 = map_dir / "map1.xml";
    CFileName map0 = map_dir / "map.xml";

    bool result = true;

    // we will keep five maps
    if( CFileSystem::IsFile(map5) ){
        result &= CFileSystem::RemoveFile(map5);
    }
    if( CFileSystem::IsFile(map4) ){
        result &= CFileSystem::Rename(map4,map5);
    }
    if( CFileSystem::IsFile(map3) ){
        result &= CFileSystem::Rename(map3,map4);
    }
    if( CFileSystem::IsFile(map2) ){
        result &= CFileSystem::Rename(map2,map3);
    }
    if( CFileSystem::IsFile(map1) ){
        result &= CFileSystem::Rename(map1,map2);
    }
    if( CFileSystem::IsFile(map0) ){
        result &= CFileSystem::Rename(map0,map1);
    }

    return(result);
}

//------------------------------------------------------------------------------

bool CMap::UndoMapChange(void)
{
    if( GetNumOfUndoMapChanges() == 0 ){
        ES_ERROR("no undo is possible");
        return(false);
    }

    CFileName map_dir = AMSGlobalConfig.GetETCDIR() / "map";

    CFileName map5 = map_dir / "map5.xml";
    CFileName map4 = map_dir / "map4.xml";
    CFileName map3 = map_dir / "map3.xml";
    CFileName map2 = map_dir / "map2.xml";
    CFileName map1 = map_dir / "map1.xml";
    CFileName map0 = map_dir / "map.xml";

    bool result = true;

    if( CFileSystem::IsFile(map0) ){
         result &= CFileSystem::RemoveFile(map0);
    }

    if( CFileSystem::IsFile(map1) ){
        result &= CFileSystem::Rename(map1,map0);
    }
    if( CFileSystem::IsFile(map2) ){
        result &= CFileSystem::Rename(map2,map1);
    }
    if( CFileSystem::IsFile(map3) ){
        result &= CFileSystem::Rename(map3,map2);
    }
    if( CFileSystem::IsFile(map4) ){
        result &= CFileSystem::Rename(map4,map3);
    }
    if( CFileSystem::IsFile(map5) ){
        result &= CFileSystem::Rename(map5,map4);
    }

    return(result);
}

//------------------------------------------------------------------------------

int CMap::GetNumOfUndoMapChanges(void)
{
    CFileName map_dir = AMSGlobalConfig.GetETCDIR() / "map";

    CFileName map5 = map_dir / "map5.xml";
    CFileName map4 = map_dir / "map4.xml";
    CFileName map3 = map_dir / "map3.xml";
    CFileName map2 = map_dir / "map2.xml";
    CFileName map1 = map_dir / "map1.xml";

    int count = 0;
    if( CFileSystem::IsFile(map5) ){
        count++;
    }
    if( CFileSystem::IsFile(map4) ){
        count++;
    }
    if( CFileSystem::IsFile(map3) ){
        count++;
    }
    if( CFileSystem::IsFile(map2) ){
        count++;
    }
    if( CFileSystem::IsFile(map1) ){
        count++;
    }

    return(count);
}

//------------------------------------------------------------------------------

void CMap::LoadSiteAliases(void)
{
    CFileName aliases_name = AMSGlobalConfig.GetETCDIR() / "map" / "aliases.xml";

    // generate all site alias
    list<string> all_sites;
    GetAllSites(all_sites);
    SiteAliases["all"] = all_sites;

    if( CFileSystem::IsFile(aliases_name) == false ){
        return; // no aliases found
    }

    CXMLDocument    xml_doc;
    CXMLParser      xml_parser;
    xml_parser.SetOutputXMLNode(&xml_doc);

    if( xml_parser.Parse(aliases_name) == false ) {
        CSmallString error;
        error <<  "unable to parse aliases file (" << aliases_name << ")";
        ES_ERROR(error);
        return;
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
}

//------------------------------------------------------------------------------

void CMap::PrintSiteAliases(std::ostream& vout)
{
    vout << endl;
    vout << "#    Alias     Sites                                        " << endl;
    vout << "# ----------- ----------------------------------------------" << endl;

    std::map<std::string, std::list<std::string> >::iterator it = SiteAliases.begin();
    std::map<std::string, std::list<std::string> >::iterator ie = SiteAliases.end();

    while( it != ie ){
        vout << left << setw(13) << (*it).first << " ";
        vout << join((*it).second," ") << endl;
        it++;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CMap::AddBuildsForSites(std::ostream& vout,const CSmallString& sites,
                             const CSmallString& filter)
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

    bool result = true;

    it = site_list.begin();
    ie = site_list.end();
    while( it != ie ){
        string site = *it;
        it++;

        std::list<std::string>::iterator    it = AutoPrefixes.begin();
        std::list<std::string>::iterator    ie = AutoPrefixes.end();

        bool found = false;
        while( it != ie ){
            CSmallString auto_prefix = *it;
            if( AddBuilds(vout,site,auto_prefix,filter) == true ){
                found = true;
                break;
            }
            it++;
        }
        if( found ) continue;

        CSmallString error;
        error << "unable to add build '" << filter << "'";
        ES_ERROR(error);
        result = false;
    }

    return(result);
}

//------------------------------------------------------------------------------

bool CMap::AddBuilds(std::ostream& vout,const CSmallString& site,const CSmallString& prefix,const CSmallString& filter)
{
    CXMLElement* p_site = FindSite(site,true);
    if( p_site == NULL ){
        ES_ERROR("p_site is NULL");
        return(false);
    }

    CFileName path = AMSGlobalConfig.GetETCDIR()  / "map" / "builds" / prefix;
    if( CFileSystem::IsDirectory(path) == false ){
        return(false);
    }

    vout << endl;
    vout << "Site          : " << site << endl;
    vout << "Prefix        : " << prefix << endl;
    vout << "Build filter  : " << filter << endl;
    vout << endl;

    // get prefix filter
    CSmallString build_filter;
    build_filter = filter;

    // add only if it is not present
    if( build_filter.FindSubString(".bld") == -1 ){
        build_filter += ".bld";
    }

    int num = 0;

    CDirectoryEnum build_enum(path);
    build_enum.StartFindFile(build_filter);

    CFileName build_name;
    while( build_enum .FindFile(build_name) ) {
        if( build_name == "." ) continue;
        if( build_name == ".." ) continue;
        CSmallString build_name_noex = build_name.GetFileNameWithoutExt();

        vout << "  >> <blue>" << build_name_noex << "</blue>";

        // add build into the map
        CSmallString module_name;
        CUtils::ParseModuleName(build_name_noex,module_name);

        CXMLElement* p_mod = FindModule(p_site,module_name,true);
        if( p_mod == NULL ){
            ES_TRACE_ERROR("p_mod is NULL");
            return(false);
        }

        CXMLElement* p_build = FindBuild(p_mod,build_name_noex,true);
        if( p_build == NULL ){
            ES_TRACE_ERROR("p_build is NULL");
            return(false);
        }

        p_build->SetAttribute("prefix",prefix);

        // open build file and get verindx
        if(  InjectVerIndx(p_build) == false ){
            ES_TRACE_ERROR("unable to inject verindex");
        }

        num++;

        vout << endl;
    }

    if( num >= 1 ) vout << endl;
    vout << "Number of builds : " << num << endl;

    return(num > 0);
}

//------------------------------------------------------------------------------

bool CMap::InjectVerIndx(CXMLElement* p_build)
{
    if( p_build == NULL ){
        ES_ERROR("p_build is NULL");
        return(false);
    }

    CSmallString    prefix;
    CSmallString    build;

    p_build->GetAttribute("prefix",prefix);
    p_build->GetAttribute("name",build);

    CFileName       build_full_name;

    build_full_name = AMSGlobalConfig.GetETCDIR() / "map" / "builds" / prefix / build + ".bld";

    CXMLDocument    xml_doc;
    CXMLParser      xml_parser;

    xml_parser.SetOutputXMLNode(&xml_doc);

    if( xml_parser.Parse(build_full_name) == false ){
        CSmallString error;
        error << "unable to open build file " << build_full_name;
        ES_ERROR(error);
        return(false);
    }

    CXMLElement* p_build_ele = xml_doc.GetChildElementByPath("build");
    if( p_build_ele == NULL ){
        CSmallString error;
        error << "unable to open 'build' element in file " << build_full_name;
        ES_ERROR(error);
        return(false);
    }

    double verindx = 1.0;
    p_build_ele->GetAttribute("verindx",verindx);
    p_build->SetAttribute("verindx",verindx);

    return(true);
}

//------------------------------------------------------------------------------

void CMap::RemoveBuildsForSites(std::ostream& vout,const CSmallString& sites,const CSmallString& filter)
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

    it = site_list.begin();
    ie = site_list.end();
    while( it != ie ){
        RemoveBuilds(vout,*it,filter);
        it++;
    }
}

//------------------------------------------------------------------------------

void CMap::RemoveBuilds(std::ostream& vout,const CSmallString& site,const CSmallString& filter)
{
    vout << endl;
    vout << "Site          : " << site << endl;
    vout << "Build filter  : " << filter << endl;

    CXMLElement* p_site = FindSite(site,false);
    if( p_site == NULL ){
        vout << "<red>  >> site " << site << " is not defined in the map</red>" << endl;
        return;
    }

    CXMLIterator    I(p_site);
    CXMLElement*    p_mod;
    int             num = 0;

    while( (p_mod = I.GetNextChildElement("module")) != NULL ){
        CSmallString sname;
        p_mod->GetAttribute("name",sname);

        CXMLIterator    J(p_mod);
        CXMLElement*    p_build;
        bool first = true;
        while( (p_build = J.GetNextChildElement("build")) != NULL ){
            CSmallString bname;
            p_build->GetAttribute("name",bname);

            if( fnmatch(filter,bname,0) == 0 ) {
                if( first == true ) {
                    vout << endl;
                    vout << "<yellow>>> module " << sname << "</yellow>" << endl;
                    first = false;
                }
                vout << "<blue>   |- " << setw(42) << left << bname << "</blue> - removed" << endl;
                delete p_build;
                num++;
            }

        }

        // count number of builds
        if( J.GetNumberOfChildElements("build") == 0 ){
            delete p_mod;
        }
    }

    vout << endl;
    vout << "Number of removed builds : " << num << endl;
}

//------------------------------------------------------------------------------

void CMap::GetAllSites(std::list<std::string>& sites)
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

bool CMap::SetDefaultForSites(const CSmallString& sites,const CSmallString& defname)
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

    bool result = true;
    it = site_list.begin();
    ie = site_list.end();
    while( it != ie ){
        result &= SetDefault(*it,defname);
        it++;
    }

    return(result);
}

//------------------------------------------------------------------------------

bool CMap::SetDefault(const CSmallString& site,const CSmallString& defname)
{
    CXMLElement* p_site = FindSite(site,true);
    if( p_site == NULL ){
        ES_ERROR("p_site is NULL");
        return(false);
    }

    CSmallString module_name;
    CUtils::ParseModuleName(defname,module_name);

    CXMLElement* p_mod = FindModule(p_site,module_name,true);
    if( p_mod == NULL ){
        ES_TRACE_ERROR("p_mod is NULL");
        return(false);
    }

    p_mod->SetAttribute("default",defname);

    return(true);
}

//------------------------------------------------------------------------------

bool CMap::RemoveDefaultForSites(const CSmallString& sites,const CSmallString& modname)
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

    bool result = true;
    it = site_list.begin();
    ie = site_list.end();
    while( it != ie ){
        result &= RemoveDefault(*it,modname);
        it++;
    }
    return(result);
}

//------------------------------------------------------------------------------

bool CMap::RemoveDefault(const CSmallString& site,const CSmallString& modname)
{
    CXMLElement* p_site = FindSite(site,false);
    if( p_site == NULL ){
        CSmallString error;
        error << "site '" << site << "' was not found";
        ES_ERROR(error);
        return(false);
    }

    CSmallString module_name;
    CUtils::ParseModuleName(modname,module_name);

    CXMLElement* p_mod = FindModule(p_site,module_name,false);
    if( p_mod == NULL ){
        CSmallString error;
        error << "module '" << module_name << "' was not found";
        ES_ERROR(error);
        return(false);
    }

    p_mod->RemoveAttribute("default");

    return(true);
}

//------------------------------------------------------------------------------

bool CMap::RemoveModuleForSites(const CSmallString& sites,const CSmallString& modname)
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

    bool result = true;
    it = site_list.begin();
    ie = site_list.end();
    while( it != ie ){
        result &= RemoveModule(*it,modname);
        it++;
    }
    return(result);
}

//------------------------------------------------------------------------------

bool CMap::RemoveModule(const CSmallString& site,const CSmallString& modname)
{
    CXMLElement* p_site = FindSite(site,false);
    if( p_site == NULL ){
        CSmallString error;
        error << "site '" << site << "' was not found";
        ES_ERROR(error);
        return(false);
    }

    CSmallString module_name;
    CUtils::ParseModuleName(modname,module_name);

    CXMLElement* p_mod = FindModule(p_site,module_name,false);
    if( p_mod == NULL ){
        CSmallString error;
        error << "module '" << module_name << "' was not found";
        ES_ERROR(error);
        return(false);
    }

    delete p_mod;

    return(true);
}

//------------------------------------------------------------------------------

void CMap::ShowPrefixes(std::ostream& vout)
{
    vector<string>  prefixes;

    ListPrefixes(prefixes);

    // print prefixes
    vector<string>::iterator it = prefixes.begin();
    vector<string>::iterator ie = prefixes.end();

    while( it != ie ){
        vout << *it << endl;
        it++;
    }
}

//------------------------------------------------------------------------------

void CMap::ListPrefixes(std::vector<std::string>& prefixes)
{
    CFileName path = AMSGlobalConfig.GetETCDIR()  / "map" / "builds";

    prefixes.clear();

    // list prefixes
    CDirectoryEnum dir_enum(path);
    dir_enum.StartFindFile("*");
    CFileName prefix;
    while( dir_enum.FindFile(prefix) ) {
        if( prefix == "." ) continue;
        if( prefix == ".." ) continue;
        if( CFileSystem::IsDirectory(path / prefix) == true ){
            if( (prefix.GetLength() > 0) && (prefix[0] != '.') ){
                prefixes.push_back(string(prefix));
            }
        }
    }
    dir_enum.EndFindFile();

    // sort prefixes
    sort(prefixes.begin(),prefixes.end());
}

//------------------------------------------------------------------------------

void CMap::ShowBuilds(std::ostream& vout,const CSmallString& prefix,const CSmallString& filter)
{
    CFileName path = AMSGlobalConfig.GetETCDIR()  / "map" / "builds";

    // get prefix filter
    CSmallString prefix_filter;
    CSmallString build_filter;

    prefix_filter = prefix;
    build_filter = filter;

    vout << endl;
    vout << "Prefix filter : " << prefix_filter << endl;
    vout << "Build filter  : " << build_filter << endl;

    if( build_filter.FindSubString(".bld") == -1 ){
        build_filter += ".bld";
    }

    vector<string>  prefixes;

    CDirectoryEnum dir_enum(path);
    dir_enum.StartFindFile(prefix_filter);
    CFileName list_prefix;
    while( dir_enum.FindFile(list_prefix) ) {
        if( prefix == "." ) continue;
        if( prefix == ".." ) continue;
        if( CFileSystem::IsDirectory(path / list_prefix) == true ){
            if( (list_prefix.GetLength() > 0) && (list_prefix[0] != '.') ){
                prefixes.push_back(string(list_prefix));
            }
        }
    }
    dir_enum.EndFindFile();

    // sort prefixes
    sort(prefixes.begin(),prefixes.end());

    // list builds for each prefix
    vector<string>::iterator it = prefixes.begin();
    vector<string>::iterator ie = prefixes.end();

    int num = 0;
    while( it != ie ){
        vout << endl;

        CFileName list_prefix(*it);
        vout << "# prefix <yellow>" << list_prefix << "</yellow> ===" << endl;

        vector<string>  builds;
        CDirectoryEnum build_enum(path / list_prefix);
        build_enum.StartFindFile(build_filter);

        CFileName build_name;
        while( build_enum .FindFile(build_name) ) {
            if( build_name == "." ) continue;
            if( build_name == ".." ) continue;
            builds.push_back(string(build_name.GetFileNameWithoutExt()));
        }
        build_enum.EndFindFile();

        // sort builds
        sort(builds.begin(),builds.end());

        // print builds
        vector<string>::iterator bit = builds.begin();
        vector<string>::iterator bie = builds.end();

        while( bit != bie ){
            vout << "  >> <blue>" << *bit << "</blue>" << endl;
            bit++;
        }
        num += builds.size();

        it++;
    }

    vout << endl;
    vout << "Number of builds : " << num << endl;
}

//------------------------------------------------------------------------------

void CMap::ShowAutoBuilds(std::ostream& vout,const CSmallString& site_name,const CSmallString& filter,
                     const CSmallString& prefix)
{
    std::set<SFullBuild>  builds;

    if( prefix != NULL ){
        // prefix specific
        ListBuilds(prefix,filter,builds);
    } else {
        // site specific
        ListBuilds(site_name,filter,builds);

        // autosite
        // is within autoprefix?
        std::list<std::string>::iterator    it = AutoPrefixes.begin();
        std::list<std::string>::iterator    ie = AutoPrefixes.end();

        while( it != ie ){
            CSmallString auto_prefix = *it;
            ListBuilds(auto_prefix,filter,builds);
            it++;
        }
    }

    // print builds
    std::set<SFullBuild>::iterator    ibt = builds.begin();
    std::set<SFullBuild>::iterator    ibe = builds.end();

    while( ibt != ibe ){
        SFullBuild bld = *ibt;
        vout << bld.prefix << "/" << bld.build << endl;
        ibt++;
    }
}

//------------------------------------------------------------------------------

void CMap::ShowBestBuild(std::ostream& vout,const CSmallString& site_name,const CSmallString& module,
                     const CSmallString& prefix)
{
    std::set<SFullBuild>  builds;

    CSmallString name,ver,arch,mode;
    CUtils::ParseModuleName(module,name,ver,arch,mode);

    CSmallString filter;
    if( ver == NULL ) ver = "*";
    if( arch == NULL ) arch = "*";
    if( mode == NULL ) mode = "*";
    filter << name << ":" << ver << ":" << arch << ":" << mode;

    if( prefix != NULL ){
        // prefix specific
        ListBuilds(prefix,filter,builds);
    } else {
        // site specific
        ListBuilds(site_name,filter,builds);

        // autosite
        // is within autoprefix?
        std::list<std::string>::iterator    it = AutoPrefixes.begin();
        std::list<std::string>::iterator    ie = AutoPrefixes.end();

        while( it != ie ){
            CSmallString auto_prefix = *it;
            ListBuilds(auto_prefix,filter,builds);
            it++;
        }
    }

    // no build was found
    if( builds.size() == 0 ) return;

    // load builds and determine the best one
    std::set<SFullBuild>::iterator    ibt = builds.begin();
    std::set<SFullBuild>::iterator    ibe = builds.end();

    SFullBuild best_build;
    double     best_verindx = 0.0;

    while( ibt != ibe ){

        SFullBuild bld = *ibt;

        CFileName full_build_name;

        full_build_name = GetBuildName(site_name,bld.build,bld.prefix);
        if( full_build_name == NULL ){
            ES_ERROR("build does not exist");
            return;
        }

        CXMLDocument    xml_doc;
        CXMLParser      xml_parser;
        xml_parser.SetOutputXMLNode(&xml_doc);

        if( xml_parser.Parse(full_build_name) == false ) {
            CSmallString error;
            error <<  "unable to parse build file (" << full_build_name << ")";
            ES_ERROR(error);
            return;
        }

        double verindx = 0.0;

        CXMLElement* p_bld = xml_doc.GetChildElementByPath("build");
        if( p_bld != NULL ){
            p_bld->GetAttribute("verindx",verindx);
        }

        if( (ibt == builds.begin()) || (verindx > best_verindx) ){
            best_build = bld;
            best_verindx = verindx;
        }

        ibt++;
    }

    vout << best_build.prefix << "/" << best_build.build;

}

//------------------------------------------------------------------------------

bool CMap::ShowPkgDir(std::ostream& vout,const CSmallString& site_name,
                  const CSmallString& build,const CSmallString& prefix)
{
    std::set<SFullBuild>  builds;

    CSmallString filter = build;

    if( prefix != NULL ){
        // prefix specific
        ListBuilds(prefix,filter,builds);
    } else {
        // site specific
        ListBuilds(site_name,filter,builds);

        // autosite
        // is within autoprefix?
        std::list<std::string>::iterator    it = AutoPrefixes.begin();
        std::list<std::string>::iterator    ie = AutoPrefixes.end();

        while( it != ie ){
            CSmallString auto_prefix = *it;
            ListBuilds(auto_prefix,filter,builds);
            it++;
        }
    }

    // if more builds - terminate
    if( builds.size() != 1 ) return(false);

    SFullBuild bld = *(builds.begin());

    CFileName full_build_name;

    full_build_name = GetBuildName(site_name,bld.build,bld.prefix);
    if( full_build_name == NULL ){
        ES_ERROR("build does not exist");
        return(false);
    }

    CXMLDocument    xml_doc;
    CXMLParser      xml_parser;
    xml_parser.SetOutputXMLNode(&xml_doc);

    if( xml_parser.Parse(full_build_name) == false ) {
        CSmallString error;
        error <<  "unable to parse build file (" << full_build_name << ")";
        ES_ERROR(error);
        return(false);
    }
    CXMLElement* p_bld = xml_doc.GetChildElementByPath("build");
    CSmallString dir = CCache::GetVariableValue(p_bld,"AMS_PACKAGE_DIR");

    vout << dir;
    return(true);
}

//------------------------------------------------------------------------------

bool SFullBuild::operator < (const SFullBuild& right) const
{
    if( prefix < right.prefix ) return(true);
    if( prefix > right.prefix ) return(false);
    return( build < right.build );
}

//------------------------------------------------------------------------------

void CMap::ListBuilds(const CSmallString& prefix,const CSmallString& filter,std::set<SFullBuild>& builds)
{
    CFileName path = AMSGlobalConfig.GetETCDIR()  / "map" / "builds";

    if( CFileSystem::IsDirectory(path / prefix) == false ) return;

    CDirectoryEnum build_enum(path / prefix);

    CSmallString build_filter = filter;
    if( build_filter.FindSubString(".bld") == -1 ){
        build_filter += ".bld";
    }

    build_enum.StartFindFile(build_filter);

    CFileName build_name;
    while( build_enum .FindFile(build_name) ) {
        if( build_name == "." ) continue;
        if( build_name == ".." ) continue;
        SFullBuild build;
        build.prefix = prefix;
        build.build = string(build_name.GetFileNameWithoutExt());
        builds.insert(build);
    }
    build_enum.EndFindFile();
}

//------------------------------------------------------------------------------

void CMap::ListBuilds(const CSmallString& prefix,std::vector<SFullBuild>& builds)
{
    CFileName path = AMSGlobalConfig.GetETCDIR()  / "map" / "builds";

    if( CFileSystem::IsDirectory(path / prefix) == false ) return;

    CDirectoryEnum build_enum(path / prefix);

    build_enum.StartFindFile("*.bld");

    CFileName build_name;
    while( build_enum .FindFile(build_name) ) {
        if( build_name == "." ) continue;
        if( build_name == ".." ) continue;
        SFullBuild build;
        build.prefix = prefix;
        build.build = string(build_name.GetFileNameWithoutExt());
        builds.push_back(build);
    }
    build_enum.EndFindFile();
}

//------------------------------------------------------------------------------

void CMap::ShowAllBuilds(std::ostream& vout)
{
    vector<string>  prefixes;

    ListPrefixes(prefixes);

    // list builds for each prefix
    vector<string>::iterator it = prefixes.begin();
    vector<string>::iterator ie = prefixes.end();

    vector<SFullBuild> builds;

    while( it != ie ){
        ListBuilds(*it,builds);
        it++;
    }

    vout << endl;
    vout << "# Prefix             Build" << endl;
    vout << "# ------------------ -----------------------------------------------------------" << endl;

    sort(builds.begin(),builds.end());

    vector<SFullBuild>::iterator bit = builds.begin();
    vector<SFullBuild>::iterator bie = builds.end();

    while( bit != bie ){
        vout << left << setw(20) << (*bit).prefix << " " << (*bit).build << endl;
        bit++;
    }
}

//------------------------------------------------------------------------------

bool CMap::ShowMapForSites(std::ostream& vout,const CSmallString& sites)
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

    bool result = true;
    it = site_list.begin();
    ie = site_list.end();
    while( it != ie ){
        result &= ShowMap(vout,*it);
        it++;
    }
    return(result);
}

//------------------------------------------------------------------------------

bool CMap::ShowMap(ostream& vout,const CSmallString& site)
{
    CXMLElement* p_site = FindSite(site,false);
    if( p_site == NULL ){
        CSmallString error;
        error << "site '" << site << "' was not found";
        ES_ERROR(error);
        return(false);
    }

    vout << endl;
    vout << "Map for site: " << site << endl;

    CXMLIterator    J(p_site);
    CXMLElement*    p_mod;

    while( (p_mod = J.GetNextChildElement("module")) != NULL ){

        vout << endl;

        CSmallString modname,defname;
        p_mod->GetAttribute("name",modname);
        p_mod->GetAttribute("default",defname);

        vout << "<yellow>  >> module " << modname << "</yellow>" << endl;

        CXMLIterator    I(p_mod);
        CXMLElement*    p_build;
        CSmallString    local_defname;
        double          index = 1.0;
        bool            index_set = false;

        while( (p_build = I.GetNextChildElement("build")) != NULL ){
            CSmallString buildname;
            CSmallString prefixname;
            double       lindex = 1.0;
            p_build->GetAttribute("name",buildname);
            p_build->GetAttribute("prefix",prefixname);
            p_build->GetAttribute("verindx",lindex);
            if( ! index_set ){
                index = lindex;
                index_set = true;
                CSmallString dname,dver;
                CUtils::ParseModuleName(buildname,dname,dver);
                local_defname = dname + ":" + dver + ":auto:auto";
            }
            if( lindex > index ){
                index = lindex;
                CSmallString dname,dver;
                CUtils::ParseModuleName(buildname,dname,dver);
                local_defname = dname + ":" + dver + ":auto:auto";
            }

            vout << "<blue>     |- " << setw(42) << left << buildname;
            vout << fixed << right << setw(5) << setprecision(2) << lindex << " " << prefixname << "</blue>" << endl;
        }

        // default build
        CSmallString auto_def;
        if( defname == NULL ){
            defname = local_defname;
            auto_def = "auto";
        } else {
            auto_def = "defined";
        }

        vout << "<purple>     Def: " << setw(40) << left << local_defname <<  auto_def << "</purple>" << endl;

        // documentation
        CFileName path = AMSGlobalConfig.GetETCDIR()  / "map" / "docs";

        if( CFileSystem::IsFile(path / site / modname + ".doc" ) ){
            vout << "<green>     Doc: " << setw(40) << left << modname <<  site << "</green>" << endl;
        } else if ( CFileSystem::IsFile(path / "common" / modname + ".doc") ){
            vout << "<green>     Doc: " << setw(40) << left << modname <<  "common" << "</green>" << endl;
        } else {
            vout << "<green>     Doc: " << setw(40) << left << "none" << "</green>" << endl;
        }
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CMap::RemoveMapForSites(std::ostream& vout,const CSmallString& sites)
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

    vout << endl;

    bool result = true;
    it = site_list.begin();
    ie = site_list.end();
    while( it != ie ){
        result &= RemoveMap(vout,*it);
        it++;
    }
    if( site_list.size() > 0 ) vout << endl;

    return(result);
}

//------------------------------------------------------------------------------

bool CMap::RemoveMap(std::ostream& vout,const CSmallString& site)
{
    vout << "Map for site: " << site;
    CXMLElement* p_site = FindSite(site,false);
    if( p_site == NULL ){
        vout << " <red>-not found-</red>" << endl;
        CSmallString error;
        error << "site '" << site << "' was not found";
        return(false);
    }
    delete p_site;
    vout << " <green>found and removed</green>" << endl;
    return(true);
}

//------------------------------------------------------------------------------

bool CMap::CopyMap(const CSmallString& site1,const CSmallString& site2)
{
    CXMLElement* p_site1 = FindSite(site1,false);
    if( p_site1 == NULL ){
        CSmallString error;
        error << "site1 '" << site1 << "' was not found";
        ES_ERROR(error);
        return(false);
    }
    CXMLElement* p_site2 = FindSite(site2,false);
    if( p_site1 != NULL ){
        // delete site
        delete p_site2;
    }
    // create target
    p_site2 = FindSite(site2,true);

    // copy site map
    p_site2->CopyContentsFrom(p_site1);

    // update site name
    p_site2->SetAttribute("name",site2);

    return(true);
}

//------------------------------------------------------------------------------

void CMap::UpdateVerIndexes(std::ostream& vout)
{
    CXMLElement* p_sites = MapDoc.GetChildElementByPath("map");

    CXMLIterator    S(p_sites);
    CXMLElement*    p_site;

    while( (p_site = S.GetNextChildElement("site")) != NULL ){
        CSmallString sitename;
        p_site->GetAttribute("name",sitename);

        vout << endl;
        vout << "Site: " << sitename << endl;

        CXMLIterator    J(p_site);
        CXMLElement*    p_mod;

        while( (p_mod = J.GetNextChildElement("module")) != NULL ){
            CSmallString modname;
            p_mod->GetAttribute("name",modname);

            vout <<  "<yellow>  >> module " << modname << "</yellow>" << endl;

            CXMLIterator    I(p_mod);
            CXMLElement*    p_build;

            while( (p_build = I.GetNextChildElement("build")) != NULL ){

                CSmallString buildname;
                p_build->GetAttribute("name",buildname);

                vout << "<blue>     |- " << setw(42) << left << buildname;

                if( InjectVerIndx(p_build) ) {
                    vout << "<green>[OK]</green>" << endl;
                } else {
                    vout << "<red>[FAILED]</red>" << endl;
                }
            }
        }
    }

    vout << endl;
}

//------------------------------------------------------------------------------

double CMap::GetNewVerIndex(const CSmallString& module)
{
    CXMLElement* p_sites = MapDoc.GetChildElementByPath("map");

    double verindex = 0.0;

    CSmallString modname = CUtils::GetModuleName(module);

    CXMLIterator    S(p_sites);
    CXMLElement*    p_site;

    while( (p_site = S.GetNextChildElement("site")) != NULL ){

        CXMLIterator    J(p_site);
        CXMLElement*    p_mod;

        while( (p_mod = J.GetNextChildElement("module")) != NULL ){
            CSmallString lmodname;
            p_mod->GetAttribute("name",lmodname);
            if( lmodname != modname ) continue;

            CXMLIterator    I(p_mod);
            CXMLElement*    p_build;

            while( (p_build = I.GetNextChildElement("build")) != NULL ){
                double lverindex = 0.0;
                p_build->GetAttribute("verindx",lverindex);
                if( lverindex > verindex ){
                    verindex = lverindex;
                }
            }
        }
    }

    verindex++;

    return(verindex);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CMap::RemoveOrphanSites(std::ostream& vout)
{
    // go through the list of all available sites -------------
    CXMLElement* p_site = MapDoc.GetChildElementByPath("map/site");

    while( p_site != NULL ){
        CSmallString name;
        CXMLElement* p_next = p_site->GetNextSiblingElement("site");
        if( p_site->GetAttribute("name",name) == true ) {
            vout << setw(20) << left << name;
            CSmallString site_sid = CUtils::GetSiteID(name);
            if( site_sid == NULL ) {
                vout << "<red>not found - removed</red>" << endl;
                delete p_site;
            } else {
                vout << "<green>" << site_sid << " - found and kept</green>" << endl;
            }
        }
        p_site = p_next;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CXMLElement* CMap::FindSite(const CSmallString& name,bool create)
{
    CXMLElement* p_map = MapDoc.GetChildElementByPath("map",true);

    if( p_map == NULL ){
        ES_ERROR("p_map is NULL");
        return(NULL);
    }

    CXMLIterator    I(p_map);
    CXMLElement*    p_site;

    while( (p_site = I.GetNextChildElement("site")) != NULL ){
        CSmallString sname;
        p_site->GetAttribute("name",sname);
        if( sname == name ) return(p_site);
    }

    if( create == true ){
        p_site = p_map->CreateChildElement("site");
        if( p_site == NULL ){
            ES_ERROR("p_site is NULL");
            return(NULL);
        }
        p_site->SetAttribute("name",name);
    }

    return(p_site);
}

//------------------------------------------------------------------------------

CXMLElement* CMap::FindModule(CXMLElement* p_site,const CSmallString& name,bool create)
{
    if( p_site == NULL ){
        ES_ERROR("p_site is NULL");
        return(NULL);
    }

    CXMLIterator    I(p_site);
    CXMLElement*    p_mod;

    while( (p_mod = I.GetNextChildElement("module")) != NULL ){
        CSmallString sname;
        p_mod->GetAttribute("name",sname);
        if( sname == name ) return(p_mod);
    }

    if( create == true ){
        p_mod = p_site->CreateChildElement("module");
        if( p_mod == NULL ){
            ES_ERROR("p_mod is NULL");
            return(NULL);
        }
        p_mod->SetAttribute("name",name);
    }

    return(p_mod);
}

//------------------------------------------------------------------------------

CXMLElement* CMap::FindBuild(CXMLElement* p_mod,const CSmallString& build,bool create)
{
    if( p_mod == NULL ){
        ES_ERROR("p_mod is NULL");
        return(NULL);
    }

    CXMLIterator    I(p_mod);
    CXMLElement*    p_build;

    while( (p_build = I.GetNextChildElement("build")) != NULL ){
        CSmallString sname;
        p_build->GetAttribute("name",sname);
        if( sname == build ) return(p_build);
    }

    if( create == true ){
        p_build = p_mod->CreateChildElement("build");
        if( p_build == NULL ){
            ES_ERROR("p_build is NULL");
            return(NULL);
        }
        p_build->SetAttribute("name",build);
    }

    return(p_build);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CMap::DistributeAll(std::ostream& vout)
{
    BuildCache.clear();

    vout << endl;
    vout << "Cleaning sites ..." << endl;
    if( CleanSiteModules(vout) == false ) {
        ES_TRACE_ERROR("unable to clean sites");
        return(false);
    }

    vout << endl;
    vout << "Distributing sites ..." << endl;
    if( DistributeSiteModules(vout) == false ) {
        ES_TRACE_ERROR("unable to distribute sites");
        return(false);
    }
    vout << endl;

    return(true);
}

//------------------------------------------------------------------------------

bool CMap::CleanSiteModules(std::ostream& vout)
{
    // go through the list of all available sites -------------
    CFileName site_dir = AMSGlobalConfig.GetETCDIR() / "sites";
    CDirectoryEnum dir_enum(site_dir);

    dir_enum.StartFindFile("*");
    CFileName site_sid;
    while( dir_enum.FindFile(site_sid) ) {
        CAmsUUID    site_id;
        CSite       site;
        if( site_id.LoadFromString(site_sid) == false ) continue;
        if( site.LoadConfig(site_sid) == false ) continue;

        vout << "   <cyan>" << setw(20) << left << site.GetName() << "</cyan> .. ";

        CFileName module_dir;
        module_dir = AMSGlobalConfig.GetETCDIR() / "sites" / site_sid / "modules";

        // is that directory exist?
        if( CFileSystem::IsDirectory(module_dir) == true ) {
            // delete contents of it
            if( CFileSystem::RemoveDirContents(module_dir,true) == false ) {
                vout << "<red>[FAILED]</red>" << endl;
                CSmallString error;
                error << "unable to delete directory contents '" << module_dir << "'";
                ES_ERROR(error);
                return(false);
            }
        } else {
            // create directory
            if( CFileSystem::CreateDir(module_dir) == false ) {
                vout << "<red>[FAILED]</red>" << endl;
                CSmallString error;
                error << "unable to create directory '" << module_dir << "'";
                ES_ERROR(error);
                return(false);
            }
        }

        vout << "<green>[OK]</green>" << endl;
    }
    dir_enum.EndFindFile();

    return(true);
}

//------------------------------------------------------------------------------

bool CMap::DistributeSiteModules(std::ostream& vout)
{
    // go through the list of all available sites -------------
    CFileName site_dir = AMSGlobalConfig.GetETCDIR() / "sites";
    CDirectoryEnum dir_enum(site_dir);

    dir_enum.StartFindFile("*");
    CFileName site_sid;
    while( dir_enum.FindFile(site_sid) ) {
        CAmsUUID    site_id;
        CSite       site;
        if( site_id.LoadFromString(site_sid) == false ) continue;
        if( site.LoadConfig(site_sid) == false ) continue;

        if( DistributeSiteMap(vout,&site) == false ){
            ES_TRACE_ERROR("unable to distribute site");
            return(false);
        }

    }
    dir_enum.EndFindFile();

    return(true);
}

//------------------------------------------------------------------------------

bool CMap::DistributeSiteMap(std::ostream& vout,CSite* p_site)
{
    if( p_site == NULL ){
        INVALID_ARGUMENT("p_site == NULL");
    }

    vout << endl;
    vout << "# ------------------------------------------------------------------------------" << endl;
    vout << "# Map for site: <cyan>" + p_site->GetName() + "</cyan>" << endl;
    vout << "# ------------------------------------------------------------------------------" << endl;

    CXMLElement* p_smod = FindSite(p_site->GetName(),false);
    if( p_smod == NULL ){
        vout << "<red>   >> no such site in the map</red>" << endl;
        return(true);
    }

    CXMLIterator    J(p_smod);
    CXMLElement*    p_mod;

    while( (p_mod = J.GetNextChildElement("module")) != NULL ){
        if( DistributeSiteModuleMap(vout,p_mod,p_site) == false ) return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CMap::DistributeSiteModuleMap(std::ostream& vout, CXMLElement* p_mod,CSite* p_site)
{
    if( p_mod == NULL ){
        INVALID_ARGUMENT("p_mod == NULL");
    }
    if( p_site == NULL ){
        INVALID_ARGUMENT("p_site == NULL");
    }

    CSmallString modname;
    CSmallString defname;

    if( p_mod->GetAttribute("name",modname) == false ){
        ES_ERROR("unable to get 'name' attribute");
        return(false);
    }
    p_mod->GetAttribute("default",defname); // optional

    vout << endl;
    vout << "<yellow>  >> module " << modname << "</yellow>" << endl;

    CSmallString site = p_site->GetName();
    CSmallString site_sid = p_site->GetID();

    // create module file --------------------------------------------------
    CFileName mod_name;
    mod_name = AMSGlobalConfig.GetETCDIR() / "sites" / site_sid / "modules" / modname + ".mod" ;

    CXMLDocument xml_out;

    if( CFileSystem::IsFile(mod_name) == true ) {
        CSmallString error;
        error << "module file '" << mod_name << "' already exists!";
        ES_ERROR(error);
        return(false);
    }

    // set comment ---------------------------------------------------------
    xml_out.CreateChildDeclaration();
    xml_out.CreateChildText("\n",true);
    xml_out.CreateChildComment("Advanced Module System (AMS) module file");
    xml_out.CreateChildText("\n",true);
    CXMLElement* p_mele = xml_out.CreateChildElement("module");
    p_mele->SetAttribute("name",modname);
    p_mele->CreateChildText("\n",true);

    // create build element  -----------------------------------------------
    CXMLElement* p_builds = p_mele->CreateChildElement("builds");
    p_builds->CreateChildText("\n",true);
    p_mele->CreateChildText("\n",true);

    // create default element  ---------------------------------------------
    CXMLElement* p_default = p_mele->CreateChildElement("default");
    p_mele->CreateChildText("\n",true);

    // create other elements  ----------------------------------------------
    if( InjectRootDocumentation(p_mod,p_mele,site) == false ) {
        ES_ERROR("unable to inject documentation");
        return(false);
    }

    // inject individual builds
    CXMLIterator    I(p_mod);
    CXMLElement*    p_build;
    CSmallString    local_defname;
    double          index = 0.0;
    bool            index_set = false;

    while( (p_build = I.GetNextChildElement("build")) != NULL ){
        CSmallString buildname;
        CSmallString prefixname;
        double       lindex;
        p_build->GetAttribute("name",buildname);
        p_build->GetAttribute("prefix",prefixname);
        p_build->GetAttribute("verindx",lindex);
        if( ! index_set ){
            index = lindex;
            index_set = true;
            CSmallString dname,dver;
            CUtils::ParseModuleName(buildname,dname,dver);
            local_defname = dname + ":" + dver + ":auto:auto";
        }
        if( lindex > index ){
            index = lindex;
            CSmallString dname,dver;
            CUtils::ParseModuleName(buildname,dname,dver);
            local_defname = dname + ":" + dver + ":auto:auto";
        }

        vout << "<blue>     |- " << setw(42) << left << buildname << setw(15) << prefixname << "</blue>  .. ";

        // inject build
        if( InjectBuildIntoSite(p_builds,prefixname,buildname) == false ){
            vout << "<red>[FAILED]</red>" << endl;
            return(false);
        }
        vout << "<green>[OK]</green>" << endl;
    }

    // inject default element ----------------------------------------------

    // default build
    CSmallString auto_def;
    if( defname == NULL ){
        defname = local_defname;
        auto_def = "auto";
    } else {
        auto_def = "defined";
    }

    vout << "<purple>     Def: " << setw(40) << left << local_defname << setw(15) << auto_def << "</purple>  .. ";

    CSmallString lname;
    CSmallString lver;
    CSmallString larch;
    CSmallString lmode;
    CSmallString ldefname(local_defname);

    CUtils::ParseModuleName(ldefname,lname,lver,larch,lmode);

    p_default->SetAttribute("ver",lver);
    p_default->SetAttribute("arch",larch);
    p_default->SetAttribute("mode",lmode);

    vout << "<green>[OK]</green>" << endl;

    CXMLPrinter xml_printer;
    xml_printer.SetPrintedXMLNode(&xml_out);
    xml_printer.SetPrintAsItIs(true);

    if( xml_printer.Print(mod_name) == false ){
        CSmallString error;
        error << "unable to save module file '" << mod_name << "'";
        ES_ERROR(error);
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CMap::InjectBuildIntoSite(CXMLElement* p_builds,const CSmallString& prefix,const CSmallString& build)
{
    CFileName rkey;
    CFileName rname;
    rkey = CFileName(prefix) / build;
    rname = AMSGlobalConfig.GetETCDIR() / "map" / "builds" / rkey + ".bld";

    // is it cached?
    CXMLDocumentPtr p_build = BuildCache[string(rkey)];
    if( p_build == NULL ){
        p_build = CXMLDocumentPtr(new CXMLDocument);
        // open build file
        CXMLParser   xml_parser;

        xml_parser.SetOutputXMLNode(p_build.get());
        if( xml_parser.Parse(rname) == false ) {
            CSmallString error;
            error << "unable to parse build '" << rname << "'";
            ES_ERROR(error);
            return(false);
        }
        BuildCache[string(rkey)] = p_build;
    }

    // is it really a build file?
    CXMLElement* p_rel = p_build->GetFirstChildElement("build");
    if( p_rel == NULL ) {
        CSmallString error;
        error << "build file '" << rname << "' does not contain build element";
        ES_ERROR(error);
        return(false);
    }

    // delete sites element
    CXMLElement* p_sites = p_rel->GetFirstChildElement("sites");
    if( p_sites != NULL ) delete p_sites;

    CSmallString lname;
    CSmallString lver;
    CSmallString larch;
    CSmallString lmode;

    bool result = true;
    result &= p_rel->GetAttribute("name",lname);
    result &= p_rel->GetAttribute("ver",lver);
    result &= p_rel->GetAttribute("arch",larch);
    result &= p_rel->GetAttribute("mode",lmode);
    if( result == false ) {
        ES_ERROR("some attribute is missing");
        return(false);
    }

    CXMLElement* p_nrel;

    // inject build
    if( (p_nrel = dynamic_cast<CXMLElement*>(p_rel->DuplicateNode(p_builds))) == NULL ) {
        ES_ERROR("unable to inject build into module");
        return(false);
    }

    // remove attribute name from build
    p_nrel->RemoveAttribute("name");

    // inject prefix attribute
    p_nrel->SetAttribute("prefix",prefix);

    return(true);
}

//------------------------------------------------------------------------------

bool CMap::InjectRootDocumentation(CXMLElement* p_sele,CXMLElement* p_mele,const CSmallString& site)
{
    CSmallString mname;
    p_mele->GetAttribute("name",mname);

    CSmallString prefix = site;

    // determine prefix

    // simply take the one for which the documentatation exists
    CFileName doc_name;
    doc_name = AMSGlobalConfig.GetETCDIR() / "map" / "docs" / prefix / mname + ".doc";
    if( CFileSystem::IsFile(doc_name) != true ) {
        prefix = NULL;
    }

    // try auto prefixes
    if( prefix == NULL ){
        std::list<std::string>::iterator    it = AutoPrefixes.begin();
        std::list<std::string>::iterator    ie = AutoPrefixes.end();

        while( it != ie ){
            prefix = CSmallString(*it);
            CFileName doc_name;
            doc_name = AMSGlobalConfig.GetETCDIR() / "map" / "docs" / prefix / mname + ".doc";
            if( CFileSystem::IsFile(doc_name) == true ) break;
            prefix = NULL;
            it++;
        }
    }

    // load documentation
    string rkey;
    doc_name = AMSGlobalConfig.GetETCDIR() / "map" / "docs" / prefix / mname + ".doc";
    rkey = string(CFileName(prefix) / mname);
    if( CFileSystem::IsFile(doc_name) == false ) return(true); // no documentation is available

    // is it cached?
    CXMLDocumentPtr p_docxml = DocCache[string(rkey)];
    if( p_docxml == NULL ){
        p_docxml = CXMLDocumentPtr(new CXMLDocument);
        // open build file
        CXMLParser   xml_parser;

        xml_parser.SetOutputXMLNode(p_docxml.get());
        xml_parser.EnableWhiteCharacters(true);
        if( xml_parser.Parse(doc_name) == false ) {
            CSmallString error;
            error << "unable to parse documentation file " << doc_name;
            ES_ERROR(error);
            return(false);
        }
        DocCache[string(rkey)] = p_docxml;
    }

    // ! check consistency ---------------------------------------------
    CXMLElement* p_mod = p_docxml->GetChildElementByPath("module");
    if( p_mod == NULL ) {
        CSmallString error;
        error << "documentation file " << doc_name << " does not contain module element";
        ES_ERROR(error);
        return(false);
    }

    CSmallString modname;
    p_mod->GetAttribute("name",modname);
    if( modname != mname ) {
        CSmallString error;
        error << "module name mismatch for documentation file" << doc_name;
        ES_ERROR(error);
        return(false);
    }
    // documentation ----------------------------------------
    CXMLElement* p_doc = p_docxml->GetChildElementByPath("module/documentation");
    if( p_doc != NULL ) {
        if( p_doc->DuplicateNode(p_mele) == false ) {
            CSmallString error;
            error << "unable to inject documentation file " << doc_name;
            ES_ERROR(error);
            return(false);
        }
        p_mele->CreateChildText("\n",true);
    }

    // ACL ----------------------------------------
    CXMLElement* p_acl = p_docxml->GetChildElementByPath("module/acl");
    if( p_acl != NULL ) {
        if( p_acl->DuplicateNode(p_mele) == false ) {
            CSmallString error;
            error << "unable to inject documentation file " << doc_name;
            ES_ERROR(error);
            return(false);
        }
        p_mele->CreateChildText("\n",true);
    }

    // dependencies ----------------------------------------
    CXMLElement* p_deps = p_docxml->GetChildElementByPath("module/dependencies");
    if( p_deps != NULL ) {
        if( p_deps->DuplicateNode(p_mele) == false ) {
            CSmallString error;
            error << "unable to inject documentation file " << doc_name;
            ES_ERROR(error);
            return(false);
        }
        p_mele->CreateChildText("\n",true);
    }

    // default version --------------------------------------
    CXMLElement* p_def = p_docxml->GetChildElementByPath("module/default");
    if( p_def != NULL ) {
        if( p_def->DuplicateNode(p_mele) == false ) {
            CSmallString error;
            error << "unable to inject default item from file " << doc_name;
            ES_ERROR(error);
            return(false);
        }
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CMap::IsBuild(const CSmallString& site_name,const CSmallString& build_name,
                   const CSmallString& prefix)
{
    return( GetBuildName(site_name,build_name,prefix) != NULL );
}

//------------------------------------------------------------------------------

const CSmallString CMap::GetBuildName(const CSmallString& site_name,
                        const CSmallString& build_name,const CSmallString& prefix)
{
    CSmallString build_name_ext = build_name;

    // add only if it is not present
    if( build_name_ext.FindSubString(".bld") == -1 ){
        build_name_ext += ".bld";
    }

    CFileName all_builds = AMSGlobalConfig.GetETCDIR() / "map" / "builds";
    CFileName full_build_name;

    if( prefix != NULL ){
        // is prefix specific
        full_build_name = all_builds / prefix / build_name_ext;
        if( CFileSystem::IsFile(full_build_name) ){
            return(full_build_name);
        }
    } else {
        // is site specific build?
        full_build_name = all_builds / site_name / build_name_ext;
        if( CFileSystem::IsFile(full_build_name) ){
            return(full_build_name);
        }

        // is within autoprefix?
        std::list<std::string>::iterator    it = AutoPrefixes.begin();
        std::list<std::string>::iterator    ie = AutoPrefixes.end();

        while( it != ie ){
            CSmallString auto_prefix = *it;

            full_build_name = all_builds / auto_prefix / build_name_ext;
            if( CFileSystem::IsFile(full_build_name) ){
                return(full_build_name);
            }
            it++;
        }
    }

    return("");
}

//------------------------------------------------------------------------------

void CMap::ShowSyncDeps(std::ostream& vout,const CSmallString& site_name,
                        const CSmallString& build,const CSmallString& prefix,bool deep)
{
    std::list<std::string> deps;
    AddSyncDeps(site_name,build,prefix,deps,deep);

    deps.sort();
    deps.unique();

    std::list<std::string>::iterator it = deps.begin();
    std::list<std::string>::iterator ie = deps.end();

    while( it != ie ){
        vout << *it << endl;
        it++;
    }
}

//------------------------------------------------------------------------------

void CMap::AddSyncDeps(const CSmallString& site_name,const CSmallString& build_name,
                       const CSmallString& prefix,std::list<std::string>& deps,bool deep)
{
    CFileName full_build_name;

    // is prefix specific
    full_build_name = GetBuildName(site_name,build_name,prefix);
    if( full_build_name == NULL ){
        ES_ERROR("build does not exist");
        return;
    }

    CXMLDocument    xml_doc;
    CXMLParser      xml_parser;
    xml_parser.SetOutputXMLNode(&xml_doc);

    if( xml_parser.Parse(full_build_name) == false ) {
        CSmallString error;
        error <<  "unable to parse build file (" << full_build_name << ")";
        ES_ERROR(error);
        return;
    }

    CXMLElement* p_ele = xml_doc.GetChildElementByPath("build/dependencies/syncdepend");
    while( p_ele != NULL ) {
        std::string sync_build_name;
        if( p_ele->GetAttribute("build",sync_build_name) == true ){
            if( find(deps.begin(), deps.end(), sync_build_name) == deps.end() ){
                deps.push_back(sync_build_name);
                if( deep ) AddSyncDeps(site_name,sync_build_name,prefix,deps,true);
            }
        }
        p_ele = p_ele->GetNextSiblingElement("syncdepend");
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

