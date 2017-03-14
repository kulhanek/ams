#ifndef SoftConfigH
#define SoftConfigH
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

#include <AMSMainHeader.hpp>
#include <XMLDocument.hpp>

//------------------------------------------------------------------------------

#define DEFAULT_UMASK "0077"

//------------------------------------------------------------------------------

class AMS_PACKAGE CSoftConfig {
public:
// constructor and destructor -------------------------------------------------
    CSoftConfig(void);

// input/output methods -------------------------------------------------------
    /// load user config
    void LoadUserConfig(void);

    /// save user config
    bool SaveUserConfig(void);

    /// clear user auto-restored modules config
    void ClearAutorestoredConfig(void);

    /// does user want system autoloaded modules?
    bool AreSystemAutoloadedModulesDisabled(void);

// information methods -------------------------------------------------------
    /// get user autoloaded modules
    CXMLElement* GetAutoloadedModules(void);

    /// print the list of autorestored modules
    bool PrintAutorestoredModules(FILE* fout=NULL);

    /// add module to the list of user auto-restored modules
    void AddAutorestoredModule(const CSmallString& module);

    /// remove module from the list of user auto-restored modules
    bool RemoveAutorestoredModule(const CSmallString& module);

    /// is module in the list of user auto-restored modules
    bool IsAutorestoredModule(const CSmallString& module);

    /// get user primary group
    const CSmallString GetUserGroup(void);

    /// set user primary group
    void SetUserGroup(const CSmallString& group);

    /// get user umask
    const CSmallString GetUserUMask(void);

    /// set user umask
    void SetUserUMask(const CSmallString& umask);

    /// get site priorities
    const CSmallString GetSitePriorities(void);

    /// set site priorities
    void SetSitePriorities(const CSmallString& pri);

    /// get default module priorities
    const CSmallString GetDefaultModulePriorities(void);

    /// get module priorities
    const CSmallString GetModulePriorities(void);

    /// set module priorities
    void SetModulePriorities(const CSmallString& pri);

    /// is available site
    bool IsAvailableSite(const CSmallString& name);

    // section of private data ----------------------------------------------------
private:
    // FIX ME - shall we allow setup per site?
    // CXMLDocument    SiteConfig;     // user site config
    CXMLDocument    CommonConfig;   // user global config
};

//------------------------------------------------------------------------------

extern CSoftConfig SoftConfig;

//------------------------------------------------------------------------------

#endif
