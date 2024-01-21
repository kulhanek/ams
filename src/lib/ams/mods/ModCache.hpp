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
    /// return cache element
    CXMLElement* GetCacheElement(void);

    /// get module element
    CXMLElement* GetModule(const CSmallString& name,bool create=false);

    /// create module element
    CXMLElement* CreateModule(const CSmallString& name);

    /// return module build for specified module
    static CXMLElement* GetBuild(CXMLElement* p_mele,
                            const CSmallString& ver,
                            const CSmallString& arch,
                            const CSmallString& mode);

    /// return default setup for the module
    static bool GetModuleDefaults(CXMLElement* p_mele,
                           CSmallString& ver, CSmallString& arch, CSmallString& mode);

    /// return module documentation
    static CXMLElement* GetModuleDoc(CXMLElement* p_mele);

    /// get module versions
    static void GetModuleVersions(CXMLElement* p_mele, std::list<CSmallString>& list);

    /// get module versions - sorted by veridx
    static void GetModuleVersionsSorted(CXMLElement* p_mele, std::list<CSmallString>& list);

    /// get module builds
    static void GetModuleBuildsSorted(CXMLElement* p_mele, std::list<CSmallString>& list);

    /// get module builds
    static void GetModuleBuildsSorted(CXMLElement* p_mele, const CSmallString& vers, std::list<CSmallString>& list);

    /// check if module can be exported - default true
    static bool CanModuleBeExported(CXMLElement* p_mele);

    /// check module version
    static bool CheckModuleVersion(CXMLElement* p_mele,const CSmallString& ver);

    /// return value of build variable
    static const CSmallString GetVariableValue(CXMLElement* p_rele, const CSmallString& name);

    /// get module bundle
    static const CSmallString  GetBundleName(CXMLElement* p_mele);

    /// get module bundle maintainer
    static const CSmallString  GetBundleMaintainer(CXMLElement* p_mele);

    /// get module bundle contact
    static const CSmallString  GetBundleContact(CXMLElement* p_mele);

// permissions
    /// test if permission is granted (only module)
    static bool IsPermissionGrantedForModule(CXMLElement* p_mele);

    /// test if permission is granted (only build)
    static bool IsPermissionGrantedForBuild(CXMLElement* p_bele);

    /// test if permission is granted (only acl)
    static bool IsPermissionGranted(CXMLElement* p_acl);

// information methods ---------------------------------------------------------
    /// get module categories
    void GetCategories(std::list<CSmallString>& list);

    /// get modules in the given categories
    void GetModules(const CSmallString& category, std::list<CSmallString>& list,bool includever=false);

    /// get list of builds for cgen
    void GetBuildsForCGen(std::list<CSmallString>& list,int numparts);

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

    /// get column size for all modules
    int GetModulePrintSize(bool includever);

    /// does module need GPU?
    bool DoesItNeedGPU(const CSmallString& name);

    /// get new version index for the build
    double GetNewVerIndex(const CSmallString& build);

    /// is autoload enabled for the module
    bool IsAutoloadEnabled(const CSmallString& name);

// section of protected data ---------------------------------------------------
protected:
    CXMLDocument    Cache;
};

//------------------------------------------------------------------------------

extern CModCache   ModCache;

//-----------------------------------------------------------------------------

#endif
