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

#include "MapManipOptions.hpp"

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CMapManipOptions::CMapManipOptions(void)
{
    SetShowMiniUsage(true);
    SetAllowProgArgs(true);
}

//------------------------------------------------------------------------------

int CMapManipOptions::CheckOptions(void)
{
    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

int CMapManipOptions::FinalizeOptions(void)
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

int CMapManipOptions::CheckArguments(void)
{
    if( GetNumberOfProgArgs() == 0 ) {
        if( IsVerbose() ) {
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: incorrect number of arguments\n",
                    (const char*)GetProgramName());
            IsError = true;
        }
        return(SO_OPTS_ERROR);
    }

    Action = GetProgArg(0);

    if( GetNumberOfProgArgs() == 1 ) {
        if( GetProgArg(0) == "showallbuilds" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "showprefixes" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "aliases" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "numofundos" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "undo" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "refbuilds" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "refdocs" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "distribute" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "rmorphansites" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "rmorphanbuilds" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "updateverindexes" ) return(SO_CONTINUE);
        if( IsVerbose() ) {
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: specified action '%s' has incorrect number of arguments or it is unsupported action\n",
                    (const char*)GetProgramName(), (const char*)GetProgArg(0));
            IsError = true;
        }
        return(SO_OPTS_ERROR);
    }

    if( GetNumberOfProgArgs() == 2 ) {
        if( GetProgArg(0) == "showmap" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "rmmap" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "newverindex" ) return(SO_CONTINUE);
        if( IsVerbose() ) {
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: specified action '%s' has incorrect number of arguments or it is unsupported action\n",
                    (const char*)GetProgramName(), (const char*)GetProgArg(0));
            IsError = true;
        }
        return(SO_OPTS_ERROR);
    }

    if( GetNumberOfProgArgs() == 3 ) {
        if( GetProgArg(0) == "showbuilds" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "showautobuilds" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "bestbuild" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "isbuild" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "showsyncdeps" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "getpkgdir" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "setdefault" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "rmdefault" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "rmmodule" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "rmbuilds" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "addbuilds" ) return(SO_CONTINUE);
        if( GetProgArg(0) == "cpmap" ) return(SO_CONTINUE);

        if( IsVerbose() ) {
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: specified action '%s' has incorrect number of arguments or it is unsupported action\n",
                    (const char*)GetProgramName(), (const char*)GetProgArg(0));
            IsError = true;
        }
        return(SO_OPTS_ERROR);
    }

    if( IsVerbose() ) {
        if( IsError == false ) fprintf(stderr,"\n");
        fprintf(stderr,"%s: incorrect number of arguments orit is unsupported action\n",
                (const char*)GetProgramName());
        IsError = true;
    }
    return(SO_OPTS_ERROR);

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString& CMapManipOptions::GetArgAction(void) const
{
    return(Action);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
