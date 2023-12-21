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

#include <ModuleController.hpp>
#include <AMSRegistry.hpp>
#include <Shell.hpp>
#include <ErrorSystem.hpp>
#include <ModUtils.hpp>
#include <Utils.hpp>
#include <PrintEngine.hpp>
#include <Module.hpp>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/classification.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//------------------------------------------------------------------------------

CModuleController  ModuleController;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CModuleController::InitModuleControllerConfig(void)
{
// these are host specific informations; they can be restored from AMS registry in jobs
    BundleName  = AMSRegistry.GetBundleName();
    BundlePath  = AMSRegistry.GetBundlePath();

// these are runtime informations
    std::string sActiveModules   = std::string(CShell::GetSystemVariable("AMS_ACTIVE_MODULES"));
    if( ! sActiveModules.empty() ) split(ActiveModules,sActiveModules,is_any_of("|"),boost::token_compress_on);

    std::string sExportedModules = std::string(CShell::GetSystemVariable("AMS_EXPORTED_MODULES"));
    if( ! sExportedModules.empty() ) split(ExportedModules,sExportedModules,is_any_of("|"),boost::token_compress_on);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CModuleController::LoadBundles(EModBundleCache type)
{
    std::list<CFileName>    names;
    std::list<CFileName>    paths;

    std::string sname(BundleName);
    std::string spath(BundlePath);

    split(names,sname,is_any_of(","));
    split(paths,spath,is_any_of(":"));

    for(CFileName name : names){
        for(CFileName path : paths){
            if( CModBundle::IsBundle(path,name) == false ) continue;
            CModBundlePtr p_bundle(new CModBundle());
            if( p_bundle->InitBundle(path/name) == false ){
                // this is fishy - record
                CSmallString warning;
                warning << "unable to init bunde '" << path / name << "'";
                ES_WARNING(warning);
                continue;
            }
            if( p_bundle->LoadCache(type) == false ){
                // this is fishy - record
                CSmallString warning;
                warning << "unable to load cache for bundle '" << path / name << "'";
                ES_WARNING(warning);
                continue;
            }
            Bundles.push_back(p_bundle);
            break;
        }
    }
}

//------------------------------------------------------------------------------

void CModuleController::PrintBundlesInfo(CVerboseStr& vout)
{
    vout << endl;
    vout << "# *** Bundle Setup *** " << endl;
    vout << "# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    CUtils::PrintTokens(vout,"# Bundle names : ",BundleName,",",80,'#');
    CUtils::PrintTokens(vout,"# Bundle paths : ",BundlePath,":",80,'#');

    vout << endl;
    vout << ">>>>> Loaded Bundles" << endl;
    for( CModBundlePtr p_bundle : Bundles ){
        vout << endl;
        p_bundle->PrintInfo(vout,true);
    }
}

//------------------------------------------------------------------------------

void CModuleController::MergeBundles(void)
{
    for( CModBundlePtr p_bundle : Bundles ){
        CXMLElement* p_cache = p_bundle->GetCacheElement();
        CXMLElement* p_config = p_bundle->GetBundleElement();
        ModCache.MergeWithCache(p_cache,p_config);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CModuleController::IsModuleActive(const CSmallString& module)
{
    CSmallString name,ver,arch,para;
    CModUtils::ParseModuleName(module,name,ver,arch,para);

    for(CSmallString mactive : ActiveModules){
        CSmallString lname,lver,larch,lpara;
        CModUtils::ParseModuleName(mactive,lname,lver,larch,lpara);
        if( lname == name ) {
            if( ver == NULL ) return(true);
            if( lver == ver ) {
                if( arch == NULL ) return(true);
                if( larch == arch ) {
                    if( para == NULL ) return(true);
                    if( lpara == para ) return(true);
                }
            }
        }
    }

    return(false);
}

//------------------------------------------------------------------------------

bool CModuleController::IsModuleExported(const CSmallString& module)
{
    CSmallString name,ver,arch,para;
    CModUtils::ParseModuleName(module,name,ver,arch,para);

    for(CSmallString mexported : ExportedModules){
        CSmallString lname,lver,larch,lpara;
        CModUtils::ParseModuleName(mexported,lname,lver,larch,lpara);
        if( lname == name ) {
            return(true);
        }
    }

    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CModuleController::GetActiveModuleVersion(const CSmallString& module,
                                               CSmallString& actver)
{
    actver = NULL;

    CSmallString name,ver,arch,para;
    CModUtils::ParseModuleName(module,name,ver,arch,para);

    for(CSmallString mactive : ActiveModules){
        CSmallString lname,lver,larch,lpara;
        CModUtils::ParseModuleName(mactive,lname,lver,larch,lpara);
        if( lname == name ) {
            if( ver == NULL ) {
                actver = lver;
                return(true);
            }
            if( lver == ver ) {
                if( arch == NULL ) {
                    actver = lver;
                    return(true);
                }
                if( larch == arch ) {
                    if( para == NULL ) {
                        actver = lver;
                        return(true);
                    }
                    if( lpara == para ) {
                        actver = lver;
                        return(true);
                    }
                }
            }
        }
    }

    return(false);
}

//------------------------------------------------------------------------------

const CSmallString CModuleController::GetActiveModules(void)
{
    CSmallString    amods;
    bool            first = true;
    for(CSmallString mod : ActiveModules){
        if( ! first ) amods << "|";
        amods << mod;
        first = false;
    }
    return(amods);
}

//------------------------------------------------------------------------------

const CSmallString CModuleController::GetExportedModules(void)
{
    CSmallString    emods;
    bool            first = true;
    for(CSmallString mod : ExportedModules){
        if( ! first ) emods << "|";
        emods << mod;
        first = false;
    }
    return(emods);
}

//------------------------------------------------------------------------------

const CSmallString CModuleController::GetActiveModuleSpecification(const CSmallString& name)
{
    for(CSmallString mactive : ActiveModules){
        if( CModUtils::GetModuleName(mactive) == name ) return(mactive);
    }
    return("");
}

//------------------------------------------------------------------------------

const CSmallString CModuleController::GetExportedModuleSpecification(const CSmallString& name)
{
    for(CSmallString mexported : ExportedModules){
        if( CModUtils::GetModuleName(mexported) == name ) return(mexported);
    }
    return("");
}

//-----------------------------------------------------------------------------

void CModuleController::UpdateActiveModules(const CSmallString& module,
                                            bool add_module)
{
    ActiveModules.remove(module);
    if( add_module) ActiveModules.push_back(module);
}

//-----------------------------------------------------------------------------

void CModuleController::UpdateExportedModules(const CSmallString& module,
                                              bool add_module)
{
    ExportedModules.remove(module);
    if( add_module) ExportedModules.push_back(module);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CModuleController::PrintModActiveModules(CTerminal& terminal)
{
    PrintEngine.PrintHeader(terminal,"ACTIVE MODULES",EPEHS_SECTION);

    int maxmodlen = 0;
    for(CSmallString mactive : ActiveModules){
        int len = mactive.GetLength();
        if( len > maxmodlen ) maxmodlen = len;
    }
    // do not count exported modules - their names are always shorter
    maxmodlen++;

    terminal.Printf("\n");
    if( ActiveModules.size() == 0 ){
        std::list<CSmallString> none;
        none.push_back("-none-");
        PrintEngine.PrintItems(terminal,none,maxmodlen);
    } else {
        PrintEngine.PrintItems(terminal,ActiveModules,maxmodlen);
    }
}

//------------------------------------------------------------------------------

void CModuleController::PrintModExportedModules(CTerminal& terminal)
{
    PrintEngine.PrintHeader(terminal,"EXPORTED MODULES",EPEHS_SECTION);

    int maxmodlen = 0;
    for(CSmallString mactive : ActiveModules){
        int len = mactive.GetLength();
        if( len > maxmodlen ) maxmodlen = len;
    }
    // do not count exported modules - their names are always shorter
    maxmodlen++;

    terminal.Printf("\n");
    if( ExportedModules.size() == 0 ){
        std::list<CSmallString> none;
        none.push_back("-none-");
        PrintEngine.PrintItems(terminal,none,maxmodlen);
    } else {
        PrintEngine.PrintItems(terminal,ExportedModules,maxmodlen);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CModuleController::ReactivateModules(CVerboseStr& vout)
{
    bool result = true;
    for(CSmallString mod : ActiveModules){
        bool exported = IsModuleExported(mod);
        result &= Module.AddModule(vout,mod,false,!exported);
    }
    return(result);
}

//------------------------------------------------------------------------------

bool CModuleController::PurgeModules(CVerboseStr& vout)
{
    std::list<CSmallString> modules = ActiveModules;
    modules.reverse();

    std::list<CSmallString>::iterator it = modules.begin();
    std::list<CSmallString>::iterator ie = modules.end();
    bool result = true;

    while( it != ie ){
        CSmallString module = *it;
        result &= Module.RemoveModule(vout,module);
        it++;
    }

    return(result);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

