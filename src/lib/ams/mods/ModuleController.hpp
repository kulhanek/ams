#ifndef SiteControllerH
#define SiteControllerH
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

//------------------------------------------------------------------------------

class AMS_PACKAGE CSiteController {
public:

    // information about modules ---------------------------------------------------
        /// check if module is active
        bool IsModuleActive(const CSmallString& module);

        /// get version of active module
        bool GetActiveModuleVersion(const CSmallString& module,CSmallString& actver);

        /// return complete specification of active modules
        const CSmallString& GetActiveModules(void);

        /// return export specification of exported modules
        const CSmallString& GetExportedModules(void);

        /// return complete specification of active module
        const CSmallString GetActiveModuleSpecification(const CSmallString& name);

        /// return export specification of active module
        const CSmallString GetExportedModuleSpecification(const CSmallString& name);

    // update module info ---------------------------------------------------------
        /// update list of active modules
        void UpdateActiveModules(const CSmallString& module,bool add_module);

        /// update list of exported modules
        void UpdateExportedModules(const CSmallString& module,bool add_module);

        /// set list of exported modules
        void SetExportedModules(const CSmallString& modules);

// section of private data ----------------------------------------------------
private:
    CSmallString    ActiveModules;      // list of active modules
    CSmallString    ExportedModules;    // list of exported modules
};

//------------------------------------------------------------------------------

extern CSiteController  SiteController;

//------------------------------------------------------------------------------

#endif
