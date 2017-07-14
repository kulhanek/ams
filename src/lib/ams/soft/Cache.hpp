#ifndef CacheH
#define CacheH
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
#include <FileName.hpp>
#include <VerboseStr.hpp>
#include <vector>
#include <list>
#include <string>

//-----------------------------------------------------------------------------

/* ======= MODULE BLOCK STRUCTURE ========
<module name=""
        enabled="true/false"
        export="true/false"
        >
    <build ver="" arch="" para="">
        <variable name="" value=""
                  operation="append/preppend/set"
                  priority="normal/modaction"/>
        <script name="" type="child" priority="normal/modaction"/>
        <deps>
            <dep name="" type="add/post/sync/rm/deb"/>
        </deps>
    </build>
    <deps>
        <dep name="" type="add/post/sync/rm/deb"/>
    </deps>
    <acl>
    </acl>
    <doc>
    </doc>
    <default ver="" arch="" para=""/>
</module>
*/

//-----------------------------------------------------------------------------

class AMS_PACKAGE CCache {
public:
// constructor and destructors -------------------------------------------------
    CCache(void);

// input/output methods --------------------------------------------------------
    /// init whole module cache
    bool LoadCache(bool loadbig=false);

    /// init whole module cache
    bool LoadCache(const CSmallString& site_sid,bool loadbig=false);

    /// save system cache to file
    bool SaveCache(bool savebig);

    /// get root element of cache
    CXMLElement* GetRootElementOfCache(void);

// executive methods from module-cache -----------------------------------------
    /// rebuild system cache from individual module specifications
    bool RebuildCache(CVerboseStr& vout,bool as_it_is);
    
    /// remove documentation
    void RemoveDocumentation(void);

    /// split cache into individual modules
    bool SplitCache(CVerboseStr& vout);

    /// split cache into individual builds
    bool SplitMoreCache(CVerboseStr& vout);

    /// test module cache syntax
    bool CheckCacheSyntax(CVerboseStr& vout);

    /// clear cache and create empty cache element
    void ClearCache(void);

// www information methods -----------------------------------------------------
    /// get module description
    CXMLElement* GetModuleDescription(CXMLElement* p_module);

    /// search cache for modules matching pattern
    bool SearchCache(const CSmallString& pattern,
                            std::vector<CXMLElement*>& hits);

    /// is there a cyclic dependency
    bool TestCrossDependency(const CSmallString& module,const CSmallString& depmodule);

    /// test if the specified module partially exist in the cache
    bool TestModuleByPartialName(const CSmallString& module);

    /// test if permission is granted (only module)
    bool IsPermissionGrantedForModule(CXMLElement* p_module);

    /// test if permission is granted (module and all build)
    bool IsPermissionGrantedForModuleByAnyMeans(CXMLElement* p_module);

    /// test if permission is granted (only build)
    bool IsPermissionGrantedForBuild(CXMLElement* p_build);

    /// test if permission is granted (only acl)
    bool IsPermissionGranted(CXMLElement* p_acl);

    /// test if module has any compatible build
    bool HasModuleAnyCompatibleBuild(CXMLElement* p_cmod,const CSmallString& arch);

// information methods ---------------------------------------------------------
    /// return module specification for module with name
    CXMLElement* GetModule(const CSmallString& name);

    /// return module build for specified module
    CXMLElement* GetBuild(CXMLElement* p_module,
                            const CSmallString& ver,
                            const CSmallString& arch,
                            const CSmallString& mode);

    /// return default setup of module
    bool GetModuleDefaults(CXMLElement* p_module,
                            CSmallString& ver,
                            CSmallString& arch,
                            CSmallString& mode);

    /// return value of build variable
    static const CSmallString GetVariableValue(CXMLElement* p_rele,
                            const CSmallString& name);

    /// check module version
    bool CheckModuleVersion(CXMLElement* p_module,const CSmallString& ver);

    /// check module version and architecture
    bool CheckModuleArchitecture(CXMLElement* p_module,
                            const CSmallString& ver,
                            const CSmallString& arch);

    /// check module version, architecture, and parallel mode
    bool CheckModuleMode(CXMLElement* p_module,
                            const CSmallString& ver,
                            const CSmallString& arch,
                            const CSmallString& mode);

    /// check if module can be exported - default true
    bool CanModuleBeExported(CXMLElement* p_module);

    /// return list of all versions
    void GetAllModuleVersions(CXMLElement* p_module, std::list<CSmallString>& vers);

    /// remove all builds with given architecture token
    void RemoveBuildsWithArchToken(const CSmallString& token);

    /// does it need GPU resources
    bool DoesItNeedGPU(const CSmallString& name);

// print information methods ---------------------------------------------------
    /// get length of longest module spec (name+ver+arch+mode)
    int GetSizeOfLongestModuleSpecification(void);

    /// get length of longest module name (name + ver)
    int GetSizeOfLongestModuleName(bool include_ver=true);

    /// return sorted vector of module versions
    bool GetSortedModuleVersions(const CSmallString& mod_name,std::vector<std::string>& versions);

// section of private data -----------------------------------------------------
private:
    CXMLDocument    Cache;

// section of private methods --------------------------------------------------
    /// rebuild cache from information stored in directory
    bool RebuildCacheFromDirectory(CVerboseStr& vout,
                            CXMLElement* p_mele,
                            const CFileName& module_dir,bool as_it_is);

    /// add module to the cache
    bool AddModule(CVerboseStr& vout,
                            const CFileName& module_dir,
                            const CFileName& name,
                            CXMLElement* p_cele,bool as_it_is);

    /// set build variable
    bool SetVariableValue(CXMLElement* p_rele,
                            const CSmallString& name,
                            const CSmallString& value,
                            const CSmallString& priority);

// syntax check methods --------------------------------------------------------
    /// check module element syntax
    bool CheckModuleSyntax(CVerboseStr& vout,CXMLElement* p_module);

    /// check syntax of documentation element
    bool CheckModuleDocSyntax(CVerboseStr& vout,CXMLElement* p_module,CXMLElement* p_doc);

    /// check syntax of ACL element
    bool CheckModuleACLSyntax(CVerboseStr& vout,CXMLElement* p_builds);

    /// check syntax of builds element
    bool CheckModuleBuildsSyntax(CVerboseStr& vout,CXMLElement* p_builds);

    /// check syntax of build element
    bool CheckModuleBuildSyntax(CVerboseStr& vout,CXMLElement* p_build);

    /// check syntax of build element
    bool CheckModuleSetupSyntax(CVerboseStr& vout,CXMLElement* p_setup);

    /// check syntax of dependecies element
    bool CheckModuleDepsSyntax(CVerboseStr& vout,CXMLElement* p_dependencies);

    /// check syntax of default element
    bool CheckModuleDefaultSyntax(CVerboseStr& vout,CXMLElement* p_default);

    /// check syntax of script element
    bool CheckModuleScriptSyntax(CVerboseStr& vout,CXMLElement* p_script);

    /// check syntax of variable element
    bool CheckModuleVariableSyntax(CVerboseStr& vout,CXMLElement* p_variable);

    /// check syntax of variable element
    bool CheckModuleAliasSyntax(CVerboseStr& vout,CXMLElement* p_alias);
};

//-----------------------------------------------------------------------------

extern CCache   Cache;

//-----------------------------------------------------------------------------

#endif
