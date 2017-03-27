#ifndef UserH
#define UserH
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

#include <AMSMainHeader.hpp>
#include <sys/types.h>
#include <SmallString.hpp>
#include <vector>
#include <list>
#include <iostream>
#include <XMLDocument.hpp>
#include <VerboseStr.hpp>

// -----------------------------------------------------------------------------

class AMS_PACKAGE CUser {
public:
// constructor -----------------------------------------------------------------
        CUser(void);

// executive methods -----------------------------------------------------------
    /// load user configuration file
    void InitUserFile(const CSmallString& site_sid);

    /// init class by current userid
    bool InitGlobalSetup(void);

    /// init current user
    void InitUser(void);

    /// clear all setup
    void ClearAll(void);

    /// set user name
    void SetUserName(const CSmallString& name);

// informational methods -------------------------------------------------------
    /// get user name
    const CSmallString& GetName(void) const;

    /// get real group
    const CSmallString& GetRGroup(void) const;

    /// get effective group
    const CSmallString& GetEGroup(void) const;

    /// get umask
    const CSmallString GetUMask(void) const;

    /// get umask permission bits
    const CSmallString GetUMaskPermissions(void) const;

    /// get all user groups separated by comma
    const CSmallString GetGroups(void);

    /// check group
    bool IsInGroup(const CSmallString& grpname);

    /// print info about user
    void PrintUserDetailedInfo(CVerboseStr& vout);

    /// print user compact info
    void PrintUserInfo(CVerboseStr& vout);

    /// is user name provided?
    bool IsUserNameProvided(void);

    /// print all posix groups
    void PrintPosixGroups(void);

    /// is posix group
    bool IsPosixGroup(const CSmallString& group);

    /// return user requested umask
    const CSmallString GetRequestedUserUMask(void);

    /// get user ID
    uid_t GetUserID(void) const;

    /// get group ID
    gid_t GetGroupID(const CSmallString& name);

// section of private data -----------------------------------------------------
private:
    CSmallString                SiteSID;
    CSmallString                SiteConfigInUse;
    CXMLDocument                Users;                  // user configuration
    // user identification
    uid_t                       UID;
    gid_t                       RGID;
    gid_t                       EGID;
    CSmallString                ProvidedName;
    CSmallString                Name;
    CSmallString                RGroup;
    CSmallString                EGroup;
    std::vector<CSmallString>   AllPosixGroups;
    std::vector<CSmallString>   AllUserGroups;      // all user group that are available

    // access groups
    std::vector<CSmallString>   DefaultGroups;
    std::vector<CSmallString>   PosixGroups;
    std::vector<CSmallString>   UserGroups;
    std::list<CSmallString>     AllGroups;          // only those groups in which the user belongs

    /// init default tokens
    void InitDefaultGroups(CXMLElement* p_ele);

    /// init posix groups
    void InitPosixGroups(CXMLElement* p_ele);

    /// init user groups
    void InitGroupsGroups(CXMLElement* p_ele);

    /// init class by username
    bool InitGlobalSetup(const CSmallString& name);

    /// init class by userid
    bool InitGlobalSetup(uid_t userid);

    /// get section groups
    const CSmallString GetSecGroups(std::vector<CSmallString>& list);
};

// -----------------------------------------------------------------------------

extern CUser    User;

// -----------------------------------------------------------------------------

#endif
