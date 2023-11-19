#ifndef HostH
#define HostH
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

#include <AMSMainHeader.hpp>
#include <XMLDocument.hpp>
#include <SmallString.hpp>
#include <HostSubSystem.hpp>
#include <list>

//------------------------------------------------------------------------------

class AMS_PACKAGE CHost {
public:
// constructor and destructor --------------------------------------------------
    CHost(void);

// input methods ---------------------------------------------------------------
    /// init host subsystems
    void InitHostSubSystems(const CFileName& host_subsystems);

    /// init current host
    void InitHost(bool nocache=false);

// information methods ---------------------------------------------------------
    /// get host name
    const CSmallString GetHostName(void);

    /// print info
    void PrintHostInfo(CVerboseStr& vout);

    /// print host info for site
    void PrintHostInfoForSite(CVerboseStr& vout);

    /// print node info
    void PrintNodeResources(CVerboseStr& vout);

// selected resources ----------------------------------------------------------

    /// return number of accesible CPUs
    int GetNCPUs(void);

    /// return number of accesible GPUs
    int GetNGPUs(void);

    /// return number of nodes
    int GetNNodes(void);

// available resources ----------------------------------------------------------

    /// return number of CPUs per node
    int GetNumOfHostCPUs(void);

    /// return number of threads per node
    int GetNumOfHostThreads(void);

    /// return number of GPUs per node
    int GetNumOfHostGPUs(void);

    /// return system architecture tokens separated by ,
    const CSmallString GetArchTokens(void);

    /// has token?
    bool HasToken(const CSmallString& token);

    /// set number of CPUs per node
    void SetNumOfHostCPUs(int ncpus);

    /// set number of threads per node
    void SetNumOfHostThreads(int nthreads);

    /// set number of GPUs per node
    void SetNumOfHostGPUs(int ngpus);

    /// add architecture token
    void AddArchToken(const CSmallString& token);

// section of private date -----------------------------------------------------
private:
    CSmallString                    HostName;
    std::list<CHostSubSystemPtr>    HostSubSystems;

    CFileName                       HostCacheName;
    CSmallString                    HostCacheKey;       // global cache key to invalidate all caches
    bool                            HostCacheLoaded;
    CXMLDocument                    HostCache;
    int                             HostCacheTime;

// available resources
    int                             NumOfCPUs;
    int                             NumOfGPUs;
    int                             NumOfNodes;

// host resources
    int                             NumOfHostCPUs;
    int                             NumOfHostThreads;
    int                             NumOfHostGPUs;
    std::list<CSmallString>         HostTokens;

    /// load host cache
    void LoadCache(void);

    /// save host cache
    void SaveCache(void);

    /// is loaded from cache
    bool IsLoadedFromCache(void);

    /// cache validity in seconds
    long int CacheValidity(void);

    /// return default host cache name
    const CFileName GetDefaultHostCacheName(void);
};

// -----------------------------------------------------------------------------

extern CHost    Host;

//------------------------------------------------------------------------------

#endif
