// =============================================================================
// AMS
// -----------------------------------------------------------------------------
//    Copyright (C) 2023      Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include "SiteCmd.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <AMSRegistry.hpp>
#include <HostGroup.hpp>
#include <Host.hpp>
#include <User.hpp>
#include <Site.hpp>
#include <SiteController.hpp>
#include <Utils.hpp>
#include <Shell.hpp>
#include <ShellProcessor.hpp>
#include <PrintEngine.hpp>
#include <FileName.hpp>
#include <Module.hpp>
#include <ModuleController.hpp>
#include <UserUtils.hpp>

//------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------

#define SITE_STATUS_OK                  0
#define SITE_ERROR_CONFIG_PROBLEM       1
#define SITE_ERROR_SITE_NOT_FOUND       2
#define SITE_ERROR_NOT_ALLOWED          3

//------------------------------------------------------------------------------

MAIN_ENTRY(CSiteCmd)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CSiteCmd::Init(int argc, char* argv[])
{
    ForcePrintErrors = false;
    ExitCode = 0;

// init site controller - MUST BE HERE to get active site name
    SiteController.InitSiteControllerConfig();

    // encode program options, all check procedures are done inside of Options
    int result = Options.ParseCmdLine(argc,argv);

    // should we exit or was it error?
    if( result != SO_CONTINUE ) return(result);

    // output must be directed to stderr
    // stdout is used for shell processor
    Console.Attach(stderr);

    // attach verbose stream to terminal stream and set desired verbosity level
    vout.Attach(Console);
    if( Options.GetOptVerbose() ) {
        vout.Verbosity(CVerboseStr::high);
    } else {
        vout.Verbosity(CVerboseStr::low);
    }

    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << endl;
    vout << "# ==============================================================================" << endl;
    vout << "# site (AMS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    vout << low;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CSiteCmd::Run(void)
{
    // set module flags
    Module.SetFlags(MFB_SYSTEM);
    if( CShell::GetSystemVariable("_INFINITY_JOB_") == "_INFINITY_JOB_" ) {
        Module.SetFlags(MFB_INFINITY);
    }

// ignore all if LC_SSH_AMS_IGNORE_SITE_INIT is set
    if( SiteController.IsSiteInitIgnored() && (Options.GetOptForce() == false) ){
        vout << endl;
        vout << ">>> INFO: site init is ignored (LC_SSH_AMS_IGNORE_SITE_INIT)" << endl;
        return(true);
    }

// init AMS registry
    AMSRegistry.LoadRegistry();

// init host group
    HostGroup.InitHostsConfig();
    HostGroup.InitHostGroup();

// init host
    Host.InitHostSubSystems(HostGroup.GetHostSubSystems());
    Host.InitHost();

// init user
    User.InitUserConfig();
    User.InitUser();

//// init site controller - MOVED TO Init()
//    SiteController.InitSiteControllerConfig();

// init module controller
    ModuleController.InitModuleControllerConfig();

// ----------------------------------------------
    if( Options.GetArgAction() == "activate" ) {
        int error = ActivateSite();

        if( error > 0 ) {
            vout << low;
            vout << endl;
            vout << "<red>>>> ERROR:</red> Unable to activate the <u>" << Options.GetArgSite() << "</u> site." << endl;
        }

        switch(error){
            case SITE_ERROR_CONFIG_PROBLEM:
                vout << "           Configuration problems." << endl;
                ForcePrintErrors = true;
                ExitCode = 1;
                return(false);
            case SITE_ERROR_SITE_NOT_FOUND:
                vout << "           The site is not defined in the AMS database (mispelled name?)." << endl;
                ExitCode = 1;
                return(false);
            case SITE_ERROR_NOT_ALLOWED:
                vout << "           The site is not allowed on the '<u>" << Host.GetHostName() << "</u>' host." << endl;
                ExitCode = 1;
                return(false);
        }
        return(true);
    }
// ----------------------------------------------
    else if( Options.GetArgAction() == "avail" ) {
        AvailSites();
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "info" ) {

        if( Options.GetArgSite() == NULL ){
            vout << low;
            vout << endl;
            vout << "<red>>>> ERROR:</red> No site is provided / active!" << endl;
            ExitCode = 1;
            return(false);
        }

        int error = InfoSite();

        if( error > 0 ) {
            vout << low;
            vout << endl;
            vout << "<red>>>> ERROR:</red> Unable to provide information about the site <u>" << Options.GetArgSite() << "</u>." << endl;
        }

        switch(error){
            case SITE_ERROR_CONFIG_PROBLEM:
                vout << "           Configuration problems." << endl;
                ForcePrintErrors = true;
                ExitCode = 1;
                return(false);
            case SITE_ERROR_SITE_NOT_FOUND:
                vout << "           The site is not defined in the AMS database (mispelled name?)." << endl;
                ExitCode = 1;
                return(false);
        }
        return(true);
    }
// ----------------------------------------------
    else if( Options.GetArgAction() == "disp" ) {

        if( Options.GetArgSite() == NULL ){
            vout << low;
            vout << endl;
            vout << "<red>>>> ERROR:</red> No site is provided / active!" << endl;
            ExitCode = 1;
            return(false);
        }

        int error = DispSite();

        if( error > 0 ) {
            vout << low;
            vout << endl;
            vout << "<red>>>> ERROR:</red> Unable to display the site <u>" << Options.GetArgSite() << "</u> setup." << endl;
        }

        switch(error){
            case SITE_ERROR_CONFIG_PROBLEM:
                vout << "           Configuration problems." << endl;
                ForcePrintErrors = true;
                ExitCode = 1;
                return(false);
            case SITE_ERROR_SITE_NOT_FOUND:
                vout << "           The site is not defined in the AMS database (mispelled name?)." << endl;
                ExitCode = 1;
                return(false);
        }
        return(true);
    }
// ----------------------------------------------
    else if( Options.GetArgAction() == "listamods" ) {

        if( Options.GetArgSite() == NULL ){
            vout << low;
            vout << endl;
            vout << "<red>>>> ERROR:</red> No site is provided / active!" << endl;
            ExitCode = 1;
            return(false);
        }

        int error = ListAMods();

        if( error > 0 ) {
            vout << low;
            vout << endl;
            vout << "<red>>>> ERROR:</red> Unable to list autoloaded modules for the site <u>" << Options.GetArgSite() << "</u>." << endl;
        }

        switch(error){
            case SITE_ERROR_CONFIG_PROBLEM:
                vout << "           Configuration problems." << endl;
                ForcePrintErrors = true;
                ExitCode = 1;
                return(false);
            case SITE_ERROR_SITE_NOT_FOUND:
                vout << "           The site is not defined in the AMS database (mispelled name?)." << endl;
                ExitCode = 1;
                return(false);
        }
        return(true);

        return(true);
    }   
// ----------------------------------------------
    else if( Options.GetArgAction() == "init" ) {

        int error = InitSite();

        if( error > 0 ) {
            vout << low;
            vout << endl;
            vout << "<red>>>> ERROR:</red> Unable to init the site <u>" << Options.GetArgSite() << "</u> setup." << endl;
        }

        switch(error){
            case SITE_ERROR_CONFIG_PROBLEM:
                vout << "           Configuration problems." << endl;
                ForcePrintErrors = true;
                ExitCode = 1;
                return(false);
            case SITE_ERROR_SITE_NOT_FOUND:
                vout << "           The site is not defined in the AMS database (mispelled name?)." << endl;
                ExitCode = 1;
                return(false);
        }
        return(true);
    }
// ----------------------------------------------
    else {
        ES_ERROR("not implemented");
        ExitCode = 1;
        return(false);
    }
}

//------------------------------------------------------------------------------

void CSiteCmd::Finalize(void)
{
    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << endl;
    vout << "# ==============================================================================" << endl;
    vout << "# site (AMS utility) terminated at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    if( ErrorSystem.IsError() || (ErrorSystem.IsAnyRecord() && Options.GetOptVerbose()) ){
        if( ForcePrintErrors ) vout << low;
        ErrorSystem.PrintErrors(stderr);
        if( ForcePrintErrors ) vout << endl;
    }

    if( (Options.GetArgAction() == "activate") ||
        (Options.GetArgAction() == "avail") ||
        (Options.GetArgAction() == "disp") ||
        (Options.GetArgAction() == "info") ) {
        vout << low;
        if( ! ForcePrintErrors ) vout << endl;
    }

    if( ErrorSystem.IsError() ) {
        ExitCode = 1;
        ShellProcessor.RollBack();
    }

    ShellProcessor.SetExitCode(ExitCode);
    ShellProcessor.BuildEnvironment();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CSiteCmd::ActivateSite(void)
{
    // special case - ignore
    if( (Options.GetArgSite() == "none") || (Options.GetArgSite() == NULL) ) return(SITE_STATUS_OK);

    ModuleController.LoadBundles(EMBC_SMALL);
    ModuleController.MergeBundles();

    vout << high;
    ModuleController.PrintBundlesInfo(vout);

    Module.SetFlags(Module.GetFlags() | MFB_AUTOLOADED);

    CFileName active_site = SiteController.GetActiveSite();

    // deactivate current site
    if( active_site != NULL ) {
        DeactivateSite(active_site);
    }

    active_site = Options.GetArgSite();

    CFileName site_config = SiteController.GetSiteConfig(active_site);
    if( site_config == NULL ){
        CSmallString error;
        error << "specified site '" << active_site << "' was not found (activate)";
        ES_TRACE_ERROR(error);
        return(SITE_ERROR_CONFIG_PROBLEM);
    }

    CSite site;
    if( site.LoadConfig(site_config) == false ){
        CSmallString error;
        error << "unable to load site configuration from '" << site_config << "' (activate)";
        ES_TRACE_ERROR(error);
        return(SITE_ERROR_CONFIG_PROBLEM);
    }

    if( HostGroup.IsSiteAllowed(site.GetName()) == false ) {
        return(SITE_ERROR_NOT_ALLOWED);
    }

    if( site.ActivateSite() == false ){
        CSmallString error;
        error << "unable to activate site environment '" << site.GetName() << "' (activate)";
        ES_TRACE_ERROR(error);
        return(SITE_ERROR_CONFIG_PROBLEM);
    }

    vout << low;
    site.PrintShortSiteInfo(vout);

    std::list<CSmallString> modules;
    HostGroup.GetHostsConfigAutoLoadedModules(modules);
    HostGroup.GetHostGroupAutoLoadedModules(modules);
    site.GetAutoLoadedModules(modules);
    AMSRegistry.GetUserAutoLoadedModules(modules);

    vout << high;
    bool result = true;
    for( CSmallString module : modules ){
        // ignore errors from autoloaded modules
        Module.AddModule(vout,module,false,true);
    }

    vout << low;
    if( ErrorSystem.IsError() || (result == false) ){
        return(SITE_ERROR_CONFIG_PROBLEM);
    }

    return(SITE_STATUS_OK);
}

//------------------------------------------------------------------------------

bool CSiteCmd::DeactivateSite(const CSmallString& site_name)
{
    bool result = true;
    CFileName site_config = SiteController.GetSiteConfig(site_name);
    if( site_config == NULL ){
        CSmallString error;
        error << "specified site '" << site_name << "' was not found (deactivate)";
        ES_TRACE_ERROR(error);
        // ignore error
        result = false;
    }

    CSite site;

    if( result ){
        if( site.LoadConfig(site_config) == false ){
            CSmallString error;
            error << "unable to load site configuration from '" << site_config << "' (deactivate)";
            ES_TRACE_ERROR(error);
            // ignore error
            result = false;
        }
    }

    if( result ){
        // purge all modules
        if( ModuleController.PurgeModules(vout) == false ){
            CSmallString error;
            error << "current active site '" << site_name << "' cannot be deactivated";
            ES_TRACE_ERROR(error);
            // ignore error
            result = false;
        }
    }

    if( result ){
        // destroy environment
        if( site.DeactivateSite() == false ){
            CSmallString error;
            error << "current active site '" << site_name << "' cannot be deactivated";
            ES_TRACE_ERROR(error);
            // ignore error
            result = false;
        }
    }

    return(result);
}

//------------------------------------------------------------------------------

void CSiteCmd::AvailSites(void)
{
    PrintEngine.InitPrintProfile();

    std::list<CSmallString> sites;
    if( Options.GetOptAll() ){
        SiteController.GetAllSites(sites);
        PrintEngine.PrintHeader(Console.GetTerminal(),"ALL SITES",EPEHS_SITE);
    } else {
        SiteController.GetAvailableSites(sites);
        PrintEngine.PrintHeader(Console.GetTerminal(),"AVAILABLE SITES",EPEHS_SITE);
    }
    Console.GetTerminal().Printf("\n");
    PrintEngine.PrintItems(Console.GetTerminal(),sites);
}

//------------------------------------------------------------------------------

int CSiteCmd::InfoSite(void)
{
    CFileName site_config = SiteController.GetSiteConfig(Options.GetArgSite());
    if( site_config == NULL ){
        CSmallString error;
        error << "specified site '" << Options.GetArgSite() << "' was not found";
        ES_TRACE_ERROR(error);
        return(SITE_ERROR_SITE_NOT_FOUND);
    }

    CSite site;
    if( site.LoadConfig(site_config) == false ){
        CSmallString error;
        error << "unable to load site configuration from '" << site_config << "'";
        ES_TRACE_ERROR(error);
        return(SITE_ERROR_CONFIG_PROBLEM);
    }

    site.PrintShortSiteInfo(vout);

    return(SITE_STATUS_OK);
}

//------------------------------------------------------------------------------

int CSiteCmd::DispSite(void)
{
    CFileName site_config = SiteController.GetSiteConfig(Options.GetArgSite());
    if( site_config == NULL ){
        CSmallString error;
        error << "specified site '" << Options.GetArgSite() << "' was not found";
        ES_TRACE_ERROR(error);
        return(SITE_ERROR_SITE_NOT_FOUND);
    }

    CSite site;
    if( site.LoadConfig(site_config) == false ){
        CSmallString error;
        error << "unable to load site configuration from '" << site_config << "'";
        ES_TRACE_ERROR(error);
        return(SITE_ERROR_CONFIG_PROBLEM);
    }

    ModuleController.LoadBundles(EMBC_SMALL);
    ModuleController.MergeBundles();

    site.PrintFullSiteInfo(vout);

    return(SITE_STATUS_OK);
}

//------------------------------------------------------------------------------

int CSiteCmd::ListAMods(void)
{
    CFileName site_config = SiteController.GetSiteConfig(Options.GetArgSite());
    if( site_config == NULL ){
        CSmallString error;
        error << "specified site '" << Options.GetArgSite() << "' was not found";
        ES_TRACE_ERROR(error);
        return(SITE_ERROR_SITE_NOT_FOUND);
    }

    CSite site;
    if( site.LoadConfig(site_config) == false ){
        CSmallString error;
        error << "unable to load site configuration from '" << site_config << "'";
        ES_TRACE_ERROR(error);
        return(SITE_ERROR_CONFIG_PROBLEM);
    }

    ModuleController.LoadBundles(EMBC_SMALL);
    ModuleController.MergeBundles();

    std::list<CSmallString> modules;
    site.GetAutoLoadedModules(modules);

    PrintEngine.InitPrintProfile();
    PrintEngine.PrintHeader(Console.GetTerminal(),"AUTOLOADED MODULES (Infinity Software Base | amsmodule)",EPEHS_SECTION);
    PrintEngine.PrintItems(Console.GetTerminal(),modules);

    return(SITE_STATUS_OK);
}

//------------------------------------------------------------------------------

const CSmallString none_if_empty(const CSmallString str)
{
    if( str == NULL ){
        return("-none-");
    } else {
        return(str);
    }
}

//------------------------------------------------------------------------------

int CSiteCmd::InitSite(void)
{
// general info about setup
    vout << high;
    vout << endl;
    vout << "# Active site    : " << none_if_empty(SiteController.GetActiveSite()) << endl;
    vout << "# SSH site       : " << none_if_empty(SiteController.GetSSHSite()) << endl;
    vout << "# Default site   : " << none_if_empty(HostGroup.GetDefaultSite()) << endl;
    vout << "# Is Batch Job   : " << bool_to_str(SiteController.IsBatchJob()) << endl;
    vout << "# Batch Job Site : " << none_if_empty(SiteController.GetBatchJobSite()) << endl;
    vout << "# Has TTY        : " << bool_to_str(SiteController.HasTTY()) << endl;

    if( SiteController.IsSiteInitIgnored() && (Options.GetOptForce() == false) ){
        vout << endl;
        vout << ">>> INFO: site init is ignored (LC_SSH_AMS_IGNORE_SITE_INIT)" << endl;
        return(SITE_STATUS_OK);
    }

    if( SiteController.IsBatchJob() && (SiteController.GetActiveSite() == NULL) &&
        (Options.GetOptForce() == false) ){
        vout << endl;
        vout << ">>> INFO: this is is a batch job which is not initialized yet" << endl;
        return(SITE_STATUS_OK);
    }

// initialize module subsystems
    vout << high;
    ModuleController.LoadBundles(EMBC_SMALL);
    ModuleController.MergeBundles();
    ModuleController.PrintBundlesInfo(vout);

    CSite   site;
    bool    reactivate = false;

// activate site if not active
    CSmallString site_name = SiteController.GetActiveSite();
    if( (site_name == NULL) || (Options.GetOptForce() == true) ){
        // in force mode we will deactivate the site
        if( site_name != NULL ){
            DeactivateSite(site_name);
        }
        // activate the site
        vout << endl;
        site_name = SiteController.GetBatchJobSite();
        if( site_name == NULL ){
            site_name = SiteController.GetSSHSite();
            if(  site_name != NULL ){
                if( ! HostGroup.IsSiteAllowed(site_name) ){
                    site_name = HostGroup.GetDefaultSite();
                    vout << ">>> the SSH site is not alloved on this host group, switching to the default site" << endl;
                } else {
                    vout << ">>> the SSH site is accepted: " << site_name << endl;
                }
            } else {
                site_name = HostGroup.GetDefaultSite();
                vout << ">>> the default site is accepted: " << site_name << endl;
            }
        } else {
            vout << ">>> the batch job site is accepted: " << site_name << endl;
        }
        if( site_name == NULL ){
            CSmallString error;
            error << "no site can be activated";
            ES_TRACE_ERROR(error);
            return(SITE_ERROR_CONFIG_PROBLEM);
        }

        CFileName site_config = SiteController.GetSiteConfig(site_name);
        if( site_config == NULL ){
            CSmallString error;
            error << "specified site '" << site_name << "' was not found (init)";
            ES_TRACE_ERROR(error);
            return(SITE_ERROR_CONFIG_PROBLEM);
        }

        if( site.LoadConfig(site_config) == false ){
            CSmallString error;
            error << "unable to load site configuration from '" << site_config << "' (init)";
            ES_TRACE_ERROR(error);
            return(SITE_ERROR_CONFIG_PROBLEM);
        }

        if( HostGroup.IsSiteAllowed(site.GetName()) == false ) {
            return(SITE_ERROR_NOT_ALLOWED);
        }

        if( site.ActivateSite() == false ){
            CSmallString error;
            error << "unable to activate site environment '" << site.GetName() << "' (init)";
            ES_TRACE_ERROR(error);
            return(SITE_ERROR_CONFIG_PROBLEM);
        }
    } else {
        vout << endl;
        vout << ">>> the site is already activated: " << site_name << endl;

        CFileName site_config = SiteController.GetSiteConfig(site_name);
        if( site_config == NULL ){
            CSmallString error;
            error << "specified site '" << site_name << "' was not found (init)";
            ES_TRACE_ERROR(error);
            return(SITE_ERROR_CONFIG_PROBLEM);
        }

        if( site.LoadConfig(site_config) == false ){
            CSmallString error;
            error << "unable to load site configuration from '" << site_config << "' (init)";
            ES_TRACE_ERROR(error);
            return(SITE_ERROR_CONFIG_PROBLEM);
        }

        if( site.ActivateSite() == false ){
            CSmallString error;
            error << "unable to activate site environment '" << site.GetName() << "' (init)";
            ES_TRACE_ERROR(error);
            return(SITE_ERROR_CONFIG_PROBLEM);
        }

        // we will reactivate all active modules as the site is already activated
        reactivate = true;
    }

    // set umask but not in jobs
    char origin;
    ShellProcessor.SetUMask(CUserUtils::GetUMask(User.GetRequestedUserUMaskMode(origin)));

// print site info if TTY is available
    if( (SiteController.HasTTY() || (Options.GetOptForce() == true)) && ( ! SiteController.IsSiteInfoPrinted() ) ) {
        vout << low;
        // umask is set above
        site.PrintShortSiteInfo(vout);
        vout << endl;
        SiteController.SetSiteInfoPrinted();
    }

// reactivate/autoload modules
    vout << high;
    if( reactivate ) {
        // site is already active - reactivate modules
        Module.SetFlags(Module.GetFlags() | MFB_REACTIVATED);
        ModuleController.ReactivateModules(vout);
    } else {
        // load autoloaded modules
        Module.SetFlags(Module.GetFlags() | MFB_AUTOLOADED);
        std::list<CSmallString> modules;
        HostGroup.GetHostsConfigAutoLoadedModules(modules);
        HostGroup.GetHostGroupAutoLoadedModules(modules);
        site.GetAutoLoadedModules(modules);
        AMSRegistry.GetUserAutoLoadedModules(modules);
        for( CSmallString module : modules ){
            // ignore errors from autoloaded modules
            Module.AddModule(vout,module,false,true);
        }
    }

// ssh setup
    if( SiteController.GetSSHSite() != NULL ){
        // restore exported modules transffered via ssh
        std::list<CSmallString> modules;
        SiteController.GetSSHExportedModules(modules);
        for( CSmallString module : modules ){
            // ignore errors from autoloaded modules
            Module.AddModule(vout,module,false,false);
        }

        // restore PWD
        if( SiteController.GetSSH_PWD() != NULL ){
            ShellProcessor.ChangeCurrentDir(SiteController.GetSSH_PWD(),true);
        }
        SiteController.UnsetSSHVariables();
    }

    if( Options.GetOptForce() == true ){
        SiteController.UnsetIgnoreSiteInitFlag();
    }

    vout << low;
    if( ErrorSystem.IsError() ){
        return(SITE_ERROR_CONFIG_PROBLEM);
    }

    return(SITE_STATUS_OK);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


