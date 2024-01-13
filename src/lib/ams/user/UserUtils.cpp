// =============================================================================
//  AMS - Advanced Module System
// -----------------------------------------------------------------------------
//     Copyright (C) 2023 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include <UserUtils.hpp>
#include <ErrorSystem.hpp>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <ShellProcessor.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CUserUtils::GetUserName(void)
{
    uid_t uid = geteuid();
    struct passwd *pw = getpwuid(uid);
    if( pw ) {
        return(pw->pw_name);
    }
    return("");
}

//------------------------------------------------------------------------------

gid_t CUserUtils::GetGroupID(const CSmallString& name,bool trynobody)
{
    struct group *p_grp = getgrnam(name);
    if( p_grp == NULL ){
        if( trynobody ){
            CSmallString error;
            error << "no gid for '" << name << "' - trying to use nogroup as bypass";
            ES_ERROR(error);
            p_grp = getgrnam("nogroup");
        }
        if( p_grp == NULL ) return(-1);
    }
    return(p_grp->gr_gid);
}

//------------------------------------------------------------------------------

mode_t CUserUtils::GetUMaskMode(void)
{
    mode_t mumask = 0;
    if(ShellProcessor.GetCurrentUMask() == "unset" ){
        mumask = umask(mumask); // get umask - it destroys current setup
        umask(mumask); // restore umask
    } else {
        mumask = GetUMaskMode(ShellProcessor.GetCurrentUMask());
    }
    return(mumask);
}

//------------------------------------------------------------------------------

const CSmallString CUserUtils::GetUMask(void)
{
    mode_t mumask = GetUMaskMode();
    return(CUserUtils::GetUMask(mumask));
}

//------------------------------------------------------------------------------

const CSmallString CUserUtils::GetUMaskPermissions(void)
{
    mode_t mumask = 0;
    if(ShellProcessor.GetCurrentUMask() == "unset" ){
        mumask = umask(mumask); // get umask - it destroys current setup
        umask(mumask); // restore umask
    } else {
        mumask = GetUMaskMode(ShellProcessor.GetCurrentUMask());
    }
    return(CUserUtils::GetUMaskPermissions(mumask));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CUserUtils::GetUMask(mode_t mumask)
{
   std::stringstream str;
   char c1 = (mumask & 0007) + '0';
   char c2 = ((mumask & 0070) >> 3) + '0';
   char c3 = ((mumask & 0700) >> 6) + '0';
   str << c3 << c2 << c1;
   return(str.str());
}

//------------------------------------------------------------------------------

mode_t CUserUtils::GetUMaskMode(const CSmallString& smask)
{
    if( smask.GetLength() < 3 ) return(0777);
    mode_t mode = ((smask[0]-'0') << 6) | ((smask[1]-'0') << 3) | (smask[2]-'0');
    return(mode);
}

//------------------------------------------------------------------------------

const CSmallString CUserUtils::GetUMaskPermissions(mode_t mumask)
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

