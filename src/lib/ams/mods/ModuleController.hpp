#ifndef ModuleControllerH
#define ModuleControllerH
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
#include <FileName.hpp>
#include <ModBundle.hpp>
#include <ShellProcessor.hpp>
#include <list>

//------------------------------------------------------------------------------

class AMS_PACKAGE CModuleController {
public:
// setup methods ---------------------------------------------------------------
    /// init module controller configuration
    void InitModuleControllerConfig(void);

// bundles operation -----------------------------------------------------------
    /// load bundles
    void LoadBundles(EModBundleCache type);

    /// print info about loaded bundles
    void PrintBundlesInfo(CVerboseStr& vout);

    /// merge them into a single cache
    void MergeBundles(void);

// information about modules ---------------------------------------------------
    /// check if module is active
    bool IsModuleActive(const CSmallString& module);

    /// check if module is exported
    bool IsModuleExported(const CSmallString& module);

    /// get version of active module
    bool GetActiveModuleVersion(const CSmallString& module,CSmallString& actver);

    /// return complete specification of active modules
    const CSmallString GetActiveModules(void);

    /// return export specification of exported modules
    const CSmallString GetExportedModules(void);

    /// return complete specification of active module
    const CSmallString GetActiveModuleSpecification(const CSmallString& name);

    /// return export specification of active module
    const CSmallString GetExportedModuleSpecification(const CSmallString& name);

// print lists -----------------------------------------------------------------
    /// print active modules
    void PrintActiveModules(CTerminal& terminal);

    /// print exported modules
    void PrintExportedModules(CTerminal& terminal);

    /// print user auto-loaded modules
    void PrintUserAutoLoadedModules(CTerminal& terminal);

// update module info ----------------------------------------------------------
    /// update list of active modules
    void UpdateActiveModules(const CSmallString& module,EModuleAction action);

    /// update list of exported modules
    void UpdateExportedModules(const CSmallString& module,EModuleAction action);

// execution methods -----------------------------------------------------------
    /// reactivate modules
    bool ReactivateModules(CVerboseStr& vout);

    /// reactivate modules
    bool PurgeModules(CVerboseStr& vout);

// section of private data -----------------------------------------------------
private:
    std::list<CSmallString>     ActiveModules;      // list of active modules
    std::list<CSmallString>     ExportedModules;    // list of exported modules

    CFileName                   BundleName;
    CFileName                   BundlePath;
    std::list<CModBundlePtr>    Bundles;
};

//------------------------------------------------------------------------------

extern CModuleController  ModuleController;

//------------------------------------------------------------------------------

#endif
