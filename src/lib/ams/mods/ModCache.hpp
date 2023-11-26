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

// information methods ---------------------------------------------------------
    /// get module element
    CXMLElement* GetModule(const CSmallString& name,bool create=false);

    /// create module element
    CXMLElement* CreateModule(const CSmallString& name);

    /// get module categories
    void GetCategories(std::list<CSmallString>& list);

    /// get modules in the given categories
    void GetModules(const CSmallString& category, std::list<CSmallString>& list,bool includever=false);

    /// get module versions
    void GetModuleVersions(CXMLElement* p_mele, std::list<CSmallString>& list);

    /// get module versions - sorted by veridx
    void GetModuleVersionsSorted(CXMLElement* p_mele, std::list<CSmallString>& list);

    /// print available modules
    void PrintAvail(CTerminal& terminal,bool includever=false,bool includesys=false);

// section of protected data ---------------------------------------------------
protected:
    CXMLDocument    Cache;
};

//------------------------------------------------------------------------------

extern CModCache   ModCache;

//-----------------------------------------------------------------------------

#endif
