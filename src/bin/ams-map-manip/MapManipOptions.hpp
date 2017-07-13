#ifndef MapManipOptionsH
#define MapManipOptionsH
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

class CMapManipOptions : public CSimpleOptions {
public:
    // constructor - tune option setup
    CMapManipOptions(void);

    // program name and description -----------------------------------------------
    CSO_PROG_NAME_BEGIN
    "ams-map-manip"
    CSO_PROG_NAME_END

    CSO_PROG_DESC_BEGIN
    "<b>Manipulates with site maps."
    CSO_PROG_DESC_END

    // program arguments short description
    CSO_PROG_ARGS_SHORT_DESC_BEGIN
    "action [arguments]"
    CSO_PROG_ARGS_SHORT_DESC_END

    // program arguments long description
    CSO_PROG_ARGS_LONG_DESC_BEGIN
    "<b>Arguments:</b>\n"
    "      <b><cyan>action</cyan></b>       operation which should be performed\n\n"
    "Supported actions:\n"
    "      <green>showallbuilds</green>                - show all builds\n"
    "      <green>showprefixes</green>                 - show available build prefixes\n"
    "      <green>showbuilds</green>     prefix filter - show builds from <u>prefix</u> satisfying <u>filter</u>\n"
    "      <green>isbuild</green>        site   build  - test if <u>build</u> exists for given site\n"
    "      <green>showautobuilds</green> site   filter - show builds satisfying <u>filter</u> from <u>prefix</u>/<u>site</u>/<u>autoprefix</u>\n"
    "      <green>bestbuild</green>      site   module - show best build for <u>module</u> from <u>prefix</u>/<u>site</u>/<u>autoprefix</u>\n"
    "      <green>showsyncdeps</green>   site   build  - show sync dependencies for <u>build</u>\n"
    "      <green>deepsyncdeps</green>   site   build  - show sync dependencies for <u>build</u>\n"
    "      <green>getpkgdir</green>      site   build  - get value of AMS_PACKAGE_DIR for <u>build</u> (only one build)\n"
    "\n"
    "      <green>addbuilds</green>  site1[,...] filter  - add builds from <u>prefix</u> satisfying <u>filter</u> to <u>site1</u>, ...\n"
    "      <green>rmbuilds</green>   site1[,...] filter  - remove builds satisfying <u>filter</u> from <u>site1</u>, ...\n"
    "\n"
    "      <green>setdefault</green> site1[,...] default - set <u>default</u> module build for specified <u>site1</u>, ...\n"
    "      <green>rmdefault</green>  site1[,...] module  - remove default build of <u>module</u> from <u>site1</u>, ...\n"
    "      <green>rmmodule</green>   site1[,...] module  - remove <u>module</u> from <u>site1</u>, ...\n"
    "      <green>newverindex</green>            module  - get new version index for module\n"
    "      <green>updateverindexes</green>               - update version indexes in the entire map\n"
    "\n"
    "      <green>showmap</green>       site1[,...]  - show map for <u>site1</u>, ...\n"
    "      <green>rmmap</green>         site1[,...]  - remove map for <u>site1</u>, ...\n"
    "      <green>cpmap</green>         site1 sites2 - copy map from <u>site1</u> to <u>site2</u>\n"
    "      <green>rmorphansites</green>              - remove maps for sites that are not in AMS database\n"
    "      <green>rmorphanbuilds</green>             - remove builds that have AMS_PACKAGE_DIR set but that package directory does not exist\n"
    "      <green>refbuilds</green>                  - refactor all builds\n"
    "      <green>distribute</green>                 - distribute documentation and builds to individual site caches\n"
    "      <green>aliases</green>                    - print site aliases\n"
    "      <green>undo</green>                       - undo last map change\n"
    "      <green>numofundos</green>                 - number of map changes that can be undone\n"
    CSO_PROG_ARGS_LONG_DESC_END

    CSO_PROG_VERS_BEGIN
    LibBuildVersion_AMS
    CSO_PROG_VERS_END

    // list of all options and arguments ------------------------------------------
    CSO_LIST_BEGIN
    // options ------------------------------
    CSO_OPT(CSmallString,Prefix)
    CSO_OPT(bool,Help)
    CSO_OPT(bool,Version)
    CSO_OPT(bool,Verbose)
    CSO_LIST_END

    CSO_MAP_BEGIN
    // description of options -----------------------------------------------------
    CSO_MAP_OPT(CSmallString,                           /* option type */
                Prefix,                        /* option name */
                "",                          /* default value */
                false,                          /* is option mandatory */
                'p',                           /* short option name */
                "prefix",                      /* long option name */
                "MAME",                           /* parametr name */
                "change default prefix")   /* option description */
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
