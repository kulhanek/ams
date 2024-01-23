// =============================================================================
//  AMS - Advanced Module System
// -----------------------------------------------------------------------------
//     Copyright (C) 2023 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2020 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include <HostGroup.hpp>
#include <XMLElement.hpp>
#include <AMSRegistry.hpp>
#include <ErrorSystem.hpp>
#include <XMLParser.hpp>
#include <FileSystem.hpp>
#include <ShellProcessor.hpp>
#include <ModCache.hpp>
#include <Utils.hpp>
#include <fnmatch.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//------------------------------------------------------------------------------

CHostGroup   HostGroup;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CHostGroup::CHostGroup(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CHostGroup::InitHostsConfig(void)
{
    HostsConfigFile = AMSRegistry.GetHostsConfigFile();

    if( CFileSystem::IsFile(HostsConfigFile) == false ){
        // no group file
        CSmallString warning;
        warning << "no groups config file '" << HostsConfigFile << "'";
        ES_WARNING(warning);
        return;
    }

    CXMLParser xml_parser;
    xml_parser.SetOutputXMLNode(&HostsConfig);
    if( xml_parser.Parse(HostsConfigFile) == false ){
        ErrorSystem.RemoveAllErrors(); // avoid global error
        CSmallString warning;
        warning << "unable to parse groups config file '" << HostsConfigFile << "'";
        ES_WARNING(warning);
        return;
    }
}

//------------------------------------------------------------------------------

void CHostGroup::InitHostGroup(void)
{
    HostGroupFile = AMSRegistry.GetHostGroup();

    if( CFileSystem::IsFile(HostGroupFile) == false ){
        // no group file
        CSmallString error;
        error << "no host group file '" << HostGroupFile << "'";
        RUNTIME_ERROR(error);
    }

    CXMLParser xml_parser;
    xml_parser.SetOutputXMLNode(&HostGroup);
    if( xml_parser.Parse(HostGroupFile) == false ){
        CSmallString error;
        error << "unable to parse host group file '" << HostGroupFile << "'";
        RUNTIME_ERROR(error);
    }
}

//------------------------------------------------------------------------------

void CHostGroup::InitAllHostGroups(void)
{
    CXMLElement* p_ele = AllHostGroups.CreateChildElement("groups");
    std::list<CFileName> host_files;

    CFileName paths = AMSRegistry.GetHostGroupsSearchPaths();
    CUtils::FindAllFilesInPaths(paths,"*.xml",host_files);

    for(CFileName host_file : host_files){
        CXMLDocument xml_document;
        CXMLParser xml_parser;
        xml_parser.SetOutputXMLNode(&xml_document);
        if( xml_parser.Parse(host_file) == false ){
            CSmallString error;
            error << "unable to parse host group file '" << host_file << "'";
            RUNTIME_ERROR(error);
        }
        CXMLElement* p_hele = xml_document.GetFirstChildElement("group");
        if( p_hele != NULL ){
            p_hele->DuplicateNode(p_ele);
        }
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CHostGroup::GetDefaultHostCacheKey(void)
{
    CXMLElement* p_config = HostsConfig.GetFirstChildElement("config");
    CSmallString key("xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx");
    if( p_config == NULL ) return(key);
    p_config->GetAttribute("key",key);
    return(key);
}

//------------------------------------------------------------------------------

const CFileName CHostGroup::GetDefaultHostSubSystems(void)
{
    CXMLElement* p_config = HostsConfig.GetFirstChildElement("config");
    CSmallString hostsubsystems("default");
    if( p_config == NULL ) return(hostsubsystems);
    p_config->GetAttribute("hostsubsystems",hostsubsystems);
    return(hostsubsystems);
}

//------------------------------------------------------------------------------

CXMLElement* CHostGroup::GetParallelModes(void)
{
    CXMLElement* p_mele = HostsConfig.GetChildElementByPath("config/modes");
    if( p_mele == NULL ){
        RUNTIME_ERROR("unable to open hosts config/modes element");
    }
    return(p_mele);
}

//------------------------------------------------------------------------------

int CHostGroup::GetArchTokenScore(const CSmallString& token)
{
    CXMLElement* p_tele = HostsConfig.GetChildElementByPath("config/tokens/token");

    while( p_tele != NULL ){
        CSmallString stname;
        p_tele->GetAttribute("name",stname);
        if( stname == token ){
            int score = 0;
            p_tele->GetAttribute("score",score);
            return(score);
        }
        p_tele = p_tele->GetNextSiblingElement("token");
    }

    return(0);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CFileName CHostGroup::GetHostGroupFile(void) const
{
    return(HostGroupFile);
}

//------------------------------------------------------------------------------

const CFileName CHostGroup::GetHostGroupName(void)
{
    CFileName group_name;
    CXMLElement* p_grp = HostGroup.GetFirstChildElement("group");
    if( p_grp != NULL ){
        if( p_grp->GetAttribute("name",group_name) == true ){
            return(group_name);
        }
    }
    return(group_name);
}

//------------------------------------------------------------------------------

const CFileName CHostGroup::GetHostGroupNickName(void)
{
    CFileName group_name;
    CXMLElement* p_grp = HostGroup.GetFirstChildElement("group");
    if( p_grp != NULL ){
        if( p_grp->GetAttribute("nickname",group_name) == true ){
            return(group_name);
        }
    }
    return(group_name);
}

//------------------------------------------------------------------------------

void CHostGroup::PrintHostGroupInfo(CVerboseStr& vout)
{
    vout << endl;
    vout << "# Host group name: " << GetHostGroupName() << endl;
    vout << "# Host group path: " << GetHostGroupFile() << endl;
}

//------------------------------------------------------------------------------

const CFileName CHostGroup::GetHostSubSystems(void)
{
    CFileName host_subsystems;
    CXMLElement* p_grp = HostGroup.GetFirstChildElement("group");
    if( p_grp != NULL ){
        if( p_grp->GetAttribute("hostsubsystems",host_subsystems) == true ){
            return(host_subsystems);
        }
    }
    host_subsystems = GetDefaultHostSubSystems();
    return(host_subsystems);
}

//------------------------------------------------------------------------------

const CSmallString CHostGroup::GetDefaultSite(void)
{
    CXMLElement* p_ele = HostGroup.GetChildElementByPath("group/sites");
    if( p_ele == NULL ){
        CSmallString warning;
        warning << "no sites defined in the file '" << HostGroupFile << "'";
        RUNTIME_ERROR(warning);
    }
    CSmallString default_site;
    if( p_ele->GetAttribute("default",default_site) == false ){
        CSmallString warning;
        warning << "no default site defined in sites of the file '" << HostGroupFile << "'";
        RUNTIME_ERROR(warning);
    }
    return(default_site);
}

//------------------------------------------------------------------------------

void CHostGroup::GetAllowedSites(std::set<CSmallString>& list)
{
    CXMLElement* p_ele = HostGroup.GetChildElementByPath("group/sites");
    if( p_ele == NULL ){
        CSmallString warning;
        warning << "no sites defined in the file '" << HostGroupFile << "'";
        RUNTIME_ERROR(warning);
    }

    std::string allowed_sites;
    if( p_ele->GetAttribute("allowed",allowed_sites) == true ){
        split(list,allowed_sites,boost::is_any_of(","));
    }

    // add the default site
    list.insert(GetDefaultSite());
}

//------------------------------------------------------------------------------

void CHostGroup::GetTransferableSites(std::set<CSmallString>& list)
{
    CXMLElement* p_ele = HostGroup.GetChildElementByPath("group/sites");
    if( p_ele == NULL ){
        CSmallString warning;
        warning << "no sites defined in the file '" << HostGroupFile << "'";
        RUNTIME_ERROR(warning);
    }
    std::string transferable_sites;
    if( p_ele->GetAttribute("transferable",transferable_sites) == true ){
        split(list,transferable_sites,boost::is_any_of(","));
    }

    // add the default site
    list.insert(GetDefaultSite());
}

//------------------------------------------------------------------------------

bool CHostGroup::IsSiteAllowed(const CSmallString& name)
{
    std::set<CSmallString> allowed_sites;
    GetAllowedSites(allowed_sites);
    return( std::find(allowed_sites.begin(), allowed_sites.end(), name) != allowed_sites.end() );
}

//------------------------------------------------------------------------------

bool CHostGroup::IsSiteTransferable(const CSmallString& name)
{
    std::set<CSmallString> allowed_sites;
    GetAllowedSites(allowed_sites);
    return( std::find(allowed_sites.begin(), allowed_sites.end(), name) != allowed_sites.end() );
}

//------------------------------------------------------------------------------

const CSmallString CHostGroup::GetGroupNS(void)
{
    CXMLElement* p_grp = HostGroup.GetFirstChildElement("group");
    if( p_grp == NULL ) return("na");
    CSmallString gns("na");
    p_grp->GetAttribute("groupns",gns);
    return(gns);
}

//------------------------------------------------------------------------------

const CSmallString CHostGroup::GetRealm(void)
{
    CXMLElement* p_grp = HostGroup.GetFirstChildElement("group");
    if( p_grp == NULL ) return("NONE");
    CSmallString realm("NONE");
    p_grp->GetAttribute("realm",realm);
    return(realm);
}

//------------------------------------------------------------------------------

void CHostGroup::GetHostGroupAutoLoadedModules(std::list<CSmallString>& modules,bool withorigin)
{
    CSmallString flavour = AMSRegistry.GetUserSiteFlavour();

    CXMLElement* p_ele = GetHostGroupAutoLoadedModules();
    if( p_ele ){
        p_ele = p_ele->GetFirstChildElement("module");
    }
    while( p_ele ){
        CSmallString mname,mflavour;
        p_ele->GetAttribute("name",mname);
        p_ele->GetAttribute("flavour",mflavour);
        if( (mname != NULL) && ((mflavour == NULL) || (mflavour == flavour))){
            bool enabled = ModCache.IsAutoloadEnabled(mname);
            if( withorigin ){
                mname << "[hostgroup:" << GetHostGroupName();
                if( mflavour != NULL ) mname << "@" << mflavour;
                mname << "]";
                modules.push_back(mname);
            } else {
                if( enabled ) modules.push_back(mname);
            }

        }
        p_ele = p_ele->GetNextSiblingElement("module");
    }
}

//------------------------------------------------------------------------------

CXMLElement* CHostGroup::GetHostGroupAutoLoadedModules(void)
{
    return(HostGroup.GetChildElementByPath("group/autoload"));
}

//------------------------------------------------------------------------------

CXMLElement* CHostGroup::GetHostGroupEnvironment(void)
{
    return(HostGroup.GetChildElementByPath("group/environment"));
}

//------------------------------------------------------------------------------

void CHostGroup::GetHostsConfigAutoLoadedModules(std::list<CSmallString>& modules,bool withorigin)
{
    CSmallString flavour = AMSRegistry.GetUserSiteFlavour();

    CXMLElement* p_ele = GetHostsConfigAutoLoadedModules();
    if( p_ele ){
        p_ele = p_ele->GetFirstChildElement("module");
    }
    while( p_ele ){
        CSmallString mname,mflavour;
        p_ele->GetAttribute("name",mname);
        p_ele->GetAttribute("flavour",mflavour);
        if( (mname != NULL) && ((mflavour == NULL) || (mflavour == flavour))){
            bool enabled = ModCache.IsAutoloadEnabled(mname);
            if( withorigin ){
                mname << "[hostsconfig:" << GetHostGroupName();
                if( mflavour != NULL ) mname << "@" << mflavour;
                mname << "]";
                modules.push_back(mname);
            } else {
                if( enabled ) modules.push_back(mname);
            }

        }
        p_ele = p_ele->GetNextSiblingElement("module");
    }
}

//------------------------------------------------------------------------------

CXMLElement* CHostGroup::GetHostsConfigAutoLoadedModules(void)
{
    return(HostsConfig.GetChildElementByPath("config/autoload"));
}

//------------------------------------------------------------------------------

CXMLElement* CHostGroup::GetHostsConfigEnvironment(void)
{
    return(HostsConfig.GetChildElementByPath("config/environment"));
}

//------------------------------------------------------------------------------

const CSmallString CHostGroup::GetSurrogateMachines(void)
{
    CSmallString surrogate_machines;
    CXMLElement* p_ele = HostGroup.GetChildElementByPath("group");
    if( p_ele == NULL ) return(surrogate_machines);
    p_ele->GetAttribute("surrogate_machines",surrogate_machines);
    return(surrogate_machines);
}

//------------------------------------------------------------------------------

const CSmallString CHostGroup::GetUserUMask(void)
{
    CSmallString umask;
    CXMLElement* p_ele = HostGroup.GetChildElementByPath("group");
    if( p_ele == NULL ) return(umask);
    p_ele->GetAttribute("umask",umask);
    return(umask);
}

//------------------------------------------------------------------------------

const CSmallString CHostGroup::GetHostGroupBundleSyncSuggestions(void)
{
    CSmallString profiles;
    CXMLElement* p_ele = HostGroup.GetChildElementByPath("group");
    if( p_ele == NULL ) return(profiles);
    p_ele->GetAttribute("bundle_sync_profiles",profiles);
    return(profiles);
}

//------------------------------------------------------------------------------

const CSmallString CHostGroup::GetHostGroupCoreSyncSuggestions(void)
{
    CSmallString profiles;
    CXMLElement* p_ele = HostGroup.GetChildElementByPath("group");
    if( p_ele == NULL ) return(profiles);
    p_ele->GetAttribute("core_sync_profiles",profiles);
    return(profiles);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CHostGroup::GetRealm(const CSmallString& hostname)
{
    CXMLElement* p_grp = FindGroup(hostname);
    if( p_grp == NULL ) return("NONE");
    CSmallString realm("NONE");
    p_grp->GetAttribute("realm",realm);
    return(realm);
}

//------------------------------------------------------------------------------

const CSmallString CHostGroup::GetGroupNS(const CSmallString& hostname)
{
    CXMLElement* p_grp = FindGroup(hostname);
    if( p_grp == NULL ) return("na");
    CSmallString gns("na");
    p_grp->GetAttribute("groupns",gns);
    return(gns);
}

//------------------------------------------------------------------------------

CXMLElement* CHostGroup::FindGroup(const CSmallString& hostname)
{
    CXMLElement* p_gele = AllHostGroups.GetChildElementByPath("groups/group");
    while( p_gele != NULL ){
        CXMLElement* p_host = p_gele->GetChildElementByPath("hosts/host");
        while( p_host != NULL ){
            CSmallString name;
            p_host->GetAttribute("name",name);
            if( fnmatch(name,hostname,0) == 0){
                return(p_gele);
            }
            p_host = p_host->GetNextSiblingElement();
        }
        p_gele = p_gele->GetNextSiblingElement();
    }

    return(p_gele);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CHostGroup::ExecuteStatAction(const CSmallString& action, CStatDatagramSender* p_sender)
{
    if( p_sender == NULL ){
        ES_ERROR("no stat sender");
        return(false);
    }

    int flags = p_sender->GetFlags();

    // try first the host group specific setup
    CXMLElement* p_ele = HostGroup.GetChildElementByPath("group/actions");
    if( p_ele == NULL ){
        // then try the global setup
        p_ele = HostsConfig.GetChildElementByPath("config/actions");
    }

    if( p_ele == NULL ) {
        // no actions -> return
        ES_WARNING("no actions are defined");
        return(true);
    }

    p_ele = p_ele->GetFirstChildElement("action");

    while( p_ele != NULL ) {
        CXMLElement* p_cele = p_ele;
        p_ele = p_ele->GetNextSiblingElement("action");

        CSmallString laction;
        if( p_cele->GetAttribute("name",laction) == false ) continue;
        if( laction != action ) continue;

        int lflags;
        if( p_cele->GetAttribute("flags",lflags) == true ){
            if( (flags & lflags) != flags ) continue; // incompatible flags
        }

        CSmallString server;
        if( p_cele->GetAttribute("server",server) == false ){
            ES_ERROR("no server");
            return(false);
        }

        int port = 0;
        if( p_cele->GetAttribute("port",port) == false ){
            ES_ERROR("no port");
            return(false);
        }

        CSmallString warning;
        warning << "action '" << laction << "' emmited";
        ES_WARNING(warning);

        return(p_sender->SendDataToServer(server,port));
    }

    ES_WARNING("no valid action was found");

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

