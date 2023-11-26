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

#include <ModBundle.hpp>
#include <FileSystem.hpp>
#include <DirectoryEnum.hpp>
#include <XMLDocument.hpp>
#include <XMLElement.hpp>
#include <XMLPrinter.hpp>
#include <XMLParser.hpp>
#include <ErrorSystem.hpp>
#include <XMLComment.hpp>
#include <Utils.hpp>
#include <ModUtils.hpp>
#include <iomanip>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/classification.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//------------------------------------------------------------------------------

CModBundle ModBundle;

//------------------------------------------------------------------------------

#define _AMS_BUNDLE "_ams_bundle"
#define _AMS_BLDS   "blds"

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CModBundle::CModBundle(void)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CModBundle::GetBundleRoot(const CFileName& path,CFileName& bundle_root)
{
    bundle_root = NULL;
    CFileName tested_path = path;
    while( tested_path != NULL ) {
        CFileName tested_dir = tested_path / _AMS_BUNDLE;
        if( CFileSystem::IsDirectory(tested_dir) == true ){
            bundle_root = tested_path;
            return(true);
        }
        tested_path = tested_path.GetFileDirectory();
    }
    return(false);
}

//------------------------------------------------------------------------------

bool CModBundle::CreateBundle(const CFileName& path,const CFileName& name,
                              const CSmallString& maintainer,const CSmallString& contact)
{
    BundlePath          = path;
    BundleName          = name;
    CFileName           bundle_path = BundlePath / BundleName;

    if( CFileSystem::IsDirectory(bundle_path) == true ){
        CSmallString error;
        error << "unable to create the bundle, the bundle directory '" << bundle_path << " already exists'";
        ES_ERROR(error);
        return(false);
    }

    if( CFileSystem::CreateDir(bundle_path) == false ){
        CSmallString error;
        error << "unable to create the bundle directory '" << bundle_path << "'";
        ES_ERROR(error);
        return(false);
    }

    CFileName           config_path = bundle_path / _AMS_BUNDLE;

    if( CFileSystem::CreateDir(config_path) == false ){
        CSmallString error;
        error << "unable to create the bundle config sub-directory '" << config_path << "'";
        ES_ERROR(error);
        return(false);
    }

    CFileName blds      = config_path / _AMS_BLDS;

    if( CFileSystem::CreateDir(blds) == false ){
        CSmallString error;
        error << "unable to create the bundle blds sub-directory '" << blds << "'";
        ES_ERROR(error);
        return(false);
    }

    CSmallString id   = CUtils::GenerateUUID();

    CXMLElement* p_config = Config.CreateChildElement("bundle");
    p_config->SetAttribute("name",name);
    p_config->SetAttribute("id",id);
    CXMLElement* p_maintainer = p_config->CreateChildElement("maintainer");
    p_maintainer->SetAttribute("name",maintainer);
    p_maintainer->SetAttribute("email",contact);

    CXMLPrinter xml_printer;
    CFileName   config_file = config_path / "config.xml";

    xml_printer.SetPrintedXMLNode(&Config);
    if( xml_printer.Print(config_file) == false ){
        CSmallString error;
        error << "unable to save the bundle configuration '" << config_file << "'";
        ES_ERROR(error);
        return(false);
    }

    AuditAction("created");

    return(true);
}

//------------------------------------------------------------------------------

bool CModBundle::InitBundle(const CFileName& bundle_path)
{

    BundlePath = bundle_path.GetFileDirectory();
    BundleName = bundle_path.GetFileName();

    if( CFileSystem::IsDirectory(bundle_path) == false ){
        CSmallString error;
        error << "bundle must be a directory, but this path is not: '" << bundle_path << "'";
        ES_ERROR(error);
        return(false);
    }

    CFileName config_path = bundle_path / _AMS_BUNDLE ;
    CFileName config_file = config_path / "config.xml";

    CXMLParser xml_parser;
    xml_parser.SetOutputXMLNode(&Config);
    if( xml_parser.Parse(config_file) == false ){
        CSmallString warning;
        warning << "unable to parse bundle config '" << config_file << "'";
        ES_ERROR(warning);
        return(false);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CModBundle::FindAllFragmentFiles(void)
{
    CFileName blds = BundlePath / BundleName / _AMS_BUNDLE / _AMS_BLDS;

    BldFiles.clear();
    CUtils::FindAllFiles(blds,"*.bld",BldFiles);

    DocFiles.clear();
    CUtils::FindAllFiles(blds,"*.doc",DocFiles);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CModBundle::GetID(void) const
{
    CSmallString id;
    CXMLElement* p_ele = Config.GetChildElementByPath("bundle");
    if( p_ele == NULL ) {
        ES_ERROR("unable to open bundle");
        return(id);
    }
    if( p_ele->GetAttribute("id",id) == false ) {
        ES_ERROR("unable to get the bundle ID");
    }
    return(id);
}

//------------------------------------------------------------------------------

const CSmallString CModBundle::GetMaintainerName(void)
{
    CSmallString name;
    CXMLElement* p_ele = Config.GetChildElementByPath("bundle/maintainer");
    if( p_ele == NULL ) {
        ES_ERROR("unable to open bundle/maintainer");
        return(name);
    }
    if( p_ele->GetAttribute("name",name) == false ) {
        ES_ERROR("unable to get the maintainer name");
    }
    return(name);
}

//------------------------------------------------------------------------------

const CSmallString CModBundle::GetMaintainerEMail(void)
{
    CSmallString email;
    CXMLElement* p_ele = Config.GetChildElementByPath("bundle/maintainer");
    if( p_ele == NULL ) {
        ES_ERROR("unable to open bundle/maintainer");
        return(email);
    }
    if( p_ele->GetAttribute("email",email) == false ) {
        ES_ERROR("unable to get the maintainer email");
    }
    return(email);
}

//------------------------------------------------------------------------------

void CModBundle::PrintInfo(CVerboseStr& vout)
{
    vout << "# Bundle" << endl;
    vout << "# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    vout << "# Bundle name : " << BundleName << endl;
    vout << "# Bundle path : " << BundlePath << endl;
    vout << "# Bundle ID   : " << GetID() << endl;
    vout << "# Maintainer  : " << GetMaintainerName() << " (" << GetMaintainerEMail() << ")" << endl;

    vout << "#" << endl;
    vout << "# Statistics" << endl;
    vout << "# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    vout << "# Number of doc files : " << setw(3) << DocFiles.size() << endl;
    vout << "# Number of bld files : " << setw(3) << BldFiles.size() << endl;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CModBundle::RebuildCache(CVerboseStr& vout)
{
    CFileName blds = BundlePath / BundleName / _AMS_BUNDLE / _AMS_BLDS;

// empty cache
    Cache.RemoveAllChildNodes();

    Archs.clear();
    Modes.clear();
    DisabledMods.clear();
    NoDocMods.clear();
    NumOfDocs = 0;
    NumOfBlds = 0;

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

    vout << "# Documentation file                                         Module             " << endl;
    vout << "# ---------------------------------------------------------- -------------------" << endl;

// rebuild docus first
    for(CFileName doc_file : DocFiles) {
        vout << left << setw(60) << doc_file.RelativeTo(blds) << " ";
        if( AddDocumentation(vout,p_cele,doc_file) == false ){
            vout << "<red>FAILED</red>" << endl;
            return(false);
        }
    }
    vout << "# ------------------------------------------------------------------------------" << endl;
    vout << "# Number of doc files : " << setw(3) << NumOfDocs << endl;

    vout << endl;
    vout << "# Build file                                                 Build              " << endl;
    vout << "# ---------------------------------------------------------- -------------------" << endl;

// rebuild builds
    for(CFileName bld_file : BldFiles) {
        vout << left << setw(60) << bld_file.RelativeTo(blds) << " ";
        if( AddBuild(vout,p_cele,bld_file) == false ){
            vout << "<red>FAILED</red>" << endl;
            return(false);
        }
    }
    vout << "# ------------------------------------------------------------------------------" << endl;
    vout << "# Number of bld files    : " << setw(3) << NumOfBlds << endl;

    Archs.sort();
    Archs.unique();

    Modes.sort();
    Modes.unique();

    DisabledMods.sort();
    DisabledMods.unique();

    NoDocMods.sort();
    NoDocMods.unique();

    vout << endl;
    vout << "# Statistics                                                                    " << endl;
    vout << "# ------------------------------------------------------------------------------" << endl;
    vout <<                  "# Number of doc files    : " << setw(3) << NumOfDocs << endl;
    vout <<                  "# Number of bld files    : " << setw(3) << NumOfBlds << endl;
    vout <<                  "# Disabled modules       : " << setw(3) << DisabledMods.size() << endl;
    if( DisabledMods.size() > 0 ) {
    CUtils::PrintTokens(vout,"# Disabled modules       : ",DisabledMods,80,'#');
    }
    vout <<                  "# Non-documented modules : " << setw(3) << NoDocMods.size() << endl;
    if( NoDocMods.size() > 0 ) {
    CUtils::PrintTokens(vout,"# Non-documented modules : ",NoDocMods,80,'#');
    }
    CUtils::PrintTokens(vout,"# Architecture tokens    : ",Archs,80,'#');
    CUtils::PrintTokens(vout,"# Execution mode tokens  : ",Modes,80,'#');

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CModBundle::LoadCache(void)
{
    CFileName config_dir = BundlePath / BundleName / _AMS_BUNDLE;

// load the cache
    if( LoadCacheFile(config_dir / "cache.xml" ) == false ){
        ES_ERROR("unable to load small cache");
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CModBundle::SaveCaches(void)
{
    CFileName config_dir = BundlePath / BundleName / _AMS_BUNDLE;

// save the whole cache
    if( SaveCacheFile(config_dir / "cache_big.xml" ) == false ){
        ES_ERROR("unable to save big cache");
        return(false);
    }

// clean unnecessary parts
    RemoveDocumentation();
    CXMLElement* p_cele = Cache.GetFirstChildElement("cache");
    CleanBuild(p_cele);

// save optimized cache
    if( SaveCacheFile(config_dir / "cache.xml" ) == false ){
        ES_ERROR("unable to save small cache");
        return(false);
    }
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CModBundle::AddDocumentation(CVerboseStr& vout,CXMLElement* p_cele, const CFileName& docu_file)
{
// open documentation file
    CXMLDocument module;
    CXMLParser   xml_parser;

    xml_parser.SetOutputXMLNode(&module);
    xml_parser.EnableWhiteCharacters(true);

    if( xml_parser.Parse(docu_file) == false ) {
        CSmallString error;
        error << "unable to parse moudule documentation file '" << docu_file << "'";
        ES_ERROR(error);
        return(false);
    }

// get basic info
    CXMLElement* p_mele = module.GetFirstChildElement("module");
    if( p_mele == NULL ) {
        CSmallString error;
        error << "unable to open module element for '" << docu_file << "'";
        ES_ERROR(error);
        return(false);
    }

// module name
    CSmallString    modname;
    if( p_mele->GetAttribute("name",modname) == false ) {
        CSmallString error;
        error << "name attribute is missing for '" << docu_file << "'";
        ES_ERROR(error);
        return(false);
    }
    CFileName file_name = docu_file.GetFileNameWithoutExt();
    if( modname != file_name ){
        CSmallString error;
        error << "inconsitency between the file name '" << docu_file << "' and module name '" << modname << "'";
        ES_ERROR(error);
        return(false);
    }

// now check if module is enabled
    bool enabled = true;
    p_mele->GetAttribute("enabled",enabled);

    if( enabled == false ) {;
        vout << modname << " <blue>(disabled - it is not going to be added to the cache)</blue>" << endl;
        CXMLElement* p_dele = p_cele->CreateChildElement("module");
        p_dele->SetAttribute("name",modname);
        p_dele->SetAttribute("enabled",false);
        DisabledMods.push_back(modname);
        return(true);
    }

    // include module to cache
    if( p_mele->DuplicateNode(p_cele) == NULL ) {
        CSmallString error;
        error << "unable to add '" << modname << "' module to the cache";
        ES_ERROR(error);
        return(false);
    }
    vout << modname << endl;
    NumOfDocs++;
    return(true);
}

//------------------------------------------------------------------------------

bool CModBundle::AddBuild(CVerboseStr& vout,CXMLElement* p_cele, const CFileName& build_file)
{
// open build file
    CXMLDocument build;
    CXMLParser   xml_parser;

    xml_parser.SetOutputXMLNode(&build);
    xml_parser.EnableWhiteCharacters(false);

    if( xml_parser.Parse(build_file) == false ) {
        CSmallString error;
        error << "unable to parse moudule build file '" << build_file << "'";
        ES_ERROR(error);
        return(false);
    }

// get basic info
    CXMLElement* p_bele = build.GetFirstChildElement("build");
    if( p_bele == NULL ) {
        CSmallString error;
        error << "unable to open build element for '" << build_file << "'";
        ES_ERROR(error);
        return(false);
    }

// module name
    CSmallString    modname,modver,modarch,modmode;
    if( p_bele->GetAttribute("name",modname) == false ) {
        CSmallString error;
        error << "name attribute is missing for '" << build_file << "'";
        ES_ERROR(error);
        return(false);
    }
    CFileName file_name = build_file.GetFileNameWithoutExt();
    CFileName file_modname = CModUtils::GetModuleName(file_name);
    if( modname != file_modname ){
        CSmallString error;
        error << "inconsitency between the file name '" << build_file << "' and build module name '" << modname << "'";
        ES_ERROR(error);
        return(false);
    }

// key attributes
    p_bele->GetAttribute("ver",modver);
    p_bele->GetAttribute("arch",modarch);
    p_bele->GetAttribute("mode",modmode);

    if( (modver == NULL) || (modarch == NULL) || (modmode == NULL) ){
        CSmallString error;
        error << "key build attributes messing: '" << modname << ":" << modver << ":" << modarch << ":" << modmode << "'";
        ES_ERROR(error);
        return(false);
    }

// find module or create a new in the cache
    CXMLElement* p_mele = GetModule(modname);
    if( p_mele == NULL ){
        p_mele = CreateModule(modname);
        NoDocMods.push_back(modname);
    }

// now check if module is enabled
    bool enabled = true;
    p_mele->GetAttribute("enabled",enabled);

    if( enabled == false ) {
        vout << "    module '" << modname << "' is disabled - build is not added to cache" << endl;
        return(true);
    }

// clean the build
    p_bele->RemoveAttribute("name");
    CleanBuild(p_bele);

// include module build to cache
    if( p_bele->DuplicateNode(p_mele) == NULL ) {
        CSmallString error;
        error << "unable to add '" << build_file << "' module build to the cache";
        ES_ERROR(error);
        return(false);
    }

    vout << modname << ":" << modver << ":" << modarch << ":" << modmode << endl;

    std::string             sarch(modarch);
    std::list<CSmallString> archs;
    split(archs,sarch,is_any_of("#"));

    for(CSmallString arch : archs){
        Archs.push_back(arch);
    }

    Modes.push_back(modmode);

    NumOfBlds++;
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CModBundle::AuditAction(const CSmallString& message)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================