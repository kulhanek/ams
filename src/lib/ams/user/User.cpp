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
#include <Shell.hpp>
#include <Utils.hpp>

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
    CFileName    config_name = AMSRegistry.GetUsersConfigFile();

    CXMLParser xml_parser;
    xml_parser.SetOutputXMLNode(&Config);

    if( xml_parser.Parse(config_name) == false ) {
        CSmallString    error;
        error << "unable to load users configuration file '" << config_name << "'";
        RUNTIME_ERROR(error);
    }
}

//------------------------------------------------------------------------------

bool CUser::InitUser(void)
{
    InitPosixUser();
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
            InitACLDefaultGroups(p_ele);
        }
        if( p_ele->GetName() == "posix" ){
            InitACLPosixGroups(p_ele);
        }
        if( p_ele->GetName() == "user" ){
            InitACLUserGroups(p_ele);
        }
        p_ele = p_ele->GetNextSiblingElement();
    }

// merge all tokens
    for(CSmallString group : ACLDefaultGroups){
        ACLAllGroups.push_back(group);
    }
    for(CSmallString group : ACLPosixGroups){
        ACLAllGroups.push_back(group);
    }
    for(CSmallString group : ACLUserGroups){
        ACLAllGroups.push_back(group);
    }

// sort tokens
    AllGroups.sort();
    AllGroups.unique();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CUser::InitACLDefaultGroups(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

// tokens
    std::string value;
    if( p_ele->GetAttribute("groups",value) ){
        split(ACLDefaultGroups,value,is_any_of(","));
    }
}

//------------------------------------------------------------------------------

void CUser::InitACLPosixGroups(CXMLElement* p_ele)
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
                    ACLPosixGroups.push_back(alias);
                } else {
                    ACLPosixGroups.push_back(pgrp);
                }
            }
        }
        p_fele = p_fele->GetNextSiblingElement("filter");
    }
}

//------------------------------------------------------------------------------

void CUser::InitACLUsersGroups(CXMLElement* p_ele)
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
                ACLUserGroups.push_back(grpname);
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
    return(GetGroupList(ACLAllGroups));
}

//------------------------------------------------------------------------------

bool CUser::IsInACLGroup(const CSmallString& grpname)
{
    std::list<CSmallString>::iterator   it = ACLAllGroups.begin();
    std::list<CSmallString>::iterator   ie = ACLAllGroups.end();
    while(it != ie){
        if( grpname == (*it) )  return(true);
        it++;
    }

    return(false);
}

//------------------------------------------------------------------------------

bool CUser::IsInPosixGroup(const CSmallString& group)
{
    std::vector<CSmallString>::iterator   it = PosixGroups.begin();
    std::vector<CSmallString>::iterator   ie = PosixGroups.end();
    while(it != ie){
        if( *it == group ) return(true);
        it++;
    }
    return(false);
}

//------------------------------------------------------------------------------

const CSmallString CUser::GetGroupList(std::list<CSmallString>& list)
{
    CSmallString groups;

    std::vector<CSmallString>::iterator   it = list.begin();
    std::vector<CSmallString>::iterator   ie = list.end();
    while(it != ie){
        if( it != list.begin() )  groups << ",";
        groups << (*it);
        it++;
    }
    return(groups);
}

//------------------------------------------------------------------------------

void CUser::PrintUserDetailedInfo(CVerboseStr& vout)
{
    vout << endl;
    vout << "User name           : " << Name << " (uid: " << UID << ")" << endl;
    vout << "Real group name     : " << RGroup << " (gid: " << RGID << ")" << endl;
    vout << "Eff. group name     : " << EGroup << " (gid: " << EGID << ") [umask: " << CShell::GetUMask() << " " << CShell::GetUMaskPermissions() << "]" << endl;
    vout << "Site ID             : " << SiteSID << endl;
    if( SiteConfigInUse == true ){
    vout << "Configuration realm : " << SiteConfigInUse << endl;
    } else {
    vout << "Configuration realm : default" << endl;
    }
    vout << "===================================================================" << endl;

    // parse config file -------------------------
    CXMLElement* p_ele = Users.GetFirstChildElement("config");
    if( p_ele ){
        p_ele = p_ele->GetFirstChildElement();
    }

    int pri = 0;
    while( p_ele ){
        if( p_ele->GetName() == "default" ){
            pri++;
    vout << ">>> default ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    vout << "    Priority    : " << pri << endl;
    vout << "    Groups      : " << GetSecGroups(DefaultGroups) << endl;
        }
        if( p_ele->GetName() == "posix" ){
            pri++;
    vout << ">>> posix ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    vout << "    Priority    : " << pri << endl;
    vout << "    All groups  : " << GetSecGroups(AllPosixGroups) << endl;
    vout << "    User groups : " << GetSecGroups(PosixGroups) << endl;
        }
        if( p_ele->GetName() == "groups" ){
            pri++;
    vout << ">>> groups ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    vout << "    Priority    : " << pri << endl;
    vout << "    All groups  : " << GetSecGroups(AllUserGroups) << endl;
    vout << "    User groups : " << GetSecGroups(UserGroups) << endl;
        }
        p_ele = p_ele->GetNextSiblingElement();
    }
    vout << "===================================================================" << endl;
    vout << ">>> final" << endl;
    vout << "    ACL Groups  : " << GetGroups() << endl;
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
}

//------------------------------------------------------------------------------

void CUser::PrintUserInfoForSite(CVerboseStr& vout)
{
    // user and host info
    vout << endl;
    vout << "# ~~~ User identification";
    for(int n=25; n < 80;n++) vout << "~";
    vout << endl;

    vout << "# User name  : " << GetName() << endl;
    vout << "# User group : " << GetEGroup() << " [umask: " << CShell::GetUMask() << " " << CShell::GetUMaskPermissions() << "]" << endl;
    CUtils::PrintTokens(vout,"# ACL groups : ",GetGroups());
}



//------------------------------------------------------------------------------

const CSmallString CUser::GetRequestedUserUMask(void)
{
    return( AMSRegistry.GetUserUMask() );
}

//------------------------------------------------------------------------------

uid_t CUser::GetUserID(void) const
{
    return(UID);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

