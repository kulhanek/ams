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

#include "ModuleVar.hpp"
#include <ErrorSystem.hpp>
#include <Shell.hpp>
#include <ostream>

using namespace std;

//------------------------------------------------------------------------------

MAIN_ENTRY(CModuleVar)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CModuleVar::Init(int argc, char* argv[])
{
    // encode program options, all check procedures are done inside of CABFIntOpts
    int result = Options.ParseCmdLine(argc,argv);

    // should we exit or was it error?
    if( result != SO_CONTINUE ) return(result);

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CModuleVar::Run(void)
{
    CSmallString final_value;

    if( Options.GetArgAction() == "prepend" ) {
        final_value = CShell::RemoveValue(Options.GetArgVariable(),
                                          Options.GetArgValue(),
                                          Options.GetArgDelimiter());
        final_value = CShell::PrependValue(final_value,
                                           Options.GetArgValue(),
                                           Options.GetArgDelimiter());
    } else if( Options.GetArgAction() == "append" ) {
        final_value = CShell::RemoveValue(Options.GetArgVariable(),
                                          Options.GetArgValue(),
                                          Options.GetArgDelimiter());
        final_value = CShell::AppendValue(final_value,
                                          Options.GetArgValue(),
                                          Options.GetArgDelimiter());
    } else if( Options.GetArgAction() == "remove" ) {
        final_value = CShell::RemoveValue(Options.GetArgVariable(),
                                          Options.GetArgValue(),
                                          Options.GetArgDelimiter());
    } else {
        CSmallString error;
        error << "action '" << Options.GetArgAction() << "' is not supported!";
        ES_ERROR(error);
        return(false);
    }

    cout << final_value;
    return(true);
}

//------------------------------------------------------------------------------

void CModuleVar::Finalize(void)
{
    if( Options.GetOptVerbose() || ErrorSystem.IsError() ) {
        ErrorSystem.PrintErrors();
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
