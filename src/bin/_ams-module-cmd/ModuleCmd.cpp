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

#include "ModuleCmd.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <ModCache.hpp>
#include <PrintEngine.hpp>
#include <ShellProcessor.hpp>
#include <Site.hpp>
#include <Shell.hpp>
#include <Host.hpp>
#include <User.hpp>
#include <Utils.hpp>
#include <AMSRegistry.hpp>
#include <SiteController.hpp>
#include <ModuleController.hpp>
#include <HostGroup.hpp>
#include <ModUtils.hpp>

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

//    // set module flags
//    if( Options.GetOptSystem() == true ) {
//        Actions.SetFlags(MFB_SYSTEM);
//    } else {
//        Actions.SetFlags(MFB_USER);
//    }
//    if( CShell::GetSystemVariable("_INFINITY_JOB_") == "_INFINITY_JOB_" ) {
//        Actions.SetFlags(Actions.GetFlags() | MFB_INFINITY);
//    }
//    if( Options.GetOptReExported() == true ) {
//        Actions.SetFlags(Actions.GetFlags() | MFB_REEXPORTED);
//    }

    // extra initializations ------------------------
    // initialize active site
//    if( Site.LoadConfig() == false) {
//        ES_ERROR("unable to load site config");
//        return(false);
//    }

// init site controller
    SiteController.InitSiteControllerConfig();

// init module controller
    ModuleController.InitModuleControllerConfig();


    // remove incompatible builds if the site is adaptive
//    if( Site.IsSiteAdaptive() ){
//        Site.RemoveIncompatibleBuilds();
//    }

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
//        for(int i=1; i < Options.GetNumberOfProgArgs(); i++) {
//            EActionError error = Actions.RemoveModule(vout,Options.GetProgArg(i));

//            if( error > 0 ){
//                CSmallString error;
//                error << "unable to add module '" << Options.GetProgArg(i) << "'";
//                ES_TRACE_ERROR(error);
//                vout << low;
//                vout << endl;
//                vout << "<red>>>> ERROR:</red> Unable to remove module <b>" << Options.GetProgArg(i) << "</b>." << endl;
//                ok = false;
//            }

//            switch(error){
//                case EAE_STATUS_OK:
//                    break;
//                case EAE_MODULE_NOT_FOUND:
//                    vout << "           The module was not found in the AMS database (mispelled name?)." << endl;
//                    break;
//                case EAE_NOT_ACTIVE:
//                    vout << "           The module is not active." << endl;
//                    break;
//                case EAE_BUILD_NOT_FOUND:
//                    vout << "           The module does not have suitable build for this host." << endl;
//                    vout << "           Try <b>module disp " << Options.GetProgArg(i) << "</b> to get more information." << endl;
//                    break;
//                default:
//                    ForcePrintErrors = true;
//                    break;
//            }
//        }
        if( ok == false ){
            ExitCode = 1;
        }
        return(ok);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "versions" ) {
        ModuleController.LoadBundles(EMBC_SMALL);
        ModuleController.MergeBundles();
        for(int i=1; i < Options.GetNumberOfProgArgs(); i++) {
            fprintf(stderr,"\n");
            ModCache.PrintModuleVersions(vout,Options.GetProgArg(i));
        }
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "help" ) {
        // module help
        bool ok = true;
//        PrintEngine.StartHelp();
//        for(int i=1; i < Options.GetNumberOfProgArgs(); i++) {
//            ok &= PrintEngine.AddHelp(Options.GetProgArg(i));
//        }
//        if( ok == true ) PrintEngine.ShowHelp();
        return(ok);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "builds" ) {
        ModuleController.LoadBundles(EMBC_SMALL);
        ModuleController.MergeBundles();
        for(int i=1; i < Options.GetNumberOfProgArgs(); i++) {
            fprintf(stderr,"\n");
            ModCache.PrintModuleBuilds(vout,Options.GetProgArg(i));
        }
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "origin" ) {
        ModuleController.LoadBundles(EMBC_SMALL);
        ModuleController.MergeBundles();
        for(int i=1; i < Options.GetNumberOfProgArgs(); i++) {
            fprintf(stderr,"\n");
            ModCache.PrintModuleOrigin(vout,Options.GetProgArg(i));
        }
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "disp" ) {
        // module info
        bool ok = true;
        for(int i=1; i < Options.GetNumberOfProgArgs(); i++) {
            fprintf(stderr,"\n");
//            if( PrintEngine.PrintModModuleInfo(Options.GetProgArg(i)) == false ){
//                vout << low;
//                vout << endl;
//                vout << "<red>>>> ERROR:</red> Unable to display information about module <b>" << Options.GetProgArg(i) << "</b>." << endl;
//                ForcePrintErrors = true;
//                ok = false;
//            }
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
            if( ModuleController.IsModuleActive(Options.GetProgArg(i)) == true ) {
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
    else if( Options.GetArgAction() == "getactmod" ) {
        CSmallString actver;
        if( ModuleController.GetActiveModuleVersion(Options.GetProgArg(1),actver) == false ) {
            CSmallString warning;
            warning << "module '" << Options.GetProgArg(1) << "' is not active";
            ES_WARNING(warning);
            return(false);
        }
        CSmallString name;
        CModUtils::ParseModuleName(Options.GetProgArg(1),name);
        vout << name << ":" << actver;
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "getactver" ) {
        CSmallString actver;
        if( ModuleController.GetActiveModuleVersion(Options.GetProgArg(1),actver) == false ) {
            CSmallString warning;
            warning << "module '" << Options.GetProgArg(1) << "' is no active";
            ES_WARNING(warning);
            return(false);
        }
        vout << actver;
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "avail_no_system" ) {
        ModuleController.LoadBundles(EMBC_SMALL);
        ModuleController.MergeBundles();
        PrintEngine.InitPrintProfile();
        PrintEngine.PrintHeader(Console.GetTerminal(),"AVAILABLE MODULES (Infinity Software Base | amsmodule)",EPEHS_SECTION);
        ModCache.PrintAvail(Console.GetTerminal(),Options.GetOptIncludeVersions(),false);
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "avail" ) {
        ModuleController.LoadBundles(EMBC_SMALL);
        ModuleController.MergeBundles();
        PrintEngine.InitPrintProfile();
        PrintEngine.PrintHeader(Console.GetTerminal(),"AVAILABLE MODULES (Infinity Software Base | amsmodule)",EPEHS_SECTION);
        ModCache.PrintAvail(Console.GetTerminal(),Options.GetOptIncludeVersions(),true);
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "bundles" ) {
        ModuleController.LoadBundles(EMBC_BIG);
        vout << endl;
        ModuleController.PrintBundlesInfo(vout);
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "active" ) {
        PrintEngine.InitPrintProfile();
        ModuleController.PrintModActiveModules(Console.GetTerminal());
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "exported" ) {
        PrintEngine.InitPrintProfile();
        ModuleController.PrintModExportedModules(Console.GetTerminal());
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "list" ) {
        PrintEngine.InitPrintProfile();
        ModuleController.PrintModExportedModules(Console.GetTerminal());
        ModuleController.PrintModActiveModules(Console.GetTerminal());
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "autoload" ) {
        ForcePrintErrors = true;
//        AMSUserConfig.LoadUserConfig();
//        CXMLElement* p_mod_ele = AMSUserConfig.GetAutoloadedModules();
//        Actions.SetFlags(Actions.GetFlags() ^ MFB_SYS_AUTOLOADED);
//        Actions.SetFlags(Actions.GetFlags() | MFB_USER_AUTOLOADED);
//        if( CSite::ActivateAutoloadedModules(p_mod_ele) == false ) {
//            ES_WARNING("unable to load user auto-loaded modules");
//            return(false);
//        }
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "reactivate" ) {
        ForcePrintErrors = true;
        return(ModuleController.ReactivateModules(vout));
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "purge" ) {
        return(ModuleController.PurgeModules(vout));
    }
    // ----------------------------------------------
    else {
        ES_ERROR("not implemented");
        return(false);
    }

    return(false);
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
    if( (Options.GetArgAction() != "getactmod") && (Options.GetArgAction() != "getactver") ){
        if( ! ForcePrintErrors ) vout << endl;
    }

    if( (ExitCode != 0) && (ErrorSystem.IsError()) ) {
        ShellProcessor.RollBack();
    }

    ShellProcessor.SetExitCode(ExitCode);

    // do not buld shell environment in specific cases
    if( Options.GetArgAction() != "help" ){
        ShellProcessor.BuildEnvironment();
    }
}

//------------------------------------------------------------------------------

bool CModuleCmd::AddModule(const CSmallString& module,bool do_not_export)
{
//    CSmallString name;
//    CSmallString ver;

//    CUtils::ParseModuleName(module,name,ver);

//    EActionError error = EAE_MODULE_NOT_FOUND;

    bool ok = true;

//    if( (ver == NULL) || (ver == "default") ){
//        // first, try exactly what is required
//        error = Actions.AddModule(vout,module,false,do_not_export);

//        if( error == EAE_BUILD_NOT_FOUND ){
//            // if not suitable build is found - try to downgrade version
//            std::vector<std::string> versions;
//            Cache.GetSortedModuleVersions(name,versions);

//            CSmallString dver, drch, dmode;
//            CXMLElement* p_ele = Cache.GetModule(name);
//            if( p_ele ){
//                Cache.GetModuleDefaults(p_ele,dver,drch,dmode);
//            }

//            std::vector<std::string>::iterator it = versions.begin();
//            std::vector<std::string>::iterator ie = versions.end();

//            bool default_found = false;
//            CSmallString last_vers = dver;

//            while( it != ie ){
//                CSmallString version = CSmallString(*it);

//                if( default_found == true ){
//                    vout << "<blue><b>>>> WARNING:</b></blue> No suitable build found for <b>" << name << ":" << last_vers << "</b>";
//                    vout << ", downgrading to <b>" << name << ":" << version << "</b>" << endl;
//                    CSmallString moudle_name;
//                    last_vers = version;
//                    moudle_name << name << ":" << version;
//                    error = Actions.AddModule(vout,moudle_name,false,do_not_export);
//                    if( error == EAE_STATUS_OK ) break;
//                    if( error != EAE_BUILD_NOT_FOUND ) break;
//                }

//                if( version == dver ) default_found = true;  // this must be here - we need to skip default version
//                it++;
//            }
//        }

//    } else {
//        // add exactly what is required
//        error = Actions.AddModule(vout,module,false,do_not_export);
//    }

//    if( error > 0 ){
//        CSmallString error;
//        error << "unable to add module '" << module << "'";
//        ES_TRACE_ERROR(error);
//        vout << low;
//        vout << endl;
//        vout << "<red>>>> ERROR:</red> Unable to add module <b>" << module << "</b>!" << endl;
//        ok = false;
//    }

//    switch(error){
//        case EAE_STATUS_OK:
//            break;
//        case EAE_MODULE_NOT_FOUND:
//            vout << "           The module was not found in the AMS database (mispelled name?)." << endl;
//            break;
//        case EAE_PERMISSION_DENIED:
//            vout << "           The current user is not allowed to use this module (permission denied)." << endl;
//            break;
//        case EAE_BUILD_NOT_FOUND:
//            vout << "           The module does not have suitable build for this host." << endl;
//            vout << "           Try <b>module disp " << module << "</b> to get more information." << endl;
//            break;
//        case EAE_DEPENDENCY_ERROR:
//            vout << "           Problems with module dependencies." << endl;
//            ForcePrintErrors = true;
//            break;
//        default:
//            ForcePrintErrors = true;
//            break;
//    }

    return(ok);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


