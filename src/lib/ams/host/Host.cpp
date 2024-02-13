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

#include <Host.hpp>
#include <HostGroup.hpp>
#include <ctype.h>
#include <ErrorSystem.hpp>
#include <XMLParser.hpp>
#include <XMLPrinter.hpp>
#include <XMLElement.hpp>
#include <XMLIterator.hpp>
#include <ShellProcessor.hpp>
#include <AMSRegistry.hpp>
#include <FileSystem.hpp>
#include <Utils.hpp>
#include <UserUtils.hpp>
#include <Shell.hpp>
#include <iomanip>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

//------------------------------------------------------------------------------

// how long the cache is valid in seconds
#define CACHE_VALIDITY 86400

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//------------------------------------------------------------------------------

CHost    Host;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CHost::CHost(void)
{
    NumOfCPUs           = 0;
    NumOfGPUs           = 0;
    NumOfNodes          = 0;

    NumOfHostCPUs       = 0;
    NumOfHostThreads    = 0;
    NumOfHostGPUs       = 0;

    HostCacheLoaded     = false;
    HostCacheTime       = 0;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CHost::InitHostSubSystems(const CFileName& host_subsystems)
{
// global
    HostName    = CShell::GetSystemVariable("HOSTNAME");

    NumOfCPUs = 1;
    CSmallString sNCPU;
    sNCPU = CShell::GetSystemVariable("INF_NCPUS");
    if( sNCPU != NULL ) {
        NumOfCPUs = sNCPU.ToInt();
    }

    NumOfGPUs = 0;
    CSmallString sNGPU;
    sNGPU = CShell::GetSystemVariable("INF_NGPUS");
    if( sNGPU != NULL ) {
        NumOfGPUs = sNGPU.ToInt();
    }

    NumOfNodes = 1;
    CSmallString sNNodes;
    sNNodes = CShell::GetSystemVariable("INF_NNODES");
    if( sNNodes != NULL ) {
        NumOfNodes = sNNodes.ToInt();
    }

// subsystems
    CFileName   path = AMSRegistry.GetHostSubSystemsSearchPaths();
    std::string modules = string(host_subsystems);
    std::vector<CFileName>  module_list;
    split(module_list,modules,is_any_of(","),boost::token_compress_on);

    for(CFileName module : module_list){
        CFileName full_name;
        if( CUtils::FindFile(path,module,".xml",full_name) == false ){
            // record
            continue;
        }
        CHostSubSystemPtr hs = CHostSubSystem::Create(full_name);
        if( hs == NULL ){
            // record
            continue;
        }
        HostSubSystems.push_back(hs);
    }

// cache
    HostCacheName   = GetDefaultHostCacheName();
    HostCacheLoaded = false;
    HostCacheKey    = HostGroup.GetDefaultHostCacheKey();
    HostCacheTime   = 0;

    NumOfHostCPUs       = 0;
    NumOfHostThreads    = 0;
    NumOfHostGPUs       = 0;
}

//------------------------------------------------------------------------------

void CHost::LoadCache(void)
{
    if( CFileSystem::IsFile(HostCacheName) == false ){
        // no cache file
        CSmallString warning;
        warning << "no cache file '" << HostCacheName << "'";
        ES_WARNING(warning);
        return;
    }

    CXMLParser xml_parser;
    xml_parser.SetOutputXMLNode(&HostCache);
    if( xml_parser.Parse(HostCacheName) == false ){
        ErrorSystem.RemoveAllErrors(); // avoid global error
        CSmallString warning;
        warning << "unable to parse cache '" << HostCacheName << "'";
        ES_WARNING(warning);
        return;
    }

    CXMLElement* p_ele = HostCache.GetFirstChildElement("cache");
    if( p_ele == NULL ) return;

// master ID -----------
    // this is id from the host file
    CSmallString cache_key;

    p_ele->GetAttribute("key",cache_key);
    if( cache_key != HostCacheKey ){
        ES_WARNING("master/cache keys mismatch");
        return;
    }

    // this is current time
    CSmallTimeAndDate current_time;
    current_time.GetActualTimeAndDate();
    p_ele->GetAttribute("lid",HostCacheTime);
    if( current_time.GetSecondsFromBeginning() > HostCacheTime + CACHE_VALIDITY ){
        ES_WARNING("cache too old");
        return;
    }

    HostCacheLoaded = true;
}

//------------------------------------------------------------------------------

void CHost::SaveCache(void)
{
    CXMLDocument xml_document;
    xml_document.CreateChildDeclaration();
    xml_document.CreateChildComment("AMS host cache file");
    CXMLElement* p_cache = xml_document.CreateChildElement("cache");

// header
    // this is id from the host file
    p_cache->SetAttribute("key",HostCacheKey);

    // this is current time
    CSmallTimeAndDate current_time;
    current_time.GetActualTimeAndDate();
    p_cache->SetAttribute("lid",current_time.GetSecondsFromBeginning());

// individual host submodules
    for(CHostSubSystemPtr hs : HostSubSystems){
        hs->SaveToCache(p_cache);
    }

// save to file
    CXMLPrinter xml_printer;
    xml_printer.SetPrintedXMLNode(&xml_document);
    if( xml_printer.Print(HostCacheName) == false ){
        CSmallString warning;
        warning << "unable to save cache '" << HostCacheName << "'";
        ES_WARNING(warning);
    }
}

//------------------------------------------------------------------------------

bool CHost::IsLoadedFromCache(void)
{
    return( HostCacheLoaded );
}

//------------------------------------------------------------------------------

long int CHost::CacheValidity(void)
{
    if( HostCacheLoaded == false ) return(0);
    CSmallTimeAndDate current_time;
    current_time.GetActualTimeAndDate();
    long int tv = HostCacheTime + CACHE_VALIDITY - current_time.GetSecondsFromBeginning();
    if( tv > 0 ){
        return( tv );
    } else {
        return( 0 );
    }
}

//------------------------------------------------------------------------------

void CHost::InitHost(bool nocache)
{
    // try to load cache
    if( nocache == false) LoadCache();
    CXMLElement* p_cache = HostCache.GetChildElementByPath("cache",true);

    // load cache
    for(CHostSubSystemPtr hs : HostSubSystems){
        switch(hs->LoadFromCache(p_cache)){
        case EHC_REININT:
            hs->Init();
            HostCacheLoaded = false;
            break;
        case EHC_IGNORED:
            hs->Init();
            break;
        default:
            // ignore
            break;
        }
    }

    // apply setup
    for(CHostSubSystemPtr hs : HostSubSystems){
        hs->Apply();
    }

    // sort tokens
    HostTokens.sort();
    HostTokens.unique();

    // save host cache
    if( HostCacheLoaded == false ){
        SaveCache();
    }
}

//------------------------------------------------------------------------------

void CHost::InitHost(int ncpus, int ngpus)
{
    NumOfCPUs = ncpus;
    NumOfGPUs = ngpus;
    InitHost(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CHost::GetHostName(void)
{
    return(HostName);
}

//------------------------------------------------------------------------------

int CHost::GetNCPUs(void)
{
    return(NumOfCPUs);
}

//------------------------------------------------------------------------------

int CHost::GetNNodes(void)
{
    return(NumOfNodes);
}

//------------------------------------------------------------------------------

int CHost::GetNGPUs(void)
{
    return(NumOfGPUs);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CHost::GetNumOfHostCPUs(void)
{
    return(NumOfHostCPUs);
}

//------------------------------------------------------------------------------

int CHost::GetNumOfHostThreads(void)
{
    return(NumOfHostThreads);
}

//------------------------------------------------------------------------------

int CHost::GetNumOfHostGPUs(void)
{
    return(NumOfHostGPUs);
}

//------------------------------------------------------------------------------

void CHost::SetNumOfHostCPUs(int ncpus)
{
    NumOfHostCPUs = ncpus;
}

//------------------------------------------------------------------------------

void CHost::SetNumOfHostThreads(int nthreads)
{
    NumOfHostThreads = nthreads;
}

//------------------------------------------------------------------------------

void CHost::SetNumOfHostGPUs(int ngpus)
{
    NumOfHostGPUs = ngpus;
}

//------------------------------------------------------------------------------

const CSmallString CHost::GetArchTokens(void)
{
    CSmallString tokens;

    std::list<CSmallString>::iterator    it = HostTokens.begin();
    std::list<CSmallString>::iterator    ie = HostTokens.end();

    while(it != ie){
        if( it != HostTokens.begin() )  tokens << ",";
        tokens << (*it);
        it++;
    }
    return(tokens);
}

//------------------------------------------------------------------------------

bool CHost::HasToken(const CSmallString& token)
{
    std::list<CSmallString>::iterator    it = HostTokens.begin();
    std::list<CSmallString>::iterator    ie = HostTokens.end();

    while( it != ie ){
        if( *it == token ){
            return(true);
        }
        it++;
    }

    return(false);
}

//------------------------------------------------------------------------------

void CHost::AddArchToken(const CSmallString& token)
{
    HostTokens.push_back(token);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CHost::PrintHostInfo(CVerboseStr& vout)
{
    vout << endl;
    vout << "# Full host name      : " << GetHostName() << endl;
    vout << "# Host cache key      : " << HostCacheKey << endl;
    if( HostCacheLoaded ){
    vout << "# Host cache name     : " << HostCacheName << endl;
    CSmallTime time(CacheValidity());
    vout << "# Loaded from cache ... (Cache is still valid for " << time.GetSTimeAndDay() << ")" << endl;
    } else {
    CSmallTime time(CACHE_VALIDITY);
    vout << "# No cache loaded ...   (New cache will be valid for " << time.GetSTimeAndDay() << ")" << endl;
    }
    vout << "# ==============================================================================" << endl;

    for(CHostSubSystemPtr hs : HostSubSystems){
        hs->PrintSubSystemInfo(vout);
    }

    vout << "# ==============================================================================" << endl;
    vout << endl;
    PrintHostInfoForSite(vout);
}

//------------------------------------------------------------------------------

void CHost::PrintHostInfoForSite(CVerboseStr& vout)
{
    if( IsLoadedFromCache() ){
    vout << "# ~~~ <b>Host info</b> ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ [cached] ~~" << endl;
    } else {
    vout << "# ~~~ <b>Host info</b> ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    }
    vout << "  Full host name     : " << Host.GetHostName() << endl;

    for(CHostSubSystemPtr hs : HostSubSystems){
        hs->PrintHostInfoFor(vout,EPHI_SITE);
    }

    CUtils::PrintTokens(vout,"  Host arch tokens   : ",GetArchTokens(), 80,' ');
}

//------------------------------------------------------------------------------

void CHost::PrintHostInfoForModule(CVerboseStr& vout)
{
    vout <<                  "  Requested CPUs     : " << setw(3) << GetNCPUs();
    vout <<                  "  Requested GPUs     : " << setw(3) << GetNGPUs()<< endl;
    vout <<                  "  Num of host CPUs   : " << setw(3) << GetNumOfHostCPUs();
    vout <<                  "  Num of host GPUs   : " << setw(3) << GetNumOfHostGPUs() << endl;
    vout <<                  "  Requested nodes    : " << setw(3) << GetNNodes() << endl;
    CUtils::PrintTokens(vout,"  Host arch tokens   : ",GetArchTokens(),80,' ');

    for(CHostSubSystemPtr hs : HostSubSystems){
        hs->PrintHostInfoFor(vout,EPHI_MODULE);
    }
}

//------------------------------------------------------------------------------

void CHost::PrintNodeResources(CVerboseStr& vout)
{
    for(CHostSubSystemPtr hs : HostSubSystems){
        hs->PrintNodeResources(vout);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CFileName CHost::GetDefaultHostCacheName(void)
{
    CFileName host_cache = CShell::GetSystemVariable("AMS_HOST_CACHE");
    if( host_cache != NULL ) return(host_cache);

    CFileName host_cache_dir = CShell::GetSystemVariable("AMS_HOST_CACHE_DIR");
    if( host_cache_dir == NULL ){
        host_cache_dir = "/tmp";
    }
    host_cache = host_cache_dir / "ams_cache_r09." + CUserUtils::GetUserName();
    return(host_cache);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


