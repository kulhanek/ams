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

#include "SiteCmdOptions.hpp"
#include <SiteController.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CSiteCmdOptions::CSiteCmdOptions(void)
{
    SetShowMiniUsage(true);
    SetAllowProgArgs(true);
}

//------------------------------------------------------------------------------

int CSiteCmdOptions::CheckOptions(void)
{
    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

int CSiteCmdOptions::FinalizeOptions(void)
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

int CSiteCmdOptions::CheckArguments(void)
{
    if( GetNumberOfProgArgs() == 0 ) {
        Action = "avail";
        Site = SiteController.GetActiveSite();
        return(SO_CONTINUE);
    }

    if( GetNumberOfProgArgs() == 1 ) {
        Action = GetProgArg(0);
        if( GetProgArg(0) == "activate" ) {
            if( IsVerbose() ) {
                if( IsError == false ) fprintf(stderr,"\n");
                fprintf(stderr,"%s: specified action '%s' requires the specification of site\n",
                        (const char*)GetProgramName(), (const char*)GetProgArg(0));
                IsError = true;
            }
            return(SO_OPTS_ERROR);
        }
        Site = SiteController.GetActiveSite();
        return(SO_CONTINUE);
    }

    if( GetNumberOfProgArgs() == 2 ) {
        Action = GetProgArg(0);
        if( GetProgArg(0) == "avail" ) {
            if( IsVerbose() ) {
                if( IsError == false ) fprintf(stderr,"\n");
                fprintf(stderr,"%s: action 'avail' cannot be specified with site name\n",
                        (const char*)GetProgramName());
                IsError = true;
            }
            return(SO_OPTS_ERROR);
        }
        Site = GetProgArg(1);
        return(SO_CONTINUE);
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

const CSmallString& CSiteCmdOptions::GetArgAction(void) const
{
    return(Action);
}

//------------------------------------------------------------------------------

const CSmallString& CSiteCmdOptions::GetArgSite(void) const
{
    return(Site);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
