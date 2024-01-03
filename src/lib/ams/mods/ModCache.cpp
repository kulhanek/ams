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

#include <ModCache.hpp>
#include <ErrorSystem.hpp>
#include <XMLPrinter.hpp>
#include <XMLParser.hpp>
#include <PrintEngine.hpp>
#include <FileSystem.hpp>
#include <XMLComment.hpp>
#include <ModUtils.hpp>
#include <User.hpp>

//------------------------------------------------------------------------------

using namespace std;
//using namespace boost;
//using namespace boost::algorithm;

//------------------------------------------------------------------------------

CModCache ModCache;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

class CPVerRecord {
    public:
    CPVerRecord(void);
    CSmallString    ver;
    CSmallString    arch;
    CSmallString    mode;
    double          verindx;
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
    result &= ver == left.ver;
    return(result);
}

//------------------------------------------------------------------------------

bool sort_tokens(const CPVerRecord& left,const CPVerRecord& right)
{
    if( left.ver == right.ver ) return(false);
    if( left.verindx < right.verindx ) return(false);
    if( left.verindx == right.verindx ){
        return( left.ver > right.ver );
    }
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CModCache::CModCache(void)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CModCache::LoadCacheFile(const CFileName& name)
{
    Cache.RemoveAllChildNodes();

// open cache file
    CXMLParser   xml_parser;

    xml_parser.SetOutputXMLNode(&Cache);
    xml_parser.EnableWhiteCharacters(true);

    if( xml_parser.Parse(name) == false ) {
        CSmallString error;
        error << "unable to parse module cache file '" << name << "'";
        ES_ERROR(error);
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CModCache::SaveCacheFile(const CFileName& name)
{
    // for pseudo-atomic operation
    CFileName tmp_name = name + ".tmp";

    CXMLPrinter xml_printer;

    xml_printer.SetPrintedXMLNode(&Cache);
    xml_printer.SetPrintAsItIs(true);

    if( xml_printer.Print(tmp_name) == false ) {
        CSmallString error;
        error << "unable to save module cache file '" << tmp_name << "'";
        ES_ERROR(error);
        return(false);
    }

    if( CFileSystem::Rename(tmp_name,name) == false ){
        CSmallString error;
        error << "unable to write final module cache file '" << name << "'";
        ES_ERROR(error);
        return(false);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CModCache::RemoveDocumentation(void)
{
    CXMLElement* p_cele = Cache.GetFirstChildElement("cache");
    if( p_cele == NULL ){
        RUNTIME_ERROR("unable to open cache element");
    }
    CXMLElement* p_mele = p_cele->GetFirstChildElement("module");

    while( p_mele != NULL ) {
        CXMLElement*  p_dele = p_mele->GetFirstChildElement("doc");
        p_mele = p_mele->GetNextSiblingElement("module");
        if( p_dele != NULL ) delete p_dele;
    }
}

//------------------------------------------------------------------------------

void CModCache::CleanBuild(CXMLNode* p_bele)
{
    if( p_bele == NULL ){
        RUNTIME_ERROR("no build");
    }

    CXMLNode* p_nele = p_bele->GetFirstChildNode();

    while( p_nele != NULL ) {
        if( p_nele->HasChildNodes() ){
            CleanBuild(p_nele);
        }
        CXMLNode*  p_tele = p_nele;
        p_nele = p_nele->GetNextSiblingNode();
        if( (p_tele != NULL) &&
            ( (p_tele->GetNodeType() == EXNT_COMMENT) || (p_tele->GetNodeType() == EXNT_TEXT) ) ) {
            delete p_tele;
        }
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CXMLElement* CModCache::GetCacheElement(void)
{
    CXMLElement* p_cele = Cache.GetFirstChildElement("cache");
    if( p_cele == NULL ){
        RUNTIME_ERROR("unable to open cache element");
    }
    return(p_cele);
}

//------------------------------------------------------------------------------

CXMLElement* CModCache::GetModule(const CSmallString& name,bool create)
{
    CXMLElement* p_cele = Cache.GetFirstChildElement("cache");
    if( p_cele == NULL ){
        RUNTIME_ERROR("unable to open cache element");
    }

    CXMLElement* p_mele = p_cele->GetFirstChildElement("module");
    while( p_mele != NULL ) {
        CSmallString modname;
        p_mele->GetAttribute("name",modname);
        if( modname == name ) return(p_mele);
        p_mele = p_mele->GetNextSiblingElement("module");
    }

    if( create ){
        p_mele = p_cele->CreateChildElement("module");
        p_mele->SetAttribute("name",name);
        return(p_mele);
    }

    return(NULL);
}

//------------------------------------------------------------------------------

CXMLElement* CModCache::CreateModule(const CSmallString& name)
{
    CXMLElement* p_cele = Cache.GetFirstChildElement("cache");
    if( p_cele == NULL ){
        RUNTIME_ERROR("unable to open cache element");
    }

    CXMLElement* p_mele = p_cele->CreateChildElement("module");
    p_mele->SetAttribute("name",name);
    return(p_mele);
}

//------------------------------------------------------------------------------

CXMLElement* CModCache::GetBuild(CXMLElement* p_mele,
                                const CSmallString& ver,
                                const CSmallString& arch,
                                const CSmallString& mode)
{
    if( p_mele == NULL ) return(NULL);

    CXMLElement* p_bele = p_mele->GetChildElementByPath("builds/build");

    while( p_bele != NULL ) {
        CSmallString lver;
        CSmallString larch;
        CSmallString lmode;
        p_bele->GetAttribute("ver",lver);
        p_bele->GetAttribute("arch",larch);
        p_bele->GetAttribute("mode",lmode);
        if( (lver == ver) && (larch == arch) && (lmode == mode) ) return(p_bele);
        p_bele = p_bele->GetNextSiblingElement("build");
    }

    return(NULL);
}

//------------------------------------------------------------------------------

CXMLElement* CModCache::GetModuleDoc(CXMLElement* p_module)
{
    if( p_module == NULL ) return(NULL);
    CXMLElement* p_doc = p_module->GetChildElementByPath("doc");
    return(p_doc);
}

//------------------------------------------------------------------------------

bool CModCache::GetModuleDefaults(CXMLElement* p_mele,
                                  CSmallString& ver, CSmallString& arch, CSmallString& mode)
{
    ver = NULL;
    arch = NULL;
    mode = NULL;

    if( p_mele == NULL ) return(false);

    CXMLElement* p_dele = p_mele->GetFirstChildElement("default");
    if( p_dele == NULL ) return(false);

    bool result = true;
    result &= p_dele->GetAttribute("ver",ver);
    result &= p_dele->GetAttribute("arch",arch);
    result &= p_dele->GetAttribute("mode",mode);
    return(result);
}

//------------------------------------------------------------------------------

void CModCache::GetModuleVersions(CXMLElement* p_mele, std::list<CSmallString>& list)
{
    CXMLElement*  p_bele = p_mele->GetChildElementByPath("builds/build");
    while( p_bele != NULL ) {
        CSmallString modver;
        p_bele->GetAttribute("ver",modver);
        if( modver != NULL ) list.push_back(modver);
        p_bele = p_bele->GetNextSiblingElement("build");
    }
}

//------------------------------------------------------------------------------

void CModCache::GetModuleVersionsSorted(CXMLElement* p_mele, std::list<CSmallString>& list)
{
    CXMLElement*  p_bele = p_mele->GetChildElementByPath("builds/build");

    std::list<CPVerRecord>  pvlist;

    while( p_bele != NULL ) {
        CPVerRecord verrcd;
        verrcd.verindx = 0.0;
        p_bele->GetAttribute("ver",verrcd.ver);
        p_bele->GetAttribute("verindx",verrcd.verindx);
        if( verrcd.ver != NULL ){
        pvlist.push_back(verrcd);
        }
        p_bele = p_bele->GetNextSiblingElement("build");
    }

    pvlist.sort(sort_tokens);
    pvlist.unique();

    for(CPVerRecord pvrec : pvlist){
        list.push_back(pvrec.ver);
    }
}

//------------------------------------------------------------------------------

void CModCache::GetModuleBuildsSorted(CXMLElement* p_mele, std::list<CSmallString>& list)
{
    CXMLElement*  p_bele = p_mele->GetChildElementByPath("builds/build");

    std::list<CPVerRecord>  pvlist;

    while( p_bele != NULL ) {
        CPVerRecord bldrcd;
        bldrcd.verindx = 0.0;
        p_bele->GetAttribute("ver",bldrcd.ver);
        cerr << bldrcd.ver << endl;
        p_bele->GetAttribute("arch",bldrcd.arch);
        p_bele->GetAttribute("mode",bldrcd.mode);
        p_bele->GetAttribute("verindx",bldrcd.verindx);
        if( (bldrcd.ver != NULL) && (bldrcd.arch != NULL) && (bldrcd.mode != NULL) ){
            pvlist.push_back(bldrcd);
        }
        p_bele = p_bele->GetNextSiblingElement("build");
    }

    pvlist.sort(sort_tokens);
    pvlist.unique();

    for(CPVerRecord pvrec : pvlist){
        CSmallString build;
        build << pvrec.ver << ":" << pvrec.arch << ":" << pvrec.mode;
        list.push_back(build);
    }
}

//------------------------------------------------------------------------------

bool CModCache::CanModuleBeExported(CXMLElement* p_mele)
{
    if( p_mele == NULL ) return(true);
    bool export_flag = true;
    p_mele->GetAttribute("export",export_flag);
    return(export_flag);
}

//------------------------------------------------------------------------------

bool CModCache::CheckModuleVersion(CXMLElement* p_mele,const CSmallString& ver)
{
    if( p_mele == NULL ) return(false);

    CXMLElement* p_build = p_mele->GetChildElementByPath("builds/build");

    while( p_build != NULL ) {
        CSmallString lver;
        p_build->GetAttribute("ver",lver);
        if( lver == ver ) return(true);
        p_build = p_build->GetNextSiblingElement("build");
    }

    return(false);
}

//------------------------------------------------------------------------------

bool CModCache::IsPermissionGrantedForModule(CXMLElement* p_mele)
{
    if( p_mele == NULL ){
        RUNTIME_ERROR("p_mele is NULL");
    }

    CXMLElement* p_acl = p_mele->GetFirstChildElement("acl");
    if( p_acl == NULL ) return(true);

    return(IsPermissionGranted(p_acl));
}

//------------------------------------------------------------------------------

bool CModCache::IsPermissionGrantedForBuild(CXMLElement* p_build)
{
    if( p_build == NULL ){
        RUNTIME_ERROR("p_build is NULL");
    }

    CXMLElement* p_acl = p_build->GetFirstChildElement("acl");
    if( p_acl == NULL ) return(true);

    return(IsPermissionGranted(p_acl));
}


//------------------------------------------------------------------------------

bool CModCache::IsPermissionGranted(CXMLElement* p_acl)
{
    if( p_acl == NULL ){
        RUNTIME_ERROR("p_acl is NULL");
    }

    CSmallString value = "allow";
    p_acl->GetAttribute("default",value);

    if( value == "allow" ){
        CXMLElement* p_rule = p_acl->GetFirstChildElement();
        while( p_rule != NULL ){
            CSmallString group;
            p_rule->GetAttribute("group",group);
            if( p_rule->GetName() == "allow" ){
                if( User.IsInACLGroup(group) == true ) return(true);
            }
            if( p_rule->GetName() == "deny" ){
                if( User.IsInACLGroup(group) == true ) return(false);
            }
            p_rule = p_rule->GetNextSiblingElement();
        }
        return(true);
    }

    if( value == "deny" ){
        CXMLElement* p_rule = p_acl->GetFirstChildElement();
        while( p_rule != NULL ){
            CSmallString group;
            p_rule->GetAttribute("group",group);
            if( p_rule->GetName() == "allow" ){
                if( User.IsInACLGroup(group) == true ) return(true);
            }
            if( p_rule->GetName() == "deny" ){
                if( User.IsInACLGroup(group) == true ) return(false);
            }
            p_rule = p_rule->GetNextSiblingElement();
        }
        return(false);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CModCache::GetCategories(std::list<CSmallString>& list)
{
    CXMLElement* p_cele = Cache.GetFirstChildElement("cache");
    if( p_cele == NULL ){
        ES_WARNING("unable to open cache element, no bundles loaded?");
        return;
    }

    CXMLElement* p_mele = p_cele->GetFirstChildElement("module");
    while( p_mele != NULL ) {

        CXMLElement*  p_dele = p_mele->GetChildElementByPath("categories/category");
        // support multiple categories
        while( p_dele != NULL ) {
            CSmallString cname;
            p_dele->GetAttribute("name",cname);
            if( cname != NULL ) list.push_back(cname);
            p_dele = p_dele->GetNextSiblingElement("category");
        }

        p_mele = p_mele->GetNextSiblingElement("module");
    }
}

//------------------------------------------------------------------------------

void CModCache::GetModules(const CSmallString& category, std::list<CSmallString>& list,bool includever)
{
    CXMLElement* p_cele = Cache.GetFirstChildElement("cache");
    if( p_cele == NULL ){
        ES_WARNING("unable to open cache element, no bundles loaded?");
        return;
    }

    CXMLElement* p_mele = p_cele->GetFirstChildElement("module");
    while( p_mele != NULL ) {
        bool include = false;
        bool hascat  = false;

        CXMLElement*  p_dele = p_mele->GetChildElementByPath("categories/category");
        while( p_dele != NULL ){
            CSmallString cname;
            p_dele->GetAttribute("name",cname);
            hascat = true;
            if( category == cname ) {
                include = true;
                break;
            }
            p_dele = p_dele->GetNextSiblingElement("category");
        }

        if( hascat == false ){
            if( category == "sys" ){
                include = true;
            }
        }

        if( include ){
            CSmallString modname;
            p_mele->GetAttribute("name",modname);
            if( modname != NULL ){
                if( includever ) {
                    std::list<CSmallString> vers;
                    // sorting does not have an effect as the whole list is sorted later
                    GetModuleVersions(p_mele,vers);
                    for(CSmallString modver : vers){
                        CSmallString module;
                        module << modname << ":" << modver;
                        list.push_back(module);
                    }
                } else {
                    list.push_back(modname);
                }
            }
        }

        p_mele = p_mele->GetNextSiblingElement("module");
    }
}

//------------------------------------------------------------------------------

int CModCache::GetModulePrintSize(bool includever)
{
    size_t len = 0;

    CXMLElement* p_cele = Cache.GetFirstChildElement("cache");
    if( p_cele == NULL ){
        ES_WARNING("unable to open cache element, no bundles loaded?");
        return(len);
    }

    CXMLElement* p_mele = p_cele->GetFirstChildElement("module");
    while( p_mele != NULL ) {
        CSmallString modname;
        p_mele->GetAttribute("name",modname);
        if( modname != NULL ){
            if( includever ) {
                std::list<CSmallString> vers;
                // sorting does not have an effect as the whole list is sorted later
                GetModuleVersions(p_mele,vers);
                for(CSmallString modver : vers){
                    CSmallString module;
                    module << modname << ":" << modver;
                    if( module.GetLength() > len ) len = module.GetLength();
                }
            } else {
                if( modname.GetLength() > len ) len = modname.GetLength();
            }
        }

        p_mele = p_mele->GetNextSiblingElement("module");
    }
    len++;
    return(len);
}

//------------------------------------------------------------------------------

void CModCache::GetBuildsForCGen(std::list<CSmallString>& list,int numparts)
{
    CXMLElement* p_cele = Cache.GetFirstChildElement("cache");
    if( p_cele == NULL ){
        ES_WARNING("unable to open cache element, no bundles loaded?");
        return;
    }

    CXMLElement* p_mele = p_cele->GetFirstChildElement("module");
    while( p_mele != NULL ) {

        CSmallString name;
        p_mele->GetAttribute("name",name);

        CSmallString suggestion;

        switch(numparts) {
        case 0:
            suggestion = name;
            break;
        case 1:
            suggestion = name + ":" + "default";
            break;
        case 2:
            suggestion = name + ":" + "default" + ":" + "auto";
            break;
        case 3:
            suggestion = name + ":" + "default" + ":" + "auto" + ":" + "auto";
            break;
        default:
            break;
        }

        list.push_back(suggestion);

        CXMLElement*  p_dele = p_mele->GetChildElementByPath("builds/build");
        while( p_dele != NULL ) {

            CSmallString ver;
            CSmallString arch;
            CSmallString mode;
            p_dele->GetAttribute("ver",ver);
            p_dele->GetAttribute("arch",arch);
            p_dele->GetAttribute("mode",mode);

            CSmallString suggestion;

            // how many items from name should be printed?
            switch(numparts) {
            case 0:
                suggestion = name;
                break;
            case 1:
                suggestion = name + ":" + ver;
                break;
            case 2:
                suggestion = name + ":" + ver + ":" + arch;
                break;
            case 3:
                suggestion = name + ":" + ver + ":" + arch + ":" + mode;
                break;
            default:
                break;
            }

            list.push_back(suggestion);

            p_dele = p_dele->GetNextSiblingElement("build");
        }

        p_mele = p_mele->GetNextSiblingElement("module");
    }
}

//------------------------------------------------------------------------------

void CModCache::PrintAvail(CTerminal& terminal,bool includever,bool includesys)
{
// get categories
    std::list<CSmallString> cats;
    GetCategories(cats);
    cats.sort();
    cats.unique();

    int len = GetModulePrintSize(includever);

// print modules
    for(CSmallString cat : cats){
        std::list<CSmallString> mods;
        GetModules(cat,mods,includever);
        mods.sort();
        mods.unique();
        if( mods.empty() ) continue;
        PrintEngine.PrintHeader(terminal,cat,EPEHS_CATEGORY);
        PrintEngine.PrintItems(terminal,mods,len);
    }

    if( includesys ){
        std::list<CSmallString> mods;
        GetModules("sys",mods,includever);
        mods.sort();
        mods.unique();
        if( ! mods.empty() ){
            PrintEngine.PrintHeader(terminal,"System & Uncategorized Modules",EPEHS_CATEGORY);
            PrintEngine.PrintItems(terminal,mods,len);
        }
    }
}

//------------------------------------------------------------------------------

void CModCache::PrintModuleVersions(CVerboseStr& vout, const CSmallString& module)
{
    CSmallString name = CModUtils::GetModuleName(module);

    vout << "# Module name: " << name << " (available versions, the default is bold)" << endl;
    vout << "#=======================================================================" << endl;

    CXMLElement* p_mele = GetModule(name);
    if( p_mele == NULL ){
        vout << " -module not found-" << endl;
        return;
    }

    CSmallString dver,darch,dmode;
    GetModuleDefaults(p_mele,dver,darch,dmode);

    std::list<CSmallString> vers;
    GetModuleVersionsSorted(p_mele,vers);
    for(CSmallString ver : vers){
        if( ver == dver ) vout << "<b>";
        vout << name << ":" << ver;
        if( ver == dver ) vout << "</b>";
        vout << endl;
    }
}

//------------------------------------------------------------------------------

void CModCache::PrintModuleBuilds(CVerboseStr& vout, const CSmallString& module)
{
    CSmallString name = CModUtils::GetModuleName(module);

    vout << "# Module name: " << name << " (available builds)" << endl;
    vout << "# =============================================================" << endl;

    CXMLElement* p_mele = GetModule(name);
    if( p_mele == NULL ){
        vout << " -module not found-" << endl;
        return;
    }

    std::list<CSmallString> builds;
    GetModuleBuildsSorted(p_mele,builds);
    for(CSmallString build : builds){
        vout << name << ":" << build << endl;
    }
}

//------------------------------------------------------------------------------

void CModCache::PrintModuleOrigin(CVerboseStr& vout, const CSmallString& module)
{
    CSmallString name = CModUtils::GetModuleName(module);

    vout << "# Module name: " << name << " (module origin)" << endl;
    vout << "# =============================================================" << endl;

    CXMLElement* p_mele = GetModule(name);
    if( p_mele == NULL ){
        vout << " -module not found-" << endl;
        return;
    }

    CXMLElement* p_bundle = p_mele->GetFirstChildElement("bundle");
    if( p_bundle == NULL ){
        vout << " -no origin information available-" << endl;
        return;
    }

    CSmallString bname,bpath,bid,baintainer,bcontact;

    p_bundle->GetAttribute("name",bname);
    p_bundle->GetAttribute("path",bpath);
    p_bundle->GetAttribute("id",bid);

    CXMLElement* p_maintainer = p_bundle->GetFirstChildElement("maintainer");
    if( p_maintainer ){
        p_maintainer->GetAttribute("name",baintainer);
        p_maintainer->GetAttribute("email",bcontact);
    }

    vout << "# Bundle name : " << bname << endl;
    vout << "# Bundle path : " << bpath << endl;
    vout << "# Bundle ID   : " << bid << endl;
    vout << "# Maintainer  : " << baintainer << " (" << bcontact << ")" << endl;
}

//------------------------------------------------------------------------------

int CModCache::GetNumberOfModules(void)
{
    std::list<CSmallString> mods;

    CXMLElement* p_cele = Cache.GetFirstChildElement("cache");
    if( p_cele == NULL ){
        RUNTIME_ERROR("unable to open cache element");
    }

    CXMLElement* p_mele = p_cele->GetFirstChildElement("module");
    while( p_mele != NULL ) {
        CSmallString modname;
        p_mele->GetAttribute("name",modname);
        if( modname != NULL ) mods.push_back(modname);
        p_mele = p_mele->GetNextSiblingElement("module");
    }

    mods.sort();
    mods.unique();
    return(mods.size());
}

//------------------------------------------------------------------------------

int CModCache::GetNumberOfCategories(void)
{
    std::list<CSmallString> cats;
    GetCategories(cats);
    cats.sort();
    cats.unique();
    return(cats.size());
}

//------------------------------------------------------------------------------

void CModCache::MergeWithCache(CXMLElement* p_bcele,CXMLElement* p_origin)
{
    CXMLElement* p_lcele = Cache.GetFirstChildElement("cache");
    if( p_lcele == NULL ){
        p_lcele = CreateEmptyCache();
    }
    if( p_bcele == NULL ){
        RUNTIME_ERROR("p_bcache == NULL");
    }
    CXMLElement* p_bmele = p_bcele->GetFirstChildElement("module");
    while( p_bmele != NULL ) {
        CSmallString modname;
        p_bmele->GetAttribute("name",modname);
        if( GetModule(modname) == NULL ){
            // module does not exist - include module to cache
            CXMLNode* p_nmod = p_bmele->DuplicateNode(p_lcele);
            if( p_nmod == NULL ) {
                CSmallString error;
                error << "unable to add '" << modname << "' module to the cache";
                RUNTIME_ERROR(error);
            }
            if( p_origin ){
                p_origin->DuplicateNode(p_nmod);
            }
        }
        p_bmele = p_bmele->GetNextSiblingElement("module");
    }
}

//------------------------------------------------------------------------------

CXMLElement* CModCache::CreateEmptyCache(void)
{
    Cache.RemoveAllChildNodes();

// create header elements
    Cache.CreateChildDeclaration();
    CXMLComment* p_comment;

    p_comment = Cache.CreateChildComment();
    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();
    CSmallString title;
    title << "Advanced Module System (AMS) cache built at " << dt.GetSDateAndTime();
    p_comment->SetComment(title);

    p_comment = Cache.CreateChildComment();
    CSmallString warn;
    warn << "please do not edit this file (it is created by ams-bundle command)";
    p_comment->SetComment(warn);

// create root element
    CXMLElement* p_cele = Cache.CreateChildElement("cache");
    return(p_cele);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CModCache::GetVariableValue(CXMLElement* p_rele,
                                               const CSmallString& name)
{
    if( p_rele == NULL ) return("");

    CXMLElement* p_sele = p_rele->GetChildElementByPath("setup/variable");

    while( p_sele != NULL ) {

        CSmallString lname;
        CSmallString lvalue;
        p_sele->GetAttribute("name",lname);
        p_sele->GetAttribute("value",lvalue);
        if( lname == name ) return(lvalue);

        p_sele = p_sele->GetNextSiblingElement("variable");
    }
    return("");
}

//------------------------------------------------------------------------------

bool CModCache::DoesItNeedGPU(const CSmallString& name)
{
    CXMLElement* p_module = GetModule(name);
    if( p_module == NULL ){
        CSmallString warning;
        warning << "module '" << name << "'' was not found in the cache";
        ES_WARNING(warning);
        return(false);
    }

    CXMLElement* p_sele = p_module->GetChildElementByPath("builds/build");

    bool gpu = false;

    while( p_sele != NULL ) {
        CSmallString larch;
        p_sele->GetAttribute("arch",larch);
        if( (larch.FindSubString("gpu") >= 0) || (larch.FindSubString("cuda") >= 0) ){
            gpu = true;
        }
        p_sele = p_sele->GetNextSiblingElement("build");
    }

    return(gpu);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
