// =============================================================================
// ABS - Advanced Batch System
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

#include "Root.hpp"
#include "prefix.h"
#include <ErrorSystem.hpp>
#include <ostream>

using namespace std;

//------------------------------------------------------------------------------

MAIN_ENTRY(CRoot)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRoot::CRoot(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CRoot::Init(int argc,char* argv[])
{
    // encode program options
    int result = Options.ParseCmdLine(argc,argv);
    return(result);
}

//------------------------------------------------------------------------------

bool CRoot::Run(void)
{
    cout << PREFIX << endl;
    return(true);
}

//------------------------------------------------------------------------------

void CRoot::Finalize(void)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

