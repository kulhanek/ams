// =============================================================================
// AMS - Advanced Module System
// -----------------------------------------------------------------------------
//    Copyright (C) 2016      Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include "RepoIndexDiff.hpp"
#include <ErrorSystem.hpp>
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>

//------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------

MAIN_ENTRY(CRepoIndexDiff)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRepoIndexDiff::CRepoIndexDiff(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CRepoIndexDiff::Init(int argc, char* argv[])
{
    // encode program options
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
    vout << "# ams-index-diff (AMS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    vout << high;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CRepoIndexDiff::Run(void)
{
    // load two indexes
    vout << endl;
    vout << "# Loading indexes ..." << endl;
    vout << "  > Old index = " << Options.GetArgOldIndexName() << endl;
    if( OldIndex.LoadIndex(Options.GetArgOldIndexName()) == false ){
        return(false);
    }
    vout << "  > New index = " << Options.GetArgNewIndexName() << endl;
    if( NewIndex.LoadIndex(Options.GetArgNewIndexName()) == false ){
        return(false);
    }

    vout << endl;
    vout << "# Diffing two indexes ..." << endl;

    vout << low;
    NewIndex.Diff(OldIndex,vout,Options.GetOptSkipRemovedEntries(),
                  Options.GetOptSkipAddedEntries(),Options.GetOptVerbose());

    return(true);
}

//------------------------------------------------------------------------------

void CRepoIndexDiff::Finalize(void)
{
    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << endl;
    vout << "# ==============================================================================" << endl;
    vout << "# ams-index-diff (AMS utility) terminated at " << dt.GetSDateAndTime() << endl;
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


