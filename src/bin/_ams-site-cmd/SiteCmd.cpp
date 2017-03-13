// =============================================================================
// AMS
// -----------------------------------------------------------------------------
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
#include <GlobalConfig.hpp>
#include <ShellProcessor.hpp>
#include <PrintEngine.hpp>
#include <Site.hpp>
#include <Utils.hpp>
#include <SoftConfig.hpp>
#include <Actions.hpp>
#include <Shell.hpp>
#include <User.hpp>
#include <Host.hpp>

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
    if( Options.IsOptHostSet() ) {
        Host.SetHostName(Options.GetOptHost());
    }

    if( Options.IsOptUserSet() ) {
        User.SetUserName(Options.GetOptUser());
        if( Options.GetArgAction() == "activate" ){
            ES_ERROR("option --user is not allowed with action activate");
            return(false);
        }
    }

    // set module flags
    Actions.SetFlags(MFB_SYSTEM);
    if( CShell::GetSystemVariable("_INFINITY_JOB_") == "_INFINITY_JOB_" ) {
        Actions.SetFlags(MFB_INFINITY);
    }

    // init global host and user data
    Host.InitGlobalSetup();
    User.InitGlobalSetup();

    // initialize hosts -----------------------------
    Host.InitHostFile();
    Host.InitHost();

    if( GlobalConfig.GetActiveSiteID() != NULL ){

        // initialize user -----------------------------
        User.InitUserFile(GlobalConfig.GetActiveSiteID());
        User.InitUser();
    }

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
                return(false);
            case SITE_ERROR_SITE_NOT_FOUND:
                vout << "           The site is not defined in the AMS database (mispelled name?)." << endl;
                return(false);
            case SITE_ERROR_NOT_ALLOWED:
                vout << "           The site is not allowed on '<u>" << Host.GetHostName() << "</u>'." << endl;
                return(false);
        }
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "info" ) {

        if( (Options.GetArgSite() == NULL) && (GlobalConfig.GetActiveSiteID() == NULL) ){
            vout << low;
            vout << endl;
            vout << "<red>>>> ERROR:</red> No site is active!" << endl;
            return(false);
        }

        int error = InfoSite();

        if( error > 0 ) {
            vout << low;
            vout << endl;
            vout << "<red>>>> ERROR:</red> Unable to provide information about the site <u>" << Options.GetArgSite() << "</u> info." << endl;
        }

        switch(error){
            case SITE_ERROR_CONFIG_PROBLEM:
                vout << "           Configuration problems." << endl;
                ForcePrintErrors = true;
                return(false);
            case SITE_ERROR_SITE_NOT_FOUND:
                vout << "           The site is not defined in the AMS database (mispelled name?)." << endl;
                return(false);
        }
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "disp" ) {

        if( (Options.GetArgSite() == NULL) && (GlobalConfig.GetActiveSiteID() == NULL) ){
            vout << low;
            vout << endl;
            vout << "<red>>>> ERROR:</red> No site is active!" << endl;
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
                return(false);
            case SITE_ERROR_SITE_NOT_FOUND:
                vout << "           The site is not defined in the AMS database (mispelled name?)." << endl;
                return(false);
        }
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "active" ) {
        CSmallString site_sid;

        site_sid = GlobalConfig.GetActiveSiteID();

        if( site_sid == NULL ) {
            CSmallString error;
            error << "no site is active";
            ES_ERROR(error);
            return(false);
        }

        if( Site.LoadConfig(site_sid) == false ) {
            CSmallString error;
            error << "unable to load site '" << site_sid << "' config";
            ES_TRACE_ERROR(error);
            return(false);
        }

        fprintf(stderr,"%s\n",(const char*)Site.GetName());

        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "isactive" ) {
        if( IsActive() ){
            vout << "INFO: the site <u>" << Options.GetArgSite() << "</u> is active." << endl;
            return(true);
        } else {
            vout << "INFO: the site <u>" << Options.GetArgSite() << "</u> is NOT active." << endl;
            ExitCode = 1;
            return(false);
        }
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "isallowed" ) {
        if( IsAllowed() ){
            vout << "INFO: the site <u>" << Options.GetArgSite() << "</u> can be activated on '<u>" << Host.GetHostName() << "</u>'." << endl;
            return(true);
        } else {
            vout << "INFO: the site <u>" << Options.GetArgSite() << "</u> CANNOT be activated on '<u>" << Host.GetHostName() << "</u>'." << endl;
            ExitCode = 1;
            return(false);
        }
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "avail" ) {
        // load user config
        SoftConfig.LoadUserConfig();

        // initialze AMS print engine
        if( PrintEngine.LoadConfig() == false) {
            ES_TRACE_ERROR("unable to load print engine config");
            return(false);
        }
        PrintEngine.SetOutputStream(vout);

        // print available sites
        PrintEngine.PrintAvailableSites(Console.GetTerminal(),Options.GetOptAll());

        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "id" ) {
        CSmallString site_sid;

        if( Options.GetArgSite() != NULL ) {
            if( (Options.GetArgSite().GetLength() > 0) && (Options.GetArgSite()[0] == '{') ) {
                site_sid = Options.GetArgSite();
            } else {
                site_sid = CUtils::GetSiteID(Options.GetArgSite());
            }
        } else {
            site_sid = GlobalConfig.GetActiveSiteID();
        }

        if( site_sid == NULL ) {
            CSmallString error;
            error << "specified site '" << Options.GetArgSite() << "' was not found";
            ES_ERROR(error);
            return(false);
        }

        if( Site.LoadConfig(site_sid) == false ) {
            CSmallString error;
            error << "unable to load site '" << Options.GetArgSite() << "' config";
            ES_TRACE_ERROR(error);
            return(false);
        }
        fprintf(stderr,"%s",(const char*)Site.GetID());

        return(true);
    }
    // ----------------------------------------------
    else {
        ES_ERROR("not implemented");
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
        ErrorSystem.PrintErrors(vout);
        if( ForcePrintErrors ) vout << endl;
    }

    if( (Options.GetArgAction() == "activate") ||
        (Options.GetArgAction() == "avail") ||
        (Options.GetArgAction() == "disp") ||
        (Options.GetArgAction() == "info")) {
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
    // deactivate current site
    if( GlobalConfig.GetActiveSiteID() != NULL ) {

        if( Site.LoadConfig() == false ) {
            CSmallString error;
            error << "unable to load site '" << GlobalConfig.GetActiveSiteID() << "' config";
            ES_TRACE_ERROR(error);
            return(SITE_ERROR_CONFIG_PROBLEM);
        }

        if( Site.DeactivateSite() == false ) {
            CSmallString error;
            error << "unable to deactivate site '" << GlobalConfig.GetActiveSiteID() << "'";
            ES_TRACE_ERROR(error);
            return(SITE_ERROR_CONFIG_PROBLEM);
        }
    }

    // activate site
    CSmallString site_sid;
    if( (Options.GetArgSite().GetLength() > 0) && (Options.GetArgSite()[0] == '{') ) {
        site_sid = Options.GetArgSite();
        // is sid valid?
        if( CUtils::IsSiteIDValid(site_sid) == false ){
            return(SITE_ERROR_SITE_NOT_FOUND);
        }
    } else {
        site_sid = CUtils::GetSiteID(Options.GetArgSite());
        if( site_sid == NULL ) {
            CSmallString error;
            error << "specified site '" << Options.GetArgSite() << "' was not found";
            ES_ERROR(error);
            return(SITE_ERROR_SITE_NOT_FOUND);
        }
    }

    if( Site.LoadConfig(site_sid) == false ) {
        CSmallString error;
        error << "unable to load site '" << Options.GetArgSite() << "' config";
        ES_TRACE_ERROR(error);
        return(SITE_ERROR_CONFIG_PROBLEM);
    }

    if( Site.CanBeActivated() == false ) {
        return(SITE_ERROR_NOT_ALLOWED);
    }

    // reinit user and host
    // init global host and user data
    Host.ClearAll();
    Host.InitGlobalSetup();

    User.ClearAll();
    User.InitGlobalSetup();

    if( Site.ActivateSite() == false ) {
        CSmallString error;
        error << "unable to activate site '" << Options.GetArgSite() << "'";
        ES_TRACE_ERROR(error);
        return(SITE_ERROR_CONFIG_PROBLEM);
    }

    Site.PrintShortSiteInfo(vout);

    if( ErrorSystem.IsError() ){
        return(SITE_ERROR_CONFIG_PROBLEM);
    }

    return(SITE_STATUS_OK);
}

//------------------------------------------------------------------------------

int CSiteCmd::DispSite(void)
{
    CSmallString site_sid;

    if( Options.GetArgSite() != NULL ) {
        if( (Options.GetArgSite().GetLength() > 0) && (Options.GetArgSite()[0] == '{') ) {
            site_sid = Options.GetArgSite();
        } else {
            site_sid = CUtils::GetSiteID(Options.GetArgSite());
        }
    } else {
        site_sid = GlobalConfig.GetActiveSiteID();
    }

    if( CUtils::IsSiteIDValid(site_sid) == false ) {
        CSmallString error;
        error << "specified site '" << Options.GetArgSite() << "' was not found";
        ES_TRACE_ERROR(error);
        return(SITE_ERROR_SITE_NOT_FOUND);
    }

    if( Site.LoadConfig(site_sid) == false ) {
        CSmallString error;
        error << "unable to load site '" << Options.GetArgSite() << "' config";
        ES_TRACE_ERROR(error);
        return(SITE_ERROR_CONFIG_PROBLEM);
    }
    Site.PrintFullSiteInfo(vout);

    return(SITE_STATUS_OK);
}

//------------------------------------------------------------------------------

int CSiteCmd::InfoSite(void)
{
    CSmallString site_sid;

    if( Options.GetArgSite() != NULL ) {
        if( (Options.GetArgSite().GetLength() > 0) && (Options.GetArgSite()[0] == '{') ) {
            site_sid = Options.GetArgSite();
        } else {
            site_sid = CUtils::GetSiteID(Options.GetArgSite());
        }
    } else {
        site_sid = GlobalConfig.GetActiveSiteID();
    }

    if( CUtils::IsSiteIDValid(site_sid) == false ) {
        CSmallString error;
        error << "specified site '" << Options.GetArgSite() << "' was not found";
        ES_TRACE_ERROR(error);
        return(SITE_ERROR_SITE_NOT_FOUND);
    }

    if( Site.LoadConfig(site_sid) == false ) {
        CSmallString error;
        error << "unable to load site '" << Options.GetArgSite() << "' config";
        ES_TRACE_ERROR(SITE_ERROR_CONFIG_PROBLEM);
        return(false);
    }
    Site.PrintShortSiteInfo(vout);

    return(SITE_STATUS_OK);
}

//------------------------------------------------------------------------------

bool CSiteCmd::IsActive(void)
{
    CSmallString site_sid;

    site_sid = GlobalConfig.GetActiveSiteID();

    if( site_sid == NULL ) {
        CSmallString error;
        error << "no site is active";
        ES_TRACE_ERROR(error);
        return(false);
    }

    if( Site.LoadConfig(site_sid) == false ) {
        CSmallString error;
        error << "unable to load site '" << site_sid << "' config";
        ES_TRACE_ERROR(error);
        return(false);
    }

    if( (Options.GetArgSite().GetLength() > 0) && (Options.GetArgSite()[0] == '{') ) {
        return( site_sid == Options.GetArgSite() );
    } else {
        return( Site.GetName() == Options.GetArgSite() );
    }
}

//------------------------------------------------------------------------------

bool CSiteCmd::IsAllowed(void)
{
    CSmallString site_sid;
    if( (Options.GetArgSite().GetLength() > 0) && (Options.GetArgSite()[0] == '{') ) {
        site_sid = Options.GetArgSite();
    } else {
        site_sid = CUtils::GetSiteID(Options.GetArgSite());
        if( site_sid == NULL ) {
            CSmallString error;
            error << "specified site '" << Options.GetArgSite() << "' was not found";
            ES_ERROR(error);
            return(false);
        }
    }

    if( Site.LoadConfig(site_sid) == false ) {
        CSmallString error;
        error << "unable to load site '" << site_sid << "' config";
        ES_TRACE_ERROR(error);
        return(false);
    }

    return( Site.CanBeActivated() );
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


