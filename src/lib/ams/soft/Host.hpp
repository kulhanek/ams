#ifndef HostH
#define HostH
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

#include <AMSMainHeader.hpp>
#include <XMLDocument.hpp>
#include <SmallString.hpp>
#include <FileName.hpp>
#include <vector>
#include <string>
#include <list>
#include <set>
#include <VerboseStr.hpp>

//------------------------------------------------------------------------------

//<?xml version="1.0" encoding="ISO-8859-1"?>
//<!-- global hosts config -->
//<config>
//      <!-- the order determines priority, the highest priority has the latest node -->
//      <default tokens="token#token" ncpu="number"/>
//      <hosts tokens="true/false" ncpu="true/false">
//          <host name="name/filter" tokens="token#token" ncpu="number" />
//      </hosts>
//      <cpuinfo tokens="true/false" ncpu="true/false">
//          <filter value="filter"/>
//      </cpuinfo>
//      <torque tokens="true/false" ncpu="true/false">
//          <filter value="filter"/>
//      </torque>
//      <net>
//          <iface filter="filter" tokens="token#token"/>
//      </net>
//</config>

//------------------------------------------------------------------------------

class AMS_PACKAGE CHost {
public:
// constructor and destructors -------------------------------------------------
    CHost(void);

// input methods ---------------------------------------------------------------
    /// load hosts configuration file
    void InitHostFile(void);

    /// init global setup
    void InitGlobalSetup(void);

    /// init current host
    void InitHost(int ncpus,int ngpus);

    /// init current host
    void InitHost(bool nocache=false);

    /// clear all setup
    void ClearAll(void);

    /// load host cache
    void LoadCache(void);

    /// save host cache
    void SaveCache(void);

// information methods ---------------------------------------------------------
    /// print node info
    void PrintNodeInfo(CVerboseStr& vout);

    /// get host CPU model
    const CSmallString& GetCPUModel(void) const;

    /// get host GPU models
    const std::vector<std::string>& GetGPUModels(void) const;

    /// is GPU model SMP?
    bool IsGPUModelSMP(void);

    /// get host name
    const CSmallString GetHostName(void);

    /// overwrite hostname from HOSTNAME variable
    void SetHostName(const CSmallString& hostname);

    /// return number of accesible CPUs
    int GetNCPUs(void);

    /// return number of nodes
    int GetNNodes(void);

    /// return number of accesible GPUs
    int GetNGPUs(void);

    /// return number of CPUs per node
    int GetNumOfHostCPUs(void);

    /// return number of threads per node
    int GetNumOfHostThreads(void);

    /// return number of GPUs per node
    int GetNumOfHostGPUs(void);

    /// return system architecture tokens separated by ,
    const CSmallString GetArchTokens(void);

    /// get root element with parallel modes
    CXMLElement* GetRootParallelModes(void);

    /// print info
    void PrintHostDetailedInfo(CVerboseStr& vout);

    /// get arch token score
    int GetArchTokenScore(const CSmallString& token);

    /// hes token?
    bool HasToken(const CSmallString& token);

    /// is loaded from cache
    bool IsLoadedFromCache(void);

    /// cache validity in seconds
    long int CacheValidity(void);

    /// print host HW specification
    void PrintHWSpec(CVerboseStr& vout);

    /// find host group
    CXMLElement* FindGroup(void);

    /// find host group
    CXMLElement* FindGroup(const CSmallString& hostname,bool personal=false);

    /// find host group namespace
    const CSmallString GetGroupNS(void);

    /// find host group namespace
    const CSmallString GetGroupNS(const CSmallString& hostname,bool personal=false);

    /// find host realm
    const CSmallString GetRealm(void);

    /// find host realm
    const CSmallString GetRealm(const CSmallString& hostname,bool personal=false);

    /// get CUDA capabilities
    CXMLElement* GetCUDACapabilities(void);

// section of private date -----------------------------------------------------
private:
    CFileName                   CacheFileName;          // cache file name
    CXMLDocument                Hosts;                  // host configuration   
    CSmallString                Hostname;               // hostname
    bool                        AlienHost;              // hostname is overwritten
    CSmallString                ConfigRealm;            // group or default config
    CSmallString                ConfigKey;              // config key

    long int                    CacheTime;              // when the cache was created
    bool                        CacheLoaded;            // if cache is loaded it is not saved

    int                         NCPUs;                  // number of requested CPUs
    int                         NGPUs;                  // number of requested GPUs
    int                         NNodes;                 // number of requested nodes

    std::vector<std::string>    DefaultTokens;          // default tokens from host file
    int                         DefaultNumOfHostCPUs;

    std::vector<std::string>    HostTokens;             // architecture tokens from host file
    int                         HostNumOfHostCPUs;

    std::vector<std::string>    CompatTokens;           // compatibility tokens
    std::list<std::string>      AllTokens;              // all tokens

    // gpuinfo
    std::vector<std::string>    GPUInfoTokens;

// cached items ----------------------------------------------------------------
    // header
    int                         NumOfHostCPUs;          // default max. number of CPUs per node
    int                         NumOfHostThreads;
    CSmallString                CPUModelName;           // cpu model name
    CSmallString                CPURawModelName;        // cpu model name without mem
    int                         NumOfHostGPUs;          // default max. number of GPUs per node
    std::vector<std::string>    GPUModelNames;          // gpu model names
    CSmallString                GPURawModelName;        // gpu model name without mem

    // cpuinfo
    std::vector<std::string>    CPUInfoTokens;          // data from lscpu
    int                         CPUInfoNumOfHostCPUs;
    int                         CPUInfoNumOfHostThreads;
    int                         CPUInfoNumOfThreadsPerCore;
    std::vector<std::string>    CPUInfoFlags;           // list of CPU flags
    float                       CPUSpec;

    // desktop
    bool                        IsDesktop;              // is the host a personal desktop?
    int                         DesktopCPUPenalty;      // how many CPUs are not availbale for PBSPro node

    // cuda
    std::vector<std::string>    CUDATokens;
    CSmallString                CudaFilter;
    CSmallString                CudaLib;
    CSmallString                CudaVisibleDevs;
    std::vector<std::string>    GPUCapabilities;        // list of GPU capabilities

    // network tokens
    std::set<std::string>       NetDevs;        // available network devices
    CSmallString                NetFilters;
    std::vector<std::string>    NetTokens;      // network tokens

    /// init default tokens
    void InitDefaultTokens(CXMLElement* p_ele);

    /// init Hosts tokens
    void InitHostsTokens(CXMLElement* p_ele);

    /// init CPU tokens
    void InitCPUInfoTokens(CXMLElement* p_ele);

    /// init desktop tokens
    void InitDesktopTokens(CXMLElement* p_ele);

    /// init compatibility tokens
    void InitCompatibilityTokens(CXMLElement* p_ele);

    /// init GPU info tokens
    void InitGPUInfoTokens(CXMLElement* p_ele);

    /// init cuda GPU tokens
    void InitCudaGPUTokens(CXMLElement* p_ele);

    /// init network tokens
    void InitNetworkTokens(CXMLElement* p_ele);

    ///  what is enabled in configuartion section
    const CSmallString WhatIsEnabled(CXMLElement* p_ele);

    /// get section tokens
    const CSmallString GetSecTokens(std::vector<std::string>& list);
    const CSmallString GetSecTokens(std::set<std::string>& list);

    // cpuinfo
    void GetCPUModelAndFlags(CSmallString& model,std::vector<std::string>& flags)
;
};

// -----------------------------------------------------------------------------

extern CHost    Host;

//------------------------------------------------------------------------------

#endif
