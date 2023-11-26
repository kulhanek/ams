// =============================================================================
//  AMS - Advanced Module System
// -----------------------------------------------------------------------------
//     Copyright (C) 2012 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include <Shell.hpp>
#include <stdlib.h>
#include <string.h>
#include <sstream>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CShell::GetSystemVariable(const CSmallString& name)
{
    if( name == NULL ) return("");
    return(CSmallString(getenv(name)));
}

//------------------------------------------------------------------------------

bool CShell::SetSystemVariable(const CSmallString& name,const CSmallString& value)
{
    return( setenv(name,value,1) == 0 );
}

//------------------------------------------------------------------------------

const CSmallString CShell::RemoveValue(const CSmallString& value_list,
        const CSmallString& value,
        const CSmallString& delimiter)
{
    CSmallString     tmp(value_list);
    CSmallString     newlist;
    char*             p_lval;

    if( tmp == NULL ) return(newlist);

    p_lval = strtok(tmp.GetBuffer(),delimiter);
    while( p_lval != NULL ) {
        if( (strcmp(p_lval,value) != 0) && (strlen(p_lval) != 0) ) {
            if( (newlist != NULL)&&(strlen(newlist) > 0) ) newlist += delimiter;
            newlist += p_lval;
        }
        p_lval = strtok(NULL,delimiter);
    }

    return(newlist);
}

//------------------------------------------------------------------------------

const CSmallString CShell::AppendValue(const CSmallString& value_list,
        const CSmallString& value,
        const CSmallString& delimiter)
{
    if( value == NULL ) return(value_list);

    CSmallString     newlist(value_list);

    if( (newlist != NULL) && (newlist.GetLength() > 0) ) {
        newlist += delimiter;
        newlist += value;
    } else {
        newlist = value;
    }

    return(newlist);
}

//------------------------------------------------------------------------------

const CSmallString CShell::PrependValue(const CSmallString& value_list,
        const CSmallString& value,
        const CSmallString& delimiter)
{
    if( value == NULL ) return(value_list);

    CSmallString     newlist(value);

    if( (value_list != NULL) && (value_list.GetLength() > 0) ) {
        newlist += delimiter;
        newlist += value_list;
    }

    return(newlist);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


