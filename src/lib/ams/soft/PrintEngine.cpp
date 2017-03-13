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

#include <PrintEngine.hpp>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <ErrorSystem.hpp>
#include <XMLParser.hpp>
#include <XMLPrinter.hpp>
#include <XMLElement.hpp>
#include <XMLIterator.hpp>
#include <Terminal.hpp>
#include <GlobalConfig.hpp>
#include <prefix.h>
#include <Cache.hpp>
#include <AmsUUID.hpp>
#include <DirectoryEnum.hpp>
#include <Site.hpp>
#include <Utils.hpp>
#include <Actions.hpp>
#include <FileSystem.hpp>
#include <TerminalStr.hpp>
#include <Host.hpp>
#include <iomanip>
#include <algorithm>

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CPrintEngine PrintEngine;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CPrintEngine::CPrintEngine(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CPrintEngine::LoadConfig(void)
{
    bool result;

    result = LoadSystemConfig();
    result &= LoadUserConfig();

    return(result);
}

//------------------------------------------------------------------------------

bool CPrintEngine::LoadSystemConfig(void)
{
    SystemConfig.RemoveAllChildNodes();

    // load config ----------------------------------
    CFileName    config_name;

    // first try site config
    if( GlobalConfig.GetActiveSiteID() != NULL ) {
        config_name = CFileName(ETCDIR) / "sites" / GlobalConfig.GetActiveSiteID() / "print.xml";
        if( CFileSystem::IsFile(config_name) == false ) {
            // then global config
            config_name = CFileName(ETCDIR) / "default" / "print.xml";
        }
    } else {
        config_name = CFileName(ETCDIR) / "default" / "print.xml";
    }

    CXMLParser xml_parser;
    xml_parser.SetOutputXMLNode(&SystemConfig);

    if( xml_parser.Parse(config_name) == false ) {
        CSmallString error;
        error << "unable to load site '" << GlobalConfig.GetActiveSiteID()
              << "' print config (" << config_name <<")";
        ES_ERROR(error);
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CPrintEngine::LoadUserConfig(void)
{
    UserConfig.RemoveAllChildNodes();


    return(true);
}

//------------------------------------------------------------------------------

bool CPrintEngine::SaveUserConfig(void)
{

    return(true);
}

//------------------------------------------------------------------------------

bool CPrintEngine::SetUserConfigFromSystem(void)
{
    return(true);
}

//------------------------------------------------------------------------------

void CPrintEngine::ClearUserConfig(void)
{
    UserConfig.RemoveAllChildNodes();


}

//------------------------------------------------------------------------------

void CPrintEngine::SetOutputStream(CVerboseStr& fout)
{
    vout.Attach(fout);
}

//------------------------------------------------------------------------------

CXMLElement* CPrintEngine::GetRootElementOfConfig(void)
{
    CXMLElement* p_mele;
    p_mele = SystemConfig.GetFirstChildElement("print");
    return(p_mele);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CPrintEngine::PrintRawAllBuilds(void)
{
    CXMLElement*     p_mele = Cache.GetRootElementOfCache();
    if( p_mele == NULL ){
        RUNTIME_ERROR("unable to find root cache element");
    }

    CXMLElement*     p_ele;
    CXMLElement*     p_sele;

    CXMLIterator    I(p_mele);

    list<CSmallString>  builds;

    while( (p_ele = I.GetNextChildElement("module")) != NULL ) {
        CSmallString name;
        p_ele->GetAttribute("name",name);

        CXMLElement* p_list = p_ele->GetFirstChildElement("builds");

        CXMLIterator    J(p_list);

        while( (p_sele = J.GetNextChildElement("build")) != NULL ) {
            CSmallString ver;
            CSmallString arch;
            CSmallString mode;
            p_sele->GetAttribute("ver",ver);
            p_sele->GetAttribute("arch",arch);
            p_sele->GetAttribute("mode",mode);

            CSmallString full_spec = name + ":" + ver + ":" + arch + ":" + mode;
            builds.push_back(full_spec);
        }

    }

    builds.sort();

    list<CSmallString>::iterator    it = builds.begin();
    list<CSmallString>::iterator    ie = builds.end();
    while( it != ie ){
        vout << (*it) << endl;
        it++;
    }
}

//------------------------------------------------------------------------------

void CPrintEngine::PrintRawCategories(void)
{
    CXMLElement* p_ele = SystemConfig.GetChildElementByPath("print/categories");
    if( p_ele == NULL ) {
        ES_ERROR("unable to open print/catogories path");
        return;
    }

    CXMLIterator    I(p_ele);
    CXMLElement*     p_sele;

    while( (p_sele = I.GetNextChildElement("category")) != NULL ) {
        // check the number of existing modules
        CXMLIterator    M(p_sele);
        CXMLElement*     p_mele;
        int             count=0;
        while( (p_mele = M.GetNextChildElement("module")) != NULL ) {
            CSmallString    name;
            p_mele->GetAttribute("name",name);
            if( Cache.GetModule(name) != NULL ) count++;
        }
        if( count != 0 ) {
            CSmallString name;
            p_sele->GetAttribute("name",name);
            vout << name << endl;
        }
    }
}

//------------------------------------------------------------------------------

bool CPrintEngine::PrintRawModulesInCategory(const CSmallString& cat_name,
        bool include_vers)
{
    CXMLElement* p_cele = SystemConfig.GetChildElementByPath("print/catogories");
    if( p_cele == NULL ) {
        ES_ERROR("unable to open print/catogories path");
        return(false);
    }

    // find category
    CXMLIterator    I(p_cele);
    CXMLElement*    p_ele = NULL;

    while( (p_ele = I.GetNextChildElement("category")) != NULL ) {
        CSmallString name;
        p_ele->GetAttribute("name",name);
        if( name == cat_name ) break;
    }

    if( p_ele == NULL ) {
        CSmallString error;
        error << "unable to find category '" << cat_name << "'";
        ES_ERROR(error);
        return(false);
    }

    // list content of category ----------------------------------------
    CXMLIterator    J(p_ele);
    CXMLElement*    p_sele;

    while( (p_sele = J.GetNextChildElement("module")) != NULL ) {
        CSmallString name;
        p_sele->GetAttribute("name",name);

        // get module
        CXMLElement* p_module = Cache.GetModule(name);
        if( p_module != NULL ) {
            if( include_vers ) {
                vector<CSmallString>    versions;
                CXMLElement*            p_tele;
                CXMLElement*            p_list = p_module->GetFirstChildElement("builds");
                CXMLIterator            K(p_list);

                while( (p_tele = K.GetNextChildElement("build")) != NULL ) {
                    CSmallString ver;
                    p_tele->GetAttribute("ver",ver);
                    bool found = false;
                    for(unsigned int q=0; q < versions.size(); q++) {
                        if( versions[q] == ver ) {
                            found = true;
                            break;
                        }
                    }
                    if( found == false ) {
                        versions.push_back(ver);
                        CSmallString full_name;
                        full_name = name + ":" + ver;
                        vout << full_name << endl;
                    }
                }
            } else {
                vout << name << endl;
            }
        }
    }

    return(true);
}

//------------------------------------------------------------------------------


class CPVerRecord {
    public:
    CPVerRecord(void);
    CSmallString   version;
    int            verindx;
    bool operator == (const CPVerRecord& left) const;
};

//------------------------------------------------------------------------------

CPVerRecord::CPVerRecord(void)
{
    verindx = 0.0;
}

//------------------------------------------------------------------------------

bool CPVerRecord::operator == (const CPVerRecord& left) const
{
    bool result = true;
    result &= version == left.version;
    return(result);
}

//------------------------------------------------------------------------------

bool sort_tokens(const CPVerRecord& left,const CPVerRecord& right)
{
    if( left.version == right.version ) return(true);
    if( left.verindx < right.verindx ) return(true);
    if( left.verindx == right.verindx ){
        return( strcmp(left.version,right.version) < 0);
    }
    return(false);
}

//------------------------------------------------------------------------------

bool CPrintEngine::PrintRawModuleVersions(const CSmallString& mod_name)
{
    CSmallString name = CUtils::GetModuleName(mod_name);
    CXMLElement* p_ele = Cache.GetModule(name);

    if( p_ele == NULL ) {
        CSmallString error;
        error << "module '" << mod_name << "' was not found in AMS cache";
        ES_ERROR(error);
        return(false);
    }

    std::list<CPVerRecord>  versions;
    CXMLElement*            p_list = p_ele->GetFirstChildElement("builds");
    CXMLIterator            K(p_list);
    CXMLElement*            p_tele;

    while( (p_tele = K.GetNextChildElement("build")) != NULL ) {
        CPVerRecord verrcd;
        verrcd.verindx = 0.0;
        p_tele->GetAttribute("ver",verrcd.version);
        p_tele->GetAttribute("verindx",verrcd.verindx);
        versions.push_back(verrcd);
    }

    versions.sort(sort_tokens);
    versions.unique();

    std::list<CPVerRecord>::iterator it = versions.begin();
    std::list<CPVerRecord>::iterator ie = versions.end();

    while( it != ie ){
        vout << name << ":" << (*it).version << endl;
        it++;
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CPrintEngine::PrintRawModuleBuilds(const CSmallString& mod_name)
{
    CSmallString name = CUtils::GetModuleName(mod_name);
    CSmallString mver = CUtils::GetModuleVer(mod_name);
    CXMLElement* p_ele = Cache.GetModule(name);

    if( p_ele == NULL ) {
        CSmallString error;
        error << "module '" << mod_name << "' was not found in AMS cache";
        ES_ERROR(error);
        return(false);
    }

    CXMLElement*       p_list = p_ele->GetFirstChildElement("builds");
    CXMLIterator       I(p_list);
    CXMLElement*       p_sele;

    while( (p_sele = I.GetNextChildElement("build")) != NULL ) {
        CSmallString ver,arch,mode;
        p_sele->GetAttribute("ver",ver);
        p_sele->GetAttribute("arch",arch);
        p_sele->GetAttribute("mode",mode);
        if( (mver == "") || (mver == ver) ) {
            vout << name << ":" << ver << ":" << arch << ":" << mode << endl;
        }
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CPrintEngine::PrintRawModuleDisp(const CSmallString& mod_name)
{
    CSmallString name,ver,arch,mode;

    // get module specification --------------------
    if( (CUtils::ParseModuleName(mod_name,name,ver,arch,mode) == false) || (name == NULL) ) {
        ES_ERROR("module name is empty string");
        return(false);
    }

    CXMLElement* p_module = Cache.GetModule(name);

    if( p_module == NULL ) {
        CSmallString error;
        error << "module '" << mod_name << "' was not found in AMS cache";
        ES_ERROR(error);
        return(false);
    }

    // complete module specification ---------------
    CXMLElement* p_build = Cache.GetBuild(p_module,ver,arch,mode);

    if( p_build == NULL ) {
        CSmallString error;
        error << "module build '" << mod_name << "' was not found in AMS cache";
        ES_ERROR(error);
        return(false);
    }

    CUtils::PrintBuild(vout,p_build);
    return(true);
}

//------------------------------------------------------------------------------

bool CPrintEngine::PrintRawModuleDefault(const CSmallString& mod_name)
{
    CSmallString name = CUtils::GetModuleName(mod_name);
    CXMLElement* p_ele = Cache.GetModule(name);

    if( p_ele == NULL ) {
        ES_ERROR("module was not found in AMS cache");
        return(false);
    }

    CSmallString ver,arch,para;

    if( Cache.GetModuleDefaults(p_ele,ver,arch,para) == false ) {
        vout << endl; // no default section
        return(true);
    }

    vout << name << ":" << ver << ":" << arch << ":" << para << endl;

    return(true);
}

//------------------------------------------------------------------------------

bool CPrintEngine::PrintRawVariable(const CSmallString& build,const CSmallString& var_name)
{
    // find builds
    CSmallString name,ver,arch,mode;

    CUtils::ParseModuleName(build,name,ver,arch,mode);

    CXMLElement* p_mod = Cache.GetModule(name);
    if( p_mod == NULL ) {
        ES_ERROR("module was not found");
        return(false);
    }

    CXMLElement* p_rel = Cache.GetBuild(p_mod,ver,arch,mode);
    if( p_mod == NULL ) {
        ES_ERROR("build was not found");
        return(false);
    }

    CSmallString var_value;
    var_value = Cache.GetVariableValue(p_rel,var_name);

    vout << var_value;

    return(true);
}

//------------------------------------------------------------------------------

void CPrintEngine::PrintRawSites(void)
{
    // make list of all available sites -------------
    CDirectoryEnum dir_enum(BR_ETCDIR("/sites"));

    dir_enum.StartFindFile("*");
    CFileName site_sid;
    while( dir_enum.FindFile(site_sid) ) {
        CAmsUUID    site_id;
        CSite       site;
        if( site_id.LoadFromString(site_sid) == false ) continue;
        if( site.LoadConfig(site_sid) == false ) continue;

        vout << site.GetName() << endl;
    }
    dir_enum.EndFindFile();
}

//------------------------------------------------------------------------------

void CPrintEngine::PrintRawSystemVersion(void)
{
    vout << LibBuildVersion_AMS;
}

//------------------------------------------------------------------------------

void CPrintEngine::PrintRawDependencies(void)
{
    CXMLElement* p_sele = Cache.GetRootElementOfCache();
    if( p_sele == NULL ){
        RUNTIME_ERROR("unable to find cache element");
    }

    CXMLIterator    I(p_sele);
    CXMLElement*     p_mele;
    while( (p_mele = I.GetNextChildElement("module")) != NULL  ) {
        CSmallString name;
        p_mele->GetAttribute("name",name);

        // traverse dependencies - intermodule
        CXMLElement* p_dele;

        p_dele = p_mele->GetFirstChildElement("dependencies");
        CXMLIterator    D(p_dele);
        CXMLElement*    p_lele;

        if( (p_dele != NULL) && (p_dele->GetNumberOfChildNodes() > 0) ) {
            vout << left << name << endl;
        }

        while( (p_lele = D.GetNextChildElement()) != NULL  ) {
            CSmallString dname;
            p_lele->GetAttribute("module",dname);
            char cross_ref = '\\';
            if( Cache.TestCrossDependency(dname,name) == true ) {
                cross_ref = 'X';
            }
            if( p_lele->GetName() == "conflict" ) {
                vout << "<red>   - " << cross_ref << left << " " << dname << "</red>" << endl;
            }
            if( p_lele->GetName() == "depend" ) {
                vout <<  "<green>   + " << cross_ref << left << " " << dname << "</green>" << endl;
            }
            if( p_lele->GetName() == "postdepend" ) {
                vout <<  "<blue>   p " << cross_ref << left << " " << dname << "</blue>" << endl;
            }
            if( p_lele->GetName() == "syncdepend" ) {
                vout <<  "<yellow>   > " << cross_ref << left << " " << dname << "</yellow>" << endl;
            }
        }

        // traverse dependencies - interbuilds
        CXMLElement* p_bele;
        p_bele = p_mele->GetChildElementByPath("builds/build");

        while( p_bele != NULL ){

            CSmallString ver;
            CSmallString arch;
            CSmallString mode;
            p_bele->GetAttribute("ver",ver);
            p_bele->GetAttribute("arch",arch);
            p_bele->GetAttribute("mode",mode);

            CSmallString full_spec = name + ":" + ver + ":" + arch + ":" + mode;

            CXMLElement* p_dele;

            p_dele = p_bele->GetFirstChildElement("dependencies");
            CXMLIterator    D(p_dele);
            CXMLElement*    p_lele;

            if( (p_dele != NULL) && (p_dele->GetNumberOfChildNodes() > 0) ) {
                vout << left << full_spec << endl;
            }

            while( (p_lele = D.GetNextChildElement()) != NULL  ) {
                CSmallString dname;
                p_lele->GetAttribute("module",dname);
                char cross_ref = '\\';
                if( Cache.TestCrossDependency(dname,name) == true ) {
                    cross_ref = 'X';
                }
                if( p_lele->GetName() == "conflict" ) {
                    vout << "<red>   - " << cross_ref << left << " " << dname << "</red>" << endl;
                }
                if( p_lele->GetName() == "depend" ) {
                    vout <<  "<green>   + " << cross_ref << left << " " << dname << "</green>" << endl;
                }
                if( p_lele->GetName() == "postdepend" ) {
                    vout <<  "<blue>   p " << cross_ref << left << " " << dname << "</blue>" << endl;
                }
                if( p_lele->GetName() == "syncdepend" ) {
                    vout <<  "<yellow>   > " << cross_ref << left << " " << dname << "</yellow>" << endl;
                }
            }

            p_bele = p_bele->GetNextSiblingElement("build");
        }

    }
}

//------------------------------------------------------------------------------

void CPrintEngine::PrintRawArchs(void)
{
    list<CSmallString>   archs;

    CXMLElement* p_mele = Cache.GetRootElementOfCache();
    if( p_mele == NULL ){
        RUNTIME_ERROR("root cache element not found");
    }
    CXMLElement* p_ele = p_mele->GetFirstChildElement("module");
    while( p_ele != NULL ) {
        CXMLElement* p_sele = p_ele->GetChildElementByPath("builds/build");
        while( p_sele != NULL ) {
            CSmallString arch;
            p_sele->GetAttribute("arch",arch);
            archs.push_back(arch);
            p_sele = p_sele->GetNextSiblingElement("build");
        }
        p_ele = p_ele->GetNextSiblingElement("module");
    }

    archs.sort();
    archs.unique();

    list<CSmallString>::iterator    it = archs.begin();
    list<CSmallString>::iterator    ie = archs.end();
    while( it != ie ){
        vout << (*it) << endl;
        it++;
    }
}

//------------------------------------------------------------------------------

void CPrintEngine::PrintRawModes(void)
{
    list<CSmallString>   modes;

    CXMLElement* p_mele = Cache.GetRootElementOfCache();
    if( p_mele == NULL ){
        RUNTIME_ERROR("root cache element not found");
    }
    CXMLElement* p_ele = p_mele->GetFirstChildElement("module");
    while( p_ele != NULL ) {
        CXMLElement* p_sele = p_ele->GetChildElementByPath("builds/build");
        while( p_sele != NULL ) {
            CSmallString mode;
            p_sele->GetAttribute("mode",mode);
            modes.push_back(mode);
            p_sele = p_sele->GetNextSiblingElement("build");
        }
        p_ele = p_ele->GetNextSiblingElement("module");
    }

    modes.sort();
    modes.unique();

    list<CSmallString>::iterator    it = modes.begin();
    list<CSmallString>::iterator    ie = modes.end();
    while( it != ie ){
        vout << (*it) << endl;
        it++;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CPrintEngine::PrintModModuleVersions(const CSmallString& mod_name)
{
    CSmallString name = CUtils::GetModuleName(mod_name);

    vout << "# Module name: " << name << " (available versions)" << endl;
    vout << "#==============================================================" << endl;

    bool result = PrintRawModuleVersions(mod_name);

    return(result);
}

//------------------------------------------------------------------------------

bool CPrintEngine::PrintModModuleBuilds(const CSmallString& mod_name)
{
    CSmallString name = CUtils::GetModuleName(mod_name);

    vout << "# Module name: " << name << " (available builds)" << endl;
    vout << "# =============================================================" << endl;

    bool result = PrintRawModuleBuilds(mod_name);

    return(result);
}

//------------------------------------------------------------------------------

bool CPrintEngine::PrintModModuleInfo(const CSmallString& mod_name)
{
    // determine print level -----------------------
    Actions.SetActionPrintLevel(EAPL_VERBOSE);

    // complete module specification ---------------
    vout << "# Module specification: " << mod_name << " (disp action)" << endl;
    vout << "# =============================================================" << endl;

    // parse module input --------------------------
    CSmallString name,ver,arch,mode;

    if( (CUtils::ParseModuleName(mod_name,name,ver,arch,mode) == false) || (name == NULL) ) {
        ES_ERROR("module name is empty string");
        return(false);
    }

    // get module specification --------------------
    CXMLElement* p_module = Cache.GetModule(name);

    if( p_module == NULL ) {
        CSmallString error;
        error << "module '" << name << "' does not have any record in the AMS database";
        ES_ERROR(error);
        return(false);
    }

    if( Actions.CompleteModule(vout,p_module,name,ver,arch,mode) == false ) {
        return(false);
    }

    CXMLElement* p_build = Cache.GetBuild(p_module,ver,arch,mode);

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

    if( GlobalConfig.IsModuleActive(name) == true ) {
        vout << " INFO:     Module is active, it will be reactivated if 'add' action is used .. " << endl;
        vout << endl;
    }

    // now show module dependencies ---------------
    CXMLElement* p_ele = NULL;

    if( p_module != NULL ) {
        p_ele = p_module->GetFirstChildElement("dependencies");
    }

    if( p_ele != NULL ) {
        CXMLIterator    I(p_ele);
        CXMLElement*     p_sele;
        while( (p_sele = I.GetNextChildElement()) != NULL ) {
            if( p_sele->GetName() == "conflict" ) {
                CSmallString lmodule;
                p_sele->GetAttribute("module",lmodule);
                if( GlobalConfig.IsModuleActive(lmodule) == true ) {
                    vout << " WARNING: active module in conflict, it will be unloaded if 'add' action is used ... " << endl;
                }
            }
            if( p_sele->GetName() == "depend" ) {
                CSmallString lmodule;
                p_sele->GetAttribute("module",lmodule);
                vout << " INFO:    additional module is required, it will be loaded if 'add' action is used ... " << endl;
            }
        }
    }

    CSmallString complete_module;
    complete_module = name + ":" + ver + ":" + arch + ":" + mode;

    CSmallString exported_module;
    if( Cache.CanModuleBeExported(p_module) == true ) {
        exported_module = name + ":" + ver;
    }

    // print rest of module info -------------------
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
    if( Cache.CanModuleBeExported(p_module) == true ) {
        vout << "  Exported module    : " << exported_module << endl;
    } else {
        vout << "  Exported module    : -none- (export is not enabled)" << endl;
    }
    vout <<     "  Module build       : " << complete_module << endl;
    vout << "" << endl;

    return(PrintRawModuleDisp(complete_module));
}

//------------------------------------------------------------------------------

void CPrintEngine::PrintModActiveModules(CTerminal& terminal)
{
    terminal.EnableColors(AreColorsEnabled());

    // print section title --------------------------
    terminal.SetColors(GetSectionFgColor(),GetSectionBgColor());
    terminal.SetBold();
    terminal.PrintTitle("ACTIVE MODULES",GetSectionDelimiter());
    terminal.SetDefault();
    terminal.Printf("\n\n");

    // print active modules -------------------------
    // prepare print format
    int maxmodlen = Cache.GetSizeOfLongestModuleSpecification();

    // print active modules
    CSmallString tmp = GlobalConfig.GetActiveModules();

    char*   p_module = strtok(tmp.GetBuffer(),"|");
    int     count = 0;

    terminal.ClearList();
    while( p_module != NULL ) {
        terminal.AddItem(p_module);
        count++;
        p_module = strtok(NULL,"|");
    }

    // if no module - print [none]
    if( count == 0 ) {
        terminal.AddItem("[none]");
    }

    // final print
    terminal.SetColors(GetModuleFgColor(),GetModuleBgColor());
    terminal.PrintColumnSortedList(maxmodlen);
    terminal.SetDefault();
}

//------------------------------------------------------------------------------

void CPrintEngine::PrintModExportedModules(CTerminal& terminal)
{
    terminal.EnableColors(AreColorsEnabled());

    // print section title --------------------------
    terminal.SetColors(GetSectionFgColor(),GetSectionBgColor());
    terminal.SetBold();
    terminal.PrintTitle("EXPORTED MODULES",GetSectionDelimiter());
    terminal.SetDefault();
    terminal.Printf("\n\n");

    // print active modules -------------------------
    // prepare print format
    int maxmodlen = Cache.GetSizeOfLongestModuleSpecification();

    // print active modules
    CSmallString tmp = GlobalConfig.GetExportedModules();

    char*   p_module = strtok(tmp.GetBuffer(),"|");
    int     count = 0;

    terminal.ClearList();
    while( p_module != NULL ) {
        terminal.AddItem(p_module);
        count++;
        p_module = strtok(NULL,"|");
    }

    // if no module - print [none]
    if( count == 0 ) {
        terminal.AddItem("[none]");
    }

    // final print
    terminal.SetColors(GetModuleFgColor(),GetModuleBgColor());
    terminal.PrintColumnSortedList(maxmodlen);
    terminal.SetDefault();
}

//------------------------------------------------------------------------------

void CPrintEngine::PrintModAvailableModules(CTerminal& terminal,bool include_system,bool include_version)
{
    terminal.EnableColors(AreColorsEnabled());

    // print section title --------------------------
    PrintAMSAvailHeader(terminal);

    terminal.ClearList();

    // start traverse module cache
    CXMLElement*   p_mele = SystemConfig.GetChildElementByPath("print/categories");
    int            maxmodlen = Cache.GetSizeOfLongestModuleName(IncludeVersion()||include_version);

    CXMLIterator    I(p_mele);
    int             scount = 0;
    CXMLElement*     p_ele = NULL;

    while( (p_ele = I.GetNextChildElement("category")) != NULL ) {
        int count = 0;
        CSmallString name;
        // print name of category ------------------------------------------
        p_ele->GetAttribute("name",name);
        if( (name == "System") && (include_system == false) ) continue;
        CSmallString arch;
        p_ele->GetAttribute("arch",arch);

        // check the number of existing modules
        CXMLIterator    M(p_ele);
        CXMLElement*    p_mele;
        int             modcount=0;
        while( (p_mele = M.GetNextChildElement("module")) != NULL ) {
            CSmallString    modname;
            p_mele->GetAttribute("name",modname);
            CXMLElement* p_cmod = Cache.GetModule(modname);
            if(  p_cmod != NULL ){
                if( Cache.IsPermissionGrantedForModuleByAnyMeans(p_cmod) == true ){
                    if( ! arch.IsEmpty() ){
                        if( Cache.HasModuleAnyCompatibleBuild(p_cmod,arch) == true ){
                            modcount++;
                        }
                    } else {
                        modcount++;
                    }
                }
            }
        }
        if( modcount == 0 ) continue;

        terminal.Printf("\n");
        terminal.SetColors(GetCategoryFgColor(),GetCategoryBgColor());
        terminal.PrintTitle((const char*)name,GetCategoryDelimiter(),3);
        terminal.SetDefault();
        terminal.Printf("\n");

        // list content of category ----------------------------------------
        CXMLElement*     p_sele;
        CXMLIterator    J(p_ele);

        terminal.ClearList();
        while( (p_sele = J.GetNextChildElement("module")) != NULL ) {
            CSmallString name;
            p_sele->GetAttribute("name",name);

            // get module
            CXMLElement* p_module = Cache.GetModule(name);
            if( p_module != NULL ) {
                // is allowed on module basis
                if( Cache.IsPermissionGrantedForModule(p_module) == false ) continue;

                if( IncludeVersion() || include_version ) {
                    vector<CSmallString>    versions;
                    CXMLElement*            p_tele;
                    CXMLElement*            p_list = p_module->GetFirstChildElement("builds");
                    CXMLIterator            K(p_list);

                    while( (p_tele = K.GetNextChildElement("build")) != NULL ) {
                        // is allowed on module basis
                        if( Cache.IsPermissionGrantedForBuild(p_tele) == false ) continue;

                        CSmallString ver;
                        p_tele->GetAttribute("ver",ver);
                        bool found = false;
                        for(unsigned int q=0; q < versions.size(); q++) {
                            if( versions[q] == ver ) {
                                found = true;
                                break;
                            }
                        }
                        if( found == false ) {
                            versions.push_back(ver);
                            CSmallString full_name;
                            full_name = name + ":" + ver;
                            terminal.AddItem(full_name);
                            count++;
                        }
                    }
                } else {
                    terminal.AddItem(name);
                    count++;
                }
            }
        }
        if( count == 0 ) {
            terminal.AddItem("[none]");
        }

        terminal.SetColors(GetModuleFgColor(),GetModuleBgColor());
        terminal.PrintColumnSortedList(maxmodlen);
        terminal.SetDefault();
        scount++;
    }

    if( scount == 0 ) {
        terminal.Printf("\n");
        terminal.AddItem("[none]");
        terminal.SetColors(GetModuleFgColor(),GetModuleBgColor());
        terminal.PrintColumnSortedList(maxmodlen);
        terminal.SetDefault();
    }

}

//------------------------------------------------------------------------------

void CPrintEngine::PrintAMSAvailHeader(CTerminal& terminal)
{
    // print section title --------------------------
    terminal.SetColors(GetSectionFgColor(),GetSectionBgColor());
    terminal.SetBold();
    terminal.PrintTitle("AVAILABLE MODULES (Infinity Software Base | amsmodule)",GetSectionDelimiter());
    terminal.SetDefault();
    terminal.Printf("\n");
}

//------------------------------------------------------------------------------

void CPrintEngine::PrintSYSAvailHeader(CTerminal& terminal)
{
    // print section title --------------------------
    terminal.SetColors(GetSectionFgColor(),GetSectionBgColor());
    terminal.SetBold();
    terminal.PrintTitle("AVAILABLE MODULES (System Software Base | sysmodule)",GetSectionDelimiter());
    terminal.SetDefault();
    terminal.Printf("\n");
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CPrintEngine::PrintAvailableSites(CTerminal& terminal,bool print_all)
{
    // setup terminal -------------------------------
    terminal.EnableColors(AreColorsEnabled());

    // make list of all available sites -------------
    CDirectoryEnum dir_enum(BR_ETCDIR("/sites"));

    dir_enum.StartFindFile("*");
    CFileName site_sid;
    terminal.ClearList();

    vector<string>  sites;

    while( dir_enum.FindFile(site_sid) ) {
        CAmsUUID    site_id;
        CSite       site;
        if( site_id.LoadFromString(site_sid) == false ) continue;
        if( site.LoadConfig(site_sid) == false ) continue;

        CSmallString name = site.GetName();
        if( site.IsActive() ) {
            name = "[" + name + "]";
        } else {
            if( print_all && site.CanBeActivated() ) name = name + "*";
        }

        if( print_all || site.CanBeActivated() ) {
            sites.push_back(string(name));
        }
    }
    dir_enum.EndFindFile();

    sort(sites.begin(),sites.end());

    vector<string>::iterator    it = sites.begin();
    vector<string>::iterator    ie = sites.end();

    while( it != ie ){
        terminal.AddItem(*it);
        it++;
    }

    // print header ---------------------------------
    terminal.Printf("\n");
    terminal.SetColors(GetSiteSectionFgColor(),
                       GetSiteSectionBgColor());
    terminal.SetBold();
    terminal.PrintTitle("AVAILABLE SITES",GetSiteSectionDelimiter(),3);
    terminal.SetDefault();
    terminal.Printf("\n\n");

    // print the list -------------------------------
    terminal.PrintColumnSortedList();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CPrintEngine::ListModAvailableModules(CTemplateParams& params,bool include_vers)
{
    // start traverse module cache
    CXMLElement*   p_mele = SystemConfig.GetChildElementByPath("print/categories");

    CXMLIterator    I(p_mele);
    CXMLElement*    p_ele = NULL;

    params.StartCycle("CATEGORIES");

    while( (p_ele = I.GetNextChildElement("category")) != NULL ) {
        CSmallString name;
        // print name of category ------------------------------------------
        p_ele->GetAttribute("name",name);

        // check the number of existing modules
        CXMLIterator    M(p_ele);
        CXMLElement*    p_mele;
        int             modcount=0;
        while( (p_mele = M.GetNextChildElement("module")) != NULL ) {
            CSmallString    modname;
            p_mele->GetAttribute("name",modname);
            if( Cache.GetModule(modname) != NULL ) modcount++;
        }
        if( modcount == 0 ) continue;

        params.SetParam("CATEGORY",name);

        params.StartCycle("MODULES");

        // list content of category ----------------------------------------
        CXMLElement*     p_sele;
        CXMLIterator    J(p_ele);

        while( (p_sele = J.GetNextChildElement("module")) != NULL ) {
            CSmallString name;
            p_sele->GetAttribute("name",name);

            // get module
            CXMLElement* p_module = Cache.GetModule(name);
            if( p_module != NULL ) {
                if( include_vers ) {
                    vector<CSmallString>    versions;
                    CXMLElement*            p_tele;
                    CXMLElement*            p_list = p_module->GetFirstChildElement("builds");
                    CXMLIterator            K(p_list);

                    while( (p_tele = K.GetNextChildElement("build")) != NULL ) {
                        CSmallString ver;
                        p_tele->GetAttribute("ver",ver);
                        bool found = false;
                        for(unsigned int q=0; q < versions.size(); q++) {
                            if( versions[q] == ver ) {
                                found = true;
                                break;
                            }
                        }
                        if( found == false ) {
                            versions.push_back(ver);
                            CSmallString full_name;
                            full_name = name + ":" + ver;
                            params.SetParam("MODULE",full_name);
                            params.NextRun();
                        }
                    }
                } else {
                    params.SetParam("MODULE",name);
                    params.NextRun();
                }
            }
        }
        params.EndCycle("MODULES");
        params.NextRun();
    }

    params.EndCycle("CATEGORIES");
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CPrintEngine::AreColorsEnabled(bool& usersetup)
{
    usersetup = true;

    bool setup = false;
    CXMLElement* p_ele = UserConfig.GetChildElementByPath("user/print");
    if( p_ele ) {
        if( p_ele->GetAttribute("UseColors",setup) == true ) {
            return(setup);
        }
    }

    usersetup = false;
    p_ele = SystemConfig.GetChildElementByPath("print/config");
    if( p_ele == NULL ) {
        ES_ERROR("no user or system configuration");
        return(setup);
    }
    if( p_ele->GetAttribute("UseColors",setup) == false ) {
        ES_ERROR("unable to get setup item");
        return(setup);
    }

    return(setup);
}

//------------------------------------------------------------------------------

char CPrintEngine::GetSiteSectionDelimiter(bool& usersetup)
{
    usersetup = true;

    char setup = ' ';
    CXMLElement* p_ele = UserConfig.GetChildElementByPath("user/print");
    if( p_ele ) {
        if( p_ele->GetAttribute("SiteSectionDelimiter",setup) == true ) {
            return(setup);
        }
    }

    usersetup = false;
    p_ele = SystemConfig.GetChildElementByPath("print/config");
    if( p_ele == NULL ) {
        ES_ERROR("no user or system configuration");
        return(setup);
    }
    if( p_ele->GetAttribute("SiteSectionDelimiter",setup) == false ) {
        ES_ERROR("unable to get setup item");
        return(setup);
    }
    return(setup);
}

//------------------------------------------------------------------------------

int  CPrintEngine::GetSiteSectionBgColor(bool& usersetup)
{
    usersetup = true;

    int setup = -1;
    CXMLElement* p_ele = UserConfig.GetChildElementByPath("user/print");
    if( p_ele ) {
        if( p_ele->GetAttribute("SiteSectionBgColor",setup) == true ) {
            return(setup);
        }
    }

    usersetup = false;
    p_ele = SystemConfig.GetChildElementByPath("print/config");
    if( p_ele == NULL ) {
        ES_ERROR("no user or system configuration");
        return(setup);
    }
    if( p_ele->GetAttribute("SiteSectionBgColor",setup) == false ) {
        ES_ERROR("unable to get setup item");
        return(setup);
    }
    return(setup);
}

//------------------------------------------------------------------------------

int  CPrintEngine::GetSiteSectionFgColor(bool& usersetup)
{
    usersetup = true;

    int setup = -1;
    CXMLElement* p_ele = UserConfig.GetChildElementByPath("user/print");
    if( p_ele ) {
        if( p_ele->GetAttribute("SiteSectionFgColor",setup) == true ) {
            return(setup);
        }
    }

    usersetup = false;
    p_ele = SystemConfig.GetChildElementByPath("print/config");
    if( p_ele == NULL ) {
        ES_ERROR("no user or system configuration");
        return(setup);
    }
    if( p_ele->GetAttribute("SiteSectionFgColor",setup) == false ) {
        ES_ERROR("unable to get setup item");
        return(setup);
    }
    return(setup);
}

//------------------------------------------------------------------------------

char CPrintEngine::GetSectionDelimiter(bool& usersetup)
{
    usersetup = true;

    char setup = ' ';
    CXMLElement* p_ele = UserConfig.GetChildElementByPath("user/print");
    if( p_ele ) {
        if( p_ele->GetAttribute("SectionDelimiter",setup) == true ) {
            return(setup);
        }
    }

    usersetup = false;
    p_ele = SystemConfig.GetChildElementByPath("print/config");
    if( p_ele == NULL ) {
        ES_ERROR("no user or system configuration");
        return(setup);
    }
    if( p_ele->GetAttribute("SectionDelimiter",setup) == false ) {
        ES_ERROR("unable to get setup item");
        return(setup);
    }
    return(setup);
}

//------------------------------------------------------------------------------

int  CPrintEngine::GetSectionBgColor(bool& usersetup)
{
    usersetup = true;

    int setup = -1;
    CXMLElement* p_ele = UserConfig.GetChildElementByPath("user/print");
    if( p_ele ) {
        if( p_ele->GetAttribute("SectionBgColor",setup) == true ) {
            return(setup);
        }
    }

    usersetup = false;
    p_ele = SystemConfig.GetChildElementByPath("print/config");
    if( p_ele == NULL ) {
        ES_ERROR("no user or system configuration");
        return(setup);
    }
    if( p_ele->GetAttribute("SectionBgColor",setup) == false ) {
        ES_ERROR("unable to get setup item");
        return(setup);
    }
    return(setup);
}

//------------------------------------------------------------------------------

int  CPrintEngine::GetSectionFgColor(bool& usersetup)
{
    usersetup = true;

    int setup = -1;
    CXMLElement* p_ele = UserConfig.GetChildElementByPath("user/print");
    if( p_ele ) {
        if( p_ele->GetAttribute("SectionFgColor",setup) == true ) {
            return(setup);
        }
    }

    usersetup = false;
    p_ele = SystemConfig.GetChildElementByPath("print/config");
    if( p_ele == NULL ) {
        ES_ERROR("no user or system configuration");
        return(setup);
    }
    if( p_ele->GetAttribute("SectionFgColor",setup) == false ) {
        ES_ERROR("unable to get setup item");
        return(setup);
    }
    return(setup);
}

//------------------------------------------------------------------------------

char CPrintEngine::GetCategoryDelimiter(bool& usersetup)
{
    usersetup = true;

    char setup = ' ';
    CXMLElement* p_ele = UserConfig.GetChildElementByPath("user/print");
    if( p_ele ) {
        if( p_ele->GetAttribute("CategoryDelimiter",setup) == true ) {
            return(setup);
        }
    }

    usersetup = false;
    p_ele = SystemConfig.GetChildElementByPath("print/config");
    if( p_ele == NULL ) {
        ES_ERROR("no user or system configuration");
        return(setup);
    }
    if( p_ele->GetAttribute("CategoryDelimiter",setup) == false ) {
        ES_ERROR("unable to get setup item");
        return(setup);
    }
    return(setup);
}

//------------------------------------------------------------------------------

int  CPrintEngine::GetCategoryBgColor(bool& usersetup)
{
    usersetup = true;

    int setup = -1;
    CXMLElement* p_ele = UserConfig.GetChildElementByPath("user/print");
    if( p_ele ) {
        if( p_ele->GetAttribute("CategoryBgColor",setup) == true ) {
            return(setup);
        }
    }

    usersetup = false;
    p_ele = SystemConfig.GetChildElementByPath("print/config");
    if( p_ele == NULL ) {
        ES_ERROR("no user or system configuration");
        return(setup);
    }
    if( p_ele->GetAttribute("CategoryBgColor",setup) == false ) {
        ES_ERROR("unable to get setup item");
        return(setup);
    }
    return(setup);
}

//------------------------------------------------------------------------------

int  CPrintEngine::GetCategoryFgColor(bool& usersetup)
{
    usersetup = true;

    int setup = -1;
    CXMLElement* p_ele = UserConfig.GetChildElementByPath("user/print");
    if( p_ele ) {
        if( p_ele->GetAttribute("CategoryFgColor",setup) == true ) {
            return(setup);
        }
    }

    usersetup = false;
    p_ele = SystemConfig.GetChildElementByPath("print/config");
    if( p_ele == NULL ) {
        ES_ERROR("no user or system configuration");
        return(setup);
    }
    if( p_ele->GetAttribute("CategoryFgColor",setup) == false ) {
        ES_ERROR("unable to get setup item");
        return(setup);
    }
    return(setup);
}

//------------------------------------------------------------------------------

int  CPrintEngine::GetModuleBgColor(bool& usersetup)
{
    usersetup = true;

    int setup = -1;
    CXMLElement* p_ele = UserConfig.GetChildElementByPath("user/print");
    if( p_ele ) {
        if( p_ele->GetAttribute("ModuleBgColor",setup) == true ) {
            return(setup);
        }
    }

    usersetup = false;
    p_ele = SystemConfig.GetChildElementByPath("print/config");
    if( p_ele == NULL ) {
        ES_ERROR("no user or system configuration");
        return(setup);
    }
    if( p_ele->GetAttribute("ModuleBgColor",setup) == false ) {
        ES_ERROR("unable to get setup item");
        return(setup);
    }
    return(setup);
}

//------------------------------------------------------------------------------

int  CPrintEngine::GetModuleFgColor(bool& usersetup)
{
    usersetup = true;

    int setup = -1;
    CXMLElement* p_ele = UserConfig.GetChildElementByPath("user/print");
    if( p_ele ) {
        if( p_ele->GetAttribute("ModuleFgColor",setup) == true ) {
            return(setup);
        }
    }

    usersetup = false;
    p_ele = SystemConfig.GetChildElementByPath("print/config");
    if( p_ele == NULL ) {
        ES_ERROR("no user or system configuration");
        return(setup);
    }
    if( p_ele->GetAttribute("ModuleFgColor",setup) == false ) {
        ES_ERROR("unable to get setup item");
        return(setup);
    }
    return(setup);
}

//------------------------------------------------------------------------------

bool CPrintEngine::IncludeVersion(bool& usersetup)
{
    usersetup = true;

    bool setup = false;
    CXMLElement* p_ele = UserConfig.GetChildElementByPath("user/print");
    if( p_ele ) {
        if( p_ele->GetAttribute("IncludeVersion",setup) == true ) {
            return(setup);
        }
    }

    usersetup = false;
    p_ele = SystemConfig.GetChildElementByPath("print/config");
    if( p_ele == NULL ) {
        ES_ERROR("no user or system configuration");
        return(setup);
    }
    if( p_ele->GetAttribute("IncludeVersion",setup) == false ) {
        ES_ERROR("unable to get setup item");
        return(setup);
    }
    return(setup);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CPrintEngine::AreColorsEnabled(void)
{
    bool usersetup;
    return(AreColorsEnabled(usersetup));
}

//------------------------------------------------------------------------------

char CPrintEngine::GetSiteSectionDelimiter(void)
{
    bool usersetup;
    return(GetSiteSectionDelimiter(usersetup));
}

//------------------------------------------------------------------------------

int  CPrintEngine::GetSiteSectionBgColor(void)
{
    bool usersetup;
    return(GetSiteSectionBgColor(usersetup));
}

//------------------------------------------------------------------------------

int  CPrintEngine::GetSiteSectionFgColor(void)
{
    bool usersetup;
    return(GetSiteSectionFgColor(usersetup));
}

//------------------------------------------------------------------------------

char CPrintEngine::GetSectionDelimiter(void)
{
    bool usersetup;
    return(GetSectionDelimiter(usersetup));
}

//------------------------------------------------------------------------------

int  CPrintEngine::GetSectionBgColor(void)
{
    bool usersetup;
    return(GetSectionBgColor(usersetup));
}

//------------------------------------------------------------------------------

int  CPrintEngine::GetSectionFgColor(void)
{
    bool usersetup;
    return(GetSectionFgColor(usersetup));
}

//------------------------------------------------------------------------------

char CPrintEngine::GetCategoryDelimiter(void)
{
    bool usersetup;
    return(GetCategoryDelimiter(usersetup));
}

//------------------------------------------------------------------------------

int  CPrintEngine::GetCategoryBgColor(void)
{
    bool usersetup;
    return(GetCategoryBgColor(usersetup));
}

//------------------------------------------------------------------------------

int  CPrintEngine::GetCategoryFgColor(void)
{
    bool usersetup;
    return(GetCategoryFgColor(usersetup));
}

//------------------------------------------------------------------------------

int  CPrintEngine::GetModuleBgColor(void)
{
    bool usersetup;
    return(GetModuleBgColor(usersetup));
}

//------------------------------------------------------------------------------

int  CPrintEngine::GetModuleFgColor(void)
{
    bool usersetup;
    return(GetModuleFgColor(usersetup));
}

//------------------------------------------------------------------------------

bool CPrintEngine::IncludeVersion(void)
{
    bool usersetup;
    return(IncludeVersion(usersetup));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CPrintEngine::EnableColors(bool set)
{
    CXMLElement* p_ele = UserConfig.GetChildElementByPath("user/print",true);
    p_ele->SetAttribute("UseColors",set);
}

//------------------------------------------------------------------------------

void CPrintEngine::SetSiteSectionDelimiter(char delim)
{
    CXMLElement* p_ele = UserConfig.GetChildElementByPath("user/print",true);
    p_ele->SetAttribute("SiteSectionDelimiter",delim);
}

//------------------------------------------------------------------------------

void CPrintEngine::SetSiteSectionBgColor(int color)
{
    CXMLElement* p_ele = UserConfig.GetChildElementByPath("user/print",true);
    p_ele->SetAttribute("SiteSectionBgColor",color);
}

//------------------------------------------------------------------------------

void CPrintEngine::SetSiteSectionFgColor(int color)
{
    CXMLElement* p_ele = UserConfig.GetChildElementByPath("user/print",true);
    p_ele->SetAttribute("SiteSectionFgColor",color);
}

//------------------------------------------------------------------------------

void CPrintEngine::SetSectionDelimiter(char delim)
{
    CXMLElement* p_ele = UserConfig.GetChildElementByPath("user/print",true);
    p_ele->SetAttribute("SectionDelimiter",delim);
}

//------------------------------------------------------------------------------

void CPrintEngine::SetSectionBgColor(int color)
{
    CXMLElement* p_ele = UserConfig.GetChildElementByPath("user/print",true);
    p_ele->SetAttribute("SectionBgColor",color);
}

//------------------------------------------------------------------------------

void CPrintEngine::SetSectionFgColor(int color)
{
    CXMLElement* p_ele = UserConfig.GetChildElementByPath("user/print",true);
    p_ele->SetAttribute("SectionFgColor",color);
}

//------------------------------------------------------------------------------

void CPrintEngine::SetCategoryDelimiter(char delim)
{
    CXMLElement* p_ele = UserConfig.GetChildElementByPath("user/print",true);
    p_ele->SetAttribute("CategoryDelimiter",delim);
}

//------------------------------------------------------------------------------

void CPrintEngine::SetCategoryBgColor(int color)
{
    CXMLElement* p_ele = UserConfig.GetChildElementByPath("user/print",true);
    p_ele->SetAttribute("CategoryBgColor",color);
}

//------------------------------------------------------------------------------

void CPrintEngine::SetCategoryFgColor(int color)
{
    CXMLElement* p_ele = UserConfig.GetChildElementByPath("user/print",true);
    p_ele->SetAttribute("CategoryFgColor",color) ;
}

//------------------------------------------------------------------------------

void CPrintEngine::SetModuleBgColor(int color)
{
    CXMLElement* p_ele = UserConfig.GetChildElementByPath("user/print",true);
    p_ele->SetAttribute("ModuleBgColor",color);
}

//------------------------------------------------------------------------------

void CPrintEngine::SetModuleFgColor(int color)
{
    CXMLElement* p_ele = UserConfig.GetChildElementByPath("user/print",true);
    p_ele->SetAttribute("ModuleFgColor",color);
}

//------------------------------------------------------------------------------

void CPrintEngine::SetIncludeVersion(bool set)
{
    CXMLElement* p_ele = UserConfig.GetChildElementByPath("user/print",true);
    p_ele->SetAttribute("IncludeVersion",set);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

