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
#include <FileName.hpp>
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

    /// get all posix groups separated by comma
    const CSmallString GetPosixGroups(void);

    /// check group
    bool IsInACLGroup(const CSmallString& grpname);

    /// is in posix group
    bool IsInPosixGroup(const CSmallString& group);

// informational methods -------------------------------------------------------

    /// get user umask
    const CSmallString GetUserUMask(void);

    /// get user umask - for job
    const CSmallString GetJobUMask(void);

    /// return user requested umask
    /// umask origins
    /// S - system
    /// D - default
    /// H - host group
    /// G - ACL group
    /// U - user
    /// J - job
    mode_t GetRequestedUserUMaskMode(char& origin);

// informational methods -------------------------------------------------------
    /// print info about user
    void PrintUserDetailedInfo(CVerboseStr& vout);

    /// print user compact info
    void PrintUserInfo(CVerboseStr& vout);

    /// print user compact info
    void PrintUserInfoForSite(CVerboseStr& vout);

// section of private data -----------------------------------------------------
private:
    CFileName                   ConfigName;
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
    std::list<CSmallString>     DefaultACLGroups;
    std::list<CSmallString>     PosixACLGroups;
    std::list<CSmallString>     AMSACLGroups;
    std::list<CSmallString>     AllACLGroups;          // only those groups in which the user belongs

    // umask
    CSmallString                UMask;

    /// init user from the system
    void InitPosixUser(void);

    /// init user from user.xml file
    void InitAMSUser(void);

    /// init default tokens
    void InitDefaultACLGroups(CXMLElement* p_ele);

    /// init posix groups
    void InitPosixACLGroups(CXMLElement* p_ele);

    /// init user groups
    void InitAMSACLGroups(CXMLElement* p_ele);

    /// get group list - comma separated
    const CSmallString GetGroupList(std::list<CSmallString>& list,const CSmallString delim=",");
};

// -----------------------------------------------------------------------------

extern CUser    User;

// -----------------------------------------------------------------------------

#endif
