#ifndef SiteControllerH
#define SiteControllerH
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
#include <FileName.hpp>
#include <list>

//------------------------------------------------------------------------------

class AMS_PACKAGE CSiteController {
public:
// constructor and destructor --------------------------------------------------
    CSiteController(void);
    ~CSiteController(void);

// setup methods ---------------------------------------------------------------
    /// init site controller configuration
    void InitSiteControllerConfig(void);

// site methods ----------------------------------------------------------------
    /// get active site
    const CSmallString& GetActiveSite(void) const;

    /// get active site
    void SetActiveSite(const CSmallString& name);

    /// get site config
    const CFileName GetSiteConfig(const CSmallString& name);

    /// list available sites
    void GetAvailableSites(std::list<CSmallString>& list,bool plain=false);

    /// list all sites
    void GetAllSites(std::list<CSmallString>& list);

    /// get site name from ssh wrapper
    static const CSmallString  GetSSHSite(void);

    /// is TTY available for stdin?
    static bool  HasTTY(void);

    /// is a batch job?
    static bool  IsBatchJob(void);

    /// was site info already printed?
    static bool IsSiteInfoPrinted(void);

    /// set taht site infor was already shown to users
    static void SetSiteInfoPrinted(void);

// section of private data ----------------------------------------------------
private:
    CSmallString    ActiveSite;
};

//------------------------------------------------------------------------------

extern CSiteController  SiteController;

//------------------------------------------------------------------------------

#endif
