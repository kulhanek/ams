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
    CSmallString    version;
    int             verindx;
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
    if( left.version == right.version ) return(false);
    if( left.verindx < right.verindx ) return(false);
    if( left.verindx == right.verindx ){
        return( left.version > right.version );
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

void CModCache::GetCategories(std::list<CSmallString>& list)
{
    CXMLElement* p_cele = Cache.GetFirstChildElement("cache");
    if( p_cele == NULL ){
        RUNTIME_ERROR("unable to open cache element");
    }

    CXMLElement* p_mele = p_cele->GetFirstChildElement("module");

    while( p_mele != NULL ) {

        CXMLElement*  p_dele = p_mele->GetFirstChildElement("category");
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
        RUNTIME_ERROR("unable to open cache element");
    }

    CXMLElement* p_mele = p_cele->GetFirstChildElement("module");

    while( p_mele != NULL ) {
        bool include = false;

        CXMLElement*  p_dele = p_mele->GetFirstChildElement("category");
        if( p_dele != NULL ){
            CSmallString cname;
            p_dele->GetAttribute("name",cname);
            if( category == cname ) {
                include = true;
            }
        } else {
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

void CModCache::GetModuleVersions(CXMLElement* p_mele, std::list<CSmallString>& list)
{
    CXMLElement*  p_bele = p_mele->GetFirstChildElement("build");

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
    CXMLElement*  p_bele = p_mele->GetFirstChildElement("build");

    std::list<CPVerRecord>  pvlist;

    while( p_bele != NULL ) {
        CPVerRecord verrcd;
        verrcd.verindx = 0.0;
        p_bele->GetAttribute("ver",verrcd.version);
        p_bele->GetAttribute("verindx",verrcd.verindx);
        if( verrcd.version != NULL ){
        pvlist.push_back(verrcd);
        }
        p_bele = p_bele->GetNextSiblingElement("build");
    }

    pvlist.sort(sort_tokens);
    pvlist.unique();

    for(CPVerRecord pvrec : pvlist){
        list.push_back(pvrec.version);
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

// print modules
    for(CSmallString cat : cats){
        std::list<CSmallString> mods;
        GetModules(cat,mods,includever);
        mods.sort();
        mods.unique();
        if( mods.empty() ) continue;
        PrintEngine.PrintHeader(terminal,cat,EPEHS_CATEGORY);
        PrintEngine.PrintItems(terminal,mods);
    }

    if( includesys ){
        std::list<CSmallString> mods;
        GetModules("sys",mods,includever);
        mods.sort();
        mods.unique();
        if( ! mods.empty() ){
            PrintEngine.PrintHeader(terminal,"System & Uncategorized Modules",EPEHS_CATEGORY);
            PrintEngine.PrintItems(terminal,mods);
        }
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
