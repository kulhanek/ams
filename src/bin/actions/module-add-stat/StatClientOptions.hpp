#ifndef StatClientOptionsH
#define StatClientOptionsH
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

#include <AMSMainHeader.hpp>
#include <SimpleOptions.hpp>

//------------------------------------------------------------------------------

class CStatClientOptions : public CSimpleOptions {
public:
    // constructor - tune option setup
    CStatClientOptions(void);

    // program name and description -----------------------------------------------
    CSO_PROG_NAME_BEGIN
    "module-add-stat"
    CSO_PROG_NAME_END

    CSO_PROG_DESC_BEGIN
    "It sends a UDP datagram to the ams-isoftstat server providing information about module add action. "
    "Collected information are site id (from AMS_SITE variable), "
    "build name (from build argument), user name (from USER/LOGNAME variable), "
    "date and time (local current time), and status flag."
    CSO_PROG_DESC_END

    CSO_PROG_VERS_BEGIN
    LibBuildVersion_AMS
    CSO_PROG_VERS_END

    // list of all options and arguments ------------------------------------------
    CSO_LIST_BEGIN
    // arguments ------------------------------
    CSO_ARG(CSmallString,ServerName)
    CSO_ARG(CSmallString,Build)
    CSO_ARG(CSmallString,Bundle)
    CSO_ARG(int,Flags)
    CSO_ARG(CSmallString,Verbosity)
    // options ------------------------------
    CSO_OPT(int,Port)
    CSO_OPT(bool,Help)
    CSO_OPT(bool,Version)
    CSO_OPT(bool,Verbose)
    CSO_LIST_END

    CSO_MAP_BEGIN
    // description of arguments ---------------------------------------------------
    CSO_MAP_ARG(CSmallString,                   /* argument type */
                ServerName,                          /* argument name */
                NULL,                           /* default value */
                true,                           /* is argument mandatory */
                "servername",                        /* parametr name */
                "IP address or name of server, which collects statististics\n")   /* argument description */
    //----------------------------------------------------------------------
    CSO_MAP_ARG(CSmallString,                   /* argument type */
                Build,                          /* argument name */
                NULL,                           /* default value */
                true,                           /* is argument mandatory */
                "build",                        /* parametr name */
                "module build, which is added\n")   /* argument description */
    //----------------------------------------------------------------------
    CSO_MAP_ARG(CSmallString,                   /* argument type */
                Bundle,                          /* argument name */
                NULL,                           /* default value */
                true,                           /* is argument mandatory */
                "bundle",                        /* parametr name */
                "name of bundle\n")   /* argument description */
    //----------------------------------------------------------------------
    CSO_MAP_ARG(int,                   /* argument type */
                Flags,                          /* argument name */
                0,                           /* default value */
                true,                           /* is argument mandatory */
                "flags",                        /* parametr name */
                "flags for module build")   /* argument description */
    //----------------------------------------------------------------------
    CSO_MAP_ARG(CSmallString,                   /* argument type */
                Verbosity,                          /* argument name */
                NULL,                           /* default value */
                true,                           /* is argument mandatory */
                "verbosity",                        /* parametr name */
                "module print info verbosity\n")   /* argument description */
    // description of options -----------------------------------------------------
    CSO_MAP_OPT(int,                            /* option type */
                Port,                           /* option name */
                32598,                          /* default value */
                false,                          /* is option mandatory */
                'p',                           /* short option name */
                "port",                      /* long option name */
                "PORT",                           /* parametr name */
                "port number for communication (32597 by default)")   /* option description */
    //----------------------------------------------------------------------
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

    // final operation with options ------------------------------------------------
private:
    virtual int CheckOptions(void);
    virtual int FinalizeOptions(void);
    virtual int CheckArguments(void);
};

//------------------------------------------------------------------------------

#endif
