// =============================================================================
//  AMS - Advanced Module System
// -----------------------------------------------------------------------------
//     Copyright (C) 2012 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2011 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2004,2005,2008,2010 Petr Kulhanek (kulhanek@chemi.muni.cz)
//
//     This library is free software; you can redistribute it and/or
//     modify it under the terms of the GNU Lesser General Public
//     License as published by the Free Software Foundation; either
//     version 2.1 of the License, or (at your option) any later version.
//
//     This library is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//     Lesser General Public License for more details.
//
//     You should have received a copy of the GNU Lesser General Public
//     License along with this library; if not, write to the Free Software
//     Foundation, Inc., 51 Franklin Street, Fifth Floor,
//     Boston, MA  02110-1301  USA
// =============================================================================

#include <AmsUUID.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CAmsUUID::CAmsUUID(void)
{
}

// -----------------------------------------------------------------------------

CAmsUUID::CAmsUUID(const CSmallString& extuuid)
{
    LoadFromString(extuuid);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

// {class_name:xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}

bool CAmsUUID::LoadFromString(const CSmallString& string)
{
    int namelength=0;

    CSmallString tmp(string);

    for(unsigned int i=1; i<string.GetLength(); i++) {
        if( tmp.GetBuffer()[i] == ':'  ) {
            namelength = i-1;
            break;
        }
    }

    if(namelength == 0)return(false);
    if( ClassName.SetLength(namelength) == false) return(false);

    ClassName = string.GetSubString(1,namelength);
    return(SetFromStringForm(string.GetSubString(namelength+2,string.GetLength()-3-namelength)));
}

// -----------------------------------------------------------------------------

const CSmallString CAmsUUID::GetFullStringForm(void) const
{
    CSmallString string;
    string = "{" + ClassName + ":" + GetStringForm() + "}";
    return(string);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString& CAmsUUID::GetClassName(void) const
{
    return(ClassName);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
