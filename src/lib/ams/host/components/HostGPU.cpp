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

#include <HostGPU.hpp>
#include <ErrorSystem.hpp>
#include <Utils.hpp>
#include <Host.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CHostSubSystemGPU::CHostSubSystemGPU(const CFileName& config_file)
    : CHostSubSystem(config_file)
{
    NumOfAvailableGPUs = 0;
}

//------------------------------------------------------------------------------

CHostSubSystemGPU::~CHostSubSystemGPU(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CHostSubSystemGPU::Init(void)
{
    CXMLElement* p_ele = GetConfig("gpu");
    if( p_ele == NULL ){
        INVALID_ARGUMENT("config element 'gpu' is NULL");
    }

    NumOfAvailableGPUs = Host.GetNGPUs();
    if( NumOfAvailableGPUs > 0 ){
        // default tokens
        string value;
        if( p_ele->GetAttribute("tokens",value) ){
            split(ArchTokens,value,is_any_of("#"));
        }
    }
}

//------------------------------------------------------------------------------

void CHostSubSystemGPU::Apply(void)
{
    for(CSmallString token : ArchTokens){
        Host.AddArchToken(token);
    }
}

//------------------------------------------------------------------------------

void CHostSubSystemGPU::PrintSubSystemInfo(CVerboseStr& vout)
{
    vout << ">>> gpu ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    vout <<                  "    Configuration  : " << GetConfigFile() <<  endl;
    vout <<                  "    Available GPUs : " << NumOfAvailableGPUs << endl;
    CUtils::PrintTokens(vout,"    Tokens         : ", GetTokenList(ArchTokens),80);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

