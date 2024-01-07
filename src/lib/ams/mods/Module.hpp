#ifndef ModuleH
#define ModuleH
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
#include <SmallString.hpp>
#include <XMLElement.hpp>
#include <VerboseStr.hpp>
#include <ShellProcessor.hpp>
#include <list>

//------------------------------------------------------------------------------

// module is actived by the site command
#define MFB_SYSTEM          (1 << 0)

// module is activated by an user
#define MFB_USER            (1 << 1)

// module is activated within infinity job
#define MFB_INFINITY        (1 << 2)

// module is reactivated
#define MFB_REACTIVATED     (1 << 16)

// module is reexported
#define MFB_REEXPORTED      (1 << 17)

// module was autoloaded
#define MFB_AUTOLOADED      (1 << 18)

//-----------------------------------------------------------------------------

enum EModulePrintLevel {
    EAPL_NONE,
    EAPL_SHORT,
    EAPL_FULL,
    EAPL_VERBOSE
};

//-----------------------------------------------------------------------------

enum EModuleError {
    EAE_STATUS_OK = 0,
    EAE_CONFIG_ERROR,
    EAE_MODULE_NOT_FOUND,
    EAE_BUILD_NOT_FOUND,
    EAE_DEPENDENCY_ERROR,
    EAE_NOT_ACTIVE,
    EAE_PERMISSION_DENIED
};

//-----------------------------------------------------------------------------

class AMS_PACKAGE CModule {
public:
// constructor and destructors ------------------------------------------------
    CModule(void);

// executive methods ----------------------------------------------------------
    /// add module - fordep is for depended modules
    EModuleError AddModule(CVerboseStr& vout,CSmallString module,bool fordep=false,bool do_not_export=false);

    /// remove module
    EModuleError RemoveModule(CVerboseStr& vout,CSmallString module);

    /// set print level
    void SetPrintLevel(EModulePrintLevel set);

    /// set module export option
    void SetModuleExportFlag(bool set);

    /// set module flags
    void SetFlags(int flags);

    /// set module flags
    int  GetFlags(void);

// print methods ---------------------------------------------------------------

    /// print informations about module activation
    bool PrintModuleInfo(CVerboseStr& vout,const CSmallString& mod_name);

    /// print informations about module build
    bool PrintBuildInfo(CVerboseStr& vout,const CSmallString& mod_name);

    void StartHelp(void);
    bool AddHelp(const CSmallString& mod_name);
    bool ShowHelp(void);

// section of private methods --------------------------------------------------
private:
    EModulePrintLevel           GlobalPrintLevel;
    int                         Level;
    bool                        ModuleExportFlag;
    int                         ModuleFlags;        // module flags used for statistics
    std::list<CSmallString>     DepList;            // dependency list - to avoid dependency cycles

    CXMLDocument                HTMLHelp;

    /// complete module build
    bool CompleteModule(CVerboseStr& vout,CXMLElement* p_module,
                                     CSmallString& name,
                                     CSmallString& ver,
                                     CSmallString& arch,
                                     CSmallString& mode);

// actions related ------------------------------
    /// solve module deps
    bool SolveModuleDeps(CVerboseStr& vout,CXMLElement* p_dep_container);

    /// solve module desp  -  after module is activated
    bool SolveModulePostDeps(CVerboseStr& vout,CXMLElement* p_dep_container);

    /// determine acceptable architecture for module
    bool DetermineArch(CVerboseStr& vout,CXMLElement* p_module,
                            const CSmallString& ver, CSmallString& arch);

    /// determine acceptable architecture for module
    bool DetermineArchAuto(CVerboseStr& vout,CXMLElement* p_module,
                            const CSmallString& ver, CSmallString& arch);

    /// determine acceptable architecture for module
    bool DetermineArchUser(CVerboseStr& vout,CXMLElement* p_module,
                            const CSmallString& ver, CSmallString& arch);

    /// determine acceptable parallel mode for module
    bool DetermineMode(CVerboseStr& vout,CXMLElement* p_module,
                      const CSmallString& ver, const CSmallString& arch, CSmallString& mode);

    /// determine acceptable parallel mode for module
    bool DetermineModeAuto(CVerboseStr& vout,CXMLElement* p_module,
                      const CSmallString& ver, const CSmallString& arch, CSmallString& mode);

    /// determine acceptable parallel mode for module
    bool DetermineModeUser(CVerboseStr& vout,CXMLElement* p_module,
                      const CSmallString& ver, const CSmallString& arch, CSmallString& mode);

    /// update environment according to module specification
    bool PrepareModuleEnvironment(CXMLElement* p_build,
                                    const CSmallString& complete_module,
                                    const CSmallString& exported_module,
                                    EModuleAction action);

    /// return the name of bundle maintainer
    const CSmallString GetMaintainerName(CXMLElement* p_module);

    /// return support contact email
    const CSmallString GetMaintainerEMail(CXMLElement* p_module);

    /// return bundle name
    const CSmallString GetBundleName(CXMLElement* p_module);

// architectures
    /// compare two architectures
    static bool AreSameTokens(const CSmallString& user_arch,const CSmallString& build_arch);

    /// compare two architectures
    static bool AreSameTokens(const CSmallString& user_arch,const CSmallString& build_arch,
                              int& matches,int& maxmatches);

    /// for module help
    void PreprocessHelpHeaders(CXMLElement* p_ele);
};

//-----------------------------------------------------------------------------

extern CModule Module;

//-----------------------------------------------------------------------------

#endif
