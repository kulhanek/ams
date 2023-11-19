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

//------------------------------------------------------------------------------

class AMS_PACKAGE CAMSRegistry {
public:
// constructor and destructor --------------------------------------------------
    CAMSRegistry(void);

// registry initialization -----------------------------------------------------
    /// init all registry
    void LoadRegistry(void);

    /// save user registry
    void SaveUserConfig(void);

    /// save all registry into specified file
    void SaveRegistry(const CFileName& registry_name);

// system config ---------------------------------------------------------------
    /// return infinity root directory
    const CFileName GetAMSRootDIR(void);

    /// return infinity etc directory
    const CFileName GetETCDIR(void);

// user preferences ------------------------------------------------------------
    /// get user umask
    const CSmallString GetUserUMask(void);

    /// set user umask
    void SetUserUMask(const CSmallString& umask);

// host configuration ----------------------------------------------------------
    /// return full file name to the hosts configurations
    const CFileName GetHostsConfigFile(void);

    /// return full file name to the host group definition
    const CFileName GetHostGroup(void);

    /// return column separated paths for host groups configurations
    const CFileName GetDefaultHostGroupsPath(void);

    /// return column separated paths for host submodule configurations
    const CFileName GetDefaultHostSubSystemsPath(void);

// users configuration ---------------------------------------------------------
    /// return full file name to the users configurations
    const CFileName GetUsersConfigFile(void);

// section of private data ----------------------------------------------------
private:
    CFileName       AMSRoot;            // ams root directory - read from AMS_ROOT variable
    CXMLDocument    Config;             // global config data

    /// get user global setup dir
    const CFileName GetUserGlobalConfigDir(void);
};

//------------------------------------------------------------------------------

extern AMS_PACKAGE CAMSRegistry AMSRegistry;

//------------------------------------------------------------------------------

#endif
