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

#include <Utils.hpp>
#include <string.h>
#include <Site.hpp>
#include <DirectoryEnum.hpp>
#include <FileName.hpp>
#include <FileSystem.hpp>
#include <AmsUUID.hpp>
#include <errno.h>
#include <ErrorSystem.hpp>
#include <XMLIterator.hpp>
#include <AMSRegistry.hpp>
#include <iomanip>
#include <list>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <XMLElement.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

ActiveSiteID       = CShell::GetSystemVariable("AMS_SITE");
ActiveSiteName     = CUtils::GetSiteName(ActiveSiteID);

const CSmallString& CAMSRegistry::GetActiveSiteID(void)
{
    return(ActiveSiteID);
}

//------------------------------------------------------------------------------

const CSmallString CAMSRegistry::GetActiveSiteName(void)
{
    return(ActiveSiteName);
}

//------------------------------------------------------------------------------

void CAMSRegistry::SetActiveSiteID(const CSmallString &site_id)
{
    ActiveSiteID    = site_id;
    ActiveSiteName  = CUtils::GetSiteName(ActiveSiteID);
}

//------------------------------------------------------------------------------

void CAMSRegistry::SetActiveSiteID(const CAmsUUID& site_id)
{
    SetActiveSiteID(site_id.GetFullStringForm());
}

const CSmallString CSiteController::GetSiteID(const CSmallString& site_name)
{
    // go through the list of all available sites -------------
    CDirectoryEnum dir_enum(AMSRegistry.GetETCDIR() / "sites");

    dir_enum.StartFindFile("*");
    CFileName site_sid;
    while( dir_enum.FindFile(site_sid) ) {
        CAmsUUID    site_id;
        CSite       site;
        if( site_id.LoadFromString(site_sid) == false ) continue;
        if( site.LoadConfig(site_sid) == false ) continue;

        // site was found - return site id
        if( site.GetName() == site_name ) return(site.GetID());
    }
    dir_enum.EndFindFile();

    // site was not found or it is not correctly configured
    return("");
}

//------------------------------------------------------------------------------

const CSmallString CSiteController::GetSiteName(const CSmallString& site_sid)
{
    if( site_sid == NULL ) return("");
    CSite   site;
    if( site.LoadConfig(site_sid) == false ){
        return("");
    }
    return(site.GetName());
}

//------------------------------------------------------------------------------

bool CSiteController::IsSiteIDValid(const CSmallString& site_id)
{
    CFileName config_path = AMSRegistry.GetETCDIR();
    config_path = config_path / "sites" / site_id / "site.xml";

    if( CFileSystem::IsFile(config_path) == false ) return(false);

    CSite   site;
    return(site.LoadConfig(site_id));
}


//------------------------------------------------------------------------------

bool CSite::IsActive(void)
{
    // get active site id
    CSmallString active_site = AMSRegistry.GetActiveSiteID();

    // compare with site id
    return((active_site == GetID()) && (active_site.GetLength() > 0));
}



//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

