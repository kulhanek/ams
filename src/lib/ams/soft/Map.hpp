#ifndef MapH
#define MapH
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
#include <SmallString.hpp>
#include <XMLDocument.hpp>
#include <XMLElement.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <map>
#include <list>
#include <set>
#include <vector>

//------------------------------------------------------------------------------

typedef boost::shared_ptr<CXMLDocument>  CXMLDocumentPtr;
class CSite;

//------------------------------------------------------------------------------

struct SFullBuild {
    std::string prefix;
    std::string build;

    bool operator < (const SFullBuild& right) const;
};

//------------------------------------------------------------------------------

/// map builds to individual sites

class AMS_PACKAGE CMap {
public:
// constructor and destructor --------------------------------------------------
    CMap(void);

// input/output methods --------------------------------------------------------
    /// load map
    bool LoadMap(void);

    /// save map
    bool SaveMap(void);

    /// load auto prefixes
    bool LoadAutoPrefixes(const CSmallString& user_prefix);

    /// undo map change
    bool UndoMapChange(void);

    /// return number of possible undo changes
    int GetNumOfUndoMapChanges(void);

    /// load site aliases
    void LoadSiteAliases(void);

    /// print aliases
    void PrintSiteAliases(std::ostream& vout);

// executive methods -----------------------------------------------------------
    /// add site to builds satisfying filter condition
    bool AddBuildsForSites(std::ostream& vout,const CSmallString& sites,const CSmallString& filter);

    /// add site to builds satisfying filter condition
    bool AddBuilds(std::ostream& vout,const CSmallString& site,const CSmallString& prefix,const CSmallString& filter);

    /// remove site from builds satisfying filter condition
    void RemoveBuildsForSites(std::ostream& vout,const CSmallString& sites,const CSmallString& filter);

    /// remove site from builds satisfying filter condition
    void RemoveBuilds(std::ostream& vout,const CSmallString& site,const CSmallString& filter);

    /// set default for given site list
    bool SetDefaultForSites(const CSmallString& sites,const CSmallString& defname);

    /// set default for given site
    bool SetDefault(const CSmallString& site,const CSmallString& defname);

    /// remove default from given site list
    bool RemoveDefaultForSites(const CSmallString& sites,const CSmallString& modname);

    /// remove default from given site
    bool RemoveDefault(const CSmallString& site,const CSmallString& modname);

    /// remove module from given site list
    bool RemoveModuleForSites(const CSmallString& sites,const CSmallString& modname);

    /// remove module from given site
    bool RemoveModule(const CSmallString& site,const CSmallString& modname);

    /// list available build prefixes
    void ShowPrefixes(std::ostream& vout);

    /// show all builds
    void ShowAllBuilds(std::ostream& vout);

    /// show builds from given prefix and satysfying given filter condition
    void ShowBuilds(std::ostream& vout,const CSmallString& prefix,const CSmallString& filter);

    /// print all builds satisfying filter, prefix/site/autoprefix
    void ShowAutoBuilds(std::ostream& vout,const CSmallString& site_name,const CSmallString& filter,
                         const CSmallString& prefix);

    /// print best build for module employing prefix/site/autoprefix
    void ShowBestBuild(std::ostream& vout,const CSmallString& site_name,const CSmallString& module,
                         const CSmallString& prefix);

    /// print AMS_PACKAGE_DIR
    bool ShowPkgDir(std::ostream& vout,const CSmallString& site_name,
                      const CSmallString& build,const CSmallString& prefix);

    /// show all maps
    bool ShowMapForSites(std::ostream& vout,const CSmallString& sites);

    /// show map
    bool ShowMap(std::ostream& vout,const CSmallString& site);

    /// remove map for site list
    bool RemoveMapForSites(std::ostream& vout,const CSmallString& sites);

    /// remove map for site
    bool RemoveMap(std::ostream& vout,const CSmallString& site);

    /// copy site map
    bool CopyMap(const CSmallString& site1,const CSmallString& site2);

    /// update version indexes in the map
    void UpdateVerIndexes(std::ostream& vout);

    /// get new version index for module
    double GetNewVerIndex(const CSmallString& module);

    /// remove orphan sites from the map
    void RemoveOrphanSites(std::ostream& vout);

    /// remove orphan bulds
    void RemoveOrphanBuilds(std::ostream& vout);

    /// refactor builds
    bool RefactorBuilds(std::ostream& vout);

    /// refactor documentation files
    bool RefactorDocs(std::ostream& vout);

    /// perform whole mapping procedure
    bool DistributeAll(std::ostream& vout);

    /// clean modules on all sites
    bool CleanSiteModules(std::ostream& vout);

    /// distribute modules on all sites
    bool DistributeSiteModules(std::ostream& vout);

    /// check if the build exists
    bool IsBuild(const CSmallString& site_name,const CSmallString& build_name,
                 const CSmallString& prefix);

    /// print sync deps
    void ShowSyncDeps(std::ostream& vout,const CSmallString& site_name,
                      const CSmallString& build,const CSmallString& prefix,bool deep);

// section of private data ----------------------------------------------------
private:
    CXMLDocument                                    MapDoc;
    std::map<std::string, std::list<std::string> >  SiteAliases;
    std::map<std::string,CXMLDocumentPtr>           BuildCache;
    std::map<std::string,CXMLDocumentPtr>           DocCache;
    std::list<std::string>                          AutoPrefixes;

// helper methods -------------------------------------------------------------
    CXMLElement* FindSite(const CSmallString& name,bool create);
    CXMLElement* FindModule(CXMLElement* p_site,const CSmallString& name,bool create);
    CXMLElement* FindBuild(CXMLElement* p_mod,const CSmallString& build,bool create);

    bool DistributeSiteMap(std::ostream& vout,CSite* p_site);
    bool DistributeSiteModuleMap(std::ostream& vout, CXMLElement* p_mod,CSite* p_site);
    bool InjectBuildIntoSite(CXMLElement* p_builds,const CSmallString& prefix,const CSmallString& build);
    bool InjectRootDocumentation(CXMLElement* p_sele,CXMLElement* p_mele,const CSmallString& site);
    bool InjectVerIndx(CXMLElement* p_build);
    void GetAllSites(std::list<std::string>& sites);
    bool BackupMap(void);
    void AddSyncDeps(const CSmallString& site_name,const CSmallString& build_name,
                     const CSmallString& prefix,std::list<std::string>& deps,bool deep);
    void ListBuilds(const CSmallString& prefix,std::vector<SFullBuild>& builds);
    void ListBuilds(const CSmallString& prefix,const CSmallString& filter,std::set<SFullBuild>& builds);
    void ListPrefixes(std::vector<std::string>& prefixes);
    const CSmallString GetBuildName(const CSmallString& site_name,const CSmallString& build_name,
                                   const CSmallString& prefix);
    void RemoveOrphanBuilds(CFileName buildlib, std::ostream& vout);
    CXMLElement* PopulateCache(const CSmallString& site_name,const CSmallString& module,const CSmallString& prefix,CSmallString& best_ver);
    void PrintBestBuild(std::ostream& vout,const CSmallString& site_name,const CSmallString& my_build,
                          const CSmallString& prefix);
    bool RefactorBuilds(CFileName buildlib, std::ostream& vout);
    bool RefactorBuild(CFileName buildname);
    void RefactorDeps(CXMLElement* p_deps);
    bool RefactorDocs(CFileName buildlib, std::ostream& vout);
    bool RefactorDoc(CFileName docname);
 };

//------------------------------------------------------------------------------

#endif
