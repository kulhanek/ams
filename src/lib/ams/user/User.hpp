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
#include <XMLDocument.hpp>
#include <VerboseStr.hpp>
#include <list>

// -----------------------------------------------------------------------------

class AMS_PACKAGE CUser {
public:
// constructor -----------------------------------------------------------------
        CUser(void);

// executive methods -----------------------------------------------------------
    /// load user global configuration file
    void InitUserConfig(void);

    /// init current user
    void InitUser(void);

// informational methods -------------------------------------------------------
    /// get user name
    const CSmallString& GetName(void) const;

    /// get user ID
    uid_t GetUserID(void) const;

    /// get real group
    const CSmallString& GetRGroup(void) const;

    /// get effective group
    const CSmallString& GetEGroup(void) const;

    /// get all user groups separated by comma
    const CSmallString GetACLGroups(void);

    /// check group
    bool IsInACLGroup(const CSmallString& grpname);

    /// is in posix group
    bool IsInPosixGroup(const CSmallString& group);

    /// return user requested umask
    const CSmallString GetRequestedUserUMask(void);

// informational methods -------------------------------------------------------
    /// print info about user
    void PrintUserDetailedInfo(CVerboseStr& vout);

    /// print user compact info
    void PrintUserInfo(CVerboseStr& vout);

    /// print user compact info
    void PrintUserInfoForSite(CVerboseStr& vout);

// section of private data -----------------------------------------------------
private:
    CXMLDocument                Config;                  // user configuration
    // user identification
    uid_t                       UID;
    gid_t                       RGID;
    gid_t                       EGID;
    CSmallString                Name;
    CSmallString                RGroup;
    CSmallString                EGroup;
    std::list<CSmallString>     PosixGroups;

    // access groups
    std::list<CSmallString>     ACLDefaultGroups;
    std::list<CSmallString>     ACLPosixGroups;
    std::list<CSmallString>     ACLUserGroups;
    std::list<CSmallString>     ACLAllGroups;          // only those groups in which the user belongs

    /// init default tokens
    void InitACLDefaultGroups(CXMLElement* p_ele);

    /// init posix groups
    void InitACLPosixGroups(CXMLElement* p_ele);

    /// init user groups
    void InitACLUsersGroups(CXMLElement* p_ele);

    /// init class by username
    bool InitGlobalSetup(const CSmallString& name);

    /// init class by userid
    bool InitGlobalSetup(uid_t userid);

    /// get group list
    const CSmallString GetGroupList(std::list<CSmallString>& list);
};

// -----------------------------------------------------------------------------

extern CUser    User;

// -----------------------------------------------------------------------------

#endif
