#ifndef SiteCmdOptsH
#define SiteCmdOptsH
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

#include <SimpleOptions.hpp>
#include <AMSMainHeader.hpp>

//------------------------------------------------------------------------------

class CSiteCmdOptions : public CSimpleOptions {
public:
    // constructor - tune option setup
    CSiteCmdOptions(void);

    // program name and description -----------------------------------------------
    CSO_PROG_NAME_BEGIN
    "site"
    CSO_PROG_NAME_END

    CSO_PROG_DESC_BEGIN
    "Activate particular site or print details about available or active sites."
    CSO_PROG_DESC_END

    // program arguments short description
    CSO_PROG_ARGS_SHORT_DESC_BEGIN
    "[action] [site]"
    CSO_PROG_ARGS_SHORT_DESC_END

    // program arguments long description
    CSO_PROG_ARGS_LONG_DESC_BEGIN
    "<b>Arguments:</b>\n"
    "   <b><cyan>action</cyan></b>      operation, which should be performed (avail by default)\n"
    "   <b><cyan>site</cyan></b>        site on which the action is performed\n"
    "\n"
    "Supported actions:\n"
    "   <green>activate</green>    activate site\n"
    "   <green>avail</green>       list available sites\n"
    "   <green>info</green>        info about site\n"
    "   <green>disp</green>        deeper info about site\n"
    "   <green>active</green>      the name of active site\n"
    "   <green>isactive</green>    return zero if the site is active\n"
    "   <green>isallowed</green>   return zero if the site can be activated on the host\n"
    "   <green>id</green>          return site id\n"
    "   <green>listamods</green>   list autoloaded modules\n"
    CSO_PROG_ARGS_LONG_DESC_END

    CSO_PROG_VERS_BEGIN
        LibBuildVersion_AMS
    CSO_PROG_VERS_END

    // list of all options and arguments ------------------------------------------
    CSO_LIST_BEGIN
    // options ------------------------------
    CSO_OPT(bool,All)
    CSO_OPT(CSmallString,Host)
    CSO_OPT(CSmallString,User)
    CSO_OPT(bool,Help)
    CSO_OPT(bool,Version)
    CSO_OPT(bool,Verbose)
    CSO_LIST_END

    CSO_MAP_BEGIN
    // description of options -----------------------------------------------------
    CSO_MAP_OPT(bool,                           /* option type */
                Verbose,                        /* option name */
                false,                          /* default value */
                false,                          /* is option mandatory */
                'v',                           /* short option name */
                "verbose",                      /* long option name */
                NULL,                           /* parametr name */
                "increase output verbosity")   /* option description */
    //----------------------------------------------------------------------
    CSO_MAP_OPT(bool,                           /* option type */
                All,                        /* option name */
                false,                          /* default value */
                false,                          /* is option mandatory */
                'a',                           /* short option name */
                "all",                      /* long option name */
                NULL,                           /* parametr name */
                "print all sites with 'avail' action")   /* option description */
    //----------------------------------------------------------------------
    CSO_MAP_OPT(CSmallString,                           /* option type */
                Host,                        /* option name */
                "",                          /* default value */
                false,                          /* is option mandatory */
                '\0',                           /* short option name */
                "host",                      /* long option name */
                "HOSTNAME",                           /* parametr name */
                "override the name of hostname taken from HOSTNANE system variable")   /* option description */
    //----------------------------------------------------------------------
    CSO_MAP_OPT(CSmallString,                           /* option type */
                User,                        /* option name */
                "",                          /* default value */
                false,                          /* is option mandatory */
                '\0',                           /* short option name */
                "user",                      /* long option name */
                "NAME",                           /* parametr name */
                "override the name of current user")   /* option description */
    //----------------------------------------------------------------------
    CSO_MAP_OPT(bool,                           /* option type */
                Version,                        /* option name */
                false,                          /* default value */
                false,                          /* is option mandatory */
                '\0',                           /* short option name */
                "version",                      /* long option name */
                NULL,                           /* parametr name */
                "output version information and exit")   /* option description */
    //----------------------------------------------------------------------
    CSO_MAP_OPT(bool,                           /* option type */
                Help,                        /* option name */
                false,                          /* default value */
                false,                          /* is option mandatory */
                'h',                           /* short option name */
                "help",                      /* long option name */
                NULL,                           /* parametr name */
                "display this help and exit")   /* option description */
    CSO_MAP_END

    /// return action name
    const CSmallString& GetArgAction(void) const;

    /// return site name
    const CSmallString& GetArgSite(void) const;


    // final operation with options ------------------------------------------------
private:
    virtual int CheckOptions(void);
    virtual int FinalizeOptions(void);
    virtual int CheckArguments(void);

    CSmallString    Action;
    CSmallString    Site;
};

//------------------------------------------------------------------------------

#endif
