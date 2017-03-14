#ifndef GlobalConfigH
#define GlobalConfigH
// =============================================================================
//  AMS - Advanced Module System
// -----------------------------------------------------------------------------
//     Copyright (C) 2012 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include <AMSMainHeader.hpp>
#include <XMLDocument.hpp>
#include <FileName.hpp>
#include <AmsUUID.hpp>

//------------------------------------------------------------------------------

class AMS_PACKAGE CGlobalConfig {
public:
    // constructor and destructor -------------------------------------------------
    CGlobalConfig(void);

// information about active site ----------------------------------------------
    /// get ID of active site
    const CSmallString& GetActiveSiteID(void);

    /// get name of active site
    const CSmallString GetActiveSiteName(void);

    /// set active site ID
    void SetActiveSiteID(const CSmallString& site_id);

    /// set active site ID
    void SetActiveSiteID(const CAmsUUID& site_id);

// user config -----------------------------------------------------------------
    /// return infinity root directory
    const CFileName GetAMSRootDir(void);

// user config -----------------------------------------------------------------
    /// return the name of user config directory for active site
    const CFileName GetUserSiteConfigDir(void);

    /// return the name of user global config directory
    const CFileName GetUserGlobalConfigDir(void);

    /// return the name of any user config directory
    const CFileName GetUserConfigDir(const CFileName& sub_dir);

// information about modules ---------------------------------------------------
    /// check if module is active
    bool IsModuleActive(const CSmallString& module);

    /// get version of active module
    bool GetActiveModuleVersion(const CSmallString& module,CSmallString& actver);

    /// return complete specification of active modules
    const CSmallString& GetActiveModules(void);

    /// return export specification of exported modules
    const CSmallString& GetExportedModules(void);

    /// return complete specification of active module
    const CSmallString GetActiveModuleSpecification(const CSmallString& name);

    /// return export specification of active module
    const CSmallString GetExportedModuleSpecification(const CSmallString& name);

// update module info ---------------------------------------------------------
    /// update list of active modules
    void UpdateActiveModules(const CSmallString& module,bool add_module);

    /// update list of exported modules
    void UpdateExportedModules(const CSmallString& module,bool add_module);

    /// set list of exported modules
    void SetExportedModules(const CSmallString& modules);

// information about architectures --------------------------------------------


// section of private data ----------------------------------------------------
private:
    // this values are initialized in constructor and can be updated anytime
    // by CSite during site activation/deactivation
    CSmallString    ActiveSiteID;
    CSmallString    ActiveSiteName;
    CSmallString    ActiveModules;      // list of active modules
    CSmallString    ExportedModules;    // list of exported modules

    CXMLDocument    Config;             // global config data
};

//------------------------------------------------------------------------------

extern CGlobalConfig GlobalConfig;

//------------------------------------------------------------------------------

#endif
