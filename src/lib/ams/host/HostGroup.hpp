#ifndef HostGroupH
#define HostGroupH
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

#include <AMSMainHeader.hpp>
#include <FileName.hpp>
#include <SmallString.hpp>
#include <XMLDocument.hpp>
#include <VerboseStr.hpp>

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

class AMS_PACKAGE CHostGroup {
public:
// constructor and destructors -------------------------------------------------
    CHostGroup(void);

// setup methods ---------------------------------------------------------------
    /// init hosts global configuration
    void InitHostsConfig(void);

    /// init host group
    void InitHostGroup(void);

    /// init all host group
    void InitAllHostGroups(void);

// information methods - global configuration ----------------------------------
    /// return default host cache key
    const CSmallString GetDefaultHostCacheKey(void);

    /// return comma separated names of host submodule configurations
    const CFileName GetDefaultHostSubSystems(void);

// information methods - active host group -------------------------------------
    /// get file name with the host group definition
    const CFileName GetHostGroupFile(void) const;

    /// get name of the host group
    const CFileName GetHostGroupName(void);

    /// print host group info
    void PrintHostGroupInfo(CVerboseStr& vout);

    /// get host subsystems modules
    const CFileName GetHostSubSystems(void);

    /// get default site
    const CSmallString GetDefaultSite(void);

    /// find host group namespace
    const CSmallString GetGroupNS(void);

    /// find host realm
    const CSmallString GetRealm(void);

// information methods for all host groups -------------------------------------

    /// find host group namespace
    const CSmallString GetGroupNS(const CSmallString& hostname);

    /// find host realm
    const CSmallString GetRealm(const CSmallString& hostname);

// section of private date -----------------------------------------------------
private:
    CFileName    HostsConfigFile;
    CXMLDocument HostsConfig;

    CFileName    HostGroupFile;
    CXMLDocument HostGroup;

    CXMLDocument AllHostGroups;

    /// find group in all host groups
    CXMLElement* FindGroup(const CSmallString& hostname);
};

// -----------------------------------------------------------------------------

extern CHostGroup   HostGroup;

//------------------------------------------------------------------------------

#endif