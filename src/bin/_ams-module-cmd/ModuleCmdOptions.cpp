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

#include "ModuleCmdOptions.hpp"

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CModuleCmdOptions::CModuleCmdOptions(void)
{
    SetShowMiniUsage(true);
    SetAllowProgArgs(true);
}

//------------------------------------------------------------------------------

int CModuleCmdOptions::CheckOptions(void)
{
    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

int CModuleCmdOptions::FinalizeOptions(void)
{
    bool ret_opt = false;

    if( GetOptHelp() == true ) {
        PrintUsage(stderr);
        ret_opt = true;
    }

    if( GetOptVersion() == true ) {
        PrintVersion(stderr);
        ret_opt = true;
    }

    if( (GetOptColors() != "auto") && (GetOptColors() != "always") ) {
        if( IsError == false ) fprintf(stderr,"\n");
        fprintf(stderr,"%s: incorrect colors mode '%s'. It should be 'always'' or 'auto'.\n",
                (const char*)GetProgramName(),(const char*)GetOptColors());
        IsError = true;
    }

    if( ret_opt == true ) {
        fprintf(stderr,"\n");
        return(SO_EXIT);
    }

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

int CModuleCmdOptions::CheckArguments(void)
{
    if( GetNumberOfProgArgs() == 0 ) {
        Action = "avail_no_system";
        return(SO_CONTINUE);
    }

    if( GetNumberOfProgArgs() == 1 ) {
        Action = GetProgArg(0);
        if( GetProgArg(0) == "avail" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "active" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "exported" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "list" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "reactivate" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "autoload" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "purge" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "syshdr" ) return(SO_CONTINUE);
        if( (GetProgArg(0) == "add") ||
                (GetProgArg(0) == "remove") ||
                (GetProgArg(0) == "versions") ||
                (GetProgArg(0) == "builds") ||
                (GetProgArg(0) == "disp") ||
                (GetProgArg(0) == "isactive") ) {
            if( IsVerbose() ) {
                if( IsError == false ) fprintf(stderr,"\n");
                fprintf(stderr,"%s: specified action '%s' requires the specification of module\n",
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

    if( GetNumberOfProgArgs() > 1 ) {
        Action = GetProgArg(0);
        if( GetProgArg(0) == "add" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "activate" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "remove" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "versions" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "builds" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "disp" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "isactive" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "getactver" ) {
            if( GetNumberOfProgArgs() != 2 ) {
                if( IsVerbose() ) {
                    if( IsError == false ) fprintf(stderr,"\n");
                    fprintf(stderr,"%s: specified action '%s' requires only one module name\n",
                            (const char*)GetProgramName(), (const char*)GetProgArg(0));
                    IsError = true;
                    return(SO_OPTS_ERROR);
                }
            }
            return(SO_CONTINUE);
        }
        if( (GetProgArg(0) == "avail") ||
                (GetProgArg(0) == "active") ||
                (GetProgArg(0) == "exported") ||
                (GetProgArg(0) == "list") ||
                (GetProgArg(0) == "reactivate") ||
                (GetProgArg(0) == "purge") ) {
            if( IsVerbose() ) {
                if( IsError == false ) fprintf(stderr,"\n");
                fprintf(stderr,"%s: specified action '%s' does not require module name specification\n",
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

const CSmallString& CModuleCmdOptions::GetArgAction(void) const
{
    return(Action);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
