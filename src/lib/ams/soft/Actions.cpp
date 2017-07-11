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
#include <Actions.hpp>
#include <Cache.hpp>
#include <AMSGlobalConfig.hpp>
#include <ShellProcessor.hpp>
#include <XMLIterator.hpp>
#include <ErrorSystem.hpp>
#include <Utils.hpp>
#include <Site.hpp>
#include <XMLParser.hpp>
#include <FileSystem.hpp>
#include <fnmatch.h>
#include <Shell.hpp>
#include <Host.hpp>
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

CActions Actions;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CActions::CActions(void)
{
    GlobalPrintLevel = EAPL_FULL;
    Level = 0;
    ModuleExportFlag = true;
    ModuleFlags = 0;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CActions::SetFlags(int flags)
{
    ModuleFlags = flags;
}

//------------------------------------------------------------------------------

int  CActions::GetFlags(void)
{
    return(ModuleFlags);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CActions::ReactivateModules(std::ostream& vout)
{
    CSmallString active_modules;
    active_modules = AMSGlobalConfig.GetActiveModules();

    CSmallString exported_modules;
    exported_modules = AMSGlobalConfig.GetExportedModules();

    char* p_str;
    char* p_strtok = NULL;

    bool result = true;

    ModuleFlags |= MFB_REACTIVATED;

    p_str = strtok_r(active_modules.GetBuffer(),"|",&p_strtok);
    while( p_str != NULL ) {
        result &= AddModule(vout,p_str);
        p_str = strtok_r(NULL,"|",&p_strtok);
    }

    // keep only those modules that were exported previously
    AMSGlobalConfig.SetExportedModules(exported_modules);

    if( exported_modules != NULL ) {
        ShellProcessor.SetVariable("AMS_EXPORTED_MODULES",exported_modules);
    } else {
        ShellProcessor.UnsetVariable("AMS_EXPORTED_MODULES");
    }
}

//------------------------------------------------------------------------------

bool CActions::PurgeModules(std::ostream& vout)
{
    CSmallString active_modules;
    active_modules = AMSGlobalConfig.GetActiveModules();

    std::list<CSmallString> modules;

    char* p_str;
    char* p_strtok = NULL;

    p_str = strtok_r(active_modules.GetBuffer(),"|",&p_strtok);
    while( p_str != NULL ) {
        modules.push_back(CSmallString(p_str));
        p_str = strtok_r(NULL,"|",&p_strtok);
    }

    modules.reverse();

    std::list<CSmallString>::iterator it = modules.begin();
    std::list<CSmallString>::iterator ie = modules.end();
    bool result = true;

    while( it != ie ){
        CSmallString module = *it;
        result &= RemoveModule(vout,module);
        it++;
    }

    return(result);
}

//------------------------------------------------------------------------------

EActionError CActions::AddModule(std::ostream& vout,CSmallString module,bool fordep,bool do_not_export)
{
    module.GetSubstitute('/',':');

    // determine print level -----------------------
    EActionPrintLevel print_level =  GlobalPrintLevel;
    if( (Level > 0) && (GlobalPrintLevel != EAPL_NONE) ) print_level = EAPL_SHORT;
    Level++;

    if( (print_level == EAPL_FULL) || (print_level == EAPL_VERBOSE) ) {
        vout << endl;
        vout << "# Module specification: " << module << " (add action)" << endl;
        vout << "# =============================================================" << endl;
    }

    // parse module input --------------------------
    CSmallString name,ver,arch,mode;

    if( (CUtils::ParseModuleName(module,name,ver,arch,mode) == false) || (name == NULL) ) {
        ES_ERROR("module name is empty string");
        Level--;
        return(EAE_MODULE_NOT_FOUND);
    }

    // get module specification --------------------
    CXMLElement* p_module = Cache.GetModule(name);

    if( p_module == NULL ) {
        CSmallString error;
        error << "module '" << name << "' does not have any record in AMS software database";
        ES_TRACE_ERROR(error);
        Level--;
        return(EAE_MODULE_NOT_FOUND);
    }

    // test permission - module level
    if( Cache.IsPermissionGrantedForModule(p_module) == false ){
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

    CXMLElement* p_build = Cache.GetBuild(p_module,ver,arch,mode);

    if( p_build == NULL ) {
        CSmallString error;
        error << "build '" <<
              name << ":" << ver << ":" << arch << ":" << mode <<
              "' does not have any record in the AMS database";
        ES_ERROR(error);
        Level--;
        return(EAE_BUILD_NOT_FOUND);
    }

    // process exclude_node flag -------------------
    CSmallString exclude_node;
    p_build->GetAttribute("exclude_node",exclude_node);

    CSmallString ams_node_type;
    ams_node_type = CShell::GetSystemVariable("AMS_NODE_TYPE");

    if( exclude_node != NULL ) {
        if( fnmatch(exclude_node,ams_node_type,0) == 0 ) {
            if( (print_level == EAPL_FULL) || (print_level == EAPL_VERBOSE) ) {
                vout << "  INFO: Module activation is excluded on this node." << endl;
                vout << endl;
            }
            return(EAE_STATUS_OK);
        }
    }

    // unload module if it is already loaded -------

    bool reactivating = false;

    if( AMSGlobalConfig.IsModuleActive(name) == true ) {
        if( (print_level == EAPL_FULL) || (print_level == EAPL_VERBOSE) ) {
            vout << "  INFO:    Module is active, reactivating .. " << endl;
        }
        RemoveModule(vout,name);
        reactivating = true;
    }

    // now solve module dependencies ---------------

    // prepare module environment for dependency
    if( ShellProcessor.PrepareModuleEnvironmentForDependencies(p_build) == false ) {
        CSmallString error;
        error << "unable to solve dependencies for module '" << name << "' (PrepareModuleEnvironmentForDependencies)";
        ES_ERROR(error);
        Level--;
        return(EAE_DEPENDENCY_ERROR);
    }

    if( SolveModuleDependencies(vout,p_module) == false ) {
        CSmallString error;
        error << "unable to solve dependencies for module '" << name << "' (root)";
        ES_ERROR(error);
        Level--;
        return(EAE_DEPENDENCY_ERROR);
    }

    if( SolveModuleDependencies(vout,p_build) == false ) {
        CSmallString error;
        error << "unable to solve dependencies for module '" << name << "' (build)";
        ES_ERROR(error);
        Level--;
        return(EAE_DEPENDENCY_ERROR);
    }

    CSmallString complete_module;
    complete_module = name + ":" + ver + ":" + arch + ":" + mode;

    CSmallString exported_module;
    if( Cache.CanModuleBeExported(p_module) == true ) {
        exported_module = name + ":" + ver;
    }

    if( do_not_export ){
        exported_module = "";
    }

    // print rest of module info -------------------
    if( (print_level == EAPL_FULL) || (print_level == EAPL_VERBOSE) ) {
        vout <<     "  Requested CPUs     : " << setw(3) << Host.GetNCPUs();
        vout <<     "  Requested GPUs     : " << setw(3) << Host.GetNGPUs()<< endl;
        vout <<     "  Num of host CPUs   : " << setw(3) << Host.GetNumOfHostCPUs();
        vout <<     "  Num of host GPUs   : " << setw(3) << Host.GetNumOfHostGPUs() << endl;
        vout <<     "  Requested nodes    : " << setw(3) << Host.GetNNodes() << endl;
        vout <<     "  Host arch tokens   : " << Host.GetArchTokens() << endl;
        vout <<     "  Host SMP CPU model : " << Host.GetCPUModel() << endl;
        if( Host.GetNumOfHostGPUs() > 0 ){
        if( Host.IsGPUModelSMP() == false ){
        for(size_t i=0; i < Host.GetGPUModels().size(); i++){
        vout <<     "  Host GPU model #" << setw(1) << i+1 << "  : " << Host.GetGPUModels()[i] << endl;
        }
        } else{
        vout <<     "  Host SMP GPU model : " << Host.GetGPUModels()[0] << endl;
        }
        }
        vout <<     "# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
        if( (Cache.CanModuleBeExported(p_module) == true) && (do_not_export == false) ) {
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

    if( PrepareModuleEnvironment(p_build,complete_module,exported_module,true) == false ) {
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

    if( SolveModulePostDependencies(vout,p_build) == false ) {
        CSmallString error;
        error << "unable to solve post dependencies for module '" << name << "' (build)";
        ES_ERROR(error);
        Level--;
        return(EAE_DEPENDENCY_ERROR);
    }

    Level--;
    return(EAE_STATUS_OK);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

EActionError CActions::RemoveModule(std::ostream& vout,CSmallString module)
{
    module.GetSubstitute('/',':');

    // determine print level -----------------------
    EActionPrintLevel print_level =  GlobalPrintLevel;
    if( (Level > 0) && (GlobalPrintLevel != EAPL_NONE) ) print_level = EAPL_SHORT;
    Level++;

    if( (print_level == EAPL_FULL) || (print_level == EAPL_VERBOSE) ) {
        vout << endl;
        vout << "# Module name: " << module << " (remove action)" << endl;
        vout << "# =============================================================" << endl;
    }

    // parse module input --------------------------
    CSmallString name,ver,arch,mode;

    if( (CUtils::ParseModuleName(module,name,ver,arch,mode) == false) || (name == NULL) ) {
        ES_ERROR("module name is empty string");
        Level--;
        return(EAE_MODULE_NOT_FOUND);
    }

    // get module specification --------------------
    CXMLElement* p_module = Cache.GetModule(name);

    if( p_module == NULL ) {
        CSmallString error;
        error << "module '" << name << "' does not have any record in AMS software database";
        ES_WARNING(error);
        Level--;
        return(EAE_MODULE_NOT_FOUND);
    }

    if( AMSGlobalConfig.IsModuleActive(name) == false ) {
        CSmallString error;
        error << "unable to remove module '" << name << "' because it is not active";
        ES_ERROR(error);
        Level--;
        return(EAE_NOT_ACTIVE);
    }

    CSmallString   complete_module;
    complete_module = AMSGlobalConfig.GetActiveModuleSpecification(name);

    CSmallString exported_module;
    exported_module = AMSGlobalConfig.GetExportedModuleSpecification(name);

    if( (print_level == EAPL_FULL) || (print_level == EAPL_VERBOSE) ) {
        vout << "  Module build : " << complete_module << endl;
    }

    if( print_level == EAPL_SHORT ) {
        vout << "           Unload module : " << complete_module << endl;
    }

    if( CUtils::ParseModuleName(complete_module,name,ver,arch,mode) == false ) {
        ES_ERROR("unable to parse complete module specification");
        Level--;
        return(EAE_CONFIG_ERROR);
    }

    CXMLElement* p_build = Cache.GetBuild(p_module,ver,arch,mode);

    if( p_build == NULL ) {
        ES_ERROR("unable to get module build");
        Level--;
        return(EAE_BUILD_NOT_FOUND);
    }

    if( PrepareModuleEnvironment(p_build,complete_module,exported_module,false) == false ) {
        Level--;
        return(EAE_CONFIG_ERROR);
    }

    Level--;
    return(EAE_STATUS_OK);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CActions::SetActionPrintLevel(EActionPrintLevel set)
{
    GlobalPrintLevel = set;
}

//-----------------------------------------------------------------------------

void CActions::SetModuleExportFlag(bool set)
{
    ModuleExportFlag = set;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CActions::SolveModuleDependencies(std::ostream& vout,CXMLElement* p_dep_container)
{
    CXMLElement* p_ele = NULL;

    if( p_dep_container != NULL ) {
        p_ele = p_dep_container->GetFirstChildElement("dependencies");
    }

    if( p_ele == NULL ) {
        return(true);        // no dependencies - skipping
    }

    CXMLIterator    I(p_ele);
    CXMLElement*     p_sele;

    bool result = true;
    int count = 0;

    while( (p_sele = I.GetNextChildElement()) != NULL ) {
        if( p_sele->GetName() == "conflict" ) {
            CSmallString lmodule;
            p_sele->GetAttribute("module",lmodule);
            if( AMSGlobalConfig.IsModuleActive(lmodule) == true ) {
                if( GlobalPrintLevel != EAPL_NONE ) {
                    vout << "  WARNING: active module in conflict, unloading ... " << endl;
                }
                result &= RemoveModule(vout,lmodule) == EAE_STATUS_OK;
            }
        }
        if( p_sele->GetName() == "depend" ) {
            CSmallString lmodule;
            CSmallString lmodname;
            CSmallString lmodver;
            p_sele->GetAttribute("module",lmodule);

            CUtils::ParseModuleName(lmodule,lmodname,lmodver);
            // is module already added?
            bool found = false;
            for(unsigned int i=0; i<DepList.size(); i++) {
                if( DepList[i] == lmodname ) {
                    found = true;
                    break;
                }
            }
            if( GlobalPrintLevel != EAPL_NONE ) {
                if( Level == 1 ) {
                    vout << "  INFO:    additional module " << lmodule << " is required, loading ... " << endl;
                    count++;
                }
                if( found == true ) {
                    vout << "           " << lmodule << " is skipped due to cyclic dependency" << endl;
                }
            }

            if( found == false) {
                result &= AddModule(vout,lmodule,true) == EAE_STATUS_OK;
            }
        }
    }

    if( (Level == 1) && ( count > 0) ) {
        vout << "" << endl;
    }

    return(result);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CActions::SolveModulePostDependencies(std::ostream& vout,CXMLElement* p_dep_container)
{
    CXMLElement* p_ele = NULL;

    if( p_dep_container != NULL ) {
        p_ele = p_dep_container->GetFirstChildElement("dependencies");
    }

    if( p_ele == NULL ) {
        return(true);        // no dependencies - skipping
    }

    CXMLIterator    I(p_ele);
    CXMLElement*     p_sele;

    bool result = true;
    int count = 0;

    while( (p_sele = I.GetNextChildElement()) != NULL ) {
        if( p_sele->GetName() == "postdepend" ) {
            CSmallString lmodule;
            CSmallString lmodname;
            CSmallString lmodver;
            p_sele->GetAttribute("module",lmodule);

            CUtils::ParseModuleName(lmodule,lmodname,lmodver);
            // is module already added?
            bool found = false;
            for(unsigned int i=0; i<DepList.size(); i++) {
                if( DepList[i] == lmodname ) {
                    found = true;
                    break;
                }
            }
            if( GlobalPrintLevel != EAPL_NONE ) {
                if( Level == 1 ) {
                    vout << "  INFO:    additional module " << lmodule << " is required, loading ... " << endl;
                    count++;
                }
                if( found == true ) {
                    vout << "           " << lmodule << " is skipped due to cyclic dependency" << endl;
                }
            }

            if( found == false) result &= AddModule(vout,lmodule,true) != EAE_STATUS_OK;
        }
    }

    if( (Level == 1) && ( count > 0) ) {
        //vout << "" << endl;
    }

    return(result);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CActions::PrepareModuleEnvironment(CXMLElement* p_build,
        const CSmallString& complete_module,
        const CSmallString& exported_module,
        bool add_module)
{
    // prepare module environment ------------------
    if( ShellProcessor.PrepareModuleEnvironmentForModActionI(p_build) == false ) {
        ES_ERROR("unable to prepare EnvironmentForModActionI");
        return(false);
    }

    // install hooks from module config ------------

    if( add_module == true ) {
        // do postaction if necessary
        CSmallString args;
        args << "\""+complete_module+"\" " << ModuleFlags;

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

        if( Site.ExecuteModaction("add",args) == false ) {
            return(false);
        }
    } else {
        if( Site.ExecuteModaction("remove","\""+complete_module+"\"") == false ) {
            return(false);
        }
    }

    if( ShellProcessor.PrepareModuleEnvironmentForLowPriority(p_build,add_module) == false ) {
        return(false);
    }

    if( ShellProcessor.PrepareModuleEnvironmentForModActionII(p_build) == false ) {
        return(false);
    }

    // now update AMS_ACTIVE_MODULES and AMS_EXPORTED_MODULES variables

    if( add_module == true ) {
        ShellProcessor.AppendValueToVariable("AMS_ACTIVE_MODULES",complete_module,"|");
        AMSGlobalConfig.UpdateActiveModules(complete_module,true);

        if( (ModuleExportFlag == true) && (exported_module != NULL) ) {
            ShellProcessor.AppendValueToVariable("AMS_EXPORTED_MODULES",exported_module,"|");
            AMSGlobalConfig.UpdateExportedModules(exported_module,true);
        }
    } else {
        ShellProcessor.RemoveValueFromVariable("AMS_ACTIVE_MODULES",complete_module,"|");
        AMSGlobalConfig.UpdateActiveModules(complete_module,false);

        if( exported_module != NULL ) {
            ShellProcessor.RemoveValueFromVariable("AMS_EXPORTED_MODULES",exported_module,"|");
            AMSGlobalConfig.UpdateExportedModules(exported_module,false);
        }
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CActions::RemoveModule(const CSmallString& module_list,const CSmallString& name)
{
    CSmallString     tmp(module_list);
    CSmallString     newlist;
    char*             p_lvar;

    if( tmp == NULL ) return(newlist);

    p_lvar = strtok(tmp.GetBuffer(),"|");
    while( p_lvar != NULL ) {
        if( (strlen(p_lvar) != 0) && (strstr(p_lvar,name) != p_lvar) ) {
            if( newlist != NULL ) newlist += "|";
            newlist += p_lvar;
        }
        p_lvar = strtok(NULL,"|");
    }

    return(newlist);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CActions::AppendModule(const CSmallString& module_list,const CSmallString& module)
{
    CSmallString     newlist(module_list);

    if( newlist != NULL ) {
        if( module != NULL ) {
            newlist += "|";
            newlist += module;
        }
    } else {
        newlist = module;
    }

    return(newlist);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CActions::CompleteModule(std::ostream& vout,CXMLElement* p_module,
                                CSmallString& name,
                                CSmallString& ver,
                                CSmallString& arch,
                                CSmallString& mode)
{
    // get default setup ---------------------------
    CSmallString defver;
    CSmallString defarch;
    CSmallString defpar;

    if( Cache.GetModuleDefaults(p_module,defver,defarch,defpar) == false ) {
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
        if( Cache.CanModuleBeExported(p_module) == true ) {
            vout << " INFO: Exportable    : yes" << endl;
        } else {
            vout << " INFO: Exportable    : no" << endl;
        }
        vout << " INFO:" << endl;
    }

    // test permission - module level
    if( Cache.IsPermissionGrantedForModule(p_module) == false ){
        CSmallString error;
        error << "module '" << name << "' is not allowed for the current user";
        ES_ERROR(error);
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
        if( Cache.CheckModuleVersion(p_module,defver) == false ) {
            CSmallString error;
            error << "no build was found for default version '" << defver
                  << "' of module '" << name << "'";
            ES_ERROR(error);
            return(false);
        }

        ver = defver;
    } else {
        // use user specified version
        if( Cache.CheckModuleVersion(p_module,ver) == false ) {
            CSmallString error;
            error << "no build was found for specified version '" << ver
                  << "' of module '" << name << "'";
            ES_ERROR(error);
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

    if( DetermineArchitecture(vout,p_module,ver,arch) == false ) {
        CSmallString error;
        error << "no build was found for module '" << name
              << "' with version '" << ver << "' and architecture '" << arch << "'";
        ES_ERROR(error);
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
        ES_ERROR(error);
        return(false);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CActions::DetermineArchitecture(std::ostream& vout,
                                        CXMLElement* p_module,
                                        const CSmallString& ver,
                                        CSmallString& arch)
{
    CSmallString sys_arch = Host.GetArchTokens();
    CSmallString user_arch = arch;

    if( (GlobalPrintLevel == EAPL_VERBOSE) && (arch == "auto" ) ) {
        vout << " INFO: Host architecture tokens : " << sys_arch << endl;
        vout << " INFO: Requested architecture   : " << user_arch <<  endl;
    }

    if( arch == "auto" ){

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
                if( Cache.IsPermissionGrantedForBuild(p_build) == false ){
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
                            score += Host.GetArchTokenScore(*sit);
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

    } else {

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
                if( Cache.IsPermissionGrantedForBuild(p_build) == false ){
                    if( GlobalPrintLevel == EAPL_VERBOSE ) {
                        CSmallString bam;
                        bam = CSmallString(barch) + ":" + bmode;
                        vout << " INFO:   -> Build architecture '" << barch << "' from '" << bam << "' is not allowed for the current user." << endl;
                    }
                    p_build = p_build->GetNextSiblingElement("build");
                    continue;
                }

                int matches,maxmatches;
                bool found = CUtils::AreSameTokens(user_arch,barch,matches,maxmatches);

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
    }

    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CActions::DetermineMode(std::ostream& vout,CXMLElement* p_module,
                                const CSmallString& ver,
                                const CSmallString& arch,
                                CSmallString& mode)
{
    CSmallString user_mode = mode;

    if( mode == "auto" ){

        // first determine allowed tokens and their score
        std::list<std::string>      Modes;
        std::map<std::string,int>   ModeScore;

        CXMLElement* p_ele = Host.GetRootParallelModes();
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
                ES_ERROR("unknown element in host modes");
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
            std::list<std::string>::iterator mit = Modes.begin();
            std::list<std::string>::iterator mie = Modes.end();

            while( mit != mie ){
                vout << " INFO:   -> " << left << setw(10) << *mit << right << setw(5) << ModeScore[*mit] << endl;
                mit++;
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
            if( CUtils::AreSameTokens(arch,barch) == false ){
                p_build = p_build->GetNextSiblingElement("build");
                continue;
            }

            string bmode;
            p_build->GetAttribute("mode",bmode);

            // test permission - build level
            if( Cache.IsPermissionGrantedForBuild(p_build) == false ){
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

    } else {

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
            if( CUtils::AreSameTokens(arch,barch) == false ){
                p_build = p_build->GetNextSiblingElement("build");
                continue;
            }

            CSmallString bmode;
            p_build->GetAttribute("mode",bmode);

            // test permission - build level
            if( Cache.IsPermissionGrantedForBuild(p_build) == false ){
                if( GlobalPrintLevel == EAPL_VERBOSE ) {
                    vout << " INFO:   -> Tested build '" << setw(20) << bmode << "' is not allowed for the current user." << endl;
                }
                p_build = p_build->GetNextSiblingElement("build");
                continue;
            }

            int matches,maxmatches;
            if( CUtils::AreSameTokens(mode,bmode,matches,maxmatches) ){
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
    }

    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

