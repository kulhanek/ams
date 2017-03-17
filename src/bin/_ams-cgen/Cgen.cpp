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

#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <Cache.hpp>
#include <PrintEngine.hpp>
#include <AMSCompletion.hpp>
#include <AMSGlobalConfig.hpp>
#include <User.hpp>
#include "Cgen.hpp"

//------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------

MAIN_ENTRY(CCgen)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CCgen::CCgen(void)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CCgen::Init(int argc, char* argv[])
{
    // encode program options, all check procedures are done inside of CABFIntOpts
    int result = Options.ParseCmdLine(argc,argv);

    // should we exit or was it error?
    if( result != SO_CONTINUE ) return(result);

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
    vout << "# _ams-cgen (AMS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    vout << low;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CCgen::Run(void)
{
    // init completion system
    if( AMSCompletion.InitCompletion() == false ) return(false);

    // load module cache only if site is active
    if( AMSGlobalConfig.GetActiveSiteID() != NULL ) {
        if( Cache.LoadCache() == false) return(-1);
        // do not load user config for PrintEngine
        // because PrintEngine is used only for ams-print, which
        // is system utility
        if( PrintEngine.LoadConfig() == false) return(false);

        // initialize user -----------------------------
        User.InitGlobalSetup();
        User.InitUserFile(AMSGlobalConfig.GetActiveSiteID());
        User.InitUser();
    }

    AMSCompletion.GetSuggestions();

    return(true);
}

//------------------------------------------------------------------------------

void CCgen::Finalize(void)
{
    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << "# ==============================================================================" << endl;
    vout << "# _ams-cgen (AMS utility) terminated at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    if( ErrorSystem.IsError() || (ErrorSystem.IsAnyRecord() && Options.GetOptVerbose()) ){
        vout << low;
        ErrorSystem.PrintErrors(vout);
    }

    vout << endl;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================




