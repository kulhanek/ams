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

#include <HostCompat.hpp>
#include <Utils.hpp>
#include <Host.hpp>
#include <ErrorSystem.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CHostSubSystemCompat::CHostSubSystemCompat(const CFileName& config_file)
    : CHostSubSystem(config_file)
{
}

//------------------------------------------------------------------------------

CHostSubSystemCompat::~CHostSubSystemCompat(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CHostSubSystemCompat::Apply(void)
{
    CXMLElement* p_ele = GetConfig("compat");
    if( p_ele == NULL ){
        INVALID_ARGUMENT("config element 'compat' is NULL");
    }

    string stokens = string(Host.GetArchTokens());
    std::list<string> tokens;

    split(tokens,stokens,is_any_of(","));

    // add all compatibility tokens to ArchTokens
    std::list<string>::iterator it = tokens.begin();
    std::list<string>::iterator ie = tokens.end();
    while( it != ie ){
        CXMLElement* p_arch = p_ele->GetFirstChildElement("arch");
        while( p_arch ){
            string arch_name;
            p_arch->GetAttribute("name",arch_name);
            if( arch_name == (*it) ){
                // add all compatible roots into the hosts tokens
                CXMLElement* p_nele = p_arch->GetFirstChildElement("nextmatch");
                while( p_nele ){
                    CSmallString narch;
                    p_nele->GetAttribute("name",narch);
                    ArchTokens.push_back(narch);
                    p_nele = p_nele->GetNextSiblingElement("nextmatch");
                }
            }
            p_arch = p_arch->GetNextSiblingElement("arch");
        }
        it++;
    }


    for(CSmallString token : ArchTokens){
        Host.AddArchToken(token);
    }
}

//------------------------------------------------------------------------------

void CHostSubSystemCompat::PrintSubSystemInfo(CVerboseStr& vout)
{
    vout << ">>> compat ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    vout <<                  "    Configuration  : " << GetConfigFile() <<  endl;
    CUtils::PrintTokens(vout,"    Compat tokens  : ",GetTokenList(ArchTokens), 80);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

