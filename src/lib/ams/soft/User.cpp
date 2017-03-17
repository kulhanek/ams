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
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <ErrorSystem.hpp>
#include <stdlib.h>
#include <string.h>
#include <FileName.hpp>
#include <XMLParser.hpp>
#include "prefix.h"
#include <FileSystem.hpp>
#include <XMLElement.hpp>
#include <AMSGlobalConfig.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <SoftConfig.hpp>

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

void CUser::InitUserFile(const CSmallString& site_sid)
{
    SiteSID = site_sid;
    CFileName    config_name;

    // first try site config
    config_name = CFileName(ETCDIR) / "sites" / site_sid / "user.xml";
    if( CFileSystem::IsFile(config_name) == false ) {
        // then global config
        config_name = CFileName(ETCDIR) / "default" / "user.xml";
    } else {
        SiteConfigInUse = site_sid;
    }

    CXMLParser xml_parser;
    xml_parser.SetOutputXMLNode(&Users);

    if( xml_parser.Parse(config_name) == false ) {
        CSmallString    error;
        error << "unable to load '" << config_name << "' file for site '" << site_sid << "'";
        RUNTIME_ERROR(error);
    }
}

//------------------------------------------------------------------------------

bool CUser::InitGlobalSetup(void)
{
    if( ProvidedName == NULL ){
        uid_t uid =  getuid();
        return(InitGlobalSetup(uid));
    } else {
        return(InitGlobalSetup(ProvidedName));
    }
}

//------------------------------------------------------------------------------

void CUser::InitUser(void)
{
    InitGlobalSetup();

    // parse config file -------------------------
    CXMLElement* p_ele = Users.GetFirstChildElement("config");
    if( p_ele ){
        p_ele = p_ele->GetFirstChildElement();
    }

    while( p_ele ){
        if( p_ele->GetName() == "default" ){
            InitDefaultGroups(p_ele);
        }
        if( p_ele->GetName() == "posix" ){
            InitPosixGroups(p_ele);
        }
        if( p_ele->GetName() == "groups" ){
            InitGroupsGroups(p_ele);
        }
        p_ele = p_ele->GetNextSiblingElement();
    }

    // merge all tokens
    for(size_t i=0; i < DefaultGroups.size(); i++){
        AllGroups.push_back(DefaultGroups[i]);
    }
    for(size_t i=0; i < PosixGroups.size(); i++){
        AllGroups.push_back(PosixGroups[i]);
    }
    for(size_t i=0; i < UserGroups.size(); i++){
        AllGroups.push_back(UserGroups[i]);
    }

    // sort tokens
    AllGroups.sort();
    AllGroups.unique();
}

//------------------------------------------------------------------------------

void CUser::ClearAll(void)
{
    UID = -1;
    RGID = -1;
    EGID = -1;
    Name = NULL;
    RGroup = NULL;
    EGroup = NULL;
    SiteSID = NULL;
    SiteConfigInUse = NULL;
    AllPosixGroups.clear();
    DefaultGroups.clear();
    PosixGroups.clear();
    UserGroups.clear();
    AllUserGroups.clear();
    AllGroups.clear();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CUser::InitGlobalSetup(const CSmallString& name)
{
    struct passwd* pwd;
    errno = 0;
    pwd = getpwnam(name);
    if( pwd == NULL ){
        CSmallString error;
        error << "unable to get user ID of '" << name << "' (" << strerror(errno) << ")";
        ES_ERROR(error);
        return(false);
    }
    return(InitGlobalSetup(pwd->pw_uid));
}

//------------------------------------------------------------------------------

bool CUser::InitGlobalSetup(uid_t userid)
{
    AllPosixGroups.clear();

    struct passwd* pwd;
    errno = 0;
    pwd = getpwuid(userid);
    if( pwd == NULL ){
        CSmallString error;
        error << "unable to get passwd info of '" << userid << "' (" << strerror(errno) << ")";
        ES_ERROR(error);
        return(false);
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
        ES_ERROR(error);
        return(false);
    }

    // determine primary group
    RGroup = grp->gr_name;

    grp = getgrgid(EGID);
    if( grp == NULL ){
        CSmallString error;
        error << "unable to get effective group info of '" << EGID << "' (" << strerror(errno) << ")";
        ES_ERROR(error);
        return(false);
    }

    // determine primary group
    EGroup = grp->gr_name;

    // get groups
    int ngroups = 0;
    getgrouplist(Name,RGID,NULL,&ngroups);
    if( ngroups < 0 ){
        ES_ERROR("no groups for user");
        return(false);
    }

    gid_t* groups = (gid_t*)malloc(ngroups * sizeof (gid_t));
    getgrouplist(Name,RGID,groups,&ngroups);

    for(int i=0; i < ngroups; i++){
        grp = getgrgid(groups[i]);
        if( grp != NULL ) AllPosixGroups.push_back(grp->gr_name);
    }

    free(groups);

    return(true);
}

//------------------------------------------------------------------------------

void CUser::SetUserName(const CSmallString& name)
{
    ProvidedName = name;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CUser::InitDefaultGroups(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL")
    }

    // tokens
    string value;
    std::vector<string> groups;
    if( p_ele->GetAttribute("groups",value) ){
        split(groups,value,is_any_of(","));
    }

    // copy tokens
    vector<string>::iterator  it = groups.begin();
    vector<string>::iterator  ie = groups.end();
    while( it != ie ){
        DefaultGroups.push_back(*it);
        it++;
    }

}

//------------------------------------------------------------------------------

void CUser::InitPosixGroups(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL")
    }

    // filter posix group tokens
    CXMLElement* p_fele = p_ele->GetFirstChildElement("filter");
    while( p_fele != NULL ){
        CSmallString filter;
        p_fele->GetAttribute("value",filter);
        vector<CSmallString>::iterator  it = AllPosixGroups.begin();
        vector<CSmallString>::iterator  ie = AllPosixGroups.end();
        while( it != ie ){
            if( fnmatch(filter,(*it),0) == 0 ){
                CSmallString alias;
                p_fele->GetAttribute("alias",alias);
                if( alias != NULL ){
                    PosixGroups.push_back(alias);
                } else {
                    PosixGroups.push_back(*it);
                }
            }
            it++;
        }
        p_fele = p_fele->GetNextSiblingElement("filter");
    }
}

//------------------------------------------------------------------------------

void CUser::InitGroupsGroups(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL")
    }

    // filter posix group tokens
    CXMLElement* p_fele = p_ele->GetFirstChildElement("group");
    while( p_fele != NULL ){
        CSmallString grpname;
        p_fele->GetAttribute("name",grpname);
        AllUserGroups.push_back(grpname);

        CXMLElement* p_uele = p_fele->GetFirstChildElement("user");
        while( p_uele != NULL ){
            CSmallString usrname;
            p_uele->GetAttribute("name",usrname);
            if( usrname == Name ){
                UserGroups.push_back(grpname);
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

const CSmallString CUser::GetUMask(void) const
{
   mode_t mumask = 0;
   mumask = umask(mumask); // get umask - it destroys current setup
   umask(mumask); // restore umask
   stringstream str;
   char c1 = (mumask & 0007) + '0';
   char c2 = ((mumask & 0070) >> 3) + '0';
   char c3 = ((mumask & 0700) >> 6) + '0';
   str << c3 << c2 << c1;
   return(str.str());
}

//------------------------------------------------------------------------------

const CSmallString CUser::GetUMaskPermissions(void) const
{
    mode_t mumask = 0;
    mumask = umask(mumask); // get umask - it destroys current setup
    umask(mumask); // restore umask
    stringstream str;
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

//------------------------------------------------------------------------------

const CSmallString CUser::GetGroups(void)
{
    CSmallString groups;

    std::list<CSmallString>::iterator   it = AllGroups.begin();
    std::list<CSmallString>::iterator   ie = AllGroups.end();
    while(it != ie){
        if( it != AllGroups.begin() )  groups << ",";
        groups << (*it);
        it++;
    }
    return(groups);
}

//------------------------------------------------------------------------------

bool CUser::IsInGroup(const CSmallString& grpname)
{
    std::list<CSmallString>::iterator   it = AllGroups.begin();
    std::list<CSmallString>::iterator   ie = AllGroups.end();
    while(it != ie){
        if( grpname == (*it) )  return(true);
        it++;
    }

    return(false);
}

//------------------------------------------------------------------------------

bool CUser::IsUserNameProvided(void)
{
    return(ProvidedName != NULL);
}

//------------------------------------------------------------------------------

void CUser::PrintUserDetailedInfo(CVerboseStr& vout)
{
    vout << endl;
    vout << "User name           : " << Name << " (uid: " << UID << ")" << endl;
    vout << "Real group name     : " << RGroup << " (gid: " << RGID << ")" << endl;
    vout << "Eff. group name     : " << EGroup << " (gid: " << EGID << ") [umask: " << GetUMask() << " " << GetUMaskPermissions() << "]" << endl;
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

const CSmallString CUser::GetSecGroups(std::vector<CSmallString>& list)
{
    CSmallString tokens;

    std::vector<CSmallString>::iterator   it = list.begin();
    std::vector<CSmallString>::iterator   ie = list.end();
    while(it != ie){
        if( it != list.begin() )  tokens << ",";
        tokens << (*it);
        it++;
    }

    if( tokens == NULL ){
        tokens << "-none-";
    }

    return(tokens);
}

//------------------------------------------------------------------------------

void CUser::PrintPosixGroups(void)
{
    std::vector<CSmallString>::iterator   it = AllPosixGroups.begin();
    std::vector<CSmallString>::iterator   ie = AllPosixGroups.end();
    while(it != ie){
        CSmallString group = *it;
        if( group == RGroup ){
            printf("  [%s]",(const char*)group);
        } else {
            printf("  %s",(const char*)group);
        }
        if( group == EGroup ){
            printf("*\n");
        } else {
            printf("\n");
        }
        it++;
    }
}

//------------------------------------------------------------------------------

bool CUser::IsPosixGroup(const CSmallString& group)
{
    std::vector<CSmallString>::iterator   it = AllPosixGroups.begin();
    std::vector<CSmallString>::iterator   ie = AllPosixGroups.end();
    while(it != ie){
        if( *it == group ) return(true);
        it++;
    }
    return(false);
}

//------------------------------------------------------------------------------

const CSmallString CUser::GetRequestedUserUMask(void)
{
    return( SoftConfig.GetUserUMask() );
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

