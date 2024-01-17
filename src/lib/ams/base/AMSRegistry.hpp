#ifndef AMSRegistryH
#define AMSRegistryH
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

#include <AMSMainHeader.hpp>
#include <XMLDocument.hpp>
#include <FileName.hpp>
#include <AmsUUID.hpp>
#include <list>

//------------------------------------------------------------------------------

class AMS_PACKAGE CAMSRegistry {
public:
// constructor and destructor --------------------------------------------------
    CAMSRegistry(void);

// registry initialization -----------------------------------------------------
    /// init all registry
    void LoadRegistry(void);

    /// save user registry
    bool SaveUserConfig(void);

    /// save all registry into specified file
    bool SaveRegistry(const CFileName& registry_name);

// system config ---------------------------------------------------------------
    /// get system variable either form the shell environment or the registry
    const CSmallString GetSystemVariable(const CSmallString& name);

    /// return infinity root directory
    const CFileName GetAMSRootDIR(void);

    /// return infinity etc directory
    const CFileName GetETCDIR(void);

    /// get full pathname to mod action command
    const CFileName GetModActionPath(const CFileName& action_command);

// user preferences ------------------------------------------------------------
    /// get user umask
    const CSmallString GetUserUMask(void);

    /// set user umask
    void SetUserUMask(const CSmallString& umask);

    /// get default umask
    const CSmallString GetDefaultUMask(void);

    /// get site flavour
    const CSmallString GetUserSiteFlavour(void);

    /// get site flavour
    void SetUserSiteFlavour(const CSmallString& site_flavour);

// -----------------------------------------------
    /// get list of autoloaded modules
    void GetUserAutoLoadedModules(std::list<CSmallString>& modules,bool withorigin=false);

    /// get list of autoloaded modules
    CXMLElement* GetUserAutoLoadedModules(void);

    /// is module autoloaded
    bool IsUserAutoLoadedModule(const CSmallString& name);

    /// add module into auto-loaded list
    void AddUserAutoLoadedModule(const CSmallString& name);

    /// remove module from auto-loaded list
    void RemoveUserAutoLoadedModule(const CSmallString& name);

    /// remove all modules from auto-loaded list
    void RemoveAllUserAutoLoadedModules(void);

// -----------------------------------------------
    /// get list of print profiles
    void GetPrintProfiles(std::list<CSmallString>& profiles);

    /// is existing user print profile?
    bool IsUserPrintProfile(const CSmallString& name);

    /// get user print profile name
    const CSmallString GetUserPrintProfile(void);

    /// set user print profile, NULL - set default
    void SetUserPrintProfile(const CSmallString& name);

// -----------------------------------------------
    /// get list of user bundle names
    void GetUserBundleNames(std::list<CSmallString>& names);

    /// is bundle name specified?
    bool IsUserBundleName(const CSmallString& name);

    /// add user bundle name
    void AddUserBundleName(const CSmallString& name);

    /// remove user bundle name
    void RemoveUserBundleName(const CSmallString& name);

    /// remove all user bundle names
    void RemoveAllUserBundleNames(void);

    /// get user bundle path
    const CFileName GetUserBundlePath(void);

    /// set user bundle paths
    void SetUserBundlePath(const CFileName& path);

// host configuration ----------------------------------------------------------
    /// return full file name to the hosts configurations
    const CFileName GetHostsConfigFile(void);

    /// return full file name to the host group definition
    const CFileName GetHostGroup(void);

    /// return column separated paths for host groups configurations
    const CFileName GetHostGroupsSearchPaths(void);

    /// return column separated paths for host submodule configurations
    const CFileName GetHostSubSystemsSearchPaths(void);

// users configuration ---------------------------------------------------------
    /// return full file name to the users configurations
    const CFileName GetUsersConfigFile(void);

// site configuration ----------------------------------------------------------
    /// return column separated paths for site configurations
    const CFileName GetSiteSearchPaths(void);

// print engine setup ----------------------------------------------------------
    /// return column separated paths for print profiles
    const CFileName GetPrintProfileSearchPaths(void);

    /// return full file name to the print profile
    const CFileName GetPrintProfileFile(void);

// bundle configuration --------------------------------------------------------
    /// comma separated bundle names
    const CFileName GetBundleName(void);

    /// column separated bundle search paths
    const CFileName GetBundlePath(void);

// ABS integration -------------------------------------------------------------
    /// get ABS configuration
    CXMLElement* GetABSConfiguration(void);

// section of private data -----------------------------------------------------
private:
    CFileName       AMSRoot;            // ams root directory - read from AMS_ROOT_V9 variable
    CXMLDocument    Config;             // global config data

    /// get user global setup
    const CFileName GetUserGlobalConfig(void);

    /// set registry value
    void SetRegistryVariable(const CSmallString& name);
};

//------------------------------------------------------------------------------

extern AMS_PACKAGE CAMSRegistry AMSRegistry;

//------------------------------------------------------------------------------

#endif
