#ifndef BundleCmdOptsH
#define BundleCmdOptsH
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

#include <SimpleOptions.hpp>
#include <AMSMainHeader.hpp>

//------------------------------------------------------------------------------

class CBundleCmdOptions : public CSimpleOptions {
public:
    // constructor - tune option setup
    CBundleCmdOptions(void);

    // program name and description -----------------------------------------------
    CSO_PROG_NAME_BEGIN
    "ams-bundle"
    CSO_PROG_NAME_END

    CSO_PROG_DESC_BEGIN
    "Utility for managing module bundles."
    CSO_PROG_DESC_END

    // program arguments short description
    CSO_PROG_ARGS_SHORT_DESC_BEGIN
    "[args]"
    CSO_PROG_ARGS_SHORT_DESC_END

    // program arguments long description
    CSO_PROG_ARGS_LONG_DESC_BEGIN
    "<green>[--force] create name maintainer contact</green>  create a new bundle in the current working directory\n"
    "<green>[info]</green>                                    print information about the bundle\n"
    "<green>[--incvers] avail</green>                         print modules in the bundle\n"
    "<green>rebuild</green>                                   rebuild the bundle cache\n"
    "<green>[--personal] index new</green>                    calculate new index for builds\n"
    "<green>[--skipremoved] [--skipadded] index diff</green>  compare new and old indexes\n"
    "<green>index commit</green>                              commit new index as old index\n"
    CSO_PROG_ARGS_LONG_DESC_END

    CSO_PROG_VERS_BEGIN
        LibBuildVersion_AMS
    CSO_PROG_VERS_END

    // list of all options and arguments ------------------------------------------
    CSO_LIST_BEGIN
    // options ------------------------------
    CSO_OPT(bool,Force)
    CSO_OPT(bool,Personal)
    CSO_OPT(bool,SkipRemovedEntries)
    CSO_OPT(bool,SkipAddedEntries)
    CSO_OPT(bool,IncludeVersions)
    CSO_OPT(bool,Help)
    CSO_OPT(bool,Version)
    CSO_OPT(bool,Verbose)
    CSO_LIST_END

    CSO_MAP_BEGIN
    // description of options -----------------------------------------------------
    CSO_MAP_OPT(bool,                           /* option type */
                Force,                        /* option name */
                false,                          /* default value */
                false,                          /* is option mandatory */
                'f',                           /* short option name */
                "force",                      /* long option name */
                NULL,                           /* parametr name */
                "create the bundle even if the root directory already exists")   /* option description */
    //----------------------------------------------------------------------
    CSO_MAP_OPT(bool,                           /* option type */
                Personal,                        /* option name */
                false,                          /* default value */
                false,                          /* is option mandatory */
                'p',                           /* short option name */
                "personal",                      /* long option name */
                NULL,                           /* parametr name */
                "treat the bundle as personal")   /* option description */
    //----------------------------------------------------------------------
    CSO_MAP_OPT(bool,                           /* option type */
                SkipRemovedEntries,                        /* option name */
                false,                          /* default value */
                false,                          /* is option mandatory */
                '\0',                           /* short option name */
                "skipremoved",                      /* long option name */
                NULL,                           /* parametr name */
                "skip removed index entries")   /* option description */
    //----------------------------------------------------------------------
    CSO_MAP_OPT(bool,                           /* option type */
                SkipAddedEntries,                        /* option name */
                false,                          /* default value */
                false,                          /* is option mandatory */
                '\0',                           /* short option name */
                "skipadded",                      /* long option name */
                NULL,                           /* parametr name */
                "skip added index entries")   /* option description */
    //----------------------------------------------------------------------
    CSO_MAP_OPT(bool,                           /* option type */
                IncludeVersions,                        /* option name */
                false,                          /* default value */
                false,                          /* is option mandatory */
                'i',                           /* short option name */
                "incvers",                      /* long option name */
                NULL,                           /* parametr name */
                "print with module versions")   /* option description */
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

    /// return action name
    const CSmallString& GetArgAction(void) const;

    // final operation with options ------------------------------------------------
private:
    virtual int CheckOptions(void);
    virtual int FinalizeOptions(void);
    virtual int CheckArguments(void);

    CSmallString    Action;
};

//------------------------------------------------------------------------------

#endif
