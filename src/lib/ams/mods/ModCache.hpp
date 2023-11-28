#ifndef ModCacheH
#define ModCacheH
// =============================================================================
//  AMS - Advanced Module System
// -----------------------------------------------------------------------------
//     Copyright (C) 2023 Petr Kulhanek (kulhanek@chemi.muni.cz)
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
#include <FileName.hpp>
#include <Terminal.hpp>
#include <VerboseStr.hpp>
#include <list>

//------------------------------------------------------------------------------

class AMS_PACKAGE CModCache {
public:
// constructor and destructors -------------------------------------------------
    CModCache(void);

// setup methods ---------------------------------------------------------------
    /// load a single cache file
    bool LoadCacheFile(const CFileName& name);

    /// save a single cache file
    bool SaveCacheFile(const CFileName& name);

// executive methods -----------------------------------------------------------
    /// remove documentation elements
    void RemoveDocumentation(void);

    /// clean build - remove comments and text nodes
    void CleanBuild(CXMLNode* p_bele);

    // merge caches - p_origin is bundle config
    void MergeWithCache(CXMLElement* p_bcele,CXMLElement* p_origin=NULL);

    /// create empty cache and return pointer to <cache> element
    CXMLElement* CreateEmptyCache(void);

// information methods ---------------------------------------------------------
    /// get module element
    CXMLElement* GetModule(const CSmallString& name,bool create=false);

    /// create module element
    CXMLElement* CreateModule(const CSmallString& name);

    /// return default setup for the module
    bool GetModuleDefaults(CXMLElement* p_mele,
                           CSmallString& ver, CSmallString& arch, CSmallString& mode);

    /// get module categories
    void GetCategories(std::list<CSmallString>& list);

    /// get modules in the given categories
    void GetModules(const CSmallString& category, std::list<CSmallString>& list,bool includever=false);

    /// get module versions
    void GetModuleVersions(CXMLElement* p_mele, std::list<CSmallString>& list);

    /// get module versions - sorted by veridx
    void GetModuleVersionsSorted(CXMLElement* p_mele, std::list<CSmallString>& list);

    /// get module builds
    void GetModuleBuildsSorted(CXMLElement* p_mele, std::list<CSmallString>& list);

    /// print available modules
    void PrintAvail(CTerminal& terminal,bool includever=false,bool includesys=false);

    /// print module versions
    void PrintModuleVersions(CVerboseStr& out, const CSmallString& module);

    /// print module builds
    void PrintModuleBuilds(CVerboseStr& vout, const CSmallString& module);

    /// print module origin
    void PrintModuleOrigin(CVerboseStr& vout, const CSmallString& module);

    /// get number of modules
    int GetNumberOfModules(void);

    /// get number of categories
    int GetNumberOfCategories(void);

    /// return cache element
    CXMLElement* GetCacheElement(void);

// section of protected data ---------------------------------------------------
protected:
    CXMLDocument    Cache;
};

//------------------------------------------------------------------------------

extern CModCache   ModCache;

//-----------------------------------------------------------------------------

#endif
