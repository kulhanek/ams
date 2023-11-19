// =============================================================================
//  AMS - Advanced Module System
// -----------------------------------------------------------------------------
//     Copyright (C) 2023 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2020 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include <HostDesktop.hpp>
#include <Host.hpp>
#include <ErrorSystem.hpp>
#include <fnmatch.h>

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CHostSubSystemDesktop::CHostSubSystemDesktop(const CFileName& config_file)
    : CHostSubSystem(config_file)
{
    CachedData = false;
    IsDesktop = false;
    CPUPenalty = 0;
}

//------------------------------------------------------------------------------

CHostSubSystemDesktop::~CHostSubSystemDesktop(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CHostSubSystemDesktop::Init(void)
{
    CXMLElement* p_ele = GetConfig("desktop");
    if( p_ele == NULL ){
        INVALID_ARGUMENT("config element 'desktop' is NULL");
    }

    p_ele->GetAttribute("penalty",CPUPenalty);

    CXMLElement* p_fele = p_ele->GetFirstChildElement("host");
    while( p_fele != NULL ){
        CSmallString filter,cmd;

        // load config
        bool success = true;

        success &= p_fele->GetAttribute("filter",filter);
        success &= p_fele->GetAttribute("cmd",cmd);

        // move to next record
        p_fele = p_fele->GetNextSiblingElement("host");

        if( success == false ){
            ES_WARNING("undefined filter and/or cmd attributes for host element");
            continue;
        }

        // does host match Hostname
        if( fnmatch(filter,Host.GetHostName(),0) != 0 ){
            CSmallString warning;
            warning << "desktop: host '" << Host.GetHostName() << "' does not match filter '" << filter << "'";
            ES_WARNING(warning);
            continue;
        }

        // execute command
        success = false;
        FILE* p_file = popen(cmd,"r");
        if( p_file != NULL ){
            success = pclose(p_file) == 0;
        }
        // was is OK?
        if( success ){
            IsDesktop = true;
            break;
        }
    }

    CachedData = false;
}

//------------------------------------------------------------------------------

void CHostSubSystemDesktop::Apply(void)
{
    if( IsDesktop ){
        int ncpus = Host.GetNumOfHostCPUs();
        ncpus -= CPUPenalty;
        Host.SetNumOfHostCPUs(ncpus);
    }
}

//------------------------------------------------------------------------------

EHostCacheMode CHostSubSystemDesktop::LoadFromCache(CXMLElement* p_ele)
{
    CXMLElement* p_cele = p_ele->GetFirstChildElement("desktop");
    if( p_cele == NULL ) return(EHC_REININT);

    bool result = true;
    result &= p_cele->GetAttribute("st",IsDesktop);
    result &= p_cele->GetAttribute("pn",CPUPenalty);

    CachedData = result;
    if( CachedData ){
        return(EHC_LOADED);
    }
    return(EHC_REININT);
}

//------------------------------------------------------------------------------

void CHostSubSystemDesktop::SaveToCache(CXMLElement* p_ele)
{
    CXMLElement* p_cele = p_ele->CreateChildElement("desktop");
    p_cele->SetAttribute("st",IsDesktop);
    p_cele->SetAttribute("pn",CPUPenalty);
}

//------------------------------------------------------------------------------

void CHostSubSystemDesktop::PrintSubSystemInfo(CVerboseStr& vout)
{
    if( CachedData ){
    vout << ">>> desktop ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ [cached] ~~" << endl;
    } else {
    vout << ">>> desktop ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    }

    vout << "    Configuration  : " << GetConfigFile() <<  endl;
    if( IsDesktop ){
    vout << "    Status         : host is a desktop, the CPU penalty will apply" << endl;
    vout << "    CPU penalty    : " << CPUPenalty << endl;
    } else {
    vout << "    Status         : host is not a desktop" << endl;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

