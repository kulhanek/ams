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

#include <User.hpp>
#include <pwd.h>
#include <unistd.h>
#include <grp.h>
#include <fnmatch.h>
#include <errno.h>
#include <ErrorSystem.hpp>
#include <stdlib.h>
#include <string.h>
#include <FileName.hpp>
#include <XMLParser.hpp>
#include <FileSystem.hpp>
#include <XMLElement.hpp>
#include <AMSRegistry.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <UserUtils.hpp>
#include <Utils.hpp>
#include <iomanip>
#include <HostGroup.hpp>
#include <SiteController.hpp>
#include <Shell.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//------------------------------------------------------------------------------

CUser    User;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CUser::CUser(void)
{
    UID = -1;
    RGID = -1;
    EGID = -1;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CUser::InitUserConfig(void)
{
    ConfigName = AMSRegistry.GetUsersConfigFile();

    if( CFileSystem::IsFile(ConfigName) == false ){
        CSmallString    warning;
        warning << "no users configuration file '" << ConfigName << "'";
        ES_WARNING(warning);
        return;
    }

    CXMLParser xml_parser;
    xml_parser.SetOutputXMLNode(&Config);

    if( xml_parser.Parse(ConfigName) == false ) {
        CSmallString    error;
        error << "unable to load users configuration file '" << ConfigName << "'";
        RUNTIME_ERROR(error);
    }
}

//------------------------------------------------------------------------------

void CUser::InitUser(void)
{
    InitPosixUser();
    InitAMSUser();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CUser::InitPosixUser(void)
{
    uid_t userid =  getuid();

    struct passwd* pwd;
    errno = 0;
    pwd = getpwuid(userid);
    if( pwd == NULL ){
        CSmallString error;
        error << "unable to get passwd info of '" << userid << "' (" << strerror(errno) << ")";
        RUNTIME_ERROR(error);
    }

    UID = pwd->pw_uid;
    RGID = pwd->pw_gid;    // use effective group id instead of pwd->pw_gid;
    EGID = getegid();    // use effective group id instead of pwd->pw_gid;
    Name = pwd->pw_name;

    // get group name
    struct group* grp;
    grp = getgrgid(RGID);
    if( grp == NULL ){
        CSmallString error;
        error << "unable to get real group info of '" << RGID << "' (" << strerror(errno) << ")";
        RUNTIME_ERROR(error);
    }

    // determine primary group
    RGroup = grp->gr_name;

    grp = getgrgid(EGID);
    if( grp == NULL ){
        CSmallString error;
        error << "unable to get effective group info of '" << EGID << "' (" << strerror(errno) << ")";
        RUNTIME_ERROR(error);
    }

    // determine primary group
    EGroup = grp->gr_name;

    // get groups
    int ngroups = 0;
    getgrouplist(Name,RGID,NULL,&ngroups);
    if( ngroups < 0 ){
        RUNTIME_ERROR("no groups for user");
    }

    gid_t* groups = (gid_t*)malloc(ngroups * sizeof (gid_t));
    getgrouplist(Name,RGID,groups,&ngroups);

    for(int i=0; i < ngroups; i++){
        grp = getgrgid(groups[i]);
        if( grp != NULL ) PosixGroups.push_back(grp->gr_name);
    }

    free(groups);
}

//------------------------------------------------------------------------------

void CUser::InitAMSUser(void)
{
// parse config file
    CXMLElement* p_ele = Config.GetFirstChildElement("config");
    if( p_ele ){
        p_ele = p_ele->GetFirstChildElement();
    }

    while( p_ele ){
        if( p_ele->GetName() == "default" ){
            InitDefaultACLGroups(p_ele);
        }
        if( p_ele->GetName() == "posix" ){
            InitPosixACLGroups(p_ele);
        }
        if( p_ele->GetName() == "ams" ){
            InitAMSACLGroups(p_ele);
        }
        p_ele = p_ele->GetNextSiblingElement();
    }

// merge all tokens
    for(CSmallString group : DefaultACLGroups){
        AllACLGroups.push_back(group);
    }
    for(CSmallString group : PosixACLGroups){
        AllACLGroups.push_back(group);
    }
    for(CSmallString group : AMSACLGroups){
        AllACLGroups.push_back(group);
    }

// sort tokens
    AllACLGroups.sort();
    AllACLGroups.unique();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CUser::InitDefaultACLGroups(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

// tokens
    std::string value;
    if( p_ele->GetAttribute("groups",value) ){
        split(DefaultACLGroups,value,is_any_of(","));
    }
    if( UMask == NULL ){
        p_ele->GetAttribute("umask",UMask);
    }
}

//------------------------------------------------------------------------------

void CUser::InitPosixACLGroups(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

// filter posix group tokens
    CXMLElement* p_fele = p_ele->GetFirstChildElement("filter");
    while( p_fele != NULL ){
        CSmallString filter;
        p_fele->GetAttribute("value",filter);

        for(CSmallString pgrp: PosixGroups){
            if( fnmatch(filter,pgrp,0) == 0 ){
                CSmallString alias;
                p_fele->GetAttribute("alias",alias);
                if( alias != NULL ){
                    PosixACLGroups.push_back(alias);
                } else {
                    PosixACLGroups.push_back(pgrp);
                }
                if( UMask == NULL ){
                    p_fele->GetAttribute("umask",UMask);
                }
            }
        }
        p_fele = p_fele->GetNextSiblingElement("filter");
    }
}

//------------------------------------------------------------------------------

void CUser::InitAMSACLGroups(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

// filter posix group tokens
    CXMLElement* p_fele = p_ele->GetFirstChildElement("group");
    while( p_fele != NULL ){
        CSmallString grpname;
        p_fele->GetAttribute("name",grpname);

        CXMLElement* p_uele = p_fele->GetFirstChildElement("user");
        while( p_uele != NULL ){
            CSmallString usrname;
            p_uele->GetAttribute("name",usrname);
            if( usrname == Name ){
                if( UMask == NULL ){
                    p_fele->GetAttribute("umask",UMask);
                }
                AMSACLGroups.push_back(grpname);
                break;
            }
            p_uele = p_uele->GetNextSiblingElement("user");
        }
        p_fele = p_fele->GetNextSiblingElement("group");
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString& CUser::GetName(void) const
{
    return(Name);
}

//------------------------------------------------------------------------------

uid_t CUser::GetUserID(void) const
{
    return(UID);
}

//------------------------------------------------------------------------------

const CSmallString& CUser::GetRGroup(void) const
{
    return(RGroup);
}

//------------------------------------------------------------------------------

const CSmallString& CUser::GetEGroup(void) const
{
    return(EGroup);
}

//------------------------------------------------------------------------------

const CSmallString CUser::GetACLGroups(void)
{
    return(GetGroupList(AllACLGroups));
}

//------------------------------------------------------------------------------

const CSmallString CUser::GetPosixGroups(void)
{
    return(GetGroupList(PosixGroups));
}

//------------------------------------------------------------------------------

bool CUser::IsInACLGroup(const CSmallString& grpname)
{
    std::list<CSmallString>::iterator   it = AllACLGroups.begin();
    std::list<CSmallString>::iterator   ie = AllACLGroups.end();
    while(it != ie){
        if( grpname == (*it) )  return(true);
        it++;
    }

    return(false);
}

//------------------------------------------------------------------------------

bool CUser::IsInPosixGroup(const CSmallString& group)
{
    std::list<CSmallString>::iterator   it = PosixGroups.begin();
    std::list<CSmallString>::iterator   ie = PosixGroups.end();
    while(it != ie){
        if( *it == group ) return(true);
        it++;
    }
    return(false);
}

//------------------------------------------------------------------------------

const CSmallString CUser::GetGroupList(std::list<CSmallString>& list,const CSmallString delim)
{
    CSmallString groups;

    std::list<CSmallString>::iterator   it = list.begin();
    std::list<CSmallString>::iterator   ie = list.end();
    while(it != ie){
        if( it != list.begin() )  groups << delim;
        groups << (*it);
        it++;
    }    

    return(groups);
}

//------------------------------------------------------------------------------

void CUser::PrintUserDetailedInfo(CVerboseStr& vout)
{
    vout << endl;
    vout << "# User name          : " << Name << " (uid: " << UID << ")" << endl;
    vout << "# Real group name    : " << RGroup << " (gid: " << RGID << ")" << endl;
    vout << "# Eff. group name    : " << EGroup << " (gid: " << EGID << ")" << endl;
    mode_t umask_mode = CUserUtils::GetUMaskMode();
    char uorigin = 'S';
    vout << "# Current UMask      : " << CUserUtils::GetUMask(umask_mode) << "/" << uorigin;
    vout << " [" << CUserUtils::GetUMaskPermissions(umask_mode) << "]" << endl;

    vout << "# ==============================================================================" << endl;
    vout << "# Configuration      : " << ConfigName <<  endl;
    // parse config file -------------------------
    CXMLElement* p_ele = Config.GetFirstChildElement("config");
    if( p_ele ){
        p_ele = p_ele->GetFirstChildElement();
    }

    while( p_ele ){
        if( p_ele->GetName() == "default" ){
    CUtils::PrintTokens(vout,"# Deafult ACL groups : ",GetGroupList(DefaultACLGroups),80);
        }
        if( p_ele->GetName() == "posix" ){
    CUtils::PrintTokens(vout,"# Posix ACL groups   : ",GetGroupList(PosixACLGroups),80);
        }
        if( p_ele->GetName() == "ams" ){
    CUtils::PrintTokens(vout,"# AMS ACL groups     : ",GetGroupList(AMSACLGroups),80);
        }
        p_ele = p_ele->GetNextSiblingElement();
    }
    vout << "# ==============================================================================" << endl;

    CSmallString umask = GetUserUMask();
    if( umask != NULL ){
        char uorigin = 'G';
        if( umask != 0 ){}
        vout << "# Requested UMask    : " << umask << "/" << uorigin;
        vout << " [" << CUserUtils::GetUMaskPermissions(CUserUtils::GetUMaskMode(umask)) << "]" << endl;
    } else {
        vout << "# Requested UMask    : unset/G" << endl;
    }
    CUtils::PrintTokens(vout,"# Final ACL Groups   : ",GetGroupList(AllACLGroups),80);
}

//------------------------------------------------------------------------------

void CUser::PrintUserInfo(CVerboseStr& vout)
{
    vout << "# User name            : " << Name << endl;
    vout << "# User UID             : " << UID << endl;
    vout << "# Real group name      : " << RGroup << endl;
    vout << "# Real group ID        : " << RGID << endl;
    vout << "# Effective group name : " << EGroup << endl;
    vout << "# Effective group ID   : " << EGID << endl;
    vout << "# Posix groups         : " << GetPosixGroups() << endl;
    if( UMask != NULL ){
    vout << "# UMask                : " << UMask << endl;
    } else {
    vout << "# UMask                : " << "unset" << endl;
    }
}

//------------------------------------------------------------------------------

void CUser::PrintUserInfoForSite(CVerboseStr& vout)
{
    // user and host info
    vout << "# ~~~ <b>User identification</b>";
    for(int n=25; n < 80;n++) vout << "~";
    vout << endl;

    mode_t umask = CUserUtils::GetUMaskMode();
    char uorigin;
    mode_t req_umask = User.GetRequestedUserUMaskMode(uorigin);
    if( umask != req_umask ){
        uorigin = 'S';
    }
    vout <<                  "  User name  : " << GetName() << endl;
    vout <<                  "  User group : " << GetEGroup() << " [umask: " << CUserUtils::GetUMask(umask);
    vout << "/" << setw(1) << uorigin << " " << CUserUtils::GetUMaskPermissions(umask) << "]" << endl;
    CUtils::PrintTokens(vout,"  ACL groups : ",GetACLGroups(),80);
}

//------------------------------------------------------------------------------

const CSmallString CUser::GetUserUMask(void)
{
    return(UMask);
}

//------------------------------------------------------------------------------

const CSmallString CUser::GetJobUMask(void)
{
    CSmallString umask;
    if( SiteController.IsBatchJob() == true ){
        umask = CShell::GetSystemVariable("INF_UMASK");
    }
    return(umask);
}

//------------------------------------------------------------------------------

mode_t CUser::GetRequestedUserUMaskMode(char& origin)
{
    if( GetJobUMask() != NULL ){
        origin = 'J';
        return(CUserUtils::GetUMaskMode(GetJobUMask()));
    }
    if( AMSRegistry.GetUserUMask() != NULL ){
        origin = 'U';
        return(CUserUtils::GetUMaskMode(AMSRegistry.GetUserUMask()));
    }
    if( GetUserUMask() != NULL ){
        origin = 'G';
        return(CUserUtils::GetUMaskMode(GetUserUMask()));
    }
    if( HostGroup.GetUserUMask() != NULL ){
        origin = 'H';
        return(CUserUtils::GetUMaskMode(HostGroup.GetUserUMask()));
    }
    origin = 'D';
    return( CUserUtils::GetUMaskMode(AMSRegistry.GetDefaultUMask()) );
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

