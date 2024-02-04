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

#include "BundleCmdOptions.hpp"

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CBundleCmdOptions::CBundleCmdOptions(void)
{
    SetShowMiniUsage(true);
    SetAllowProgArgs(true);
}

//------------------------------------------------------------------------------

int CBundleCmdOptions::CheckOptions(void)
{
    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

int CBundleCmdOptions::FinalizeOptions(void)
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

    if( ret_opt == true ) {
        fprintf(stderr,"\n");
        return(SO_EXIT);
    }

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

int CBundleCmdOptions::CheckArguments(void)
{
    if( GetNumberOfProgArgs() == 0 ) {
        Action = "info";
        return(SO_CONTINUE);
    }

    if( GetNumberOfProgArgs() == 1 ) {
        Action = GetProgArg(0);

        if( (Action == "info") || (Action == "avail") || (Action == "rebuild") ||
            (Action == "dirname") || (Action == "rootpath") || (Action == "dpkg-deps") ) {
            return(SO_CONTINUE);
        }

        if( Action == "create" ) {
            if( IsVerbose() ) {
                if( IsError == false ) fprintf(stderr,"\n");
                fprintf(stderr,"%s: specified action '%s' requires the specification of bundle name\n",
                        (const char*)GetProgramName(), (const char*)GetProgArg(0));
                IsError = true;
            }
            return(SO_OPTS_ERROR);
        }
        if( GetProgArg(0) == "index" ) {
            if( IsVerbose() ) {
                if( IsError == false ) fprintf(stderr,"\n");
                fprintf(stderr,"%s: specified action '%s' requires the specification of sub-action\n",
                        (const char*)GetProgramName(), (const char*)GetProgArg(0));
                IsError = true;
            }
            return(SO_OPTS_ERROR);
        }
        if( GetProgArg(0) == "newverindex" ) {
            if( IsVerbose() ) {
                if( IsError == false ) fprintf(stderr,"\n");
                fprintf(stderr,"%s: specified action '%s' requires the specification of the build\n",
                        (const char*)GetProgramName(), (const char*)GetProgArg(0));
                IsError = true;
            }
            return(SO_OPTS_ERROR);
        }
        if( GetProgArg(0) == "sync" ) {
            if( IsVerbose() ) {
                if( IsError == false ) fprintf(stderr,"\n");
                fprintf(stderr,"%s: specified action '%s' requires the specification of the profile\n",
                        (const char*)GetProgramName(), (const char*)GetProgArg(0));
                IsError = true;
            }
            return(SO_OPTS_ERROR);
        }
    }

    if( GetNumberOfProgArgs() == 2 ) {
        Action = GetProgArg(0);

        if( Action == "index" ) return(SO_CONTINUE);
        if( Action == "newverindex" ) return(SO_CONTINUE);
        if( Action == "sync" ) return(SO_CONTINUE);
        if( Action == "sources" ) return(SO_CONTINUE);
        if( Action == "dirlist" ) return(SO_CONTINUE);

        if( (Action == "info") || (Action == "avail") ) {
            if( IsVerbose() ) {
                if( IsError == false ) fprintf(stderr,"\n");
                fprintf(stderr,"%s: too many arguments for action '%s'\n",
                        (const char*)GetProgramName(), (const char*)GetProgArg(0));
                IsError = true;
            }
            return(SO_OPTS_ERROR);
        }
    }

    if( GetNumberOfProgArgs() == 4 ) {
        Action = GetProgArg(0);

        if( Action == "create" ) return(SO_CONTINUE);
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

const CSmallString& CBundleCmdOptions::GetArgAction(void) const
{
    return(Action);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
