#ifndef HostSubSystemGPUNVidiaH
#define HostSubSystemGPUNVidiaH
// =============================================================================
//  AMS - Advanced Module System
// -----------------------------------------------------------------------------
//     Copyright (C) 2023 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2020 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2012 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2011 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2004,2005,2008,2010 Petr Kulhanek (kulhanek@chemi.muni.cz)
//
//     This program is free software; you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation; either version 2 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License along
//     with this program; if not, write to the Free Software Foundation, Inc.,
//     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// =============================================================================

#include <AMSMainHeader.hpp>
#include <HostSubSystem.hpp>
#include <FileName.hpp>
#include <XMLElement.hpp>
#include <VerboseStr.hpp>
#include <list>

// -----------------------------------------------------------------------------

class AMS_PACKAGE CHostSubSystemGPUNVidia : public CHostSubSystem {
public:
// constructor -----------------------------------------------------------------
    CHostSubSystemGPUNVidia(const CFileName& config_file);
    ~CHostSubSystemGPUNVidia(void);

// input methods ---------------------------------------------------------------
    /// init host subsystem
    virtual void Init(void);

    /// apply setup to Host
    virtual void Apply(void);

    /// load host subsystem information from the host cache
    virtual EHostCacheMode LoadFromCache(CXMLElement* p_ele);

    /// save host subsystem information into the host cache
    virtual void SaveToCache(CXMLElement* p_ele);

    /// print subsystem info
    virtual void PrintSubSystemInfo(CVerboseStr& vout);

    /// print node resources (for PBSPro)
    virtual void PrintNodeResources(CVerboseStr& vout);

    /// print host resources for site
    virtual void PrintHostInfoFor(CVerboseStr& vout,EPrintHostInfo mode);

// section of private data -----------------------------------------------------
private:
    bool                    CachedData;
    CSmallString            CudaDev;
    CFileName               CudaLib;
    int                     NumOfHostGPUs;
    CSmallString            GPURawModelName;    // for PBSPro
    std::list<CSmallString> GPUModels;          // gpu models, sorted by cuda capabilities
    bool                    UseCapaTokens;
    std::list<CSmallString> CapaTokens;         // cuda capability tokens
    std::list<CSmallString> ArchTokens;         // cuda arch tokens

    /// helper
    bool IsGPUModelSMP(void);
};

// -----------------------------------------------------------------------------

#endif
