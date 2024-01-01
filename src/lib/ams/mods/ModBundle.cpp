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
#include <Utils.hpp>
#include <ModUtils.hpp>
#include <iomanip>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <PrintEngine.hpp>
#include <FSIndex.hpp>
#include <UserUtils.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//------------------------------------------------------------------------------

#define _AMS_BUNDLE "_ams_bundle"
#define _AMS_BLDS   "blds"

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CModBundle::CModBundle(void)
{
    CacheType               = EMBC_NONE;
    PersonalBundle          = false;

    NumOfAllBuilds          = 0;
    NumOfUniqueBuilds       = 0;
    NumOfNonSoftRepoBuilds  = 0;
    NumOfSharedBuilds       = 0;
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

bool CModBundle::IsBundle(const CFileName& path,const CFileName& name)
{
    CFileName tested_dir = path / name / _AMS_BUNDLE;
    return(CFileSystem::IsDirectory(tested_dir));
}

//------------------------------------------------------------------------------

bool CModBundle::CreateBundle(const CFileName& path,const CFileName& name,
                              const CSmallString& maintainer,const CSmallString& contact,bool force)
{
    BundlePath          = path;
    BundleName          = name;
    CFileName           bundle_path = BundlePath / BundleName;

    if( force == false ){
        if( CFileSystem::IsDirectory(bundle_path) == true ){
            CSmallString error;
            error << "unable to create the bundle, the bundle directory '" << bundle_path << "' already exists";
            ES_ERROR(error);
            return(false);
        }
        if( CFileSystem::CreateDir(bundle_path) == false ){
            CSmallString error;
            error << "unable to create the bundle directory '" << bundle_path << "'";
            ES_ERROR(error);
            return(false);
        }
    } else {
        if( CFileSystem::IsDirectory(bundle_path) == false ){
            if( CFileSystem::CreateDir(bundle_path) == false ){
                CSmallString error;
                error << "unable to create the bundle directory '" << bundle_path << "'";
                ES_ERROR(error);
                return(false);
            }
        }
    }

    CFileName config_path = bundle_path / _AMS_BUNDLE;

    if( CFileSystem::IsDirectory(config_path) == true ){
        CSmallString error;
        error << "unable to create the bundle, the bundle config directory '" << config_path << "' already exists";
        ES_ERROR(error);
        return(false);
    }

    if( CFileSystem::CreateDir(config_path) == false ){
        CSmallString error;
        error << "unable to create the bundle config sub-directory '" << config_path << "'";
        ES_ERROR(error);
        return(false);
    }

    CFileName blds = config_path / _AMS_BLDS;

    if( CFileSystem::IsDirectory(blds) == true ){
        CSmallString error;
        error << "unable to create the bundle, the bundle bld directory '" << blds << "' already exists";
        ES_ERROR(error);
        return(false);
    }

    if( CFileSystem::CreateDir(blds) == false ){
        CSmallString error;
        error << "unable to create the bundle blds sub-directory '" << blds << "'";
        ES_ERROR(error);
        return(false);
    }

    CSmallString id = CUtils::GenerateUUID();

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

// inject path into config
    p_config->SetAttribute("path",BundlePath);

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

    CFileName config_file = BundlePath / BundleName / _AMS_BUNDLE / "config.xml";

// load config
    CXMLParser xml_parser;
    xml_parser.SetOutputXMLNode(&Config);
    if( xml_parser.Parse(config_file) == false ){
        CSmallString warning;
        warning << "unable to parse bundle config '" << config_file << "'";
        ES_ERROR(warning);
        return(false);
    }

// inject path into config
    CXMLElement* p_config = Config.GetChildElementByPath("bundle",true);
    p_config->SetAttribute("path",BundlePath);

// load audit
    CFileName audit_file = BundlePath / BundleName / _AMS_BUNDLE / "audit.xml";

    CXMLParser audit_xml_parser;
    audit_xml_parser.SetOutputXMLNode(&AuditLog);
    if( audit_xml_parser.Parse(audit_file) == false ){
        CSmallString warning;
        warning << "unable to load bundle audit log '" << audit_file << "'";
        ES_ERROR(warning);
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
    BldFiles.sort();

    DocFiles.clear();
    CUtils::FindAllFiles(blds,"*.doc",DocFiles);
    DocFiles.sort();
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

void CModBundle::PrintInfo(CVerboseStr& vout,bool mods,bool stat,bool audit)
{
    vout << "# Bundle" << endl;
    vout << "# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    vout << "# Bundle name : " << BundleName << endl;
    vout << "# Bundle path : " << BundlePath << endl;
    vout << "# Bundle ID   : " << GetID() << endl;
    vout << "# Maintainer  : " << GetMaintainerName() << " (" << GetMaintainerEMail() << ")" << endl;

    if( mods ){
    switch(CacheType) {
    case(EMBC_SMALL):
    vout << "# Cache type  : small" << endl;
        break;
    case(EMBC_BIG):
    vout << "# Cache type  : big" << endl;
        break;
    case(EMBC_NONE):
    vout << "# Cache type  : not loaded" << endl;
        break;
    }
    vout << "# Modules     : " << setw(3) << GetNumberOfModules() << endl;
    vout << "# Categories  : " << setw(3) << GetNumberOfCategories() << endl;
    }

    if( stat ){
    vout << endl;
    vout << "# Statistics" << endl;
    vout << "# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    vout << "# Number of doc files : " << setw(3) << DocFiles.size() << endl;
    vout << "# Number of bld files : " << setw(3) << BldFiles.size() << endl;
    }

    if( audit ){
    vout << endl;
    vout << "# Audit log (last 10 records)" << endl;
    vout << "# Date               User             Message" << endl;
    vout << "# ~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;

    CXMLElement* p_aele  = AuditLog.GetChildElementByPath("audit/entry");
    int count = 0;
    while( p_aele != NULL ){
        count++;
        p_aele = p_aele->GetNextSiblingElement("entry");
    }
    int skip = count - 10;
    p_aele  = AuditLog.GetChildElementByPath("audit/entry");
    while( p_aele != NULL ){
        CSmallString        message, user;
        CSmallTimeAndDate   dt;

        p_aele->GetAttribute("message",message);
        p_aele->GetAttribute("user",user);
        p_aele->GetAttribute("time",dt);

        p_aele = p_aele->GetNextSiblingElement("entry");
        if( skip > 0 ){
            skip--;
        } else {
            vout << setw(20) << left << dt.GetSDateAndTime() << " ";
            vout << setw(16) << left << user << " ";
            vout << message << endl;
        }
    }
    }
}

//------------------------------------------------------------------------------

const CFileName CModBundle::GetFullBundleName(void)
{
    CFileName full_name = BundlePath / BundleName;
    return(full_name);
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

// create empty cache
    CXMLElement* p_cele = CreateEmptyCache();

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

    RebuildModuleDefaultBuilds();

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

    AuditAction("cache rebuilded");

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CModBundle::LoadCache(EModBundleCache type)
{
    CFileName config_dir = BundlePath / BundleName / _AMS_BUNDLE;

    CacheType = EMBC_NONE;

// load the cache
    if( type == EMBC_BIG ){
        if( LoadCacheFile(config_dir / "cache_big.xml") == false ){
            ES_ERROR("unable to load big cache");
            return(false);
        }
        CacheType = EMBC_BIG;
    } else if( type == EMBC_SMALL ) {
        if( LoadCacheFile(config_dir / "cache.xml") == false ){
            ES_ERROR("unable to load small cache");
            return(false);
        }
        CacheType = EMBC_SMALL;
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

    CXMLElement* p_builds = p_mele->GetChildElementByPath("builds",true);

// clean the build
    p_bele->RemoveAttribute("name");
    CleanBuild(p_bele);

// include module build to cache
    if( p_bele->DuplicateNode(p_builds) == NULL ) {
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

//------------------------------------------------------------------------------

void CModBundle::RebuildModuleDefaultBuilds(void)
{
    CXMLElement* p_cele = GetCacheElement();
    if( p_cele == NULL ){
       ES_ERROR("unable to open cache element");
       return;
    }

    CXMLElement* p_mele = p_cele->GetFirstChildElement("module");
    while( p_mele != NULL ){
        InitDefaultBuild(p_mele);
        p_mele = p_mele->GetNextSiblingElement("module");
    }
}

//------------------------------------------------------------------------------

void CModBundle::InitDefaultBuild(CXMLElement* p_mele)
{
    if( p_mele == NULL ){
        ES_ERROR("p_mele is NULL");
        return;
    }

    if( p_mele->GetFirstChildElement("default") != NULL ){
        return; // we already have default build set in module file
    }

    CSmallString defver;
    bool         first = true;
    double       defverindx;

    CXMLElement* p_bele = p_mele->GetChildElementByPath("builds/build");
    while( p_bele != NULL ){
        CSmallString    ver;
        double          verindx;
        bool            result = true;
        result &= p_bele->GetAttribute("ver",ver);
        result &= p_bele->GetAttribute("verindx",verindx);
        if( result ){
            if( first ){
                defver = ver;
                defverindx = verindx;
                first = false;
            }
            if( verindx > defverindx ){
                defver = ver;
                defverindx = verindx;
            }
        }
        p_bele = p_bele->GetNextSiblingElement("build");
    }

    if( first == true ) return; // not found

    CXMLElement* p_def = p_mele->CreateChildElement("default");

    p_def->SetAttribute("ver",defver);
    p_def->SetAttribute("arch","auto");
    p_def->SetAttribute("mode","auto");
}

//------------------------------------------------------------------------------

CXMLElement* CModBundle::GetBundleElement(void)
{
    CXMLElement* p_ele = Config.GetFirstChildElement("bundle");
    // this is optional
    return(p_ele);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CModBundle::AuditAction(const CSmallString& message)
{
    CXMLElement* p_ele  = AuditLog.GetChildElementByPath("audit",true);
    CXMLElement* p_aele = p_ele->CreateChildElement("entry");

    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    p_aele->SetAttribute("message",message);
    p_aele->SetAttribute("user",CUserUtils::GetUserName());
    p_aele->SetAttribute("time",dt);

    CFileName audit_file = BundlePath / BundleName / _AMS_BUNDLE / "audit.xml";

// save config
    CXMLPrinter xml_printer;
    xml_printer.SetPrintedXMLNode(&AuditLog);
    if( xml_printer.Print(audit_file) == false ){
        CSmallString warning;
        warning << "unable to save bundle config '" << audit_file << "'";
        ES_ERROR(warning);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CModBundle::ListBuildsForIndex(CVerboseStr& vout,bool personal)
{
    // create list of builds
    vout << endl;
    vout << "# Assembling list of builds ..." << endl;

    NumOfAllBuilds          = 0;
    NumOfUniqueBuilds       = 0;
    NumOfNonSoftRepoBuilds  = 0;
    NumOfSharedBuilds       = 0;

    UniqueBuilds.clear();
    UniqueBuildPaths.clear();
    NewBundleIndex.Clear();

    PersonalBundle = personal;

    CXMLElement* p_mele = Cache.GetChildElementByPath("cache/module");

    while( p_mele != NULL ) {
        CSmallString name;
        p_mele->GetAttribute("name",name);
        CXMLElement* p_bele = p_mele->GetChildElementByPath("builds/build");

        while( p_bele != NULL ) {
            CXMLElement* p_sele = p_bele;
            p_bele = p_bele->GetNextSiblingElement("build");

            NumOfAllBuilds++;

            CSmallString ver,arch,mode;
            p_sele->GetAttribute("ver",ver);
            p_sele->GetAttribute("arch",arch);
            p_sele->GetAttribute("mode",mode);

            // register build
            CSmallString build_id;
            build_id << name << ":" << ver << ":" << arch << ":" << mode;

            if( UniqueBuilds.count(build_id) == 1 ) continue;
            UniqueBuilds.insert(build_id);
            NumOfUniqueBuilds++;

            CFileName package_dir;
            package_dir = CModCache::GetVariableValue(p_sele,"AMS_PACKAGE_DIR");
            if( package_dir == NULL ){
                NumOfNonSoftRepoBuilds++;
                continue;
            }

            CFileName path;
            if( package_dir[0] == '/' ){
                path = package_dir;
            } else {
                path = BundlePath / package_dir;
            }

            if( ! PersonalBundle ){
                // ignore this test for personal site as the build might not be synchronized yet
                if( CFileSystem::IsDirectory(path) == false ){
                    CSmallString error;
                    error << build_id << " -> AMS_PACKAGE_DIR: " << package_dir << " does not exist!";
                    ES_ERROR(error);
                    return(false);
                }
            }

            // register build
            if( UniqueBuildPaths.count(path) == 1 ){
                // already registered
                NumOfSharedBuilds++;
                continue;
            }
            UniqueBuildPaths.insert(path);

            // register build for index
            NewBundleIndex.Paths[build_id] = package_dir;
        }
        p_mele = p_mele->GetNextSiblingElement("module");
    }

    vout << "  > Number of module builds                              = " << NumOfAllBuilds << endl;
    vout << "  > Number of unique builds                              = " << NumOfUniqueBuilds << endl;
    vout << "  > Number of builds (no AMS_PACKAGE_DIR)                = " << NumOfNonSoftRepoBuilds << endl;
    vout << "  > Number of shared builds (the same AMS_PACKAGE_DIR)   = " << NumOfSharedBuilds << endl;
    vout << "  > Number of builds for index (AMS_PACKAGE_DIR and dir) = " << NewBundleIndex.Paths.size() << endl;

    return(true);
}

//------------------------------------------------------------------------------

void CModBundle::CalculateNewIndex(CVerboseStr& vout)
{
    // calculate index
    vout << endl;
    vout << "# Calculating index ..." << endl;

    map<CSmallString,CFileName>::iterator it = NewBundleIndex.Paths.begin();
    map<CSmallString,CFileName>::iterator ie = NewBundleIndex.Paths.end();

    CFSIndex index;
    index.RootDir = BundlePath;
    index.PersonalBundle = PersonalBundle;

    while( it != ie ){
        CSmallString    build_id    = it->first;
        CFileName       build_path  = it->second;
        string sha1 = index.CalculateBuildHash(build_path);
        NewBundleIndex.Hashes[build_id] = sha1;
        vout << sha1 << " " << build_id << endl;
        it++;
    }

    vout << endl;
    vout << "# Statistics ..." << endl;
    vout << "  > Number of stat objects  = " << index.NumOfStats << endl;

    AuditAction("new index");
}

//------------------------------------------------------------------------------

bool CModBundle::SaveNewIndex(void)
{
    CFileName index_name;
    index_name = BundlePath / BundleName / _AMS_BUNDLE / "index.new";

    return(NewBundleIndex.SaveIndex(index_name));
}

//------------------------------------------------------------------------------

bool CModBundle::CommitNewIndex(void)
{
    CFileName new_index_name;
    new_index_name = BundlePath / BundleName / _AMS_BUNDLE / "index.new";

    CFileName old_index_name;
    old_index_name = BundlePath / BundleName / _AMS_BUNDLE / "index.old";

    if( CFileSystem::CopyFile(new_index_name,old_index_name,true) == false ){
        CSmallString error;
        error << "unable to copy the new index '" << new_index_name;
        error << "' over old index '" << old_index_name << "'";
        ES_ERROR(error);
        return(false);
    }

    AuditAction("index commited");

    return(true);
}

//------------------------------------------------------------------------------

bool CModBundle::LoadIndexes(void)
{
    CFileName new_index_name;
    new_index_name = BundlePath / BundleName / _AMS_BUNDLE / "index.new";

    bool result = true;
    if( CFileSystem::IsFile(new_index_name) ){
        result &= NewBundleIndex.LoadIndex(new_index_name);
    }

    CFileName old_index_name;
    old_index_name = BundlePath / BundleName / _AMS_BUNDLE / "index.old";
    if( CFileSystem::IsFile(old_index_name) ){
        result &= OldBundleIndex.LoadIndex(old_index_name);
    }

    return(result);
}

//------------------------------------------------------------------------------

void CModBundle::DiffIndexes(CVerboseStr& vout, bool skip_removed, bool skip_added)
{
    NewBundleIndex.Diff(OldBundleIndex,vout,skip_removed,skip_added);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CModBundleIndex::LoadIndex(const CFileName& index_name)
{
    ifstream ifs(index_name);
    if( ! ifs ){
        CSmallString error;
        error << "Unable to open the index file '" << index_name << "'";
        ES_ERROR(error);
        return(false);
    }

    string  line;
    int     lino = 0;
    while( getline(ifs,line) ){
        lino++;
        string flag,sha1,build,path;
        stringstream str(line);
        str >> flag >> sha1 >> build >> path;
        if( ! str ){
            CSmallString error;
            error << "Corrupted index file '" << index_name << "' at line " << lino;
            ES_ERROR(error);
            return(false);
        }

        if( Hashes.count(build) == 1 ){
            CSmallString error;
            error << "SHA1 collision in index file '" << index_name << "' at line " << lino;
            ES_ERROR(error);
            return(false);
        }
        // flag is ignored, set only sha1,build,path
        Hashes[build] = sha1;
        Paths[build] = path;
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CModBundleIndex::SaveIndex(const CFileName& index_name)
{
    ofstream ofs(index_name);
    if( ! ofs ){
        CSmallString error;
        error << "unable to open the index file '" << index_name << "' for writing!";
        ES_ERROR(error);
        return(false);
    }

    map<CSmallString,CFileName>::iterator it = Paths.begin();
    map<CSmallString,CFileName>::iterator ie = Paths.end();

    while( it != ie ){
        CSmallString build_id = it->first;
        string sha1 = Hashes[build_id];
        ofs << "* " << sha1 << " " << build_id << " " << it->second << endl;
        it++;
    }

    if( ! ofs ){
        ES_ERROR("The index was not saved due to error!");
        return(false);
    }

    ofs.close();
    return(true);
}

//------------------------------------------------------------------------------

void CModBundleIndex::Diff(CModBundleIndex& old_index, CVerboseStr& vout, bool skip_removed, bool skip_added)
{
    vout << endl;
    vout << "# Diffing two indexes ..." << endl;

    vout << low;

    map<CSmallString,string>::iterator it;
    map<CSmallString,string>::iterator ie;

    if( skip_removed == false ){

        // determine removed entries (-)
        it = old_index.Hashes.begin();
        ie = old_index.Hashes.end();

        while( it != ie ){
            CSmallString build = it->first;
            if( Hashes.count(build) == 0 ){
                vout << "- " << old_index.Hashes[build] << " " << build << " " << old_index.Paths[build] <<  endl;
            }
            it++;
        }
    }

    // determine new entries (+) or modified (M)
    it = Hashes.begin();
    ie = Hashes.end();

    while( it != ie ){
        CSmallString build = it->first;
        if( old_index.Hashes.count(build) == 0 ){
            if( skip_added == false ){
                vout << "+ " << Hashes[build] << " " << build << " " << Paths[build] <<  endl;
            }
        } else {
            if( Hashes[build] != old_index.Hashes[build] ){
                vout << "M " << Hashes[build] << " " << build << " " << Paths[build] <<  endl;
            }
        }
        it++;
    }
}

//------------------------------------------------------------------------------

void CModBundleIndex::Clear(void)
{
    Paths.clear();
    Hashes.clear();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
