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

#include <HostOS.hpp>
#include <Host.hpp>
#include <ErrorSystem.hpp>
#include <fnmatch.h>

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CHostSubSystemOS::CHostSubSystemOS(const CFileName& config_file)
    : CHostSubSystem(config_file)
{
    CachedData = false;
}


//------------------------------------------------------------------------------

CHostSubSystemOS::~CHostSubSystemOS(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CHostSubSystemOS::Init(void)
{
    CXMLElement* p_ele = GetConfig("os");
    if( p_ele == NULL ){
        INVALID_ARGUMENT("config element 'os' is NULL");
    }

    CSmallString cmd = "/usr/bin/hostnamectl";

    FILE* p_file = popen(cmd,"r");
    if( p_file != NULL ){
        CSmallString line;
        while( line.ReadLineFromFile(p_file,true,true) ){
            if( line.FindSubString("Operating System:") != -1 ){
                int start = line.FindSubString(": ");
                int stop = line.GetLength() - 1;
                start += 2;
                if( start < stop ){
                    Distribution = line.GetSubStringFromTo(start,stop);
                }
            }
            if( line.FindSubString("Kernel:") != -1 ){
                int start = line.FindSubString(": ");
                int stop = line.GetLength() - 1;
                start += 2;
                if( start < stop ){
                    Kernel = line.GetSubStringFromTo(start,stop);
                }
            }
            if( line.FindSubString("Virtualization:") != -1 ){
                int start = line.FindSubString(": ");
                int stop = line.GetLength() - 1;
                start += 2;
                if( start < stop ){
                    Virtualization = line.GetSubStringFromTo(start,stop);
                }
            }
            if( line.FindSubString("Architecture:") != -1 ){
                int start = line.FindSubString(": ");
                int stop = line.GetLength() - 1;
                start += 2;
                if( start < stop ){
                    Architecture = line.GetSubStringFromTo(start,stop);
                }
            }
        }
        pclose(p_file);
    }

    if( Virtualization == NULL ) Virtualization = "-none-";

    CXMLElement* p_fele;
    CSmallString distro_name;
    CSmallString distro_arch;

// distribution

    p_fele = p_ele->GetFirstChildElement("distro");
    while( p_fele != NULL ){
        CSmallString name,token;

        // load config
        bool success = true;

        success &= p_fele->GetAttribute("name",name);
        success &= p_fele->GetAttribute("token",token);

        // move to next record
        p_fele = p_fele->GetNextSiblingElement("distro");

        if( success == false ){
            continue;
        }

        // does host match Hostname
        if( fnmatch(name,Distribution,0) != 0 ){
            continue;
        } else {
            distro_name = token;
        }
    }

    p_fele = p_ele->GetFirstChildElement("arch");
    while( p_fele != NULL ){
        CSmallString name,token;

        // load config
        bool success = true;

        success &= p_fele->GetAttribute("name",name);
        success &= p_fele->GetAttribute("token",token);

        // move to next record
        p_fele = p_fele->GetNextSiblingElement("arch");

        if( success == false ){
            continue;
        }

        // does host match Hostname
        if( fnmatch(name,Architecture,0) != 0 ){
            continue;
        } else {
            distro_arch = token;
        }
    }

    if( (distro_name != NULL) && (distro_arch != NULL) ){
        DistroToken = distro_arch + "-" + distro_name;
    }

    CachedData = false;
}

//------------------------------------------------------------------------------

void CHostSubSystemOS::Apply(void)
{
    if( DistroToken != NULL ){
        Host.AddArchToken(DistroToken);
    }
}

//------------------------------------------------------------------------------

EHostCacheMode CHostSubSystemOS::LoadFromCache(CXMLElement* p_ele)
{
    CXMLElement* p_cele = p_ele->GetFirstChildElement("os");
    if( p_cele == NULL ) return(EHC_REININT);

    bool result = true;
    result &= p_cele->GetAttribute("na",Distribution);
    result &= p_cele->GetAttribute("ke",Kernel);
    result &= p_cele->GetAttribute("vm",Virtualization);
    result &= p_cele->GetAttribute("ar",Architecture);
    result &= p_cele->GetAttribute("dt",DistroToken);

    CachedData = result;
    if( CachedData ){
        return(EHC_LOADED);
    }
    return(EHC_REININT);
}

//------------------------------------------------------------------------------

void CHostSubSystemOS::SaveToCache(CXMLElement* p_ele)
{
    CXMLElement* p_cele = p_ele->CreateChildElement("os");
    p_cele->SetAttribute("na",Distribution);
    p_cele->SetAttribute("ke",Kernel);
    p_cele->SetAttribute("vm",Virtualization);
    p_cele->SetAttribute("ar",Architecture);
    p_cele->SetAttribute("dt",DistroToken);
}

//------------------------------------------------------------------------------

void CHostSubSystemOS::PrintSubSystemInfo(CVerboseStr& vout)
{
    if( CachedData ){
    vout << ">>> os ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ [cached] ~~" << endl;
    } else {
    vout << ">>> os ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    }

    vout << "    Configuration  : " << GetConfigFile() <<  endl;
    vout << "    Distribution   : " << Distribution <<  endl;
    vout << "    Kernel         : " << Kernel <<  endl;
    vout << "    Virtualization : " << Virtualization <<  endl;
    vout << "    Architecture   : " << Architecture <<  endl;
    if( DistroToken != NULL ) {
    vout << "    Distro token   : " << DistroToken <<  endl;
    } else {
    vout << "    Distro token   : " << "-none-" <<  endl;
    }
}

//------------------------------------------------------------------------------

void CHostSubSystemOS::PrintHostInfoFor(CVerboseStr& vout,EPrintHostInfo mode)
{
    if( mode == EPHI_SITE ) {
    vout << "  Operating system   : " << Distribution << " | " << Kernel;
    if( Virtualization != "-none-" ){
        vout << " (" << Virtualization << ")";
    }
    vout << endl;
    }

    if( mode == EPHI_MODULE ) {
    vout << "  Operating system   : " << Distribution << " | " << Kernel;
    if( Virtualization != "-none-" ){
        vout << " (" << Virtualization << ")";
    }
    vout << endl;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

