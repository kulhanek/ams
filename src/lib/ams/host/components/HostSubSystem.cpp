// =============================================================================
//  AMS - Advanced Module System
// -----------------------------------------------------------------------------
//     Copyright (C) 2023 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include <HostSubSystem.hpp>
#include <XMLParser.hpp>
#include <ErrorSystem.hpp>
#include <FileSystem.hpp>

// host subsystems
#include <HostDefault.hpp>
#include <HostOS.hpp>
#include <HostCPU.hpp>
#include <HostGPU.hpp>
#include <HostGPU-NVidia.hpp>
#include <HostDesktop.hpp>
#include <HostNetwork.hpp>
#include <HostCompat.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CHostSubSystem::CHostSubSystem(const CFileName& config_file)
{
    ConfigFile = config_file;
    CXMLParser xml_parser;
    xml_parser.SetOutputXMLNode(&Config);

    if( xml_parser.Parse(ConfigFile) == false ) {
        CSmallString    error;
        error << "unable to load '" << ConfigFile << "'";
        RUNTIME_ERROR(error);
    }
}

//------------------------------------------------------------------------------

CHostSubSystem::~CHostSubSystem(void)
{
}

//------------------------------------------------------------------------------

CHostSubSystemPtr CHostSubSystem::Create(const CFileName& file_name)
{
    if( CFileSystem::IsFile(file_name) == false ){
        // no host subsystem file
        CSmallString error;
        error << "no host subsystem file '" << file_name << "'";
        RUNTIME_ERROR(error);
    }

    CXMLParser      xml_parser;
    CXMLDocument    xml_config;
    xml_parser.SetOutputXMLNode(&xml_config);
    if( xml_parser.Parse(file_name) == false ){
        CSmallString error;
        error << "unable to parse host subsystem file '" << file_name << "'";
        RUNTIME_ERROR(error);
    }

    CXMLElement* p_ele = xml_config.GetFirstChildElement();
    if( p_ele == NULL ){
        CSmallString error;
        error << "no child XML element in the host subsystem file '" << file_name << "'";
        RUNTIME_ERROR(error);
    }
    CSmallString name = p_ele->GetName();

    CHostSubSystemPtr sub_module;

    if( name == "default" ){
        sub_module = CHostSubSystemPtr(new CHostSubSystemDefault(file_name));
    } else if( name == "os" ){
        sub_module = CHostSubSystemPtr(new CHostSubSystemOS(file_name));
    } else if( name == "cpu" ){
        sub_module = CHostSubSystemPtr(new CHostSubSystemCPU(file_name));
    } else if( name == "gpu" ){
        sub_module = CHostSubSystemPtr(new CHostSubSystemGPU(file_name));
    } else if( name == "gpu-nvidia" ){
        sub_module = CHostSubSystemPtr(new CHostSubSystemGPUNVidia(file_name));
    } else if( name == "desktop" ){
        sub_module = CHostSubSystemPtr(new CHostSubSystemDesktop(file_name));
    } else if( name == "network" ){
        sub_module = CHostSubSystemPtr(new CHostSubSystemNetwork(file_name));
    } else if( name == "compat" ){
        sub_module = CHostSubSystemPtr(new CHostSubSystemCompat(file_name));
    } else {
        CSmallString error;
        error << "unsupported host submodule '" << name << "'";
        RUNTIME_ERROR(error);
    }

    return(sub_module);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CHostSubSystem::Init(void)
{
}

//------------------------------------------------------------------------------

void CHostSubSystem::Apply(void)
{
}

//------------------------------------------------------------------------------

const CFileName& CHostSubSystem::GetConfigFile(void)
{
    return(ConfigFile);
}

//------------------------------------------------------------------------------

CXMLElement* CHostSubSystem::GetConfig(const CSmallString& section)
{
    return(Config.GetChildElementByPath(section));
}

//------------------------------------------------------------------------------

const CSmallString CHostSubSystem::GetTokenList(std::list<CSmallString>& tokens,const CSmallString delim)
{
    CSmallString stokens;

    std::list<CSmallString>::iterator    it = tokens.begin();
    std::list<CSmallString>::iterator    ie = tokens.end();

    while(it != ie){
        if( it != tokens.begin() )  stokens << delim;
        stokens << (*it);
        it++;
    }
    return(stokens);
}

//------------------------------------------------------------------------------

EHostCacheMode CHostSubSystem::LoadFromCache(CXMLElement* p_ele)
{
    return(EHC_IGNORED);
}

//------------------------------------------------------------------------------

void CHostSubSystem::SaveToCache(CXMLElement* p_ele)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CHostSubSystem::PrintSubSystemInfo(CVerboseStr& vout)
{

}

//------------------------------------------------------------------------------

void CHostSubSystem::PrintNodeResources(CVerboseStr& vout)
{

}

//------------------------------------------------------------------------------

void CHostSubSystem::PrintHostInfoFor(CVerboseStr& vout,EPrintHostInfo mode)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

