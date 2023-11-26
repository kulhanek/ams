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

#include "BundleCmd.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <AMSRegistry.hpp>
#include <HostGroup.hpp>
#include <Host.hpp>
#include <User.hpp>
#include <FileSystem.hpp>
#include <ModBundle.hpp>
#include <PrintEngine.hpp>

//------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------

MAIN_ENTRY(CBundleCmd)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CBundleCmd::Init(int argc, char* argv[])
{
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
    vout << "# ams-bundle (AMS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    vout << low;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CBundleCmd::Run(void)
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

    // ----------------------------------------------
    if( (Options.GetArgAction() == "info") ) {
        return( InfoBundle() );
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "create" ) {
        return( CreateBundle() );
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "rebuild" ) {
        return( RebuildBundle() );
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "avail" ) {
        return( PrintAvailMods() );
    }
    // ----------------------------------------------
    else {
        CSmallString error;
        error << "not implemented action '" << Options.GetArgAction() << "'";
        ES_ERROR(error);
    }

    return(false);
}

//------------------------------------------------------------------------------

void CBundleCmd::Finalize(void)
{
    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << "# ==============================================================================" << endl;
    vout << "# ams-bundle (AMS utility) terminated at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    vout << low;
    if( ErrorSystem.IsError() || (ErrorSystem.IsAnyRecord() && Options.GetOptVerbose()) ){
        ErrorSystem.PrintErrors(vout);
    }
    vout << endl;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CBundleCmd::InfoBundle(void)
{
    CFileName cwd,bundle_root;
    CFileSystem::GetCurrentDir(cwd);
    if( CModBundle::GetBundleRoot(cwd,bundle_root) == false ){
        CSmallString error;
        error << "no bundle found from the path starting at: '" << cwd << "'";
        ES_ERROR(error);
        return(false);
    }

    CModBundle bundle;
    if( bundle.InitBundle(bundle_root) == false ){
        CSmallString error;
        error << "unable to load bundle configuration '" << bundle_root << "'";
        ES_ERROR(error);
        return(false);
    }

    vout << endl;
    vout << "# Scaning doc and bld files ... ";
    bundle.FindAllFragmentFiles();
    vout << "done" << endl;

    vout << endl;
    bundle.PrintInfo(vout);

    return(true);
}

//------------------------------------------------------------------------------

bool CBundleCmd::CreateBundle(void)
{
    CFileName cwd,bundle_root;
    CFileSystem::GetCurrentDir(cwd);

    if( CModBundle::GetBundleRoot(cwd,bundle_root) == true ){
        CSmallString error;
        error << "the current directory is the part of the existing AMS bundle";
        ES_ERROR(error);
        return(false);
    }

    if( Options.GetNumberOfProgArgs() != 4 ){
        CSmallString error;
        error << "incorrect number of arguments";
        ES_ERROR(error);
        return(false);
    }

    CModBundle bundle;
    if( bundle.CreateBundle(cwd,Options.GetProgArg(1),Options.GetProgArg(2),Options.GetProgArg(3)) == false ){
        CSmallString error;
        error << "unable to create bundle '" << cwd / Options.GetProgArg(1) << "'";
        ES_ERROR(error);
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CBundleCmd::RebuildBundle(void)
{
    CFileName cwd,bundle_root;
    CFileSystem::GetCurrentDir(cwd);
    if( CModBundle::GetBundleRoot(cwd,bundle_root) == false ){
        CSmallString error;
        error << "no bundle found from the path starting at: '" << cwd << "'";
        ES_ERROR(error);
        return(false);
    }

    CModBundle bundle;
    if( bundle.InitBundle(bundle_root) == false ){
        CSmallString error;
        error << "unable to load bundle configuration '" << bundle_root << "'";
        ES_ERROR(error);
        return(false);
    }

    vout << endl;
    vout << "# Scaning doc and bld files ... ";
    bundle.FindAllFragmentFiles();
    vout << "done" << endl;

    vout << endl;
    bundle.PrintInfo(vout);

    vout << endl;
    if( bundle.RebuildCache(vout) == false ) return(false);

    return( bundle.SaveCaches() );
}

//------------------------------------------------------------------------------

bool CBundleCmd::PrintAvailMods(void)
{
    CFileName cwd,bundle_root;
    CFileSystem::GetCurrentDir(cwd);
    if( CModBundle::GetBundleRoot(cwd,bundle_root) == false ){
        CSmallString error;
        error << "no bundle found from the path starting at: '" << cwd << "'";
        ES_ERROR(error);
        return(false);
    }

    CModBundle bundle;
    if( bundle.InitBundle(bundle_root) == false ){
        CSmallString error;
        error << "unable to load bundle configuration '" << bundle_root << "'";
        ES_ERROR(error);
        return(false);
    }

    if( bundle.LoadCache() == false ){
        CSmallString error;
        error << "unable to load bundle cache";
        ES_ERROR(error);
        return(false);
    }


    PrintEngine.InitPrintProfile();

    PrintEngine.PrintHeader(Console.GetTerminal(),"AVAILABLE MODULES (ams-bundle)",EPEHS_SECTION);
    bundle.PrintAvail(Console.GetTerminal(),Options.GetOptInludeVersions(),true);

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


