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

#include "CacheCmdOptions.hpp"

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CCacheCmdOptions::CCacheCmdOptions(void)
{
    SetShowMiniUsage(true);
    SetAllowProgArgs(true);
}

//------------------------------------------------------------------------------

int CCacheCmdOptions::CheckOptions(void)
{
    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

int CCacheCmdOptions::FinalizeOptions(void)
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

int CCacheCmdOptions::CheckArguments(void)
{
    if( GetNumberOfProgArgs() == 1 ) {
        Action = GetProgArg(0);
        if( GetProgArg(0) == "rebuild" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "split" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "splitmore" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "deps" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "syntax" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "allmods" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "allbuilds" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "archs" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "modes" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "rebuildall" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "syntaxall" ) return(SO_CONTINUE);
        if( (GetProgArg(0) == "getvariable") ) {
            if( IsVerbose() ) {
                if( IsError == false ) fprintf(stderr,"\n");
                fprintf(stderr,"%s: specified action '%s' requires additional arguments\n",
                        (const char*)GetProgramName(), (const char*)GetProgArg(0));
                IsError = true;
            }
            return(SO_OPTS_ERROR);
        }
        if( IsVerbose() ) {
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: specified action '%s' is not supported\n",
                    (const char*)GetProgramName(), (const char*)GetProgArg(0));
            IsError = true;
        }
        return(SO_OPTS_ERROR);
    }

    if( GetNumberOfProgArgs() == 2 ) {
        Action = GetProgArg(0);
        if( GetProgArg(0) == "getbuilds" ) return(SO_CONTINUE);
    }

    if( GetNumberOfProgArgs() == 3 ) {
        Action = GetProgArg(0);
        if( GetProgArg(0) == "getvariable" ) return(SO_CONTINUE);
    }

    if( IsVerbose() ) {
        if( IsError == false ) fprintf(stderr,"\n");
        fprintf(stderr,"%s: incorrect number of arguments\n",
                (const char*)GetProgramName());
        IsError = true;
    }
    return(SO_OPTS_ERROR);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString& CCacheCmdOptions::GetArgAction(void) const
{
    return(Action);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
