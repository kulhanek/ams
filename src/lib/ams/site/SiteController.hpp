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
#include <SmallString.hpp>
#include <XMLElement.hpp>

//------------------------------------------------------------------------------

class AMS_PACKAGE CSiteController {
public:
// site methods ----------------------------------------------------------------
    /// lookup for site id from site name
    static const CSmallString GetSiteID(const CSmallString& site_name);

    /// lookup for site name from site id
    static const CSmallString GetSiteName(const CSmallString& site_id);

    /// is site id valid
    static bool IsSiteIDValid(const CSmallString& site_id);

    bool IsActive(void);

    /// can be site activated on this host?
    bool CanBeActivated(void);

// section of private data ----------------------------------------------------
private:
    CSmallString    ActiveSiteName;
    CSmallString    ActiveSiteID;
};

//------------------------------------------------------------------------------

extern CSiteController  SiteController;

//------------------------------------------------------------------------------

#endif
