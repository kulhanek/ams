#ifndef SetEnvOptionsH
#define SetEnvOptionsH
// =============================================================================
// AMS
// -----------------------------------------------------------------------------
//    Copyright (C) 2012      Petr Kulhanek, kulhanek@chemi.muni.cz
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

class CSetEnvOptions : public CSimpleOptions {
public:
    // constructor - tune option setup
    CSetEnvOptions(void);

    // program name and description -----------------------------------------------
    CSO_PROG_NAME_BEGIN
    "ams-setenv"
    CSO_PROG_NAME_END

    CSO_PROG_DESC_BEGIN
    "Setup local environment for given number of CPUs and GPUs. "
    "Requested number of resources cannot exceed available resources on the local host. "
    "It updates AMS_NCPUS, AMS_NGPUS, AMS_NNODES, AMS_NODEFILE and AMS_GPUFILE "
    "as well as INF_NCPUS, INF_NGPUS, INF_NNODES, INF_NODEFILE and INF_GPUFILE variables "
    "and generates files for AMS_NODEFILE, AMS_GPUFILE and INF_NODEFILE, INF_GPUFILE."
    CSO_PROG_DESC_END

    CSO_PROG_VERS_BEGIN
    LibBuildVersion_AMS
    CSO_PROG_VERS_END

    // list of all options and arguments ------------------------------------------
    CSO_LIST_BEGIN
    // arguments ----------------------------

    // options ------------------------------
    CSO_OPT(int,NCPUs)
    CSO_OPT(int,NGPUs)
    CSO_OPT(bool,Help)
    CSO_OPT(bool,Version)
    CSO_OPT(bool,Verbose)
    CSO_LIST_END

    CSO_MAP_BEGIN
    // description of arguments ---------------------------------------------------
    CSO_MAP_OPT(int,                            /* option type */
                NCPUs,                          /* option name */
                1,                              /* default value */
                true,                           /* is argument mandatory */
                'c',                            /* short option name */
                "ncpus",                        /* long option name */
                "INT",                        /* parametr name */
                "requested number of CPUs")     /* option description */
    //----------------------------------------------------------------------
    CSO_MAP_OPT(int,                            /* option type */
                NGPUs,                          /* option name */
                0,                              /* default value */
                false,                          /* is argument mandatory */
                'g',                            /* short option name */
                "ngpus",                        /* long option name */
                "INT",                          /* parametr name */
                "requested number of GPUs")     /* option description */
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
