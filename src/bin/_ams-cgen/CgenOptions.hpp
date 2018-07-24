#ifndef CGenOptionsH
#define CGenOptionsH
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

class CCgenOptions : public CSimpleOptions {
    public:
        // constructor - tune option setup
        CCgenOptions(void);

 // program name and description -----------------------------------------------
    CSO_PROG_NAME_BEGIN
        "_ams-cgen"
    CSO_PROG_NAME_END

    CSO_PROG_DESC_BEGIN
        "Shell completion utility."
    CSO_PROG_DESC_END

    // program arguments short description
    CSO_PROG_ARGS_SHORT_DESC_BEGIN
    "arguments"
    CSO_PROG_ARGS_SHORT_DESC_END

    // program arguments long description
    CSO_PROG_ARGS_LONG_DESC_BEGIN
    "test\n"
    CSO_PROG_ARGS_LONG_DESC_END

    CSO_PROG_VERS_BEGIN
        LibBuildVersion_AMS
    CSO_PROG_VERS_END

 // list of all options and arguments ------------------------------------------
    CSO_LIST_BEGIN
    CSO_LIST_END

    CSO_MAP_BEGIN
    CSO_MAP_END

// final operation with options ------------------------------------------------
    private:
    virtual int CheckOptions(void);
    virtual int FinalizeOptions(void);
    virtual int CheckArguments(void);
    };

//------------------------------------------------------------------------------

#endif
