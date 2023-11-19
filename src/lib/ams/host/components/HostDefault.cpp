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

#include <HostDefault.hpp>
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

CHostSubSystemDefault::CHostSubSystemDefault(const CFileName& config_file)
    : CHostSubSystem(config_file)
{
    NumOfCPUs       = 1;
    NumOfThreads    = 1;
    NumOfGPUs       = 0;
}

//------------------------------------------------------------------------------

CHostSubSystemDefault::~CHostSubSystemDefault(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CHostSubSystemDefault::Init(void)
{
    CXMLElement* p_ele = GetConfig("default");
    if( p_ele == NULL ){
        INVALID_ARGUMENT("config element 'default' is NULL");
    }

    if( p_ele->GetAttribute("ncpus",NumOfCPUs) == true ){
        NumOfThreads = NumOfCPUs;
    }
    p_ele->GetAttribute("nthreads",NumOfThreads);
    p_ele->GetAttribute("ngpus",NumOfGPUs);

    // default tokens
    string value;
    if( p_ele->GetAttribute("tokens",value) ){
        split(ArchTokens,value,is_any_of("#"));
    }
}

//------------------------------------------------------------------------------

void CHostSubSystemDefault::Apply(void)
{
    Host.SetNumOfHostCPUs(NumOfCPUs);
    Host.SetNumOfHostThreads(NumOfThreads);
    Host.SetNumOfHostGPUs(NumOfGPUs);
    for(CSmallString token : ArchTokens){
        Host.AddArchToken(token);
    }
}

//------------------------------------------------------------------------------

void CHostSubSystemDefault::PrintSubSystemInfo(CVerboseStr& vout)
{
    vout << ">>> default ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    vout <<                  "    Configuration  : " << GetConfigFile() <<  endl;
    vout <<                  "    Num of CPUs    : " << NumOfCPUs << endl;
    vout <<                  "    Num of Threads : " << NumOfThreads << endl;
    vout <<                  "    Num of GPUs    : " << NumOfGPUs << endl;
    CUtils::PrintTokens(vout,"    Tokens         : ", GetTokenList(ArchTokens),80);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

