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

#include <string.h>
#include <Module.hpp>
#include <ModCache.hpp>
#include <ShellProcessor.hpp>
#include <ErrorSystem.hpp>
#include <ModUtils.hpp>
#include <ModuleController.hpp>
#include <Shell.hpp>
#include <Host.hpp>
#include <HostGroup.hpp>
#include <XMLPrinter.hpp>
#include <iomanip>
#include <map>
#include <list>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//------------------------------------------------------------------------------

CModule Module;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CModule::CModule(void)
{
    GlobalPrintLevel = EAPL_FULL;
    Level = 0;
    ModuleExportFlag = true;
    ModuleFlags = 0;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CModule::SetFlags(int flags)
{
    ModuleFlags = flags;
}

//------------------------------------------------------------------------------

int  CModule::GetFlags(void)
{
    return(ModuleFlags);
}

//------------------------------------------------------------------------------

void CModule::SetPrintLevel(EModulePrintLevel set)
{
    GlobalPrintLevel = set;
}

//-----------------------------------------------------------------------------

void CModule::SetModuleExportFlag(bool set)
{
    ModuleExportFlag = set;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

EModuleError CModule::AddModule(CVerboseStr& vout,CSmallString module,bool fordep,bool do_not_export)
{
    // determine print level -----------------------
    EModulePrintLevel print_level =  GlobalPrintLevel;
    if( (Level > 0) && (GlobalPrintLevel != EAPL_NONE) ) print_level = EAPL_SHORT;
    Level++;

    if( (print_level == EAPL_FULL) || (print_level == EAPL_VERBOSE) ) {
        vout << endl;
        vout << "# Module specification: " << module << " (add action)" << endl;
        vout << "# ==============================================================================" << endl;
    }

    // parse module input --------------------------
    CSmallString name,ver,arch,mode;

    if( (CModUtils::ParseModuleName(module,name,ver,arch,mode) == false) || (name == NULL) ) {
        ES_TRACE_ERROR("module name is empty string");
        Level--;
        return(EAE_MODULE_NOT_FOUND);
    }

    // get module specification --------------------
    CXMLElement* p_module = ModCache.GetModule(name);

    if( p_module == NULL ) {
        CSmallString error;
        error << "module '" << name << "' does not have any record in AMS software database";
        ES_TRACE_ERROR(error);
        Level--;
        return(EAE_MODULE_NOT_FOUND);
    }

    // test permission - module level
    if( CModCache::IsPermissionGrantedForModule(p_module) == false ){
        CSmallString error;
        error << "module '" << name << "' is not allowed for the current user";
        ES_TRACE_ERROR(error);
        Level--;
        return(EAE_PERMISSION_DENIED);
    }

    // clear dependency list if this module is not due to dependency roles
    if( fordep == false ) DepList.clear();

    // add module to dependency list to avoid cyclic dependency problems
    DepList.push_back(name);

    // complete module specification ---------------

    if( CompleteModule(vout,p_module,name,ver,arch,mode) == false ) {
        Level--;
        return(EAE_BUILD_NOT_FOUND);
    }

    CXMLElement* p_build = ModCache.GetBuild(p_module,ver,arch,mode);

    if( p_build == NULL ) {
        CSmallString error;
        error << "build '" <<
              name << ":" << ver << ":" << arch << ":" << mode <<
              "' does not have any record in the AMS database";
        ES_TRACE_ERROR(error);
        Level--;
        return(EAE_BUILD_NOT_FOUND);
    }

    // unload module if it is already loaded -------

    bool reactivating = false;

    if( ModuleController.IsModuleActive(name) == true ) {
        if( (print_level == EAPL_FULL) || (print_level == EAPL_VERBOSE) ) {
            vout << "  INFO:    Module is active, reactivating .. " << endl;
        }
        RemoveModule(vout,name);
        reactivating = true;
    }

    // now solve module desps ---------------

    // prepare module environment for dependency
    if( ShellProcessor.PrepareModuleEnvironmentForDeps(p_build) == false ) {
        CSmallString error;
        error << "unable to solve dependencies for module '" << name << "' (PrepareModuleEnvironmentForDeps)";
        ES_TRACE_ERROR(error);
        Level--;
        return(EAE_DEPENDENCY_ERROR);
    }

    if( SolveModuleDeps(vout,p_module) == false ) {
        CSmallString error;
        error << "unable to solve dependencies for module '" << name << "' (root)";
        ES_TRACE_ERROR(error);
        Level--;
        return(EAE_DEPENDENCY_ERROR);
    }

    if( SolveModuleDeps(vout,p_build) == false ) {
        CSmallString error;
        error << "unable to solve dependencies for module '" << name << "' (build)";
        ES_TRACE_ERROR(error);
        Level--;
        return(EAE_DEPENDENCY_ERROR);
    }

    CSmallString complete_module;
    complete_module = name + ":" + ver + ":" + arch + ":" + mode;

    CSmallString exported_module;
    if( CModCache::CanModuleBeExported(p_module) == true ) {
        exported_module = name + ":" + ver;
    }

    if( do_not_export ){
        exported_module = "";
    }

    // print rest of module info -------------------
    if( (print_level == EAPL_FULL) || (print_level == EAPL_VERBOSE) ) {
        Host.PrintHostInfoForModule(vout);
        if( (GetMaintainerName(p_module) != NULL) && (GetMaintainerEMail(p_module) != NULL) && (GetBundleName(p_module) != NULL) ){
            vout << "  Module maintainer  : " << GetMaintainerName(p_module)
                 << " (" << GetMaintainerEMail(p_module) << ")"
                 << " | Bundle: " << GetBundleName(p_module) << endl;
        }
        vout <<     "# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
        if( (CModCache::CanModuleBeExported(p_module) == true) && (do_not_export == false) ) {
            vout << "  Exported module    : " << exported_module << endl;
        } else {
            if( do_not_export ){
            vout << "  Exported module    : -none- (export is disabled)" << endl;
            } else{
            vout << "  Exported module    : -none- (export is not enabled)" << endl;
            }
        }
        vout <<     "  Module build       : " << complete_module << endl;
    }

    if( PrepareModuleEnvironment(p_build,complete_module,exported_module,EMA_ADD_MODULE) == false ) {
        Level--;
        return(EAE_CONFIG_ERROR);
    }

    if( print_level == EAPL_SHORT ) {
        if( reactivating == true ) {
            vout << "           Loaded module : " << complete_module << " (reactivated)" << endl;
        } else {
            vout << "           Loaded module : " << complete_module << endl;
        }
    }

    if( SolveModulePostDeps(vout,p_build) == false ) {
        CSmallString error;
        error << "unable to solve post dependencies for module '" << name << "' (build)";
        ES_TRACE_ERROR(error);
        Level--;
        return(EAE_DEPENDENCY_ERROR);
    }

    Level--;
    return(EAE_STATUS_OK);
}



//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

EModuleError CModule::RemoveModule(CVerboseStr& vout,CSmallString module)
{
    module.GetSubstitute('/',':');

    // determine print level -----------------------
    EModulePrintLevel print_level =  GlobalPrintLevel;
    if( (Level > 0) && (GlobalPrintLevel != EAPL_NONE) ) print_level = EAPL_SHORT;
    Level++;

    if( (print_level == EAPL_FULL) || (print_level == EAPL_VERBOSE) ) {
        vout << endl;
        vout << "# Module name: " << module << " (remove action)" << endl;
        vout << "# ==============================================================================" << endl;
    }

    // parse module input --------------------------
    CSmallString name,ver,arch,mode;

    if( (CModUtils::ParseModuleName(module,name,ver,arch,mode) == false) || (name == NULL) ) {
        ES_TRACE_ERROR("module name is empty string");
        Level--;
        return(EAE_MODULE_NOT_FOUND);
    }

    // get module specification --------------------
    CXMLElement* p_module = ModCache.GetModule(name);

    if( p_module == NULL ) {
        CSmallString error;
        error << "module '" << name << "' does not have any record in AMS software database";
        ES_WARNING(error);
        Level--;
        return(EAE_MODULE_NOT_FOUND);
    }

    if( ModuleController.IsModuleActive(name) == false ) {
        CSmallString error;
        error << "unable to remove module '" << name << "' because it is not active";
        ES_TRACE_ERROR(error);
        Level--;
        return(EAE_NOT_ACTIVE);
    }

    CSmallString   complete_module;
    complete_module = ModuleController.GetActiveModuleSpecification(name);

    CSmallString exported_module;
    exported_module = ModuleController.GetExportedModuleSpecification(name);

    if( (print_level == EAPL_FULL) || (print_level == EAPL_VERBOSE) ) {
        vout << "  Module build : " << complete_module << endl;
    }

    if( print_level == EAPL_SHORT ) {
        vout << "           Unload module : " << complete_module << endl;
    }

    if( CModUtils::ParseModuleName(complete_module,name,ver,arch,mode) == false ) {
        ES_TRACE_ERROR("unable to parse complete module specification");
        Level--;
        return(EAE_CONFIG_ERROR);
    }

    CXMLElement* p_build = ModCache.GetBuild(p_module,ver,arch,mode);

    if( p_build == NULL ) {
        ES_TRACE_ERROR("unable to get module build");
        Level--;
        return(EAE_BUILD_NOT_FOUND);
    }

    if( PrepareModuleEnvironment(p_build,complete_module,exported_module,EMA_REMOVE_MODULE) == false ) {
        Level--;
        return(EAE_CONFIG_ERROR);
    }

    Level--;
    return(EAE_STATUS_OK);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CModule::SolveModuleDeps(CVerboseStr& vout,CXMLElement* p_dep_container)
{
    if( p_dep_container == NULL ) return(true);

    CXMLElement* p_sele = p_dep_container->GetChildElementByPath("deps/dep");

    bool result = true;
    int count = 0;

    while( p_sele != NULL ) {
        if( p_sele->GetName() == "dep" ) {
            CSmallString lname,ltype;
            p_sele->GetAttribute("name",lname);
            p_sele->GetAttribute("type",ltype);
            if ( ltype == "pre" ) {

                CSmallString lmodname;
                CSmallString lmodver;

                CModUtils::ParseModuleName(lname,lmodname,lmodver);
                // is module already added?
                bool found = false;
                for(CSmallString dep_name : DepList){
                    if( dep_name == lmodname ){
                        found = true;
                        break;
                    }
                }
                if( GlobalPrintLevel != EAPL_NONE ) {
                    if( Level == 1 ) {
                        vout << "  INFO:    additional module " << lname << " is required, loading ... " << endl;
                        count++;
                    }
                    if( found == true ) {
                        vout << "           " << lname << " is skipped due to cyclic dependency" << endl;
                    }
                }

                if( found == false) {
                    result &= AddModule(vout,lname,true) == EAE_STATUS_OK;
                }

            } else if( ltype == "rm" ) {
                if( ModuleController.IsModuleActive(lname) == true ) {
                    if( GlobalPrintLevel != EAPL_NONE ) {
                        vout << "  WARNING: active module in conflict, unloading ... " << endl;
                    }
                    result &= RemoveModule(vout,lname) == EAE_STATUS_OK;
                }
            }

        }
        p_sele = p_sele->GetNextSiblingElement("dep");
    }

    if( (Level == 1) && ( count > 0) ) {
        vout << "" << endl;
    }

    return(result);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CModule::SolveModulePostDeps(CVerboseStr& vout,CXMLElement* p_dep_container)
{
    if( p_dep_container == NULL ) return(true);

    CXMLElement* p_sele = p_dep_container->GetChildElementByPath("deps/dep");

    bool result = true;
    int count = 0;

    while( p_sele != NULL ) {
        if( p_sele->GetName() == "depend" ) {
            CSmallString lname, ltype;
            p_sele->GetAttribute("name",lname);
            p_sele->GetAttribute("type",ltype);

            if( ltype == "post" ){
                CSmallString lmodname;
                CSmallString lmodver;

                CModUtils::ParseModuleName(lname,lmodname,lmodver);
                // is module already added?
                bool found = false;
                for(CSmallString dep_name : DepList){
                    if( dep_name == lmodname ){
                        found = true;
                        break;
                    }
                }
                if( GlobalPrintLevel != EAPL_NONE ) {
                    if( Level == 1 ) {
                        vout << "  INFO:    additional module " << lname << " is required, loading ... " << endl;
                        count++;
                    }
                    if( found == true ) {
                        vout << "           " << lname << " is skipped due to cyclic dependency" << endl;
                    }
                }

                if( found == false) result &= AddModule(vout,lname,true) != EAE_STATUS_OK;
            }
        }
        p_sele = p_sele->GetNextSiblingElement("dep");
    }

    if( (Level == 1) && ( count > 0) ) {
        //vout << "" << endl;
    }

    return(result);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CModule::PrepareModuleEnvironment(CXMLElement* p_build,
                                        const CSmallString& complete_module,
                                        const CSmallString& exported_module,
                                        EModuleAction action)
{
    // prepare module environment ------------------
    if( ShellProcessor.PrepareModuleEnvironmentForModActionI(p_build) == false ) {
        ES_TRACE_ERROR("unable to prepare EnvironmentForModActionI");
        return(false);
    }

    // install hooks from module config ------------

    if( action == EMA_ADD_MODULE ) {
        CXMLElement* p_builds = dynamic_cast<CXMLElement*>(p_build->GetParentNode());
        CXMLElement* p_module = dynamic_cast<CXMLElement*>(p_builds->GetParentNode());
        // do postaction if necessary
        CSmallString bundle_name = GetBundleName(p_module);
        CSmallString args;
        args << "\"" << complete_module;
        args << "\" \"" << bundle_name << "\" \"" << ModuleFlags << "\"";

        switch(GlobalPrintLevel) {
        case EAPL_NONE:
            args << " none";
            break;
        case EAPL_SHORT:
            args << " short";
            break;
        case EAPL_FULL:
            args << " full";
            break;
        case EAPL_VERBOSE:
            args << " verbose";
            break;
        }

        if( HostGroup.ExecuteModAction("add",args,ModuleFlags) == false ) {
            return(false);
        }
    } else {
        if( HostGroup.ExecuteModAction("remove","\""+complete_module+"\"",ModuleFlags) == false ) {
            return(false);
        }
    }

    if( ShellProcessor.PrepareModuleEnvironmentForLowPriority(p_build,action) == false ) {
        return(false);
    }

    if( ShellProcessor.PrepareModuleEnvironmentForModActionII(p_build) == false ) {
        return(false);
    }

    // now update AMS_ACTIVE_MODULES and AMS_EXPORTED_MODULES variables

    switch(action){
        case(EMA_ADD_MODULE):
        ShellProcessor.AppendValueToVariable("AMS_ACTIVE_MODULES",complete_module,"|");
        ModuleController.UpdateActiveModules(complete_module,EMA_ADD_MODULE);

        if( (ModuleExportFlag == true) && (exported_module != NULL) ) {
            ShellProcessor.AppendValueToVariable("AMS_EXPORTED_MODULES",exported_module,"|");
            ModuleController.UpdateExportedModules(exported_module,EMA_ADD_MODULE);
        }
        break;
    case(EMA_REMOVE_MODULE):
        ShellProcessor.RemoveValueFromVariable("AMS_ACTIVE_MODULES",complete_module,"|");
        ModuleController.UpdateActiveModules(complete_module,EMA_REMOVE_MODULE);

        if( exported_module != NULL ) {
            ShellProcessor.RemoveValueFromVariable("AMS_EXPORTED_MODULES",exported_module,"|");
            ModuleController.UpdateExportedModules(exported_module,EMA_REMOVE_MODULE);
        }
        break;
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CModule::CompleteModule(CVerboseStr& vout,CXMLElement* p_module,
                                CSmallString& name,
                                CSmallString& ver,
                                CSmallString& arch,
                                CSmallString& mode)
{
    // get default setup ---------------------------
    CSmallString defver;
    CSmallString defarch;
    CSmallString defpar;

    if( CModCache::GetModuleDefaults(p_module,defver,defarch,defpar) == false ) {
        //    CSmallString error;
        //    error << "default setup for module '" << name << "' is not provided";
        //    ES_ERROR(error);
        //    return(false);
        defver = "required";
        defarch = "auto";
        defpar = "auto";
    }

    // module name is already completed

    if( GlobalPrintLevel == EAPL_VERBOSE ) {
        vout << " INFO:" << endl;
        vout << " INFO:### NAME ####################" << endl;
        vout << " INFO: Input name    : " << name << endl;
        vout << " INFO: ----------" << endl;
        CSmallString cache_type;
        p_module->GetAttribute("cache",cache_type);
        if( cache_type == "U" ) {
            vout << " INFO: Source        : user cache" << endl;
        }
        if( cache_type == "S" ) {
            vout << " INFO: Source        : system cache" << endl;
        }
        if( CModCache::CanModuleBeExported(p_module) == true ) {
            vout << " INFO: Exportable    : yes" << endl;
        } else {
            vout << " INFO: Exportable    : no" << endl;
        }
        vout << " INFO:" << endl;
    }

    // test permission - module level
    if( CModCache::IsPermissionGrantedForModule(p_module) == false ){
        CSmallString error;
        error << "module '" << name << "' is not allowed for the current user";
        ES_TRACE_ERROR(error);
        return(false);
    }

    // now we complete module version, only two possibilities exist
    // first one is user provided version
    // second one is default version

    if( GlobalPrintLevel == EAPL_VERBOSE ) {
        vout << " INFO:### VERSION #################" << endl;
        if( ver != NULL ) {
            vout << " INFO: Input version   : " << ver<< endl;
        } else {
            vout << " INFO: Input version   : -none-" << endl;
        }
        vout << " INFO: Default version : " << defver << endl;
        vout << " INFO: ----------" << endl;
    }

    if( (ver == NULL) || (ver == "default") ) {
        // version is now default one
        if( ver == NULL ) {
            if( GlobalPrintLevel == EAPL_VERBOSE ) {
                vout << " INFO: Version is not provided." << endl;
                vout << " INFO:   -> Default version '" << defver << "' is used." << endl;
                vout << " INFO:" << endl;
            }
        }
        if( ver == "default") {
            if( GlobalPrintLevel == EAPL_VERBOSE ) {
                vout << " INFO: User requires default version." << endl;
                vout << " INFO:   -> Default version '" << defver << "' is used." << endl;
                vout << " INFO:" << endl;
            }
        }
        // user do not specify version or it wants default one
        if( CModCache::CheckModuleVersion(p_module,defver) == false ) {
            CSmallString error;
            error << "no build was found for default version '" << defver
                  << "' of module '" << name << "'";
            ES_TRACE_ERROR(error);
            return(false);
        }

        ver = defver;
    } else {
        // use user specified version
        if( CModCache::CheckModuleVersion(p_module,ver) == false ) {
            CSmallString error;
            error << "no build was found for specified version '" << ver
                  << "' of module '" << name << "'";
            ES_TRACE_ERROR(error);
            return(false);
        }
        if( GlobalPrintLevel == EAPL_VERBOSE ) {
            vout << " INFO: User specified version '" << ver << "' is used." << endl;
            vout << " INFO:" << endl;
        }
    }

    // now we need to complete module architecture -

    if( GlobalPrintLevel == EAPL_VERBOSE ) {
        vout << " INFO:### ARCHITECTURE ############" << endl;
        if( arch != NULL ) {
            vout << " INFO: Input architecture   : " << arch << endl;
        } else {
            vout << " INFO: Input architecture   : -none-" << endl;
        }
        vout << " INFO: Default architecture : " << defarch << endl;
        vout << " INFO: System arch tokens   : " << Host.GetArchTokens() << endl;
        vout << " INFO: ----------" << endl;
    }

    if( (arch == NULL) || (arch == "default") ) {
        if( arch == NULL ) {
            if( GlobalPrintLevel == EAPL_VERBOSE ) {
                vout << " INFO: Architecture is not provided." << endl;
                vout << " INFO:  -> Default architecture '"<< defarch << "' is used." << endl;
                vout << " INFO:" << endl;
            }
        }
        if( arch == "default") {
            if( GlobalPrintLevel == EAPL_VERBOSE ) {
                vout << " INFO: User requires default architecture." << endl;
                vout << " INFO:  -> Default architecture '"<< defarch << "' is used. !" << endl;
                vout << " INFO:" << endl;
            }
        }
        arch = defarch;
    }

    if( arch == "auto" ) {
        if( GlobalPrintLevel == EAPL_VERBOSE ) {
            vout << " INFO: Automatic determination of architecture is required." << endl;
            vout << " INFO:" << endl;
        }
    }

    if( DetermineArch(vout,p_module,ver,arch) == false ) {
        CSmallString error;
        error << "no build was found for module '" << name
              << "' with version '" << ver << "' and architecture '" << arch << "'";
        ES_TRACE_ERROR(error);
        return(false);
    }

    // and finaly, find best para mode -----------

    if( GlobalPrintLevel == EAPL_VERBOSE ) {
        vout << " INFO:### PARALLEL MODE #########" << endl;
        if( mode != NULL ) {
            vout << " INFO: Input parallel mode    : " << mode << endl;
        } else {
            vout << " INFO: Input parallel mode    : -none-" << endl;
        }
        vout << " INFO: Default parallel mode  : " << defpar << endl;
        vout << " INFO: Number of CPUs         : " << Host.GetNCPUs() << endl;
        vout << " INFO: Max CPUs per node      : " << Host.GetNumOfHostCPUs() << endl;
        vout << " INFO: Number of GPUs         : " << Host.GetNGPUs() << endl;
        vout << " INFO: Max GPUs per node      : " << Host.GetNumOfHostGPUs() << endl;
        vout << " INFO: ----------" << endl;
    }

    if( (mode == NULL) || (mode == "default") ) {
        if( mode == NULL ) {
            if( GlobalPrintLevel == EAPL_VERBOSE ) {
                vout << " INFO: Parallel mode is not provided." << endl;
                vout << " INFO:   -> Default parallel mode '" << defpar << "' is used." << endl;
                vout << " INFO:" << endl;
            }
        }
        if( mode == "default") {
            if( GlobalPrintLevel == EAPL_VERBOSE ) {
                vout << " INFO: User requires default parallel mode." << endl;
                vout << " INFO:   -> Default parallel mode '" << defpar << "' is used. !" << endl;
                vout << " INFO:" << endl;
            }
        }
        mode = defpar;
    }

    if( mode == "auto" ) {
        if( GlobalPrintLevel == EAPL_VERBOSE ) {
            vout << " INFO: Automatic determination of parallel mode is required." << endl;
            vout << " INFO:" << endl;
        }
    }

    if( DetermineMode(vout,p_module,ver,arch,mode) == false ) {
        CSmallString error;
        error << "no build was found for module '" << name
              << "' with version '" << ver << "', architecture '" << arch
              << "' and mode '" << mode << "'";
        ES_TRACE_ERROR(error);
        return(false);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CModule::DetermineArch(CVerboseStr& vout,
                                        CXMLElement* p_module,
                                        const CSmallString& ver,
                                        CSmallString& arch)
{
    CSmallString sys_arch = Host.GetArchTokens();

    if( (GlobalPrintLevel == EAPL_VERBOSE) && (arch == "auto" ) ) {
        vout << " INFO: Host architecture tokens : " << sys_arch << endl;
        vout << " INFO: Requested architecture   : " << arch <<  endl;
    }

    if( arch == "auto" ){
        return(DetermineArchAuto(vout,p_module,ver,arch));
    } else {
        return(DetermineArchUser(vout,p_module,ver,arch));
    }
}

//------------------------------------------------------------------------------

bool CModule::DetermineArchAuto(CVerboseStr& vout,
                                        CXMLElement* p_module,
                                        const CSmallString& ver,
                                        CSmallString& arch)
{
    CSmallString sys_arch = Host.GetArchTokens();

    if( GlobalPrintLevel == EAPL_VERBOSE ) {
        vout << " INFO:" << endl;
        vout << " INFO: Testing architectures ..." << endl;
    }

    list<string> sys_tokens;
    string       ssys_arch(sys_arch);
    split(sys_tokens,ssys_arch,is_any_of(","));

    int best_match = 0;
    int best_score = -1;
    CSmallString found_arch;

    // find the best architecture build
    CXMLElement* p_build = p_module->GetChildElementByPath("builds/build");
    while( p_build ){
        CSmallString bver,bmode;
        string barch;
        p_build->GetAttribute("ver",bver);
        p_build->GetAttribute("arch",barch);
        p_build->GetAttribute("mode",bmode);

        if( ver == bver ){
            // test permission - build level
            if( CModCache::IsPermissionGrantedForBuild(p_build) == false ){
                if( GlobalPrintLevel == EAPL_VERBOSE ) {
                    CSmallString bam;
                    bam = CSmallString(barch) + ":" + bmode;
                    vout << " INFO:   -> Build architecture " << setw(15) << left << barch << " from " << setw(25) << bam << " is not allowed for the current user." << endl;
                }
                p_build = p_build->GetNextSiblingElement("build");
                continue;
            }

            list<string> build_tokens;
            split(build_tokens,barch,is_any_of("#"));
            build_tokens.sort();
            build_tokens.unique();

            int matches = 0;
            int failures = 0;
            int score = 0;

            list<string>::iterator bit = build_tokens.begin();
            list<string>::iterator bet = build_tokens.end();

            while( bit != bet ){

                bool found = false;
                list<string>::iterator sit = sys_tokens.begin();
                list<string>::iterator set = sys_tokens.end();

                while( sit != set ){
                    if( (*bit) == (*sit) ){
                        score += HostGroup.GetArchTokenScore(*sit);
                        found = true;
                        break;
                    }
                    sit++;
                }
                if( found ){
                    matches++;
                } else {
                    failures++;
                }

                bit++;
            }
            if( GlobalPrintLevel == EAPL_VERBOSE ) {
                CSmallString bam;
                bam = CSmallString(barch) + ":" + bmode;
                vout << " INFO:   -> Tested architecture " << setw(15) << left << barch << " from " << setw(25) << bam << " has " << setw(2) << matches << " matches, " << setw(2) << failures << " failures, and score " << score << endl;
            }

            if( failures == 0 ){
                if( (best_match <= matches) && (best_score < score) ){
                    best_match = matches;
                    best_score = score;
                    found_arch = barch;
                }
            }
        }

        p_build = p_build->GetNextSiblingElement("build");
    }
    if( GlobalPrintLevel == EAPL_VERBOSE ) {
        vout << " INFO:   -> No more builds to test." << endl;
    }
    if( found_arch != NULL ){
        if( GlobalPrintLevel == EAPL_VERBOSE ) {
            vout << " INFO:   -> The best build was found for '" << found_arch << "'." << endl;
            vout << " INFO:" << endl;
        }
        arch = found_arch;
        return(true);
    } else {
        if( GlobalPrintLevel == EAPL_VERBOSE ) {
            vout << " INFO:   -> No suitable build was found for system architecture." << endl;
        }
    }

    return(false);
}

//------------------------------------------------------------------------------

bool CModule::DetermineArchUser(CVerboseStr& vout,
                                        CXMLElement* p_module,
                                        const CSmallString& ver,
                                        CSmallString& arch)
{
    CSmallString user_arch = arch;

    CSmallString found_arch;

    if( GlobalPrintLevel == EAPL_VERBOSE ) {
        vout << " INFO:" << endl;
        vout << " INFO: Testing architectures ..." << endl;
    }

    // find the exact architecture build
    CXMLElement* p_build = p_module->GetChildElementByPath("builds/build");
    while( p_build ){
        CSmallString bver;
        CSmallString barch;
        CSmallString bmode;
        p_build->GetAttribute("ver",bver);
        p_build->GetAttribute("arch",barch);
        p_build->GetAttribute("mode",bmode);
        if( ver == bver ){

            // test permission - build level
            if( CModCache::IsPermissionGrantedForBuild(p_build) == false ){
                if( GlobalPrintLevel == EAPL_VERBOSE ) {
                    CSmallString bam;
                    bam = CSmallString(barch) + ":" + bmode;
                    vout << " INFO:   -> Build architecture '" << barch << "' from '" << bam << "' is not allowed for the current user." << endl;
                }
                p_build = p_build->GetNextSiblingElement("build");
                continue;
            }

            int matches,maxmatches;
            bool found = AreSameTokens(user_arch,barch,matches,maxmatches);

            if( GlobalPrintLevel == EAPL_VERBOSE ) {
                if( found == false ){
                    CSmallString bam;
                    bam = CSmallString(barch) + ":" + bmode;
                    vout << " INFO:   -> Build architecture '" << barch << "' from '" << bam << "' does not exactly match user request '" << user_arch << "' (" << matches << " matches out of " << maxmatches << ")." << endl;
                }
            }

            if( found == true ){
                found_arch = barch;
                break;
            }
        }

        p_build = p_build->GetNextSiblingElement("build");
    }
    if( found_arch != NULL ){
        arch = found_arch;
        if( GlobalPrintLevel == EAPL_VERBOSE ) {
            vout << " INFO:   -> The best build was found for '" << arch << "'." << endl;
            vout << " INFO:" << endl;
        }
        return(true);
    } else {
        if( GlobalPrintLevel == EAPL_VERBOSE ) {
            vout << " INFO:   -> No suitable build was found for system architecture." << endl;
        }
    }

    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CModule::DetermineMode(CVerboseStr& vout,CXMLElement* p_module,
                                const CSmallString& ver,
                                const CSmallString& arch,
                                CSmallString& mode)
{
    if( mode == "auto" ){
        return(DetermineModeAuto(vout,p_module,ver,arch,mode));
    } else {
        return(DetermineModeUser(vout,p_module,ver,arch,mode));
    }
}

//------------------------------------------------------------------------------

bool CModule::DetermineModeAuto(CVerboseStr& vout,CXMLElement* p_module,
                                const CSmallString& ver,
                                const CSmallString& arch,
                                CSmallString& mode)
{
    // first determine allowed tokens and their score
    std::list<std::string>      Modes;
    std::map<std::string,int>   ModeScore;

    CXMLElement* p_ele = HostGroup.GetParallelModes();
    CXMLElement* p_mele = p_ele->GetFirstChildElement();
    while( p_mele != NULL ){
        int ncgpu = 0; // current number of cpus/gpus
        int mcgpu = 0; // max number of cpus/gpus per node
        if( p_mele->GetName() == "cmode" ){
            ncgpu = Host.GetNCPUs();
            mcgpu = Host.GetNumOfHostCPUs();
        } else if( p_mele->GetName() == "gmode" ) {
            ncgpu = Host.GetNGPUs();
            mcgpu = Host.GetNumOfHostGPUs();
        } else {
            ES_TRACE_ERROR("unknown element in host modes");
            return(false);
        }
        // add token name
        std::string name;
        p_mele->GetAttribute("name",name);

        // and determine its score
        int score = -1;
        CXMLElement* p_sele = p_mele->GetFirstChildElement();
        while( p_sele != NULL ){
            int lscore = 0;
            p_sele->GetAttribute("score",lscore);
            if( p_sele->GetName() == "one" ){
                if( ncgpu == 1 ){
                    score = lscore;
                    break;
                }
            }
            if( p_sele->GetName() == "gto" ){
                if( ncgpu > 1 ){
                    score = lscore;
                    break;
                }
            }
            if( p_sele->GetName() == "lem" ){
                if( ncgpu <= mcgpu ){
                    score = lscore;
                    break;
                }
            }
            if( p_sele->GetName() == "gtm" ){
                if( ncgpu > mcgpu ){
                    score = lscore;
                    break;
                }
            }
            p_sele = p_sele->GetNextSiblingElement();
        }
        if( score >= 0 ){
            Modes.push_back(name);
            ModeScore[name] = score;
        }
        p_mele = p_mele->GetNextSiblingElement();
    }

    if( GlobalPrintLevel == EAPL_VERBOSE ) {
        vout << " INFO: Host mode tokens with determined scores:" << endl;

        for(std::string mode : Modes){
            vout << " INFO:   -> " << left << setw(10) << mode << right << setw(5) << ModeScore[mode] << endl;
        }
    }

    if( GlobalPrintLevel == EAPL_VERBOSE ) {
        vout << " INFO:" << endl;
        vout << " INFO: Testing modes ..." << endl;
    }

    int best_match = 0;
    int best_score = -1;
    CSmallString found_mode;

    // find the best architecture build
    CXMLElement* p_build = p_module->GetChildElementByPath("builds/build");
    while( p_build ){
        CSmallString bver;
        p_build->GetAttribute("ver",bver);
        if( bver != ver ){
            p_build = p_build->GetNextSiblingElement("build");
            continue;
        }

        CSmallString barch;
        p_build->GetAttribute("arch",barch);
        if( AreSameTokens(arch,barch) == false ){
            p_build = p_build->GetNextSiblingElement("build");
            continue;
        }

        string bmode;
        p_build->GetAttribute("mode",bmode);

        // test permission - build level
        if( CModCache::IsPermissionGrantedForBuild(p_build) == false ){
            if( GlobalPrintLevel == EAPL_VERBOSE ) {
                CSmallString bam;
                bam = barch + ":" + bmode;
                vout << " INFO:   -> Tested build " << setw(20) << bam << " is not allowed for the current user." << endl;
            }
            p_build = p_build->GetNextSiblingElement("build");
            continue;
        }

        list<string> mode_tokens;
        split(mode_tokens,bmode,is_any_of("#"));
        mode_tokens.sort();
        mode_tokens.unique();

        int matches = 0;
        int failures = 0;
        int score = 0;

        list<string>::iterator bit = mode_tokens.begin();
        list<string>::iterator bet = mode_tokens.end();

        while( bit != bet ){

            bool found = false;
            list<string>::iterator sit = Modes.begin();
            list<string>::iterator set = Modes.end();

            while( sit != set ){
                if( (*bit) == (*sit) ){
                    score += ModeScore[*sit];
                    found = true;
                    break;
                }
                sit++;
            }
            if( found ){
                matches++;
            } else {
                failures++;
            }

            bit++;
        }

        if( GlobalPrintLevel == EAPL_VERBOSE ) {
            vout << " INFO:   -> Tested mode " << setw(20) << left << bmode << right << " has " << setw(2) << matches << " matches, " << setw(2) << failures << " failures, and score " << score << endl;
        }

        if( failures == 0 ){
            if( (best_match <= matches) && (best_score < score) ){
                best_match = matches;
                best_score = score;
                found_mode = bmode;
            }
        }

        p_build = p_build->GetNextSiblingElement("build");
    }

    if( GlobalPrintLevel == EAPL_VERBOSE ) {
        vout << " INFO:   -> No more builds to test." << endl;
    }
    if( found_mode != NULL ){
        if( GlobalPrintLevel == EAPL_VERBOSE ) {
            vout << " INFO:   -> The best build was found for mode '" << found_mode << "'." << endl;
            vout << " INFO:" << endl;
        }
        mode = found_mode;
        return(true);
    } else {
        if( GlobalPrintLevel == EAPL_VERBOSE ) {
            vout << " INFO:   -> No suitable build was found for system mode." << endl;
        }
    }

    return(false);
}

//------------------------------------------------------------------------------

bool CModule::DetermineModeUser(CVerboseStr& vout,CXMLElement* p_module,
                                const CSmallString& ver,
                                const CSmallString& arch,
                                CSmallString& mode)
{
    CSmallString user_mode = mode;
    CSmallString found_mode;

    if( GlobalPrintLevel == EAPL_VERBOSE ) {
        vout << " INFO:" << endl;
        vout << " INFO: Testing modes ..." << endl;
    }

    // find the exact architecture build
    CXMLElement* p_build = p_module->GetChildElementByPath("builds/build");
    while( p_build ){
        CSmallString bver;
        p_build->GetAttribute("ver",bver);
        if( bver != ver ){
            p_build = p_build->GetNextSiblingElement("build");
            continue;
        }

        CSmallString barch;
        p_build->GetAttribute("arch",barch);
        if( AreSameTokens(arch,barch) == false ){
            p_build = p_build->GetNextSiblingElement("build");
            continue;
        }

        CSmallString bmode;
        p_build->GetAttribute("mode",bmode);

        // test permission - build level
        if( CModCache::IsPermissionGrantedForBuild(p_build) == false ){
            if( GlobalPrintLevel == EAPL_VERBOSE ) {
                vout << " INFO:   -> Tested build '" << setw(20) << bmode << "' is not allowed for the current user." << endl;
            }
            p_build = p_build->GetNextSiblingElement("build");
            continue;
        }

        int matches,maxmatches;
        if( AreSameTokens(mode,bmode,matches,maxmatches) ){
            found_mode = bmode;
            break;
        }

        if( GlobalPrintLevel == EAPL_VERBOSE ) {
            vout << " INFO:   -> Tested build '" << bmode << "' does not exactly match user request '" << user_mode << "' (" << matches << " matches out of " << maxmatches << ")." << endl;
        }

        p_build = p_build->GetNextSiblingElement("build");
    }
    if( found_mode != NULL ){
        mode = found_mode;
        if( GlobalPrintLevel == EAPL_VERBOSE ) {
            vout << " INFO:   -> The best build was found for user mode '" << mode << "'." << endl;
            vout << " INFO:" << endl;
        }
        return(true);
    } else {
        if( GlobalPrintLevel == EAPL_VERBOSE ) {
            vout << " INFO:   -> No suitable build was found for user mode request." << endl;
        }
    }

    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CModule::AreSameTokens(const CSmallString& user_arch,const CSmallString& build_arch)
{
    int matches,maxmatches;
    return( AreSameTokens(user_arch,build_arch,matches,maxmatches) );
}

//------------------------------------------------------------------------------

bool CModule::AreSameTokens(const CSmallString& user_arch,const CSmallString& build_arch,int& matches,int& maxmatches)
{
    matches = 0;
    maxmatches = 0;

    string          suser_arch(user_arch);
    list<string>    user_archs;

    split(user_archs,suser_arch,is_any_of("#"));
    user_archs.sort();
    user_archs.unique();

    string          sbuild_arch(build_arch);
    list<string>    build_archs;

    split(build_archs,sbuild_arch,is_any_of("#"));
    build_archs.sort();
    build_archs.unique();

    if( build_archs.size() >= user_archs.size() ){
        maxmatches = build_archs.size();
    } else{
        maxmatches = user_archs.size();
    }

    list<string>::iterator bit = build_archs.begin();
    list<string>::iterator bet = build_archs.end();

    while( bit != bet ){
        list<string>::iterator uit = user_archs.begin();
        list<string>::iterator uet = user_archs.end();

        while( uit != uet ){
            if( (*bit) == (*uit) ){
                matches++;
                break;
            }
            uit++;
        }

        bit++;
    }

    return( matches == maxmatches );
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CModule::PrintModuleInfo(CVerboseStr& vout,const CSmallString& mod_name)
{
    // determine print level -----------------------
    SetPrintLevel(EAPL_VERBOSE);

    // complete module specification ---------------
    vout << "# Module specification: " << mod_name << " (disp action)" << endl;
    vout << "# ==============================================================================" << endl;

    // parse module input --------------------------
    CSmallString name,ver,arch,mode;

    if( (CModUtils::ParseModuleName(mod_name,name,ver,arch,mode) == false) || (name == NULL) ) {
        ES_ERROR("module name is empty string");
        return(false);
    }

    // get module specification --------------------
    CXMLElement* p_module = ModCache.GetModule(name);

    if( p_module == NULL ) {
        CSmallString error;
        error << "module '" << name << "' does not have any record in the AMS database";
        ES_ERROR(error);
        return(false);
    }

    if( CompleteModule(vout,p_module,name,ver,arch,mode) == false ) {
        return(false);
    }

    CXMLElement* p_build = ModCache.GetBuild(p_module,ver,arch,mode);

    if( p_build == NULL ) {
        CSmallString error;
        error << "build '" <<
              name << ":" << ver << ":" << arch << ":" << mode <<
              "' does not have any record in the AMS database";
        ES_ERROR(error);
        return(false);
    }

    // unload module if it is already loaded -------

    vout << endl;

    if( ModuleController.IsModuleActive(name) == true ) {
        vout << " INFO:     Module is active, it will be reactivated if 'add' action is used .. " << endl;
        vout << endl;
    }

    // now show module deps ---------------
    CXMLElement* p_sele = p_module->GetChildElementByPath("deps/dep");

    while( p_sele != NULL ) {
        CSmallString lmodule,ltype;
        p_sele->GetAttribute("name",lmodule);
        p_sele->GetAttribute("type",ltype);
        if( ltype == "pre" ){
            vout << " INFO:    additional module is required, it will be pre-loaded if 'add' action is used ... " << endl;
        }
        if( ltype == "post" ){
            vout << " INFO:    additional module is required, it will be post-loaded if 'add' action is used ... " << endl;
        }
        if( ltype == "rm" ){
            vout << " WARNING: active module in conflict, it will be unloaded if 'add' action is used ... " << endl;
        }
        p_sele = p_sele->GetNextSiblingElement("dep");
    }

    Host.PrintHostInfoForModule(vout);
    if( (GetMaintainerName(p_module) != NULL) && (GetMaintainerEMail(p_module) != NULL) && (GetBundleName(p_module) != NULL) ){
        vout << "  Module maintainer  : " << GetMaintainerName(p_module)
             << " (" << GetMaintainerEMail(p_module) << ")"
             << " | Bundle: " << GetBundleName(p_module) << endl;
    }
    vout <<     "# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;

    CSmallString exported_module;
    if( ModCache.CanModuleBeExported(p_module) == true ) {
        exported_module = name + ":" + ver;
        vout << "  Exported module    : " << exported_module << endl;
    } else {
        vout << "  Exported module    : -none- (export is not enabled)" << endl;
    }

    CSmallString complete_module;
    complete_module = name + ":" + ver + ":" + arch + ":" + mode;
    vout <<     "  Module build       : " << complete_module << endl;
    vout << "" << endl;

    return(PrintBuildInfo(vout,complete_module));
}

//------------------------------------------------------------------------------

bool CModule::PrintBuildInfo(CVerboseStr& vout,const CSmallString& mod_name)
{
    CSmallString name,ver,arch,mode;

    // get module specification --------------------
    if( (CModUtils::ParseModuleName(mod_name,name,ver,arch,mode) == false) || (name == NULL) ) {
        ES_ERROR("module name is empty string");
        return(false);
    }

    CXMLElement* p_module = ModCache.GetModule(name);

    if( p_module == NULL ) {
        CSmallString error;
        error << "module '" << mod_name << "' was not found in AMS cache";
        ES_ERROR(error);
        return(false);
    }

    // complete module specification ---------------
    CXMLElement* p_build = ModCache.GetBuild(p_module,ver,arch,mode);

    if( p_build == NULL ) {
        CSmallString error;
        error << "module build '" << mod_name << "' was not found in AMS cache";
        ES_ERROR(error);
        return(false);
    }

    CShellProcessor::PrintBuild(vout,p_build);
    return(true);
}

//------------------------------------------------------------------------------

void CModule::StartHelp(void)
{
    // remove previous contents
    HTMLHelp.RemoveAllChildNodes();

    // create header
    HTMLHelp.CreateChildDeclaration();
    HTMLHelp.CreateChildText("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">",true);

    CXMLElement* p_html = HTMLHelp.CreateChildElement("html");
    p_html->SetAttribute("xmlns","http://www.w3.org/1999/xhtml");
    p_html->SetAttribute("xml:lang","en");
    p_html->SetAttribute("lang","en");
    p_html->SetAttribute("encoding","utf-8");

    CXMLElement* p_head = p_html->CreateChildElement("head");
        CXMLElement* p_title = p_head->CreateChildElement("title");
            CSmallString title;
            title << "Advanced Module System (" << LibBuildVersion_AMS_Web << ")";
            p_title->CreateChildText(title);
        CXMLElement* p_meta = p_head->CreateChildElement("meta");
            p_meta->SetAttribute("http-equiv","Content-Type");
            p_meta->SetAttribute("content","text/html; charset=utf-8");

    p_html->CreateChildElement("body");
}

//------------------------------------------------------------------------------

bool CModule::AddHelp(const CSmallString& mod_name)
{
    CSmallString name;
    CSmallString vers;

    CModUtils::ParseModuleName(mod_name,name,vers);

    CXMLElement* p_module = ModCache.GetModule(name);
    if( p_module == NULL ) return(false);

    CSmallString dver, drch, dmode;
    CModCache::GetModuleDefaults(p_module,dver,drch,dmode);

    std::list<CSmallString> versions;
    CModCache::GetModuleVersionsSorted(p_module,versions);

    CXMLElement* p_mele = HTMLHelp.GetChildElementByPath("html/body");

    // create title
    CXMLElement* p_ele = p_mele->CreateChildElement("h1");
    CSmallString title;
    title << "Module: " << name;
    if( vers != NULL ){
        title << ":" << vers;
    }
    p_ele->CreateChildText(title);

    if( (versions.size() > 0) && (vers == NULL) ){

        // create list of versions

        CSmallString svers = "Available versions: ";
        bool first = true;
        for( CSmallString version : versions ){
            if( first == false ) svers << ", ";
            bool defver = dver == version;
            if( defver ) svers << "<b>";
            svers << name << ":" << version;
            if( defver ) svers << "</b>";
            first =  false;
        }

        p_ele = p_mele->CreateChildElement("p");
        p_ele->CreateChildText(svers);
    }

    CXMLElement* p_doc = CModCache::GetModuleDoc(p_module);
    if(  p_doc != NULL  ){
        // create title
        p_ele = p_mele->CreateChildElement("h2");
        p_ele->CreateChildText("Description");
        // insert contents
        PreprocessHelpHeaders(p_doc);
        p_mele->CopyChildNodesFrom(p_doc);
    }

    return(true);
}

//------------------------------------------------------------------------------

void CModule::PreprocessHelpHeaders(CXMLElement* p_ele)
{
    if( p_ele == NULL ) return;
    CSmallString name = p_ele->GetName();
    if( (name == "h2") || (name == "h3") || (name == "h4") ||
        (name == "h5") || (name == "h6") || (name == "h7") ){
        p_ele->SetName("h2");
    }

    CXMLElement* p_chld = p_ele->GetFirstChildElement();
    while( p_chld ){
        PreprocessHelpHeaders(p_chld);
        p_chld = p_chld->GetNextSiblingElement();
    }
}

//------------------------------------------------------------------------------

bool CModule::ShowHelp(void)
{
    CXMLPrinter pe;

    pe.SetPrintedXMLNode(&HTMLHelp);
    pe.SetPrintAsItIs(true);

    return(pe.Print(stdout));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CModule::GetMaintainerName(CXMLElement* p_module)
{
    CSmallString name;
    CXMLElement* p_ele = p_module->GetChildElementByPath("bundle/maintainer");
    if( p_ele == NULL ) return(name);
    p_ele->GetAttribute("name",name);
    return(name);
}

//------------------------------------------------------------------------------

const CSmallString CModule::GetMaintainerEMail(CXMLElement* p_module)
{
    CSmallString name;
    CXMLElement* p_ele = p_module->GetChildElementByPath("bundle/maintainer");
    if( p_ele == NULL ) return(name);
    p_ele->GetAttribute("email",name);
    return(name);
}

//------------------------------------------------------------------------------

const CSmallString CModule::GetBundleName(CXMLElement* p_module)
{
    CSmallString name;
    CXMLElement* p_ele = p_module->GetChildElementByPath("bundle");
    if( p_ele == NULL ) return(name);
    p_ele->GetAttribute("name",name);
    return(name);
}

//------------------------------------------------------------------------------

const CSmallString CModule::GetErrorStr(EModuleError error)
{
    CSmallString errstr;

    switch(error){
        case(EAE_STATUS_OK):
            break;
        case(EAE_CONFIG_ERROR):
            errstr << "configuration problem";
            break;
        case(EAE_MODULE_NOT_FOUND):
            errstr << "module not found";
            break;
        case(EAE_BUILD_NOT_FOUND):
            errstr << "build not found";
            break;
        case(EAE_DEPENDENCY_ERROR):
            errstr << "dependency error";
            break;
        case(EAE_NOT_ACTIVE):
            errstr << "module is not active";
            break;
        case(EAE_PERMISSION_DENIED):
            errstr << "persmission denied";
            break;
    }

    return(errstr);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

