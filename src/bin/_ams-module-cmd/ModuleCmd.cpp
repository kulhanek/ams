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

#include "ModuleCmd.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <AMSGlobalConfig.hpp>
#include <Cache.hpp>
#include <PrintEngine.hpp>
#include <ShellProcessor.hpp>
#include <Site.hpp>
#include <Actions.hpp>
#include <AMSUserConfig.hpp>
#include <Shell.hpp>
#include <Host.hpp>
#include <User.hpp>
#include <Utils.hpp>

//------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------

MAIN_ENTRY(CModuleCmd)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CModuleCmd::Init(int argc, char* argv[])
{
    ForcePrintErrors = false;
    ExitCode = 0;

    // encode program options, all check procedures are done inside of CABFIntOpts
    int result = Options.ParseCmdLine(argc,argv);

    // should we exit or was it error?
    if( result != SO_CONTINUE ) return(result);

    // output must be directed to stderr
    // stdout is used for shell processor
    Console.Attach(stderr);

    if( Options.GetOptColors() == "always" ) {
        Console.GetTerminal().ForceColors();
    }

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
    vout << "# module (AMS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    vout << low;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CModuleCmd::Run(void)
{
    // check if site is active
    if( AMSGlobalConfig.GetActiveSiteID() == NULL ) {
        vout << low;
        vout << endl;
        vout << "<red>>>> ERROR:</red> No site is active!" << endl;
        return(false);
    }

    // initialze AMS cache
    if( Cache.LoadCache() == false) {
        ES_ERROR("unable to load AMS cache");
        return(false);
    }

    // init global host and user data
    Host.InitGlobalSetup();
    User.InitGlobalSetup();

    // initialize hosts -----------------------------
    Host.InitHostFile();
    Host.InitHost();

    // initialize user ------------------------------
    User.InitUserFile(AMSGlobalConfig.GetActiveSiteID());
    User.InitUser();

    // set module flags
    if( Options.GetOptSystem() == true ) {
        Actions.SetFlags(MFB_SYSTEM);
    } else {
        Actions.SetFlags(MFB_USER);
    }
    if( CShell::GetSystemVariable("_INFINITY_JOB_") == "_INFINITY_JOB_" ) {
        Actions.SetFlags(Actions.GetFlags() | MFB_INFINITY);
    }
    if( Options.GetOptReExported() == true ) {
        Actions.SetFlags(Actions.GetFlags() | MFB_REEXPORTED);
    }

    // extra initializations ------------------------
    // initialize active site
    if( Site.LoadConfig() == false) {
        ES_ERROR("unable to load site config");
        return(false);
    }

    // remove incompatible builds if the site is adaptive
    if( Site.IsSiteAdaptive() ){
        Site.RemoveIncompatibleBuilds();
    }

    if( (Options.GetArgAction() == "disp") ||
            (Options.GetArgAction() == "versions") ||
            (Options.GetArgAction() == "builds") ||
            (Options.GetArgAction() == "avail") ||
            (Options.GetArgAction() == "avail_no_system") ||
            (Options.GetArgAction() == "active") ||
            (Options.GetArgAction() == "exported") ||
            (Options.GetArgAction() == "list") ||
            (Options.GetArgAction() == "syshdr") ) {

        // initialze AMS print engine
        if( PrintEngine.LoadConfig() == false) {
            ES_ERROR("unable to load print engine config");
            return(false);
        }
        PrintEngine.SetOutputStream(vout);

        // load user config
        AMSUserConfig.LoadUserConfig();
    }

    // ----------------------------------------------
    if( (Options.GetArgAction() == "add") || (Options.GetArgAction() == "activate") ) {
        // add modules
        bool ok = true;
        for(int i=1; i < Options.GetNumberOfProgArgs(); i++) {
            ok &= AddModule(Options.GetProgArg(i),Options.GetArgAction() == "activate");
        }
        if( ok == false ){
            ExitCode = 1;
        }
        return(ok);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "remove" ) {
        // remove modules
        bool ok = true;
        for(int i=1; i < Options.GetNumberOfProgArgs(); i++) {
            EActionError error = Actions.RemoveModule(vout,Options.GetProgArg(i));

            if( error > 0 ){
                CSmallString error;
                error << "unable to add module '" << Options.GetProgArg(i) << "'";
                ES_TRACE_ERROR(error);
                vout << low;
                vout << endl;
                vout << "<red>>>> ERROR:</red> Unable to remove module <b>" << Options.GetProgArg(i) << "</b>." << endl;
                ok = false;
            }

            switch(error){
                case EAE_STATUS_OK:
                    break;
                case EAE_MODULE_NOT_FOUND:
                    vout << "           The module was not found in the AMS database (mispelled name?)." << endl;
                    break;
                case EAE_NOT_ACTIVE:
                    vout << "           The module is not active." << endl;
                    break;
                case EAE_BUILD_NOT_FOUND:
                    vout << "           The module does not have suitable build for this host." << endl;
                    vout << "           Try <b>module disp " << Options.GetProgArg(i) << "</b> to get more information." << endl;
                    break;
                default:
                    ForcePrintErrors = true;
                    break;
            }
        }
        if( ok == false ){
            ExitCode = 1;
        }
        return(ok);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "versions" ) {
        // module versions
        for(int i=1; i < Options.GetNumberOfProgArgs(); i++) {
            fprintf(stderr,"\n");
            if( PrintEngine.PrintModModuleVersions(Options.GetProgArg(i)) == false ) return(false);
        }
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "help" ) {
        // module help
        bool ok = true;
        PrintEngine.StartHelp();
        for(int i=1; i < Options.GetNumberOfProgArgs(); i++) {
            ok &= PrintEngine.AddHelp(Options.GetProgArg(i));
        }
        if( ok == true ) PrintEngine.ShowHelp();
        return(ok);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "builds" ) {
        // module builds
        for(int i=1; i < Options.GetNumberOfProgArgs(); i++) {
            fprintf(stderr,"\n");
            if( PrintEngine.PrintModModuleBuilds(Options.GetProgArg(i)) == false ) return(false);
        }
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "disp" ) {
        // module info
        bool ok = true;
        for(int i=1; i < Options.GetNumberOfProgArgs(); i++) {
            fprintf(stderr,"\n");
            if( PrintEngine.PrintModModuleInfo(Options.GetProgArg(i)) == false ){
                vout << low;
                vout << endl;
                vout << "<red>>>> ERROR:</red> Unable to display information about module <b>" << Options.GetProgArg(i) << "</b>." << endl;
                ForcePrintErrors = true;
                ok = false;
            }
        }
        if( ok == false ){
            ExitCode = 1;
        }
        return(ok);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "isactive" ) {
        ForcePrintErrors = true;
        bool result = true;
        for(int i=1; i < Options.GetNumberOfProgArgs(); i++) {
            CSmallString warning;
            warning << "module '" << Options.GetProgArg(i) << "' is ";
            if( AMSGlobalConfig.IsModuleActive(Options.GetProgArg(i)) == true ) {
                warning << "active";
                result &= true;
            } else {
                warning << "not active";
                result &= false;
                ExitCode = 1;
            }
            ES_WARNING(warning);
        }
        return(result);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "getactver" ) {
        CSmallString actver;
        if( AMSGlobalConfig.GetActiveModuleVersion(Options.GetProgArg(1),actver) == false ) {
            CSmallString warning;
            warning << "module '" << Options.GetProgArg(1) << "' is no active";
            ES_WARNING(warning);
            return(false);
        }
        vout << actver << endl;
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "avail_no_system" ) {
        vout << endl;
        PrintEngine.PrintModAvailableModules(Console.GetTerminal(),false);
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "avail" ) {
        vout << endl;
        PrintEngine.PrintModAvailableModules(Console.GetTerminal(),true);
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "active" ) {
        vout << endl;
        PrintEngine.PrintModActiveModules(Console.GetTerminal());
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "exported" ) {
        vout << endl;
        PrintEngine.PrintModExportedModules(Console.GetTerminal());
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "list" ) {
        vout << endl;
        PrintEngine.PrintModExportedModules(Console.GetTerminal());
        vout << endl;
        PrintEngine.PrintModActiveModules(Console.GetTerminal());
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "autoload" ) {
        ForcePrintErrors = true;
        AMSUserConfig.LoadUserConfig();
        CXMLElement* p_mod_ele = AMSUserConfig.GetAutoloadedModules();
        Actions.SetFlags(Actions.GetFlags() ^ MFB_SYS_AUTOLOADED);
        Actions.SetFlags(Actions.GetFlags() | MFB_USER_AUTOLOADED);
        if( CSite::ActivateAutoloadedModules(p_mod_ele) == false ) {
            ES_WARNING("unable to load user auto-loaded modules");
            return(false);
        }
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "reactivate" ) {
        ForcePrintErrors = true;
        Actions.ReactivateModules(vout);
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "purge" ) {
        return(Actions.PurgeModules(vout));
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "syshdr" ) {
        vout << endl;
        PrintEngine.PrintSYSAvailHeader(Console.GetTerminal());
        return(true);
    }
    // ----------------------------------------------
    else {
        ES_ERROR("not implemented");
        return(false);
    }
}

//------------------------------------------------------------------------------

void CModuleCmd::Finalize(void)
{
    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << endl;
    vout << "# ==============================================================================" << endl;
    vout << "# module (AMS utility) terminated at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    if( ErrorSystem.IsError() || (ErrorSystem.IsAnyRecord() && Options.GetOptVerbose()) ){
        if( ForcePrintErrors ) vout << low;
        ErrorSystem.PrintErrors(vout);
        if( ForcePrintErrors ) vout << endl;
    }

    vout << low;
    if( ! ForcePrintErrors ) vout << endl;

    if( ErrorSystem.IsError() ) {
        ExitCode = 1;
        ShellProcessor.RollBack();
    }

    ShellProcessor.SetExitCode(ExitCode);
    ShellProcessor.BuildEnvironment();
}

//------------------------------------------------------------------------------

bool CModuleCmd::AddModule(const CSmallString& module,bool do_not_export)
{
    CSmallString name;
    CSmallString ver;

    CUtils::ParseModuleName(module,name,ver);

    EActionError error = EAE_MODULE_NOT_FOUND;

    bool ok = true;

    if( (ver == NULL) || (ver == "default") ){
        // first, try exactly what is required
        error = Actions.AddModule(vout,module,false,do_not_export);

        if( error == EAE_BUILD_NOT_FOUND ){
            // if not suitable build is found - try to downgrade version
            std::vector<std::string> versions;
            Cache.GetSortedModuleVersions(name,versions);

            CSmallString dver, drch, dmode;
            CXMLElement* p_ele = Cache.GetModule(name);
            if( p_ele ){
                Cache.GetModuleDefaults(p_ele,dver,drch,dmode);
            }

            std::vector<std::string>::iterator it = versions.begin();
            std::vector<std::string>::iterator ie = versions.end();

            bool default_found = false;
            CSmallString last_vers = dver;

            while( it != ie ){
                CSmallString version = CSmallString(*it);

                if( default_found == true ){
                    vout << "<blue>>>> WARNING:</blue> No suitable build found for <b>" << name << ":" << last_vers << "</b>";
                    vout << ", downgrading to <b>" << name << ":" << version << "</b>" << endl;
                    CSmallString moudle_name;
                    last_vers = version;
                    moudle_name << name << ":" << version;
                    error = Actions.AddModule(vout,moudle_name,false,do_not_export);
                    if( error == EAE_STATUS_OK ) break;
                    if( error != EAE_BUILD_NOT_FOUND ) break;
                }

                if( version == dver ) default_found = true;  // this must be here - we need to skip default version
                it++;
            }
        }

    } else {
        // add exactly what is required
        error = Actions.AddModule(vout,module,false,do_not_export);
    }

    if( error > 0 ){
        CSmallString error;
        error << "unable to add module '" << module << "'";
        ES_TRACE_ERROR(error);
        vout << low;
        vout << endl;
        vout << "<red>>>> ERROR:</red> Unable to add module <b>" << module << "</b>!" << endl;
        ok = false;
    }

    switch(error){
        case EAE_STATUS_OK:
            break;
        case EAE_MODULE_NOT_FOUND:
            vout << "           The module was not found in the AMS database (mispelled name?)." << endl;
            break;
        case EAE_PERMISSION_DENIED:
            vout << "           The current user is not allowed to use this module (permission denied)." << endl;
            break;
        case EAE_BUILD_NOT_FOUND:
            vout << "           The module does not have suitable build for this host." << endl;
            vout << "           Try <b>module disp " << module << "</b> to get more information." << endl;
            break;
        case EAE_DEPENDENCY_ERROR:
            vout << "           Problems with module dependencies." << endl;
            ForcePrintErrors = true;
            break;
        default:
            ForcePrintErrors = true;
            break;
    }

    return(ok);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


