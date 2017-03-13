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

#include <GlobalConfig.hpp>
#include <XMLParser.hpp>
#include <prefix.h>
#include <ErrorSystem.hpp>
#include <XMLElement.hpp>
#include <Shell.hpp>
#include <Utils.hpp>
#include <string.h>
#include <vector>
#include <FileSystem.hpp>

//------------------------------------------------------------------------------

CGlobalConfig GlobalConfig;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CGlobalConfig::CGlobalConfig(void)
{
    ActiveSiteID       = CShell::GetSystemVariable("AMS_SITE");
    ActiveSiteName     = CUtils::GetSiteName(ActiveSiteID);
    ActiveModules      = CShell::GetSystemVariable("AMS_ACTIVE_MODULES");
    ExportedModules    = CShell::GetSystemVariable("AMS_EXPORTED_MODULES");
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString& CGlobalConfig::GetActiveSiteID(void)
{
    return(ActiveSiteID);
}

//------------------------------------------------------------------------------

const CSmallString CGlobalConfig::GetActiveSiteName(void)
{
    return(ActiveSiteName);
}

//------------------------------------------------------------------------------

void CGlobalConfig::SetActiveSiteID(const CSmallString &site_id)
{
    ActiveSiteID    = site_id;
    ActiveSiteName  = CUtils::GetSiteName(ActiveSiteID);
}

//------------------------------------------------------------------------------

void CGlobalConfig::SetActiveSiteID(const CAmsUUID& site_id)
{
    SetActiveSiteID(site_id.GetFullStringForm());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CFileName CGlobalConfig::GetAMSRootDir(void)
{
    return( CFileName(PREFIX) );
}

//------------------------------------------------------------------------------

const CFileName CGlobalConfig::GetSystemSiteConfigDir(void)
{
    CFileName      config_name;
    CSmallString   ams_site;

    ams_site = GlobalConfig.GetActiveSiteID();
    config_name = CFileName(ETCDIR) / "sites" / ams_site / "aliases.xml";

    return(config_name);
}

//------------------------------------------------------------------------------

const CFileName CGlobalConfig::GetUserSiteConfigDir(void)
{
    return( GetUserConfigDir(GlobalConfig.GetActiveSiteID()) );
}

//------------------------------------------------------------------------------

const CFileName CGlobalConfig::GetUserGlobalConfigDir(void)
{
    return( GetUserConfigDir("") );
}

//------------------------------------------------------------------------------

const CFileName CGlobalConfig::GetUserConfigDir(const CFileName& sub_dir)
{
    CFileName user_config_dir;
    user_config_dir = CShell::GetSystemVariable("AMS_USER_CONFIG_DIR");

    if( user_config_dir == NULL ) {
        user_config_dir = CShell::GetSystemVariable("HOME");
        user_config_dir = user_config_dir / ".infinity" ;
    }

    if( sub_dir != NULL ) user_config_dir = user_config_dir / sub_dir;

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

bool CGlobalConfig::IsModuleActive(const CSmallString& module)
{
    char* p_lvar;
    if( ActiveModules == NULL ) return(false);

    CSmallString name,ver,arch,para;

    CUtils::ParseModuleName(module,name,ver,arch,para);

    CSmallString tmp(ActiveModules);

    // generate list of active modules
    std::vector<CSmallString> active_list;

    char* p_saveptr = NULL;
    p_lvar = strtok_r(tmp.GetBuffer(),"|",&p_saveptr);
    while( p_lvar != NULL ) {
        active_list.push_back(p_lvar);
        p_lvar = strtok_r(NULL,"|",&p_saveptr);
    }

    for(unsigned int i=0; i < active_list.size(); i++) {
        CSmallString mactive = active_list[i];
        CSmallString lname,lver,larch,lpara;
        CUtils::ParseModuleName(mactive,lname,lver,larch,lpara);
        if( lname == name ) {
            if( ver == NULL ) return(true);
            if( lver == ver ) {
                if( arch == NULL ) return(true);
                if( larch == arch ) {
                    if( para == NULL ) return(true);
                    if( lpara == para ) return(true);
                }
            }
        }
    }

    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CGlobalConfig::GetActiveModuleVersion(const CSmallString& module,
        CSmallString& actver)
{
    char* p_lvar;
    actver = NULL;
    if( ActiveModules == NULL ) return(false);

    CSmallString name,ver,arch,para;

    CUtils::ParseModuleName(module,name,ver,arch,para);

    CSmallString tmp(ActiveModules);

    // generate list of active modules
    std::vector<CSmallString> active_list;

    char* p_saveptr = NULL;
    p_lvar = strtok_r(tmp.GetBuffer(),"|",&p_saveptr);
    while( p_lvar != NULL ) {
        active_list.push_back(p_lvar);
        p_lvar = strtok_r(NULL,"|",&p_saveptr);
    }

    for(unsigned int i=0; i < active_list.size(); i++) {
        CSmallString mactive = active_list[i];
        CSmallString lname,lver,larch,lpara;
        CUtils::ParseModuleName(mactive,lname,lver,larch,lpara);
        if( lname == name ) {
            if( ver == NULL ) {
                actver = lver;
                return(true);
            }
            if( lver == ver ) {
                if( arch == NULL ) {
                    actver = lver;
                    return(true);
                }
                if( larch == arch ) {
                    if( para == NULL ) {
                        actver = lver;
                        return(true);
                    }
                    if( lpara == para ) {
                        actver = lver;
                        return(true);
                    }
                }
            }
        }
    }

    return(false);
}

//------------------------------------------------------------------------------

const CSmallString& CGlobalConfig::GetActiveModules(void)
{
    return(ActiveModules);
}

//------------------------------------------------------------------------------

const CSmallString& CGlobalConfig::GetExportedModules(void)
{
    return(ExportedModules);
}

//------------------------------------------------------------------------------

const CSmallString CGlobalConfig::GetActiveModuleSpecification(
    const CSmallString& name)
{
    if( ActiveModules == NULL ) return("");

    char* p_lvar;

    CSmallString tmp(ActiveModules);

    char* p_saveptr = NULL;
    p_lvar = strtok_r(tmp.GetBuffer(),"|",&p_saveptr);
    while( p_lvar != NULL ) {
        if( CUtils::GetModuleName(p_lvar) == name ) {
            return(p_lvar);
        }
        p_lvar = strtok_r(NULL,"|",&p_saveptr);
    }

    return("");
}

//------------------------------------------------------------------------------

const CSmallString CGlobalConfig::GetExportedModuleSpecification(
    const CSmallString& name)
{
    if( ExportedModules == NULL ) return("");

    char* p_lvar;

    CSmallString tmp(ExportedModules);

    char* p_saveptr = NULL;
    p_lvar = strtok_r(tmp.GetBuffer(),"|",&p_saveptr);
    while( p_lvar != NULL ) {
        if( CUtils::GetModuleName(p_lvar) == name ) {
            return(p_lvar);
        }
        p_lvar = strtok_r(NULL,"|",&p_saveptr);
    }

    return("");
}

//-----------------------------------------------------------------------------

void CGlobalConfig::UpdateActiveModules(const CSmallString& module,
        bool add_module)
{
    ActiveModules = CShell::RemoveValue(ActiveModules,module,"|");
    if( add_module) ActiveModules = CShell::AppendValue(ActiveModules,module,"|");
}

//-----------------------------------------------------------------------------

void CGlobalConfig::UpdateExportedModules(const CSmallString& module,
        bool add_module)
{
    ExportedModules = CShell::RemoveValue(ExportedModules,module,"|");
    if( add_module) ExportedModules = CShell::AppendValue(ExportedModules,module,"|");
}

//-----------------------------------------------------------------------------

void CGlobalConfig::SetExportedModules(const CSmallString& modules)
{
    ExportedModules = modules;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

