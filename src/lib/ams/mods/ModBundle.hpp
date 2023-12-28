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
#include <boost/shared_ptr.hpp>
#include <set>
#include <map>

//------------------------------------------------------------------------------

enum EModBundleCache {
    EMBC_NONE   = 0,
    EMBC_SMALL  = 1,
    EMBC_BIG    = 2,
};

//------------------------------------------------------------------------------

class AMS_PACKAGE CModBundleIndex {
public:
    /// load index
    bool LoadIndex(const CFileName& index_name);

    /// save index
    bool SaveIndex(const CFileName& index_name);

    /// diff two indexes
    void Diff(CModBundleIndex& old_index, CVerboseStr& vout, bool skip_removed, bool skip_added);

    /// clear index
    void Clear(void);

// section of public data ------------------------------------------------------
public:
    std::map<CSmallString,CFileName>    Paths;
    std::map<CSmallString,std::string>  Hashes;
};

//------------------------------------------------------------------------------

class AMS_PACKAGE CModBundle : public CModCache {
public:
// constructor and destructors -------------------------------------------------
    CModBundle(void);

// bundle methods --------------------------------------------------------------
    /// get bundle root path
    static bool GetBundleRoot(const CFileName& path,CFileName& bundle_root);

    /// is bundle?
    static bool IsBundle(const CFileName& path,const CFileName& name);

    /// initialize bundle
    bool CreateBundle(const CFileName& path,const CFileName& name,
                      const CSmallString& maintainer,const CSmallString& contact,bool froce);

    /// initialize bundle
    bool InitBundle(const CFileName& bundle_path);

// executive methods -----------------------------------------------------------
    /// find all fragment files
    void FindAllFragmentFiles(void);

    /// rebuild bundle caches
    bool RebuildCache(CVerboseStr& vout);

    /// load cache
    bool LoadCache(EModBundleCache type);

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
    void PrintInfo(CVerboseStr& vout,bool mods=false,bool stat=false,bool audit=false);

    /// return bundle config element
    CXMLElement* GetBundleElement(void);

// bundle index operation ------------------------------------------------------
    /// get list of build for index
    bool ListBuildsForIndex(CVerboseStr& vout,bool personal);

    /// calculate new index
    void CalculateNewIndex(CVerboseStr& vout);

    /// save index
    bool SaveNewIndex(void);

    /// commit index
    bool CommitNewIndex(void);

    /// load new and old indexes
    bool LoadIndexes(void);

    /// diff two indexes
    void DiffIndexes(CVerboseStr& vout, bool skip_removed, bool skip_added);

// section of private data -----------------------------------------------------
private:
    CFileName       BundlePath;
    CFileName       BundleName;
    CXMLDocument    Config;
    CXMLDocument    AuditLog;
    EModBundleCache CacheType;

    std::list<CFileName>    DocFiles;
    std::list<CFileName>    BldFiles;

    // statistics
    std::list<CSmallString>     Archs;
    std::list<CSmallString>     Modes;
    std::list<CSmallString>     DisabledMods;
    std::list<CSmallString>     NoDocMods;
    int                         NumOfDocs;      // number of doc files
    int                         NumOfBlds;      // number of bld files

    // indexes
    bool                                PersonalBundle;
    int                                 NumOfAllBuilds;
    int                                 NumOfUniqueBuilds;
    int                                 NumOfNonSoftRepoBuilds;
    int                                 NumOfSharedBuilds;
    std::set<CSmallString>              UniqueBuilds;
    std::set<CFileName>                 UniqueBuildPaths;
    CModBundleIndex                     NewBundleIndex;
    CModBundleIndex                     OldBundleIndex;

    /// record audit message
    void AuditAction(const CSmallString& message);

    /// add documentation
    bool AddDocumentation(CVerboseStr& vout,CXMLElement* p_cele, const CFileName& docu_file);

    /// add build
    bool AddBuild(CVerboseStr& vout,CXMLElement* p_cele, const CFileName& build_file);

    /// rebuild default build for all modules
    void RebuildModuleDefaultBuilds(void);

    /// init default build element
    void InitDefaultBuild(CXMLElement* p_mele);
};

//-----------------------------------------------------------------------------

typedef boost::shared_ptr<CModBundle>   CModBundlePtr;

//-----------------------------------------------------------------------------

#endif
