#ifndef ModuleCmdOptionsH
#define ModuleCmdOptionsH
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

class CModuleCmdOptions : public CSimpleOptions {
public:
    // constructor - tune option setup
    CModuleCmdOptions(void);

    // program name and description -----------------------------------------------
    CSO_PROG_NAME_BEGIN
    "module"
    CSO_PROG_NAME_END

    CSO_PROG_DESC_BEGIN
    "Activate/deactivate particular software package modules. Print available software packages, versions or builds."
    CSO_PROG_DESC_END

    // program arguments short description
    CSO_PROG_ARGS_SHORT_DESC_BEGIN
    "[action] [module1 [module2] ... ]"
    CSO_PROG_ARGS_SHORT_DESC_END

    // program arguments long description
    CSO_PROG_ARGS_LONG_DESC_BEGIN
    "<b>Arguments:</b>\n"
    "   <b><cyan>action</cyan></b>          operation, which should be performed (avail by default)\n"
    "   <b><cyan>module</cyan></b>          module name\n"
    "\n"
    "Executive actions:\n"
    "   <green><b>add</green>             activate modules</b>\n"
    "   <green><b>remove</green>          deactivate modules</b>\n"
    "   <green>activate</green>        activate modules but do not export them\n"
    "   <green>reactivate</green>      reactivate all active modules\n"
    "   <green>autoload</green>        activate autoloaded modules\n"
    "   <green>purge</green>           deactivate all active modules\n"
    "\n"
    "Informative actions:\n"
    "   <green><b>avail</green>           list available modules</b>\n"
    "   <green><b>versions</green>        show available versions of modules</b>\n"
    "   <green><b>help</green>            show help for modules if present</b>\n"
    "   <green>exported</green>        list exported modules\n"
    "   <green>active</green>          list active modules\n"
    "   <green>list</green>            list exported and active modules\n"
    "   <green>builds</green>          show available builds of a module\n"
    "   <green>disp</green>            show how module activation influences shell environment\n"
    "   <green>isactive</green>        return zero if all modules in the argument list are active\n"
    "   <green>getactmod</green>       return name:version if the module is active\n"
    "   <green>getactver</green>       return module version if the module is active\n"
    "   <green>syshdr</green>          print header for system modules\n"
    CSO_PROG_ARGS_LONG_DESC_END

    CSO_PROG_VERS_BEGIN
    LibBuildVersion_AMS
    CSO_PROG_VERS_END

    // list of all options and arguments ------------------------------------------
    CSO_LIST_BEGIN
    // options ------------------------------
    CSO_OPT(CSmallString,Colors)
    CSO_OPT(bool,ReExported)
    CSO_OPT(bool,System)
    CSO_OPT(bool,Help)
    CSO_OPT(bool,Version)
    CSO_OPT(bool,Verbose)
    CSO_LIST_END

    CSO_MAP_BEGIN
    // description of options -----------------------------------------------------
    CSO_MAP_OPT(CSmallString,                           /* option type */
                Colors,                        /* option name */
                "auto",                          /* default value */
                false,                          /* is option mandatory */
                '\0',                           /* short option name */
                "colors",                      /* long option name */
                "MODE",                           /* parametr name */
                "control colored output, allowed values are: auto, always")   /* option description */
    CSO_MAP_OPT(bool,                           /* option type */
                ReExported,                        /* option name */
                false,                          /* default value */
                false,                          /* is option mandatory */
                '\0',                           /* short option name */
                "reexported",                      /* long option name */
                NULL,                           /* parametr name */
                "set reexported flag")   /* option description */
    CSO_MAP_OPT(bool,                           /* option type */
                System,                        /* option name */
                false,                          /* default value */
                false,                          /* is option mandatory */
                '\0',                           /* short option name */
                "system",                      /* long option name */
                NULL,                           /* parametr name */
                "set system flag")   /* option description */
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
