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

#include "ModuleVarOptions.hpp"

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CModuleVarOptions::CModuleVarOptions(void)
{
    SetShowMiniUsage(true);
}

//------------------------------------------------------------------------------

int CModuleVarOptions::CheckOptions(void)
{
    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

int CModuleVarOptions::FinalizeOptions(void)
{
    bool ret_opt = false;

    if( GetOptHelp() == true ) {
        PrintUsage();
        ret_opt = true;
    }

    if( GetOptVersion() == true ) {
        PrintVersion();
        ret_opt = true;
    }

    if( ret_opt == true ) {
        printf("\n");
        return(SO_EXIT);
    }

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

int CModuleVarOptions::CheckArguments(void)
{
    if( (GetArgAction() != "prepend") &&
            (GetArgAction() != "append") &&
            (GetArgAction() != "remove") ) {
        if( IsVerbose() ) {
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: specified action '%s' is not supported\n", (const char*)GetProgramName(), (const char*)GetArgAction());
            IsError = true;
        }
        return(SO_OPTS_ERROR);
    }

    if( GetArgDelimiter().GetLength() != 1 ) {
        if( IsVerbose() ) {
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: specified delimiter '%s' has to be only one character\n", (const char*)GetProgramName(), (const char*)GetArgDelimiter());
            IsError = true;
        }
        return(SO_OPTS_ERROR);
    }

    return(SO_CONTINUE);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
