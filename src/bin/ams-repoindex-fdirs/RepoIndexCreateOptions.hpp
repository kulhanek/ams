#ifndef RepoIndexCreateOptionsH
#define RepoIndexCreateOptionsH
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

#include <SimpleOptions.hpp>
#include <AMSMainHeader.hpp>

//------------------------------------------------------------------------------

class CRepoIndexCreateOptions : public CSimpleOptions {
public:
    // constructor - tune option setup
    CRepoIndexCreateOptions(void);

    // program name and description -----------------------------------------------
    CSO_PROG_NAME_BEGIN
    "ams-repoindex-fdirs"
    CSO_PROG_NAME_END

    CSO_PROG_DESC_BEGIN
    "Create the fake repository index file containing information about subdirectories in the scanned directory."
    CSO_PROG_DESC_END

    CSO_PROG_VERS_BEGIN
    LibBuildVersion_AMS
    CSO_PROG_VERS_END

    // list of all options and arguments ------------------------------------------
    CSO_LIST_BEGIN
    // args ---------------------------------
    CSO_ARG(CSmallString,ScannedDir)
    CSO_ARG(CSmallString,OutputFile)
    // options ------------------------------
    CSO_OPT(bool,FullIndex)
    CSO_OPT(bool,Help)
    CSO_OPT(bool,Version)
    CSO_OPT(bool,Verbose)
    CSO_LIST_END

    CSO_MAP_BEGIN
    //----------------------------------------------------------------------
    CSO_MAP_ARG(CSmallString,                   /* argument type */
                ScannedDir,                          /* argument name */
                NULL,                           /* default value */
                true,                           /* is argument mandatory */
                "scanneddir",                        /* parametr name */
                "name of scanned directory")   /* argument description */
    //----------------------------------------------------------------------
    CSO_MAP_ARG(CSmallString,                   /* argument type */
                OutputFile,                          /* argument name */
                NULL,                           /* default value */
                true,                           /* is argument mandatory */
                "output",                        /* parametr name */
                "name of created index file")   /* argument description */
    //----------------------------------------------------------------------
    CSO_MAP_OPT(bool,                           /* option type */
                FullIndex,                        /* option name */
                false,                          /* default value */
                false,                          /* is option mandatory */
                'f',                           /* short option name */
                "full",                      /* long option name */
                NULL,                           /* parametr name */
                "include more (local) stat data to the index")   /* option description */
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

    CSmallString    Action;
};

//------------------------------------------------------------------------------

#endif
