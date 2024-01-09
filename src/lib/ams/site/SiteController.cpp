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

#include <SiteController.hpp>
#include <Shell.hpp>
#include <FileName.hpp>
#include <AMSRegistry.hpp>
#include <Utils.hpp>
#include <HostGroup.hpp>
#include <list>
#include <unistd.h>
#include <ShellProcessor.hpp>
#include <FileSystem.hpp>
#include <UserUtils.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;

//------------------------------------------------------------------------------

CSiteController  SiteController;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CSiteController::CSiteController(void)
{
}

//------------------------------------------------------------------------------

CSiteController::~CSiteController(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CSiteController::InitSiteControllerConfig(void)
{
    ActiveSite = CShell::GetSystemVariable("AMS_SITE");
}

//------------------------------------------------------------------------------

const CFileName CSiteController::GetSiteConfig(const CSmallString& name)
{
    CFileName search_path = AMSRegistry.GetSiteSearchPaths();
    CFileName site_config;
    CUtils::FindFile(search_path,name,".xml",site_config);
    return(site_config);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString& CSiteController::GetActiveSite(void) const
{
    return(ActiveSite);
}

//------------------------------------------------------------------------------

void CSiteController::SetActiveSite(const CSmallString& name)
{
    ActiveSite = name;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CSiteController::GetAvailableSites(std::list<CSmallString>& list,bool plain)
{
    CFileName search_path = AMSRegistry.GetSiteSearchPaths();
    std::list<CFileName> all_site_files;
    CUtils::FindAllFiles(search_path,"*.xml",all_site_files);

    std::list<CFileName> all_sites;
    for(CFileName site_file : all_site_files){
        CFileName site_name = site_file.GetFileNameWithoutExt();
        all_sites.push_back(site_name);
    }
    all_sites.sort();
    all_sites.unique();

    std::set<CSmallString> allowed_sites;
    HostGroup.GetAllowedSites(allowed_sites);

    for(CFileName site : all_sites){
        if( allowed_sites.count(site) == 0 ) continue;
        if( plain == true ){
            list.push_back(site);
        } else {
            if( GetActiveSite() == site ){
                CSmallString act_site = "[" + site + "]";
                list.push_back(act_site);
            } else {
                list.push_back(site);
            }
        }
    }
}

//------------------------------------------------------------------------------

void CSiteController::GetAllSites(std::list<CSmallString>& list)
{
    CFileName search_path = AMSRegistry.GetSiteSearchPaths();
    std::list<CFileName> all_site_files;
    CUtils::FindAllFiles(search_path,"*.xml",all_site_files);

    std::list<CFileName> all_sites;
    for(CFileName site_file : all_site_files){
        CFileName site_name = site_file.GetFileNameWithoutExt();
        all_sites.push_back(site_name);
    }
    all_sites.sort();
    all_sites.unique();

    std::set<CSmallString> allowed_sites;
    HostGroup.GetAllowedSites(allowed_sites);

    for(CFileName site : all_sites){
        if( allowed_sites.count(site) == 0 ){
            CSmallString other_site = "^" + site;
            list.push_back(other_site);
        } else {
            if( GetActiveSite() == site ){
                CSmallString act_site = "[" + site + "]";
                list.push_back(act_site);
            } else {
                list.push_back(site);
            }
        }
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CSiteController::GetSSHSite(void)
{
    CSmallString ssh_site;
    ssh_site = CShell::GetSystemVariable("LC_AMS_SSH_SITE");
    return(ssh_site);
}

//------------------------------------------------------------------------------

void CSiteController::GetSSHExportedModules(std::list<CSmallString>& modules)
{
    std::string ssh_emods;
    ssh_emods = CShell::GetSystemVariable("LC_AMS_SSH_EXPORTED_MODULES");
    if( ! ssh_emods.empty() ){
        split(modules,ssh_emods,is_any_of("|"));
    }

}

//------------------------------------------------------------------------------

const CFileName CSiteController::GetSSH_PWD(void)
{
    CFileName ssh_pwd;
    ssh_pwd = CShell::GetSystemVariable("LC_AMS_SSH_PWD");
    if( CFileSystem::IsDirectory(ssh_pwd) == false ){
        return(NULL);
    }
    CFileName home_dir = CFileName("/home") / CUserUtils::GetUserName();
    if( home_dir == ssh_pwd ){
        // do not restore home directory, which can be on the different location at remote site
        return(NULL);
    }
    return(ssh_pwd);
}

//------------------------------------------------------------------------------

void CSiteController::UnsetSSHVariables(void)
{
    ShellProcessor.UnsetVariable("LC_AMS_SSH_SITE");
    ShellProcessor.UnsetVariable("LC_AMS_SSH_EXPORTED_MODULES");
    ShellProcessor.UnsetVariable("LC_AMS_SSH_PWD");
}

//------------------------------------------------------------------------------

bool CSiteController::HasTTY(void)
{
    int file_id = fileno(stdin);
    if( isatty(file_id) != 0 ) return(true);
    return(false);
}

//------------------------------------------------------------------------------

bool CSiteController::IsBatchJob(void)
{
    CSmallString pbs_jobid;
    pbs_jobid = CShell::GetSystemVariable("PBS_JOBID");
    return( pbs_jobid != NULL );
}

//------------------------------------------------------------------------------

bool CSiteController::IsSiteInfoPrinted(void)
{
    CSmallString site_info_printed;
    site_info_printed = CShell::GetSystemVariable("AMS_SITE_INFO_PRINTED");
    return( site_info_printed == "y" );
}

//------------------------------------------------------------------------------

void CSiteController::SetSiteInfoPrinted(void)
{
    CShell::SetSystemVariable("AMS_SITE_INFO_PRINTED","y");
    ShellProcessor.SetVariable("AMS_SITE_INFO_PRINTED","y");
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

