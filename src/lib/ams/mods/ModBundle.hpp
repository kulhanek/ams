#ifndef ModBundleH
#define ModBundleH
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
#include <ModCache.hpp>
#include <FileName.hpp>
#include <XMLDocument.hpp>
#include <VerboseStr.hpp>

//------------------------------------------------------------------------------

class AMS_PACKAGE CModBundle : public CModCache {
public:
// constructor and destructors -------------------------------------------------
    CModBundle(void);

// bundle methods --------------------------------------------------------------
    /// get bundle root path
    static bool GetBundleRoot(const CFileName& path,CFileName& bundle_root);

    /// initialize bundle
    bool CreateBundle(const CFileName& path,const CFileName& name,
                      const CSmallString& maintainer,const CSmallString& contact);

    /// initialize bundle
    bool InitBundle(const CFileName& bundle_path);

// executive methods -----------------------------------------------------------
    /// find all fragment files
    void FindAllFragmentFiles(void);

    /// rebuild bundle caches
    bool RebuildCache(CVerboseStr& vout);

    /// load cache
    bool LoadCache(void);

    /// save small and big caches
    bool SaveCaches(void);

// information methods ---------------------------------------------------------
    /// return ID of the bundle
    const CSmallString GetID(void) const;

    /// return the name of bundle maintainer
    const CSmallString GetMaintainerName(void);

    /// return support contact email
    const CSmallString GetMaintainerEMail(void);

    /// print info about the bundle
    void PrintInfo(CVerboseStr& vout);

// section of private data -----------------------------------------------------
private:
    CFileName       BundlePath;
    CFileName       BundleName;
    CXMLDocument    Config;

    std::list<CFileName>    DocFiles;
    std::list<CFileName>    BldFiles;

    // statistics
    std::list<CSmallString>     Archs;
    std::list<CSmallString>     Modes;
    std::list<CSmallString>     DisabledMods;
    std::list<CSmallString>     NoDocMods;
    int                         NumOfDocs;      // number of doc files
    int                         NumOfBlds;      // number of bld files

    /// record audit message
    void AuditAction(const CSmallString& message);

    /// add documentation
    bool AddDocumentation(CVerboseStr& vout,CXMLElement* p_cele, const CFileName& docu_file);

    /// add build
    bool AddBuild(CVerboseStr& vout,CXMLElement* p_cele, const CFileName& build_file);
};

//------------------------------------------------------------------------------

extern CModBundle   ModBundle;

//-----------------------------------------------------------------------------

#endif
