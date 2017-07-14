#ifndef ActionsH
#define ActionsH
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
#include <XMLElement.hpp>
#include <XMLDocument.hpp>
#include <vector>
#include <VerboseStr.hpp>

//-----------------------------------------------------------------------------

enum EActionPrintLevel {
    EAPL_NONE,
    EAPL_SHORT,
    EAPL_FULL,
    EAPL_VERBOSE
};

enum EActionError {
    EAE_STATUS_OK = 0,
    EAE_CONFIG_ERROR,
    EAE_MODULE_NOT_FOUND,
    EAE_BUILD_NOT_FOUND,
    EAE_DEPENDENCY_ERROR,
    EAE_NOT_ACTIVE,
    EAE_PERMISSION_DENIED
};

//-----------------------------------------------------------------------------

class AMS_PACKAGE CActions {
public:
    // constructor and destructors ------------------------------------------------
    CActions(void);

    // executive methods ----------------------------------------------------------
    /// add module - fordep is for depended modules
    EActionError AddModule(std::ostream& vout,CSmallString module,bool fordep=false,bool do_not_export=false);

    /// remove module
    EActionError RemoveModule(std::ostream& vout,CSmallString module);

    /// reactivate all active modules
    void ReactivateModules(std::ostream& vout);

    /// deactivate all active modules
    bool PurgeModules(std::ostream& vout);

    /// set print level
    void SetActionPrintLevel(EActionPrintLevel set);

    /// set module export option
    void SetModuleExportFlag(bool set);

    /// complete module build
    bool CompleteModule(std::ostream& vout,CXMLElement* p_module,
                                     CSmallString& name,
                                     CSmallString& ver,
                                     CSmallString& arch,
                                     CSmallString& mode);

    /// set module flags
    void SetFlags(int flags);

    /// set module flags
    int  GetFlags(void);

    // section of private methods ------------------------------------------------
private:
    EActionPrintLevel           GlobalPrintLevel;
    int                         Level;
    bool                        ModuleExportFlag;
    int                         ModuleFlags;        // module flags used for statistics
    std::vector<CSmallString>   DepList;            // dependency list - to avoid dependency cycles

    // actions related ------------------------------
    /// solve module deps
    bool SolveModuleDeps(std::ostream& vout,CXMLElement* p_dep_container);

    /// solve module desp  -  after module is activated
    bool SolveModulePostDeps(std::ostream& vout,CXMLElement* p_dep_container);

    /// determine acceptable architecture for module
    bool DetermineArchitecture(std::ostream& vout,CXMLElement* p_module,
                                            const CSmallString& ver,
                                            CSmallString& arch);

    /// determine acceptable parallel mode for module
    bool DetermineMode(std::ostream& vout,CXMLElement* p_module,
                                          const CSmallString& ver,
                                          const CSmallString& arch,
                                          CSmallString& mode);

    /// update environment according to module specification
    bool PrepareModuleEnvironment(CXMLElement* p_build,
                                    const CSmallString& complete_module,
                                    const CSmallString& exported_module,
                                    bool add_module);

    /// remove module with name from list of module specs seprated by '|'
    const CSmallString RemoveModule(const CSmallString& module_list,
                                    const CSmallString& name);

    /// append module spec to list of module specs seprated by '|'
    const CSmallString AppendModule(const CSmallString& module_list,
                                    const CSmallString& module);

    bool TryOneParaProperty(std::ostream& vout,CXMLElement* p_module,
                                        const CSmallString& name,
                                        const CSmallString& ver,
                                        const CSmallString& arch,
                                        const CSmallString& mode);
    bool TryLEParaProperty(std::ostream& vout,CXMLElement* p_module,
                                        const CSmallString& name,
                                        const CSmallString& ver,
                                        const CSmallString& arch,
                                        const CSmallString& mode);
    bool TryGTParaProperty(std::ostream& vout,CXMLElement* p_module,
                                        const CSmallString& name,
                                        const CSmallString& ver,
                                        const CSmallString& arch,
                                        const CSmallString& mode);
    bool TryAlwaysParaProperty(std::ostream& vout,CXMLElement* p_module,
                                        const CSmallString& name,
                                        const CSmallString& ver,
                                        const CSmallString& arch,
                                        const CSmallString& mode);
};

//-----------------------------------------------------------------------------

extern CActions Actions;

//-----------------------------------------------------------------------------

#endif
