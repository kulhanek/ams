// =============================================================================
//  AMS - Advanced Module System
// -----------------------------------------------------------------------------
//     Copyright (C) 2023 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2012 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2011      Petr Kulhanek, kulhanek@chemi.muni.cz
//     Copyright (C) 2001-2008 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <AMSRegistry.hpp>
#include <XMLParser.hpp>
#include <ErrorSystem.hpp>
#include <XMLElement.hpp>
#include <Shell.hpp>
#include <Utils.hpp>
#include <FileSystem.hpp>
#include <string.h>
#include <vector>

//------------------------------------------------------------------------------

#define DEFAULT_UMASK "0077"

//------------------------------------------------------------------------------

CAMSRegistry AMSRegistry;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CAMSRegistry::CAMSRegistry(void)
{
    AMSRoot = CShell::GetSystemVariable("AMS_ROOT");
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CAMSRegistry::LoadRegistry(void)
{
    SiteFlavor = CShell::GetSystemVariable("AMS_FLAVOUR");
    if( SiteFlavor == NULL ){
        SiteFlavor = "regular";
    }
}

//------------------------------------------------------------------------------

void CAMSRegistry::SaveUserConfig(void)
{

}

//------------------------------------------------------------------------------

void CAMSRegistry::SaveRegistry(const CFileName& registry_name)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CFileName CAMSRegistry::GetAMSRootDIR(void)
{
    return( AMSRoot );
}

//------------------------------------------------------------------------------

const CFileName CAMSRegistry::GetETCDIR(void)
{
    return( AMSRoot / "etc" );
}

//------------------------------------------------------------------------------

const CFileName CAMSRegistry::GetModActionPath(const CFileName& action_command)
{
    return( AMSRoot / "bin" / "actions" / action_command );
}

//------------------------------------------------------------------------------

const CFileName CAMSRegistry::GetUserGlobalConfigDir(void)
{
    CFileName user_config_dir;
    user_config_dir = CShell::GetSystemVariable("AMS_USER_CONFIG_DIR");

    if( user_config_dir == NULL ) {
        user_config_dir = CShell::GetSystemVariable("HOME");
        user_config_dir = user_config_dir / ".ams" / LibConfigVersion_AMS ;
    }

    // is file?
    if( CFileSystem::IsFile(user_config_dir) == true ) {
        CSmallString error;
        error << "user config dir '" << user_config_dir << "' is a file";
        ES_ERROR(error);
        return("");
    }

    if( CFileSystem::IsDirectory(user_config_dir) == false ) {
        // create directory
        if( CFileSystem::CreateDir(user_config_dir) == false ) {
            CSmallString error;
            error << "unable to create user config dir '" << user_config_dir << "'";
            ES_ERROR(error);
            return("");
        }
    }

    return(user_config_dir);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CAMSRegistry::GetUserUMask(void)
{
    CXMLElement* p_ele = Config.GetChildElementByPath("user",true);
    CSmallString umask;
    p_ele->GetAttribute("umask",umask);
    if( umask == NULL ){
        umask = DEFAULT_UMASK;
    }
    return(umask);
}

//------------------------------------------------------------------------------

void CAMSRegistry::SetUserUMask(const CSmallString& umask)
{
    CXMLElement* p_ele = Config.GetChildElementByPath("user",true);
    p_ele->SetAttribute("umask",umask);
}

//------------------------------------------------------------------------------

void CAMSRegistry::GetAutoLoadedModules(std::list<CSmallString>& modules,bool withorigin)
{
    // FIXME
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CFileName CAMSRegistry::GetHostsConfigFile(void)
{
    CFileName path = CShell::GetSystemVariable("AMS_HOSTS_CONFIG");
    if( path == NULL ){
        path = AMSRegistry.GetETCDIR() / "default" / "hosts.xml";
    }
    return(path);
}

//------------------------------------------------------------------------------

const CFileName CAMSRegistry::GetHostGroup(void)
{
    CFileName host_group = CShell::GetSystemVariable("AMS_HOST_GROUP");
    if( CFileSystem::IsFile(host_group) ){
        return(host_group);
    }
    CFileName paths  = GetHostGroupsSearchPaths();
    CFileName module = host_group;
    host_group = "host group '" + host_group + "' not found";
    CUtils::FindFile(paths,module,".xml",host_group);
    return(host_group);
}

//------------------------------------------------------------------------------

const CFileName CAMSRegistry::GetHostGroupsSearchPaths(void)
{
    CFileName path = CShell::GetSystemVariable("AMS_HOST_GROUPS_PATH");
    if( path == NULL ){
        path = AMSRegistry.GetETCDIR() / "host-groups";
    } else {
        if( path[0] == ':' ){
            path = AMSRegistry.GetETCDIR() / "host-groups" + path;
        } else if ( path[path.GetLength()-1] == ':' ) {
            path = path + AMSRegistry.GetETCDIR() / "host-groups";
        }
    }
    return(path);
}

//------------------------------------------------------------------------------

const CFileName CAMSRegistry::GetHostSubSystemsSearchPaths(void)
{
    CFileName path = CShell::GetSystemVariable("AMS_HOST_SUBSYSTEMS_PATH");
    if( path == NULL ){
        path = AMSRegistry.GetETCDIR() / "default" / "host-subsystems";
    } else {
        if( path[0] == ':' ){
            path = AMSRegistry.GetETCDIR() / "default" / "host-subsystems" + path;
        } else if ( path[path.GetLength()-1] == ':' ) {
            path = path + AMSRegistry.GetETCDIR() / "default" / "host-subsystems";
        }
    }
    return(path);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CFileName CAMSRegistry::GetUsersConfigFile(void)
{
    CFileName path = AMSRegistry.GetETCDIR() / "default" / "users.xml";
    return(path);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CFileName CAMSRegistry::GetSiteSearchPaths(void)
{
    CFileName path = CShell::GetSystemVariable("AMS_SITE_PATH");
    if( path == NULL ){
        path = AMSRegistry.GetETCDIR() / "sites";
    } else {
        if( path[0] == ':' ){
            path = AMSRegistry.GetETCDIR() / "sites" + path;
        } else if ( path[path.GetLength()-1] == ':' ) {
            path = path + AMSRegistry.GetETCDIR() / "sites";
        }
    }
    return(path);
}

//------------------------------------------------------------------------------

const CSmallString CAMSRegistry::GetSiteFlavor(void) const
{
    return(SiteFlavor);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CFileName CAMSRegistry::GetPrintProfileSearchPaths(void)
{
    CFileName path = CShell::GetSystemVariable("AMS_PRINT_PROFILE_PATH");
    if( path == NULL ){
        path = AMSRegistry.GetETCDIR() / "print-profiles";
    } else {
        if( path[0] == ':' ){
            path = AMSRegistry.GetETCDIR() / "print-profiles" + path;
        } else if ( path[path.GetLength()-1] == ':' ) {
            path = path + AMSRegistry.GetETCDIR() / "print-profiles";
        }
    }
    return(path);
}

//------------------------------------------------------------------------------

const CFileName CAMSRegistry::GetPrintProfileFile(void)
{
    CSmallString profile = "default";
    // FIXME

    CFileName path = GetPrintProfileSearchPaths();
    CFileName config_file;
    if( CUtils::FindFile(path,profile,".xml",config_file) ){
        return(config_file);
    }

    // fallback to default profile
    profile = "default";
    path = AMSRegistry.GetETCDIR() / "default" / "print-profiles";
    if( CUtils::FindFile(path,profile,".xml",config_file) ){
        return(config_file);
    }

    RUNTIME_ERROR("no print profile found");
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CFileName CAMSRegistry::GetBundleName(void)
{
    CFileName name = CShell::GetSystemVariable("AMS_BUNDLE_NAME");
    return(name);
}

//------------------------------------------------------------------------------

const CFileName CAMSRegistry::GetBundlePath(void)
{
    CFileName path = CShell::GetSystemVariable("AMS_BUNDLE_PATH");
    return(path);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

