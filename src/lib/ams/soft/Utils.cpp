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

#include <Utils.hpp>
#include <string.h>
#include <Site.hpp>
#include <DirectoryEnum.hpp>
#include <FileName.hpp>
#include <FileSystem.hpp>
#include <AmsUUID.hpp>
#include <prefix.h>
#include <errno.h>
#include <ErrorSystem.hpp>
#include <XMLIterator.hpp>
#include <iomanip>
#include <list>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <XMLElement.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CUtils::GetSiteID(const CSmallString& site_name)
{
    // go through the list of all available sites -------------
    CDirectoryEnum dir_enum(BR_ETCDIR("/sites"));

    dir_enum.StartFindFile("*");
    CFileName site_sid;
    while( dir_enum.FindFile(site_sid) ) {
        CAmsUUID    site_id;
        CSite       site;
        if( site_id.LoadFromString(site_sid) == false ) continue;
        if( site.LoadConfig(site_sid) == false ) continue;

        // site was found - return site id
        if( site.GetName() == site_name ) return(site.GetID());
    }
    dir_enum.EndFindFile();

    // site was not found or it is not correctly configured
    return("");
}

//------------------------------------------------------------------------------

const CSmallString CUtils::GetSiteName(const CSmallString& site_sid)
{
    if( site_sid == NULL ) return("");
    CSite   site;
    if( site.LoadConfig(site_sid) == false ){
        return("");
    }
    return(site.GetName());
}

//------------------------------------------------------------------------------

bool CUtils::IsSiteIDValid(const CSmallString& site_id)
{
    CFileName config_path = ETCDIR;
    config_path = config_path / "sites" / site_id / "site.xml";

    if( CFileSystem::IsFile(config_path) == false ) return(false);

    CSite   site;
    return(site.LoadConfig(site_id));
}

//------------------------------------------------------------------------------

CSmallString CUtils::GenerateUUID(void)
{
    CUUID my_uuid;
    my_uuid.CreateUUID();
    return(my_uuid.GetStringForm());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CUtils::ParseModuleName(const CSmallString& module,
        CSmallString& name,
        CSmallString& ver,
        CSmallString& arch,
        CSmallString& mode)
{
    CSmallString tmp(module);
    if( tmp == NULL ) return(false);

    char* p_saveptr = NULL;
    name = strtok_r(tmp.GetBuffer(),":",&p_saveptr);
    ver = strtok_r(NULL,":",&p_saveptr);
    arch = strtok_r(NULL,":",&p_saveptr);
    mode = strtok_r(NULL,":",&p_saveptr);

    return(true);
}

//------------------------------------------------------------------------------

bool CUtils::ParseModuleName(const CSmallString& module,
        CSmallString& name,
        CSmallString& ver,
        CSmallString& arch)
{
    CSmallString tmp(module);
    if( tmp == NULL ) return(false);

    char* p_saveptr = NULL;
    name = strtok_r(tmp.GetBuffer(),":",&p_saveptr);
    ver = strtok_r(NULL,":",&p_saveptr);
    arch = strtok_r(NULL,":",&p_saveptr);

    return(true);
}

//------------------------------------------------------------------------------

bool CUtils::ParseModuleName(const CSmallString& module,
        CSmallString& name,
        CSmallString& ver)
{
    CSmallString tmp(module);
    if( tmp == NULL ) return(false);

    char* p_saveptr = NULL;
    name = strtok_r(tmp.GetBuffer(),":",&p_saveptr);
    ver = strtok_r(NULL,":",&p_saveptr);

    return(true);
}

//------------------------------------------------------------------------------

bool CUtils::ParseModuleName(const CSmallString& module,
        CSmallString& name)
{
    CSmallString tmp(module);
    if( tmp == NULL ) return(false);

    char* p_saveptr = NULL;
    name = strtok_r(tmp.GetBuffer(),":",&p_saveptr);

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CUtils::GetModuleName(const CSmallString& module)
{
    CSmallString tmp(module);
    if( tmp == NULL ) return("");

    char* p_saveptr = NULL;
    return(strtok_r(tmp.GetBuffer(),":",&p_saveptr));
}

//------------------------------------------------------------------------------

const CSmallString CUtils::GetModuleVer(const CSmallString& module)
{
    CSmallString tmp(module);
    if( tmp == NULL ) return("");

    char* p_saveptr = NULL;
    strtok_r(tmp.GetBuffer(),":",&p_saveptr);
    return(strtok_r(NULL,":",&p_saveptr));
}

//------------------------------------------------------------------------------

const CSmallString CUtils::GetModuleArch(const CSmallString& module)
{
    CSmallString tmp(module);
    if( tmp == NULL ) return("");

    char* p_saveptr = NULL;
    strtok_r(tmp.GetBuffer(),":",&p_saveptr);
    strtok_r(NULL,":",&p_saveptr);
    return(strtok_r(NULL,":",&p_saveptr));
}

//------------------------------------------------------------------------------

const CSmallString CUtils::GetModuleMode(const CSmallString& module)
{
    CSmallString tmp(module);
    if( tmp == NULL ) return("");

    char* p_saveptr = NULL;
    strtok_r(tmp.GetBuffer(),":",&p_saveptr);
    strtok_r(NULL,":",&p_saveptr);
    strtok_r(NULL,":",&p_saveptr);
    return(strtok_r(NULL,":",&p_saveptr));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CUtils::PrintBuild(std::ostream& vout,CXMLElement* p_build)
{
    // info out ----------------------------------------------------------------------------
    unsigned int col1 = 1;
    unsigned int col2 = 1;
    unsigned int col3 = 1;
    unsigned int col4 = 1;

    CUtils::GetMaxSizesForBuild(p_build,col1,col2,col3,col4);

    if( col1 < 4 ) col1 = 4;
    if( col2 < 10 ) col2 = 10;
    if( col3 < 14 ) col3 = 14;
    if( col4 < 8 ) col4 = 8;

    vout << "# What";
    vout << " Name";
    for(unsigned int i = 4; i < col1; i++) vout << " ";
    vout << " Value/Name";
    for(unsigned int i = 10; i < col2; i++) vout << " ";
    vout << " Operation/Type";
    for(unsigned int i = 14; i < col3; i++) vout << " ";
    vout << " Priority" << endl;
    vout << "# ";
    for(unsigned int i = 0; i < 4; i++) vout << "-";
    vout << " ";
    for(unsigned int i = 0; i < col1; i++) vout << "-";
    vout << " ";
    for(unsigned int i = 0; i < col2; i++) vout << "-";
    vout << " ";
    for(unsigned int i = 0; i < col3; i++) vout << "-";
    vout << " ";
    for(unsigned int i = 0; i < col4; i++) vout << "-";
    vout << endl;

    CXMLElement*    p_setup = NULL;
    if( p_build != NULL ) p_setup = p_build->GetFirstChildElement("setup");

    CXMLIterator    I(p_setup);
    CXMLElement*    p_sele;

    while( (p_sele = I.GetNextChildElement()) != NULL ) {
        if( p_sele->GetName() == "variable" ) {
            CSmallString name;
            CSmallString value;
            CSmallString operation;
            CSmallString priority;
            bool         secret = false;
            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("value",value);
            p_sele->GetAttribute("operation",operation);
            p_sele->GetAttribute("priority",priority);
            p_sele->GetAttribute("secret",secret);
            vout << left << "     V ";
            vout << setw(col1) << name << " ";
            if( secret ){
                value = "*******";
            }
            vout << setw(col2) << value << " ";
            vout << setw(col3) << operation << " ";
            vout << setw(col4) << priority << endl;
        }
        if( p_sele->GetName() == "script" ) {
            CSmallString name;
            CSmallString type;
            CSmallString priority;
            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("type",type);
            p_sele->GetAttribute("priority",priority);
            vout << "     S ";
            vout << setw(col1) << " " << " ";
            vout << setw(col2) << name << " ";
            vout << setw(col3) << type << " ";
            vout << setw(col4) << priority << endl;
        }
        if( p_sele->GetName() == "alias" ) {
            CSmallString name;
            CSmallString value;
            CSmallString priority;
            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("value",value);
            p_sele->GetAttribute("priority",priority);
            vout << "     A ";
            vout << setw(col1) << name << " ";
            vout << setw(col2) << value << " ";
            vout << setw(col3) << " " << " ";
            vout << setw(col4) << priority << endl;
        }
    }

}

//------------------------------------------------------------------------------

void CUtils::GetMaxSizesForBuild(CXMLElement* p_ele,
        unsigned int& col1,unsigned int& col2,
        unsigned int& col3,unsigned int& col4)
{
    col1 = 1;
    col2 = 1;
    col3 = 1;
    col4 = 1;

    CXMLElement*    p_setup = NULL;
    if( p_ele != NULL ) p_setup = p_ele->GetFirstChildElement("setup");

    CXMLIterator    I(p_setup);
    CXMLElement*    p_sele;

    while( (p_sele = I.GetNextChildElement()) != NULL ) {
        if( p_sele->GetName() == "variable" ) {
            CSmallString name;
            CSmallString value;
            CSmallString operation;
            CSmallString priority;
            p_sele->GetAttribute("name",name);
            if( col1 < name.GetLength() ) col1 = name.GetLength();
            p_sele->GetAttribute("value",value);
            if( col2 < value.GetLength() ) col2 = value.GetLength();
            p_sele->GetAttribute("operation",operation);
            if( col3 < operation.GetLength() ) col3 = operation.GetLength();
            p_sele->GetAttribute("priority",priority);
            if( col4 < priority.GetLength() ) col4 = priority.GetLength();
        }
        if( p_sele->GetName() == "script" ) {
            CSmallString name;
            CSmallString value;
            CSmallString type;
            CSmallString priority;
            p_sele->GetAttribute("name",name);
            if( col2 < name.GetLength() ) col2 = name.GetLength();
            p_sele->GetAttribute("type",type);
            if( col3 < type.GetLength() ) col3 = type.GetLength();
            p_sele->GetAttribute("priority",priority);
            if( col4 < priority.GetLength() ) col4 = priority.GetLength();
        }
        if( p_sele->GetName() == "alias" ) {
            CSmallString name;
            CSmallString value;
            CSmallString priority;
            p_sele->GetAttribute("name",name);
            if( col1 < name.GetLength() ) col1 = name.GetLength();
            p_sele->GetAttribute("value",value);
            if( col2 < value.GetLength() ) col2 = value.GetLength();
            p_sele->GetAttribute("priority",priority);
            if( col4 < priority.GetLength() ) col4 = priority.GetLength();
        }
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CUtils::AreSameTokens(const CSmallString& user_arch,const CSmallString& build_arch)
{
    int matches,maxmatches;
    return( AreSameTokens(user_arch,build_arch,matches,maxmatches) );
}

//------------------------------------------------------------------------------

bool CUtils::AreSameTokens(const CSmallString& user_arch,const CSmallString& build_arch,int& matches,int& maxmatches)
{
    matches = 0;
    maxmatches = 0;

    string          suser_arch(user_arch);
    list<string>    user_archs;

    split(user_archs,suser_arch,is_any_of("#"));
    user_archs.sort();
    user_archs.unique();

    string          sbuild_arch(build_arch);
    list<string>    build_archs;

    split(build_archs,sbuild_arch,is_any_of("#"));
    build_archs.sort();
    build_archs.unique();

    if( build_archs.size() >= user_archs.size() ){
        maxmatches = build_archs.size();
    } else{
        maxmatches = user_archs.size();
    }

    list<string>::iterator bit = build_archs.begin();
    list<string>::iterator bet = build_archs.end();

    while( bit != bet ){
        list<string>::iterator uit = user_archs.begin();
        list<string>::iterator uet = user_archs.end();

        while( uit != uet ){
            if( (*bit) == (*uit) ){
                matches++;
                break;
            }
            uit++;
        }

        bit++;
    }

    return( matches == maxmatches );
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

