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
#include <errno.h>
#include <ErrorSystem.hpp>
#include <XMLIterator.hpp>
#include <AMSRegistry.hpp>
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

ActiveModules      = CShell::GetSystemVariable("AMS_ACTIVE_MODULES");
ExportedModules    = CShell::GetSystemVariable("AMS_EXPORTED_MODULES");


bool CAMSRegistry::IsModuleActive(const CSmallString& module)
{
    char* p_lvar;
    if( ActiveModules == NULL ) return(false);

    CSmallString name,ver,arch,para;

    CUtils::ParseModuleName(module,name,ver,arch,para);

    CSmallString tmp(ActiveModules);

    // generate list of active modules
    std::vector<CSmallString> active_list;

    char* p_saveptr = NULL;
    p_lvar = strtok_r(tmp.GetBuffer(),"|",&p_saveptr);
    while( p_lvar != NULL ) {
        active_list.push_back(p_lvar);
        p_lvar = strtok_r(NULL,"|",&p_saveptr);
    }

    for(unsigned int i=0; i < active_list.size(); i++) {
        CSmallString mactive = active_list[i];
        CSmallString lname,lver,larch,lpara;
        CUtils::ParseModuleName(mactive,lname,lver,larch,lpara);
        if( lname == name ) {
            if( ver == NULL ) return(true);
            if( lver == ver ) {
                if( arch == NULL ) return(true);
                if( larch == arch ) {
                    if( para == NULL ) return(true);
                    if( lpara == para ) return(true);
                }
            }
        }
    }

    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CAMSRegistry::GetActiveModuleVersion(const CSmallString& module,
        CSmallString& actver)
{
    char* p_lvar;
    actver = NULL;
    if( ActiveModules == NULL ) return(false);

    CSmallString name,ver,arch,para;

    CUtils::ParseModuleName(module,name,ver,arch,para);

    CSmallString tmp(ActiveModules);

    // generate list of active modules
    std::vector<CSmallString> active_list;

    char* p_saveptr = NULL;
    p_lvar = strtok_r(tmp.GetBuffer(),"|",&p_saveptr);
    while( p_lvar != NULL ) {
        active_list.push_back(p_lvar);
        p_lvar = strtok_r(NULL,"|",&p_saveptr);
    }

    for(unsigned int i=0; i < active_list.size(); i++) {
        CSmallString mactive = active_list[i];
        CSmallString lname,lver,larch,lpara;
        CUtils::ParseModuleName(mactive,lname,lver,larch,lpara);
        if( lname == name ) {
            if( ver == NULL ) {
                actver = lver;
                return(true);
            }
            if( lver == ver ) {
                if( arch == NULL ) {
                    actver = lver;
                    return(true);
                }
                if( larch == arch ) {
                    if( para == NULL ) {
                        actver = lver;
                        return(true);
                    }
                    if( lpara == para ) {
                        actver = lver;
                        return(true);
                    }
                }
            }
        }
    }

    return(false);
}

//------------------------------------------------------------------------------

const CSmallString& CAMSRegistry::GetActiveModules(void)
{
    return(ActiveModules);
}

//------------------------------------------------------------------------------

const CSmallString& CAMSRegistry::GetExportedModules(void)
{
    return(ExportedModules);
}

//------------------------------------------------------------------------------

const CSmallString CAMSRegistry::GetActiveModuleSpecification(
    const CSmallString& name)
{
    if( ActiveModules == NULL ) return("");

    char* p_lvar;

    CSmallString tmp(ActiveModules);

    char* p_saveptr = NULL;
    p_lvar = strtok_r(tmp.GetBuffer(),"|",&p_saveptr);
    while( p_lvar != NULL ) {
        if( CUtils::GetModuleName(p_lvar) == name ) {
            return(p_lvar);
        }
        p_lvar = strtok_r(NULL,"|",&p_saveptr);
    }

    return("");
}

//------------------------------------------------------------------------------

const CSmallString CAMSRegistry::GetExportedModuleSpecification(
    const CSmallString& name)
{
    if( ExportedModules == NULL ) return("");

    char* p_lvar;

    CSmallString tmp(ExportedModules);

    char* p_saveptr = NULL;
    p_lvar = strtok_r(tmp.GetBuffer(),"|",&p_saveptr);
    while( p_lvar != NULL ) {
        if( CUtils::GetModuleName(p_lvar) == name ) {
            return(p_lvar);
        }
        p_lvar = strtok_r(NULL,"|",&p_saveptr);
    }

    return("");
}

//-----------------------------------------------------------------------------

void CAMSRegistry::UpdateActiveModules(const CSmallString& module,
        bool add_module)
{
    ActiveModules = CShell::RemoveValue(ActiveModules,module,"|");
    if( add_module) ActiveModules = CShell::AppendValue(ActiveModules,module,"|");
}

//-----------------------------------------------------------------------------

void CAMSRegistry::UpdateExportedModules(const CSmallString& module,
        bool add_module)
{
    ExportedModules = CShell::RemoveValue(ExportedModules,module,"|");
    if( add_module) ExportedModules = CShell::AppendValue(ExportedModules,module,"|");
}

//-----------------------------------------------------------------------------

void CAMSRegistry::SetExportedModules(const CSmallString& modules)
{
    ExportedModules = modules;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

