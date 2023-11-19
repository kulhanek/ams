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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
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

const CSmallString CShell::GetUserName(void)
{
    uid_t uid = geteuid();
    struct passwd *pw = getpwuid(uid);
    if( pw ) {
        return(pw->pw_name);
    }
    return("");
}

//------------------------------------------------------------------------------

const CSmallString CShell::GetUMask(void)
{
   mode_t mumask = 0;
   mumask = umask(mumask); // get umask - it destroys current setup
   umask(mumask); // restore umask
   return(CShell::GetUMask(mumask));
}

//------------------------------------------------------------------------------

const CSmallString CShell::GetUMaskPermissions(void)
{
    mode_t mumask = 0;
    mumask = umask(mumask); // get umask - it destroys current setup
    umask(mumask); // restore umask
    return(CShell::GetUMaskPermissions(mumask));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CShell::GetUMask(mode_t mumask)
{
   std::stringstream str;
   char c1 = (mumask & 0007) + '0';
   char c2 = ((mumask & 0070) >> 3) + '0';
   char c3 = ((mumask & 0700) >> 6) + '0';
   str << c3 << c2 << c1;
   return(str.str());
}

//------------------------------------------------------------------------------

mode_t CShell::GetUMaskMode(const CSmallString& smask)
{
    if( smask.GetLength() < 3 ) return(0777);
    mode_t mode = ((smask[0]-'0') << 6) | ((smask[1]-'0') << 3) | (smask[2]-'0');
    return(mode);
}

//------------------------------------------------------------------------------

const CSmallString CShell::GetUMaskPermissions(mode_t mumask)
{
    std::stringstream str;
    char c1 = (mumask & 0007);
    char c2 = ((mumask & 0070) >> 3);
    char c3 = ((mumask & 0700) >> 6);

    str << "files: ";
    if( (c3 & 04) == 0 ) str << "r"; else str << "-";
    if( (c3 & 02) == 0 ) str << "w"; else str << "-";
    str << "-";
    if( (c2 & 04) == 0 ) str << "r"; else str << "-";
    if( (c2 & 02) == 0 ) str << "w"; else str << "-";
    str << "-";
    if( (c1 & 04) == 0 ) str << "r"; else str << "-";
    if( (c1 & 02) == 0 ) str << "w"; else str << "-";
    str << "-";

    str << " dirs: ";
    if( (c3 & 04) == 0 ) str << "r"; else str << "-";
    if( (c3 & 02) == 0 ) str << "w"; else str << "-";
    if( (c3 & 01) == 0 ) str << "x"; else str << "-";
    if( (c2 & 04) == 0 ) str << "r"; else str << "-";
    if( (c2 & 02) == 0 ) str << "w"; else str << "-";
    if( (c2 & 01) == 0 ) str << "x"; else str << "-";
    if( (c1 & 04) == 0 ) str << "r"; else str << "-";
    if( (c1 & 02) == 0 ) str << "w"; else str << "-";
    if( (c1 & 01) == 0 ) str << "x"; else str << "-";

    return(str.str());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


