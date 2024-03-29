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
#include <XMLPrinter.hpp>
#include <ErrorSystem.hpp>
#include <XMLElement.hpp>
#include <Shell.hpp>
#include <Utils.hpp>
#include <FileSystem.hpp>
#include <string.h>
#include <vector>
#include <algorithm>
#include <ModCache.hpp>
#include <UserUtils.hpp>
#include <SiteController.hpp>
#include <TerminalStr.hpp>

//------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------

#define DEFAULT_UMASK "077"

//------------------------------------------------------------------------------

CAMSRegistry AMSRegistry;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CAMSRegistry::CAMSRegistry(void)
{
    AMSRoot = CShell::GetSystemVariable("AMS_ROOT_V9");
    ConfigLoaded = false;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CAMSRegistry::LoadRegistry(CVerboseStr& vout)
{
    if( ConfigLoaded ) return;

// load config
    CFileName config_name = GetUserGlobalConfig(vout);

    if( CFileSystem::IsFile(config_name) == false ){
        CSmallString warning;
        warning << "file '" << config_name << "' does not exist";
        ES_WARNING(warning);
        // silent error
        ConfigLoaded = true;
        return;
    }

    CXMLParser xml_parser;
    xml_parser.SetOutputXMLNode(&Config);
    if( xml_parser.Parse(config_name) == false ){
        ErrorSystem.RemoveAllErrors(); // avoid global error
        CSmallString warning;
        warning << "unable to parse user-registry file '" << config_name << "'";
        ES_WARNING(warning);
        return;
    }

    ConfigLoaded = true;
}

//------------------------------------------------------------------------------

bool CAMSRegistry::SaveUserConfig(void)
{
    CVerboseStr fake;
    CFileName config_name = GetUserGlobalConfig(fake);

    CXMLPrinter xml_printer;
    xml_printer.SetPrintedXMLNode(&Config);
    if( xml_printer.Print(config_name) == false ){
        CSmallString error;
        error << "unable to save user-registry file '" << config_name << "'";
        ES_ERROR(error);
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CAMSRegistry::SaveRegistry(const CFileName& registry_name)
{
    CXMLElement* p_ele = Config.GetChildElementByPath("registry/ams/variables");
    if( p_ele != NULL ){
        p_ele->RemoveAllChildNodes();
    }

    SetRegistryVariable("AMS_FLAVOUR");
    SetRegistryVariable("AMS_HOSTS_CONFIG");
    SetRegistryVariable("AMS_HOST_GROUP");
    SetRegistryVariable("AMS_HOST_GROUPS_PATH");
    SetRegistryVariable("AMS_HOST_SUBSYSTEMS_PATH");
    SetRegistryVariable("AMS_SITE_PATH");
    SetRegistryVariable("AMS_PRINT_PROFILE_PATH");
    SetRegistryVariable("AMS_BUNDLE_NAME");
    SetRegistryVariable("AMS_BUNDLE_PATH");

    CXMLPrinter xml_printer;
    xml_printer.SetPrintedXMLNode(&Config);
    if( xml_printer.Print(registry_name) == false ){
        CSmallString error;
        error << "unable to save full-registry file '" << registry_name << "'";
        ES_ERROR(error);
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

const CSmallString CAMSRegistry::GetSystemVariable(const CSmallString& name)
{
// first try registry records
    CXMLElement* p_ele = Config.GetChildElementByPath("registry/ams/variables/variable");

    while( p_ele != NULL ){
        CSmallString vname,value;
        p_ele->GetAttribute("name",vname);
        p_ele->GetAttribute("value",value);

        if( vname == name ){
            return(value);
        }

        p_ele = p_ele->GetNextSiblingElement("variable");
    }

// then try system ones
    return( CShell::GetSystemVariable(name) );
}

//------------------------------------------------------------------------------

void CAMSRegistry::SetRegistryVariable(const CSmallString& name)
{
    CSmallString value = CShell::GetSystemVariable(name);

    CXMLElement* p_ele = Config.GetChildElementByPath("registry/ams/variables",true);
    p_ele = p_ele->CreateChildElement("variable");
    p_ele->SetAttribute("name",name);
    p_ele->SetAttribute("value",value);
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

const CFileName CAMSRegistry::GetUserGlobalConfig(CVerboseStr& vout)
{
    CFileName user_config;
    user_config = CShell::GetSystemVariable("AMS_REGISTRY_CONFIG");

    if( user_config != NULL ){
        // is file?
        if( CFileSystem::IsFile(user_config) == false ) {
            CSmallString error;
            error << "user config '" << user_config << "' from AMS_REGISTRY_CONFIG is not a file";
            RUNTIME_ERROR(error);
        }
    }

    if( user_config == NULL ) {

        CFileName user_config_dir = CShell::GetSystemVariable("AMS_USER_CONFIG_DIR");

        if( user_config_dir != NULL ) {
            if( CFileSystem::IsDirectory(user_config_dir) == false ) {
                // create directory
                if( CFileSystem::CreateDir(user_config_dir) == false ) {
                    if( SiteController.HasTTY() == true ){
                        // print this only if in TTY (interactive session), RT#1255616
                        CTerminalStr        err;
                        err.Attach(stderr);
                        err << "<red><b>" << endl;
                        err << ">>> ERROR:   The non-standard AMS config directory is not accessible!" << endl;
                        err << "             Directory: '" << user_config_dir << "'" << endl;
                        err << "             Error:     " << ErrorSystem.GetLastError() << endl;
                        ErrorSystem.RemoveAllErrors();
                        err << endl;
                        err << ">>> WARNING: Switching into the emergency mode employing the standard configuration directory!" << endl;
                        err << "             This is not optimal and can lead to many problems." << endl;
                        err << "             Remove the error as soon as possible (kinit, afslog, or contact user support)!" << endl;
                        err << "             Note: A new session has to be opened after you fix the problem." << endl;
                        err << "</b></red>" << endl;
                    } else {
                        // supress all errors, RT#1255616
                        ErrorSystem.RemoveAllErrors();
                    }
                    user_config_dir = NULL;
                }
            }
        }

        if( user_config_dir == NULL ){
            user_config_dir = CShell::GetSystemVariable("HOME");
            user_config_dir = user_config_dir / ".ams" / LibConfigVersion_AMS ;
        }

        if( CFileSystem::IsDirectory(user_config_dir) == false ) {
            // create directory
            if( CFileSystem::CreateDir(user_config_dir) == false ) {
                CSmallString error;
                error << "unable to create user config dir '" << user_config_dir << "'";
                RUNTIME_ERROR(error);
            }
        }

        user_config = user_config_dir / "user-registry.xml" ;
    }

    return(user_config);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CAMSRegistry::GetUserUMask(void)
{
    CXMLElement* p_ele = Config.GetChildElementByPath("registry/ams/user",true);
    CSmallString umask;
    p_ele->GetAttribute("umask",umask);
    return(umask);
}

//------------------------------------------------------------------------------

const CSmallString CAMSRegistry::GetDefaultUMask(void)
{
    return(DEFAULT_UMASK);
}

//------------------------------------------------------------------------------

void CAMSRegistry::SetUserUMask(const CSmallString& umask)
{
    CXMLElement* p_ele = Config.GetChildElementByPath("registry/ams/user",true);
    if( (umask == NULL) || (umask == "default") ){
        p_ele->RemoveAttribute("umask");
    } else {
        p_ele->SetAttribute("umask",umask);
    }
}

//------------------------------------------------------------------------------

const CSmallString CAMSRegistry::GetUserSiteFlavour(void)
{
    CXMLElement* p_ele = Config.GetChildElementByPath("registry/ams/user",true);
    CSmallString site_flavour;
    // try user setup
    if( (p_ele->GetAttribute("site_flavour",site_flavour) == false) || (site_flavour == NULL) ){
        // try envirnoment setup
        site_flavour = GetSystemVariable("AMS_FLAVOUR");
    }
    if( site_flavour == NULL ){
        // use default
        site_flavour = "amsbase";
    }
    return(site_flavour);
}

//------------------------------------------------------------------------------

void CAMSRegistry::SetUserSiteFlavour(const CSmallString& site_flavour)
{
    CXMLElement* p_ele = Config.GetChildElementByPath("registry/ams/user",true);
    if( (site_flavour == NULL) || (site_flavour == "default") ){
        p_ele->RemoveAttribute("site_flavour");
    } else {
        p_ele->SetAttribute("site_flavour",site_flavour);
    }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CAMSRegistry::GetUserAutoLoadedModules(std::list<CSmallString>& modules,bool withorigin)
{
    CXMLElement* p_ele = GetUserAutoLoadedModules();
    if( p_ele ){
        p_ele = p_ele->GetFirstChildElement("module");
    }

    while( p_ele != NULL ){
        CSmallString mname;
        p_ele->GetAttribute("name",mname);
        if( mname != NULL ){
            bool enabled = ModCache.IsAutoloadEnabled(mname);
            if( withorigin ){
                mname << "[user]";
                modules.push_back(mname);
            } else {
                if( enabled ) modules.push_back(mname);
            }
        }
        p_ele = p_ele->GetNextSiblingElement("module");
    }
}

//------------------------------------------------------------------------------

CXMLElement* CAMSRegistry::GetUserAutoLoadedModules(void)
{
   CXMLElement* p_ele = Config.GetChildElementByPath("registry/ams/user/autoloaded");
   return(p_ele);
}

//------------------------------------------------------------------------------

bool CAMSRegistry::IsUserAutoLoadedModule(const CSmallString& name)
{
    std::list<CSmallString> modules;
    GetUserAutoLoadedModules(modules);

    return( std::find(modules.begin(), modules.end(), name) != modules.end() );
}

//------------------------------------------------------------------------------

void CAMSRegistry::AddUserAutoLoadedModule(const CSmallString& name)
{
    CXMLElement* p_ele = Config.GetChildElementByPath("registry/ams/user/autoloaded/module",true);
    p_ele->SetAttribute("name",name);
}

//------------------------------------------------------------------------------

void CAMSRegistry::RemoveUserAutoLoadedModule(const CSmallString& name)
{
    CXMLElement* p_ele = Config.GetChildElementByPath("registry/ams/user/autoloaded/module");
    if( p_ele == NULL ) return;

    while( p_ele != NULL ){
        CSmallString mname;
        p_ele->GetAttribute("name",mname);
        if( mname == name ){
            delete p_ele;
            return;
        }
        p_ele = p_ele->GetNextSiblingElement("module");
    }
}

//------------------------------------------------------------------------------

void CAMSRegistry::RemoveAllUserAutoLoadedModules(void)
{
    CXMLElement* p_ele = Config.GetChildElementByPath("registry/ams/user/autoloaded");
    if( p_ele != NULL ){
        p_ele->RemoveAllChildNodes();
    }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CAMSRegistry::GetPrintProfiles(std::list<CSmallString>& profiles)
{
    std::list<CFileName> profile_paths;

    CFileName path = GetPrintProfileSearchPaths();

    CUtils::FindAllFilesInPaths(path,"*.xml",profile_paths);

    path = AMSRegistry.GetETCDIR() / "default" / "print-profiles";

    CUtils::FindAllFilesInPaths(path,"*.xml",profile_paths);

    profiles.clear();

    for(CFileName profile_path : profile_paths){
        CSmallString profile = profile_path.GetFileNameWithoutExt();
        profiles.push_back(profile);
    }

    profiles.sort();
    profiles.unique();
}

//------------------------------------------------------------------------------

bool CAMSRegistry::IsUserPrintProfile(const CSmallString& name)
{
    std::list<CSmallString> profiles;
    GetPrintProfiles(profiles);

    return( std::find(profiles.begin(), profiles.end(), name) != profiles.end() );
}

//------------------------------------------------------------------------------

const CSmallString CAMSRegistry::GetUserPrintProfile(void)
{
    CSmallString profile = "default";

    CXMLElement* p_ele = Config.GetChildElementByPath("registry/ams/user");
    if( p_ele != NULL ){
        p_ele->GetAttribute("printprofile",profile);
    }

    return(profile);
}

//------------------------------------------------------------------------------

void CAMSRegistry::SetUserPrintProfile(const CSmallString& name)
{
    CXMLElement* p_ele = Config.GetChildElementByPath("registry/ams/user",true);
    if( (name == NULL) || (name == "default") ){
        p_ele->RemoveAttribute("printprofile");
    } else {
        p_ele->SetAttribute("printprofile",name);
    }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CAMSRegistry::GetUserBundleNames(std::list<CSmallString>& names)
{
    CXMLElement* p_ele = Config.GetChildElementByPath("registry/ams/user/bundles/bundle");
    if( p_ele == NULL ) return;

    while( p_ele != NULL ){
        CSmallString bname;
        p_ele->GetAttribute("name",bname);
        if( bname != NULL ){
            names.push_back(bname);
        }
        p_ele = p_ele->GetNextSiblingElement("bundle");
    }
}

//------------------------------------------------------------------------------

bool CAMSRegistry::IsUserBundleName(const CSmallString& name)
{
    std::list<CSmallString> bundles;
    GetUserBundleNames(bundles);

    return( std::find(bundles.begin(), bundles.end(), name) != bundles.end() );
}

//------------------------------------------------------------------------------

void CAMSRegistry::AddUserBundleName(const CSmallString& name)
{
    CXMLElement* p_ele = Config.GetChildElementByPath("registry/ams/user/bundles",true);
    CXMLElement* p_bele = p_ele->CreateChildElement("bundle");
    p_bele->SetAttribute("name",name);
}

//------------------------------------------------------------------------------

void CAMSRegistry::RemoveUserBundleName(const CSmallString& name)
{
    CXMLElement* p_ele = Config.GetChildElementByPath("registry/ams/user/bundles/bundle");
    if( p_ele == NULL ) return;

    while( p_ele != NULL ){
        CSmallString mname;
        p_ele->GetAttribute("name",mname);
        if( mname == name ){
            delete p_ele;
            return;
        }
        p_ele = p_ele->GetNextSiblingElement("bundle");
    }
}

//------------------------------------------------------------------------------

void CAMSRegistry::RemoveAllUserBundleNames(void)
{
    CXMLElement* p_ele = Config.GetChildElementByPath("registry/ams/user/bundles");
    if( p_ele != NULL ){
        p_ele->RemoveAllChildNodes();
    }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

const CFileName CAMSRegistry::GetUserBundlePath(void)
{
    CFileName path;
    CXMLElement* p_ele = Config.GetChildElementByPath("registry/ams/user/bundles");
    if( p_ele != NULL ){
        p_ele->GetAttribute("path",path);
    }
    return(path);
}

//------------------------------------------------------------------------------

void CAMSRegistry::SetUserBundlePath(const CFileName& path)
{
    CXMLElement* p_ele = Config.GetChildElementByPath("registry/ams/user/bundles",true);
    if( (path == NULL) || (path == "default") ){
        p_ele->RemoveAttribute("path");
    } else {
        p_ele->SetAttribute("path",path);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CFileName CAMSRegistry::GetHostsConfigFile(void)
{
    CFileName path = GetSystemVariable("AMS_HOSTS_CONFIG");
    if( path == NULL ){
        path = AMSRegistry.GetETCDIR() / "default" / "hosts.xml";
    }
    return(path);
}

//------------------------------------------------------------------------------

const CFileName CAMSRegistry::GetHostGroup(void)
{
    // this is from the job
    CFileName host_group = CShell::GetSystemVariable("INF_JOB_HOST_GROUP");

    // if not set, use standard setup
    if( host_group == NULL ){
       host_group = GetSystemVariable("AMS_HOST_GROUP");
    }

    if( CFileSystem::IsFile(host_group) ){
        return(host_group);
    }
    CFileName paths  = GetHostGroupsSearchPaths();
    CFileName module = host_group;

    if( host_group == NULL ){
        host_group = "-notset-";
    } else {
        host_group = host_group + "-notfound";
    }

    CUtils::FindFile(paths,module,".xml",host_group);
    return(host_group);
}

//------------------------------------------------------------------------------

const CFileName CAMSRegistry::GetHostGroupsSearchPaths(void)
{
    CFileName path = GetSystemVariable("AMS_HOST_GROUPS_PATH");
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
    CFileName path = GetSystemVariable("AMS_HOST_SUBSYSTEMS_PATH");
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
    CFileName path = GetSystemVariable("AMS_SITE_PATH");
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

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CFileName CAMSRegistry::GetPrintProfileSearchPaths(void)
{
    CFileName path = GetSystemVariable("AMS_PRINT_PROFILE_PATH");
    if( path == NULL ){
        path = AMSRegistry.GetETCDIR() / "default" / "print-profiles";
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
    CSmallString profile = GetUserPrintProfile();

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
    std::list<CSmallString> bundles;
    GetUserBundleNames(bundles);

    CSmallString name;

    bool first = true;
    for(CSmallString bundle : bundles){
        if( ! first )  name << ",";
        name << bundle;
        first = false;
    }

    CFileName sys_bundles = GetSystemVariable("AMS_BUNDLE_NAME");
    if( ! first )  name << ",";
    name << sys_bundles;

    return(name);
}

//------------------------------------------------------------------------------

const CFileName CAMSRegistry::GetBundlePath(void)
{
    CSmallString path = GetUserBundlePath();

    CFileName sys_path = GetSystemVariable("AMS_BUNDLE_PATH");
    if( path != NULL ){
        path << ":";
    }
    path << sys_path;

    return(path);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CXMLElement* CAMSRegistry::GetABSConfiguration(void)
{
    CXMLElement* p_ele = Config.GetChildElementByPath("registry/abs",true);
    return(p_ele);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

