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

#include <HostNetwork.hpp>
#include <ErrorSystem.hpp>
#include <Utils.hpp>
#include <Host.hpp>
#include <sys/types.h>
#include <ifaddrs.h>
#include <fnmatch.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CHostSubSystemNetwork::CHostSubSystemNetwork(const CFileName& config_file)
    : CHostSubSystem(config_file)
{
    CachedData = false;
}

//------------------------------------------------------------------------------

CHostSubSystemNetwork::~CHostSubSystemNetwork(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CHostSubSystemNetwork::Init(void)
{
    CXMLElement* p_ele = GetConfig("network");
    if( p_ele == NULL ){
        INVALID_ARGUMENT("config element 'network' is NULL");
    }

    // list available network interfaces
    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1) {
        ES_WARNING("unable to get list of network devices");
        return;
    }

    // Walk through linked list, maintaining head pointer so we can free list later
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        NetDevs.push_back(ifa->ifa_name);
    }

    freeifaddrs(ifaddr);

    NetDevs.unique();

    int nfilt = 0;

    CXMLElement* p_fele = p_ele->GetFirstChildElement("iface");
    while( p_fele != NULL ){
        CSmallString filter;
        std::string stokens;

        // load config
        bool success = true;

        success &= p_fele->GetAttribute("filter",filter);
        success &= p_fele->GetAttribute("tokens",stokens);

        // move to next record
        p_fele = p_fele->GetNextSiblingElement("iface");

        if( success == false ){
            ES_WARNING("undefined filter or tokens attributes for iface element");
            continue;
        }

        // does match any device
        std::list<CSmallString>::iterator   dit = NetDevs.begin();
        std::list<CSmallString>::iterator   die = NetDevs.end();

        while( dit != die ){
            CSmallString dev = *dit;
            if( fnmatch(filter,dev,0) == 0 ){
                if( nfilt == 0 ){
                    NetFilters = filter;
                } else {
                    NetFilters += ",";
                    NetFilters += filter;
                }
                nfilt++;
                // add dev tokens if available
                std::vector<std::string> tokens;
                split(tokens,stokens,is_any_of("#"));

                std::vector<std::string>::iterator  it = tokens.begin();
                std::vector<std::string>::iterator  ie = tokens.end();
                while( it != ie ){
                    ArchTokens.push_back(*it);
                    it++;
                }
            } else {
                CSmallString warning;
                warning << "net device " << dev << " does not match filter " << filter;
                ES_WARNING(warning);
            }
            dit++;
        }
    }

    if( NetFilters == NULL ){
        NetFilters = "-none matching-";
    }
    ArchTokens.unique();

    CachedData = false;
}

//------------------------------------------------------------------------------

void CHostSubSystemNetwork::Apply(void)
{
    for(CSmallString token : ArchTokens){
        Host.AddArchToken(token);
    }
}

//------------------------------------------------------------------------------

EHostCacheMode CHostSubSystemNetwork::LoadFromCache(CXMLElement* p_ele)
{
    CXMLElement* p_cele = p_ele->GetFirstChildElement("network");
    if( p_cele == NULL ) return(EHC_REININT);

    bool result = true;
    string slist;

    result &= p_cele->GetAttribute("flt",NetFilters);

    slist.clear();
    result &= p_cele->GetAttribute("dev",slist);
    if( result && (! slist.empty()) ) split(NetDevs,slist,is_any_of("#"));

    slist.clear();
    result &= p_cele->GetAttribute("tks",slist);
    if( result && (! slist.empty()) ) split(ArchTokens,slist,is_any_of("#"));

    CachedData = result;
    if( CachedData ){
        return(EHC_LOADED);
    }
    return(EHC_REININT);
}

//------------------------------------------------------------------------------

void CHostSubSystemNetwork::SaveToCache(CXMLElement* p_ele)
{
    CXMLElement* p_cele = p_ele->CreateChildElement("network");
    p_cele->SetAttribute("dev",GetTokenList(NetDevs,"#"));
    p_cele->SetAttribute("flt",NetFilters);
    p_cele->SetAttribute("tks",GetTokenList(ArchTokens,"#"));
}

//------------------------------------------------------------------------------

void CHostSubSystemNetwork::PrintSubSystemInfo(CVerboseStr& vout)
{
    if( CachedData ){
    vout << ">>> network ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ [cached] ~~" << endl;
    } else {
    vout << ">>> network ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    }
    vout <<                  "    Configuration  : " << GetConfigFile() <<  endl;
    CUtils::PrintTokens(vout,"    Net devices    : ", GetTokenList(NetDevs), 80, ' ');
    vout <<                  "    Net filters    : " << NetFilters << endl;
    CUtils::PrintTokens(vout,"    Net tokens     : ", GetTokenList(ArchTokens), 80, ' ');
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

