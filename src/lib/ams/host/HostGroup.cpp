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
#include <fnmatch.h>

//------------------------------------------------------------------------------

using namespace std;
//using namespace boost;
//using namespace boost::algorithm;

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
    // FIXME
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
    CXMLElement* p_ele = HostGroup.GetFirstChildElement("group/sites");
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

