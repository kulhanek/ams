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

#include <Cache.hpp>
#include <FileName.hpp>
#include <string.h>
#include <dirent.h>
#include <XMLParser.hpp>
#include <XMLPrinter.hpp>
#include <XMLElement.hpp>
#include <XMLComment.hpp>
#include <XMLIterator.hpp>
#include <ErrorSystem.hpp>
#include <AMSGlobalConfig.hpp>
#include <DirectoryEnum.hpp>
#include <FileSystem.hpp>
#include <Shell.hpp>
#include <Utils.hpp>
#include <XMLText.hpp>
#include <fnmatch.h>
#include <iomanip>
#include <list>
#include <User.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//------------------------------------------------------------------------------

CCache Cache;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CCache::CCache(void)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CCache::LoadCache(bool loadbig)
{
    Cache.RemoveAllChildNodes();

    if( AMSGlobalConfig.GetActiveSiteID() == NULL ) {
        ES_ERROR("no site is active");
        return(false);
    }

    CFileName    config_path;

    config_path = AMSGlobalConfig.GetETCDIR();
    if( loadbig ){
        config_path = config_path / "sites" / AMSGlobalConfig.GetActiveSiteID() / "cache_big.xml";
    } else {
        config_path = config_path / "sites" / AMSGlobalConfig.GetActiveSiteID() / "cache.xml";
    }

    if( CFileSystem::IsFile(config_path) ) {

        CXMLParser xml_parser;
        xml_parser.SetOutputXMLNode(&Cache);
        xml_parser.EnableWhiteCharacters(loadbig);

        if( xml_parser.Parse(config_path) == false ) {
            CSmallString error;
            if( loadbig ){
                error << "unable to load site '" << AMSGlobalConfig.GetActiveSiteID() << "' cache_big";
            } else {
                error << "unable to load site '" << AMSGlobalConfig.GetActiveSiteID() << "' cache";
            }
            ES_ERROR(error);
            return(false);
        }
    } else {
        Cache.CreateChildElement("cache");
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CCache::LoadCache(const CSmallString& site_sid,bool loadbig)
{
    Cache.RemoveAllChildNodes();

    CFileName    config_path;

    config_path = AMSGlobalConfig.GetETCDIR();
    if( loadbig ){
        config_path = config_path / "sites" / site_sid / "cache_big.xml";
    } else {
        config_path = config_path / "sites" / site_sid / "cache.xml";
    }

    if( CFileSystem::IsFile(config_path) ) {

        CXMLParser xml_parser;
        xml_parser.SetOutputXMLNode(&Cache);
        xml_parser.EnableWhiteCharacters(loadbig);

        if( xml_parser.Parse(config_path) == false ) {
            CSmallString error;
            if( loadbig ){
                error << "unable to load site '" << site_sid << "' cache_big";
            } else {
                error << "unable to load site '" << site_sid << "' cache";
            }
            ES_ERROR(error);
            return(false);
        }
    } else {
        Cache.CreateChildElement("cache");
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CCache::SaveCache(bool savebig)
{
    if( AMSGlobalConfig.GetActiveSiteID() == NULL ) {
        ES_ERROR("no site is active");
        return(false);
    }

    CFileName    config_path;

    config_path = AMSGlobalConfig.GetETCDIR();
    if( savebig ){
        config_path = config_path / "sites" / AMSGlobalConfig.GetActiveSiteID() / "cache_big.xml";
    } else {
        config_path = config_path / "sites" / AMSGlobalConfig.GetActiveSiteID() / "cache.xml";
    }

    CXMLPrinter xml_printer;

    xml_printer.SetPrintedXMLNode(&Cache);
    xml_printer.SetPrintAsItIs(true);

    if( xml_printer.Print(config_path) == false ) {
        CSmallString error;
        if( savebig ){
            error << "unable to save site '" << AMSGlobalConfig.GetActiveSiteID() << "' cache_big";
        } else {
            error << "unable to save site '" << AMSGlobalConfig.GetActiveSiteID() << "' cache";
        }
        ES_ERROR(error);
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

void CCache::ClearCache(void)
{
    Cache.RemoveAllChildNodes();
    Cache.CreateChildElement("cache");
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CCache::CheckCacheSyntax(CVerboseStr& vout)
{
    CXMLElement* p_sele = Cache.GetFirstChildElement("cache");
    if( p_sele == NULL ){
        RUNTIME_ERROR("unable to find cache element");
    }

    int record_len = GetSizeOfLongestModuleSpecification();

    CXMLIterator    I(p_sele);
    CXMLElement*     p_mele;

    while( (p_mele = I.GetNextChildElement("module")) != NULL  ) {
        CSmallString name;
        p_mele->GetAttribute("name",name);

        vout << left << setw(record_len) << name << " ";
        if( CheckModuleSyntax(vout,p_mele) == true ) {
            vout << "<green>[OK]</green>" << endl;
        } else {
            return(false);
        }
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CCache::RebuildCache(CVerboseStr& vout,bool as_it_is)
{
    if( AMSGlobalConfig.GetActiveSiteID() == NULL ) {
        ES_ERROR("no site is active");
        return(false);
    }

    // empty cache ---------------------------------
    Cache.RemoveAllChildNodes();

    // create header elements ----------------------
    Cache.CreateChildDeclaration();
    CXMLComment* p_comment = Cache.CreateChildComment();
    CSmallString comment;
    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();
    comment << "Advanced Module System (AMS) cache built at " << dt.GetSDateAndTime();
    p_comment->SetComment(comment);
    if( as_it_is ) Cache.CreateChildText("\n",true);

    p_comment = Cache.CreateChildComment();
    comment = "";
    comment << "please do not edit this file (it is created by ams-cache command)";
    p_comment->SetComment(comment);
    if( as_it_is ) Cache.CreateChildText("\n",true);

    // create root element -------------------------
    CXMLElement* p_mele = Cache.CreateChildElement("cache");
    if( p_mele == NULL ) {
        ES_ERROR("unable to create cache element");
        return(false);
    }
    if( as_it_is ) p_mele->CreateChildText("\n",true);

    // get list of module files --------------------
    CFileName    modules_path;

    modules_path = AMSGlobalConfig.GetETCDIR();
    modules_path = modules_path / "sites" / AMSGlobalConfig.GetActiveSiteID() / "modules";

    bool result = RebuildCacheFromDirectory(vout,p_mele,modules_path,as_it_is);

    return(result);
}

//------------------------------------------------------------------------------

void CCache::RemoveDocumentation(void)
{
    CXMLElement* p_mele;

    p_mele = Cache.GetFirstChildElement("cache");
    if( p_mele == NULL ){
        RUNTIME_ERROR("unable to find cache element");
    }

    CXMLIterator    I(p_mele);
    CXMLElement*    p_ele;

    while( (p_ele = I.GetNextChildElement("module")) != NULL ) {
        CXMLElement*    p_rels = p_ele->GetFirstChildElement("documentation");
        if( p_rels != NULL ) delete p_rels;
    }
    
}

//------------------------------------------------------------------------------

bool CCache::SplitCache(CVerboseStr& vout)
{
    CXMLElement* p_mele;

    p_mele = Cache.GetFirstChildElement("cache");
    if( p_mele == NULL ){
        RUNTIME_ERROR("unable to find cache element");
    }

    CXMLIterator    I(p_mele);
    CXMLElement*    p_ele;

    while( (p_ele = I.GetNextChildElement("module")) != NULL ) {
        CSmallString lname;
        if( p_ele->GetAttribute("name",lname) == false ) {
            ES_ERROR("unable to get name attribute");
            return(false);
        }
        // output file name --------------------------
        CFileName file_name;
        file_name = lname + ".mod";

        vout << "  extracting '" << file_name << "' ..." << endl;

        // duplicate module data ---------------------
        CXMLDocument xml_doc;

        xml_doc.CreateChildDeclaration();
        CXMLComment* p_com = xml_doc.CreateChildComment();
        if( p_com != NULL ) {
            p_com->SetComment("Advanced Module System (AMS) module file");
        }

        if( p_ele->DuplicateNode(&xml_doc) == NULL ) {
            ES_ERROR("unable to duplicate node");
            return(false);
        }

        // print module file -------------------------
        CXMLPrinter xml_printer;
        xml_printer.SetPrintedXMLNode(&xml_doc);

        if( xml_printer.Print(file_name) == false ) {
            ES_ERROR("unable to save module file");
            return(false);
        }
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CCache::SplitMoreCache(CVerboseStr& vout)
{
    CXMLElement* p_mele;

    p_mele = Cache.GetFirstChildElement("cache");
    if( p_mele == NULL ){
        RUNTIME_ERROR("unable to find cache element");
    }

    CXMLIterator    I(p_mele);
    CXMLElement*    p_ele;

    while( (p_ele = I.GetNextChildElement("module")) != NULL ) {
        CSmallString lname;
        if( p_ele->GetAttribute("name",lname) == false ) {
            ES_ERROR("unable to get name attribute");
            return(false);
        }

        CXMLElement*    p_rels = p_ele->GetFirstChildElement("builds");
        CXMLIterator    R(p_rels);
        CXMLElement*    p_rele;

        while( (p_rele = R.GetNextChildElement("build")) != NULL ) {
            CSmallString lver;
            CSmallString larch;
            CSmallString lmode;
            bool result = true;
            result &= p_rele->GetAttribute("ver",lver);
            result &= p_rele->GetAttribute("arch",larch);
            result &= p_rele->GetAttribute("mode",lmode);
            if( result == false ) {
                ES_ERROR("some attribute is missing");
                return(false);
            }

            // complete build name
            CSmallString complete_name;
            complete_name = lname + ":" + lver + ":" + larch + ":" + lmode + ".rel";

            // output file name --------------------------
            CFileName file_name;
            file_name = complete_name;

            vout << "  extracting '" << file_name << "' ..." << endl;

            // duplicate module data ---------------------
            CXMLDocument xml_doc;

            xml_doc.CreateChildDeclaration();
            CXMLComment* p_com = xml_doc.CreateChildComment();
            if( p_com != NULL ) {
                p_com->SetComment("Advanced Module System (AMS) build file");
            }

            CXMLElement* p_dupl;
            if( (p_dupl = dynamic_cast<CXMLElement*>(p_rele->DuplicateNode(&xml_doc))) == NULL ) {
                ES_ERROR("unable to duplicate build");
                return(false);
            }

            // put extra attribute required during reconstruction
            p_dupl->SetAttribute("name",lname);

            // print module file -------------------------
            CXMLPrinter xml_printer;
            xml_printer.SetPrintedXMLNode(&xml_doc);

            if( xml_printer.Print(file_name) == false ) {
                ES_ERROR("unable to save build file");
                return(false);
            }

        }
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CCache::RebuildCacheFromDirectory(CVerboseStr& vout, CXMLElement* p_mele,
                        const CFileName& module_dir, bool as_it_is)
{
    CDirectoryEnum dir_enum;

    if( dir_enum.SetDirName(module_dir) == false ) {
        ES_ERROR("unable to set directory");
        return(false);
    }

    if( dir_enum.StartFindFile("*") == false ) {
        ES_ERROR("unable to start directory enumeration");
        return(false);
    }

    CFileName name;
    while( dir_enum.FindFile(name) ) {
        if( name.GetFileNameExt() == ".mod" ) {
            vout << "  importing file '" << name << "' ..." << endl;
            if( AddModule(vout,module_dir,name,p_mele,as_it_is) == false ) return(false);
        } else {
            CFileName sub_dir = module_dir / name;
            if( (name != ".") && (name != "..") && CFileSystem::IsDirectory(sub_dir)  ) {
                if( RebuildCacheFromDirectory(vout,p_mele,sub_dir,as_it_is) == false ) return(false);
            }
        }
    }

    dir_enum.EndFindFile();

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CCache::AddModule(CVerboseStr& vout,const CFileName& module_dir,const CFileName& name,
                CXMLElement* p_cele,bool as_it_is)
{

    CFileName full_name;
    full_name = module_dir / name;

    // open module file ----------------------------
    CXMLDocument modfile;
    CXMLParser   xml_parser;

    xml_parser.SetOutputXMLNode(&modfile);
    xml_parser.EnableWhiteCharacters(as_it_is);

    if( xml_parser.Parse(full_name) == false ) {
        CSmallString error;
        error << "unable to parse file '" << full_name << "'";
        ES_ERROR(error);
        return(false);
    }

    // now check if module is enabled

    CXMLElement* p_mele = modfile.GetFirstChildElement("module");
    if( p_mele == NULL ) {
        CSmallString error;
        error << "unable to open module element for '" << full_name << "'";
        ES_ERROR(error);
        return(false);
    }

    bool            enabled = true;
    CSmallString    modname;

    if( p_mele->GetAttribute("name",modname) == false ) {
        CSmallString error;
        error << "unable to name attribute is missing for '" << full_name << "'";
        ES_ERROR(error);
        return(false);
    }

    p_mele->GetAttribute("enabled",enabled);

    if( enabled == false ) {
        vout << "    module '" << modname << "' is disabled - it is not added to cache" << endl;
        return(true);
    }

    // include module to cache
    if( p_mele->DuplicateNode(p_cele) == NULL ) {
        CSmallString error;
        error << "unable to add '" << full_name << "' module to the cache";
        ES_ERROR(error);
        return(false);
    }
    if( as_it_is ) p_cele->CreateChildText("\n",true);
    vout << "    module '" << modname << "' is added to cache" << endl;

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CCache::CheckModuleSyntax(CVerboseStr& vout,CXMLElement* p_module)
{
    if( p_module == NULL ) return(false);

    // check attributes of module element
    CSmallString attname;
    CXMLIterator    I(p_module);

    while( I.GetNextAttributeName(attname) == true ) {
        bool result = false;
        if( attname == "name" ) {
            CSmallString value;
            p_module->GetAttribute("name",value);
            if( value != NULL ) result = true;
        }
        if( attname == "enabled" ) {
            CSmallString value;
            p_module->GetAttribute("enabled",value);
            if( (value == "true") || (value == "false") ) result = true;
        }
        if( attname == "export" ) {
            CSmallString value;
            p_module->GetAttribute("export",value);
            if( (value == "true") || (value == "false") ) result = true;
        }

        if( result == false ) {
            vout << endl << endl;
            vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'module' element!" << endl;
            vout << "            '" << attname << "' is unsupported attribute, its value is empty or does not contain valid value." << endl;
            vout << "            Allowed attributes are: name, enabled, export." << endl;
            vout << endl;
            return(false);
        }
    }

    CXMLElement* p_sele = Cache.GetFirstChildElement("cache");

    // count for duplicit module records
    CXMLIterator   C(p_sele);
    CXMLElement*   p_cmod;
    CSmallString   cname;

    p_module->GetAttribute("name",cname);

    int count = 0;
    while( (p_cmod = C.GetNextChildElement("module")) != NULL ) {
        CSmallString lname;
        p_cmod->GetAttribute("name",lname);
        if( lname == cname ) count++;
    }

    if( count > 1 ) {
        vout << endl << endl;
        vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'module' element!" << endl;
        vout << "            Duplicit module was detected." << endl;
        vout << endl;
        return(false);
    }

    bool doc_available = false;

    // subblocks
    CXMLElement* p_mele;
    while( (p_mele = I.GetNextChildElement()) != NULL ) {
        bool result = false;
        if( p_mele->GetName() == "builds" ) {
            if( CheckModuleBuildsSyntax(vout,p_mele) == false ) return(false);
            result = true;
        }
        if( p_mele->GetName() == "deps" ) {
            if( CheckModuleDepsSyntax(vout,p_mele) == false ) return(false);
            result = true;
        }
        if( p_mele->GetName() == "default" ) {
            if( CheckModuleDefaultSyntax(vout,p_mele) == false ) return(false);
            result = true;
        }
        if( p_mele->GetName() == "doc" ) {
            if( CheckModuleDocSyntax(vout,p_module,p_mele) == false ) return(false);
            result = true;
            doc_available = true;
        }
        if( p_mele->GetName() == "acl" ) {
            if( CheckModuleACLSyntax(vout,p_mele) == false ) return(false);
            result = true;
        }
        if( result == false ) {
            vout << endl << endl;
            vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'module' element!" << endl;
            vout << "            Unsupported subelement - '" << p_mele->GetName() << "'" << endl;
            vout << "            Allowed subelements are: builds, deps, doc, acl, default." << endl;
            vout << endl;
            return(false);
        }
    }

    if( I.GetNumberOfChildElements("deps") > 1 ) {
        vout << endl << endl;
        vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'module' element!" << endl;
        vout << "            Only one 'deps' element is allowed in 'module' element." << endl;
        vout << endl;
        return(false);
    }

    if( I.GetNumberOfChildElements("doc") > 1 ) {
        vout << endl << endl;
        vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'module' element!" << endl;
        vout << "            Only one 'doc' element is allowed in 'module' element." << endl;
        vout << endl;
        return(false);
    }

    if( I.GetNumberOfChildElements("default") > 1 ) {
        vout << endl << endl;
        vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'module' element!" << endl;
        vout << "            Only one 'default' element is allowed in 'module' element." << endl;
        vout << endl;
        return(false);
    }

    // not used anymore as error
    if( I.GetNumberOfChildElements("default") != 1 ) {
        CSmallString warning;
        CSmallString name;
        p_module->GetAttribute("name",name);
        warning << "module " << name << " does not have default element";
        ES_WARNING(warning);
        return(true);
    }

    if( doc_available == false ){
        vout << "<blue>>>> WARNING:</blue> 'doc' element is missing! ";
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CCache::CheckModuleDocSyntax(CVerboseStr& vout,CXMLElement* p_module,CXMLElement* p_doc)
{
    if( p_doc == NULL ) return(false);

    if( p_doc->NumOfAttributes() != 0 ) {
        vout << endl << endl;
        vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'doc' element!" << endl;
        vout << "            No attributes are permitted but '" << p_doc->NumOfAttributes() << "' was/were found." << endl;
        vout << endl;
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CCache::CheckModuleACLSyntax(CVerboseStr& vout,CXMLElement* p_builds)
{
    if( p_builds == NULL ) return(false);

    // test attributes
    CXMLIterator    I(p_builds);
    CSmallString    attname;

    while( I.GetNextAttributeName(attname) == true ) {
        bool result = false;
        if( attname == "default" ) {
            CSmallString value;
            p_builds->GetAttribute("default",value);
            result = (value == "deny") || (value == "allow") ;
        }

        if( result == false ) {
            vout << endl << endl;
            vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'acl' element!" << endl;
            vout << "            '" << attname << "' is unsupported attribute, its value is empty or does not contain valid value." << endl;
            vout << "            Allowed attributes are: default." << endl;
            vout << endl;
            return(false);
        }
    }


     // test subblocks
    CXMLElement* p_mele;
    while( (p_mele = I.GetNextChildElement()) != NULL ) {
        bool result = false;
        if( p_mele->GetName() == "allow" ) {
            if( p_mele->NumOfAttributes() == 1 ){
                CSmallString group;
                if( p_mele->GetAttribute("group",group) == false ){
                    vout << endl << endl;
                    vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'allow' element!" << endl;
                    vout << "            Illegal attribute name or its value!" << endl;
                    vout << "            Allowed attributes are: group." << endl;
                    vout << endl;
                    return(false);
                }
            } else {
                vout << endl << endl;
                vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'allow' element!" << endl;
                vout << "            Only one attribute is supported!" << endl;
                vout << "            Allowed attributes are: group." << endl;
                vout << endl;
                return(false);
            }
            result = true;
        }
        if( p_mele->GetName() == "deny" ) {
            if( p_mele->NumOfAttributes() == 1 ){
                CSmallString group;
                if( p_mele->GetAttribute("group",group) == false ){
                    vout << endl << endl;
                    vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'deny' element!" << endl;
                    vout << "            Illegal attribute name or its value!" << endl;
                    vout << "            Allowed attributes are: group." << endl;
                    vout << endl;
                    return(false);
                }
            } else {
                vout << endl << endl;
                vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'deny' element!" << endl;
                vout << "            Only one attribute is supported!" << endl;
                vout << "            Allowed attributes are: group." << endl;
                vout << endl;
                return(false);
            }
            result = true;
        }
        if( result == false ) {
            vout << endl << endl;
            vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'version' element!" << endl;
            vout << "            Unsupported subelement - '" << p_mele->GetName() << "'" << endl;
            vout << "            Allowed subelements are: allow, deny." << endl;
            vout << endl;
            return(false);
        }
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CCache::CheckModuleBuildsSyntax(CVerboseStr& vout,CXMLElement* p_builds)
{
    if( p_builds == NULL ) return(false);

    if( p_builds->NumOfAttributes() != 0 ) {
        vout << endl << endl;
        vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'builds' element!" << endl;
        vout << "            No attributes are permitted but '" << p_builds->NumOfAttributes() << "' was/were found." << endl;
        vout << endl;
        return(false);
    }

    // check for duplicit build
    CXMLIterator R(p_builds);
    CXMLElement* p_rele;

    while( (p_rele=R.GetNextChildElement("build")) != NULL) {
        if( CheckModuleBuildSyntax(vout,p_rele) == false ) {
            return(false);
        }
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CCache::CheckModuleBuildSyntax(CVerboseStr& vout,CXMLElement* p_build)
{
    if( p_build == NULL ) return(false);

    // check attributes of build element
    CSmallString     attname;
    CXMLIterator    I(p_build);
    int             mandatory = 0;

    while( I.GetNextAttributeName(attname) == true ) {
        bool result = false;

        if( attname == "ver" ) {
            CSmallString value;
            p_build->GetAttribute("ver",value);
            if( value != NULL ) result = true;
            mandatory++;
        }
        if( attname == "arch" ) {
            CSmallString value;
            p_build->GetAttribute("arch",value);
            if( value != NULL ) result = true;
            mandatory++;
        }
        if( attname == "mode" ) {
            CSmallString value;
            p_build->GetAttribute("mode",value);
            if( value != NULL ) result = true;
            mandatory++;
        }
        if( attname == "prefix" ) {
            CSmallString value;
            p_build->GetAttribute("prefix",value);
            if( value != NULL ) result = true;
            mandatory++;
        }
        if( attname == "verindx" ) {
            double value;
            result = p_build->GetAttribute("verindx",value) == true;
        }
        if( attname == "exclude_node" ) {
            CSmallString value;
            p_build->GetAttribute("exclude_node",value);
            if( value != NULL ) result = true;
        }

        if( result == false ) {
            vout << endl << endl;
            vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'build' element!" << endl;
            vout << "            '" << attname << "' is unsupported attribute or its value is empty." << endl;
            vout << "            Allowed attributes are: ver, arch, mode, prefix, verindx, exclude_node." << endl;
            vout << endl;
            return(false);
        }
    }

    if( mandatory != 4 ) {
        vout << endl << endl;
        vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'build' element!" << endl;
        vout << "            Some mandatory attribute is missing, three requested but '" << mandatory << "' provided." << endl;
        vout << "            Mandatory attributes are: ver, arch, mode, prefix." << endl;
        vout << endl;
        return(false);
    }

    CSmallString ver;
    CSmallString arch;
    CSmallString mode;

    p_build->GetAttribute("ver",ver);
    p_build->GetAttribute("arch",arch);
    p_build->GetAttribute("mode",mode);

    // check for duplicit build
    CXMLIterator R(dynamic_cast<CXMLElement*>(p_build->GetParentNode()));
    CXMLElement* p_rele;

    int count = 0;

    while( (p_rele=R.GetNextChildElement("build")) != NULL) {
        CSmallString lver;
        CSmallString larch;
        CSmallString lmode;

        p_rele->GetAttribute("ver",lver);
        p_rele->GetAttribute("arch",larch);
        p_rele->GetAttribute("mode",lmode);

        if( (lver == ver) && (larch == arch) && (lmode == mode) ) count++;
    }

    if( count > 1 ) {
        vout << endl << endl;
        vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'build' element!" << endl;
        vout << "            Duplicit build was detected." << endl;
        vout << endl;
        return(false);
    }

    CXMLElement* p_mele;

    while( (p_mele = I.GetNextChildElement()) != NULL ) {
        bool result = false;
        if( p_mele->GetName() == "setup" ) {
            if( CheckModuleSetupSyntax(vout,p_mele) == false ) return(false);
            result = true;
        }
        if( p_mele->GetName() == "deps" ) {
            if( CheckModuleDepsSyntax(vout,p_mele) == false ) return(false);
            result = true;
        }
        if( p_mele->GetName() == "acl" ) {
            if( CheckModuleACLSyntax(vout,p_mele) == false ) return(false);
            result = true;
        }
        if( result == false ) {
            vout << endl << endl;
            vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'build' element!" << endl;
            vout << "            Unsupported subelement - '" << p_mele->GetName() << "'." << endl;
            vout << "            Allowed subelements are: setup, deps, acl." << endl;
            vout << endl;
            return(false);
        }
    }

    if( I.GetNumberOfChildElements("setup") > 1 ) {
        vout << endl << endl;
        vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'build' element!" << endl;
        vout << "            Only one 'setup' element is allowed in 'build' element." << endl;
        vout << endl;
        return(false);
    }

    if( I.GetNumberOfChildElements("deps") > 1 ) {
        vout << endl << endl;
        vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'build' element!" << endl;
        vout << "            Only one 'deps' element is allowed in 'build' element." << endl;
        vout << endl;
        return(false);
    }

    if( I.GetNumberOfChildElements("access") > 1 ) {
        vout << endl << endl;
        vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'build' element!" << endl;
        vout << "            Only one 'access' element is allowed in 'build' element." << endl;
        vout << endl;
        return(false);
    }

    if( I.GetNumberOfChildElements("documentation") > 1 ) {
        vout << endl << endl;
        vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'build' element!" << endl;
        vout << "            Only one 'documentation' element is allowed in 'build' element." << endl;
        vout << endl;
        return(false);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CCache::CheckModuleSetupSyntax(CVerboseStr& vout,CXMLElement* p_setup)
{
    if( p_setup == NULL ) return(false);

    if( p_setup->NumOfAttributes() != 0 ) {
        vout << endl << endl;
        vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'setup' element!" << endl;
        vout << "            No attributes are permitted but '" << p_setup->NumOfAttributes() << "' was/were found." << endl;
        vout << endl;
        return(false);
    }

    CXMLIterator    I(p_setup);

    CXMLElement* p_mele;

    while( (p_mele = I.GetNextChildElement()) != NULL ) {
        bool result = false;
        if( p_mele->GetName() == "variable" ) {
            if( CheckModuleVariableSyntax(vout,p_mele) == false ) return(false);
            result = true;
        }
        if( p_mele->GetName() == "script" ) {
            if( CheckModuleScriptSyntax(vout,p_mele) == false ) return(false);
            result = true;
        }
        if( p_mele->GetName() == "alias" ) {
            if( CheckModuleAliasSyntax(vout,p_mele) == false ) return(false);
            result = true;
        }

        if( result == false ) {
            vout << endl << endl;
            vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'build' element!" << endl;
            vout << "            Unsupported subelement - '" << p_mele->GetName() << "'." << endl;
            vout << "            Allowed subelements are: variable, script, alias." << endl;
            vout << endl;
            return(false);
        }
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CCache::CheckModuleVariableSyntax(CVerboseStr& vout,CXMLElement* p_variable)
{
    if( p_variable == NULL ) return(false);

    // check attributes of script element
    CSmallString     attname;
    CXMLIterator    I(p_variable);

    bool name_set = false;
    bool value_set = false;
    bool operation_set = false;

    while( I.GetNextAttributeName(attname) == true ) {
        bool result = false;

        if( attname == "name" ) {
            CSmallString value;
            p_variable->GetAttribute("name",value);
            if( value != NULL ) {
                result = true;
                name_set = true;
            }
        }
        if( attname == "value" ) {
            CSmallString value;
            p_variable->GetAttribute("value",value);
            result = true;
            value_set = true;
        }
        if( attname == "operation" ) {
            CSmallString value;
            p_variable->GetAttribute("operation",value);
            if( (value == "set") || (value == "append") || (value == "prepend") || (value == "keep") ) {
                result = true;
                operation_set = true;
            }
        }
        if( attname == "priority" ) {
            CSmallString value;
            p_variable->GetAttribute("priority",value);
            if( (value == "normal") || (value == "modaction") || (value == "dependency") ) result = true;
        }

        if( attname == "secret" ) {
            bool value;
            p_variable->GetAttribute("secret",value);
            result = true;
        }

        if( result == false ) {
            vout << endl << endl;
            vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'variable' element!" << endl;
            vout << "            '" << attname << "' is unsupported attribute or its value is empty." << endl;
            vout << "            Allowed attributes are: name, value, operation, priority." << endl;
            vout << endl;
            return(false);
        }
    }

    if( (name_set == false) || (value_set == false) || (operation_set == false) ) {
        vout << endl << endl;
        vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'variable' element!" << endl;
        vout << "            Missing one or more required element/s: name, value, operation." << endl;
        vout << endl;
        return(false);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CCache::CheckModuleAliasSyntax(CVerboseStr& vout,CXMLElement* p_alias)
{
    if( p_alias == NULL ) return(false);

    // check attributes of script element
    CSmallString     attname;
    CXMLIterator    I(p_alias);

    bool name_set = false;
    bool value_set = false;

    while( I.GetNextAttributeName(attname) == true ) {
        bool result = false;

        if( attname == "name" ) {
            CSmallString value;
            p_alias->GetAttribute("name",value);
            if( value != NULL ) {
                result = true;
                name_set = true;
            }
        }
        if( attname == "value" ) {
            CSmallString value;
            p_alias->GetAttribute("value",value);
            if( value != NULL ) {
                result = true;
                value_set = true;
            }
        }
        if( attname == "priority" ) {
            CSmallString value;
            p_alias->GetAttribute("priority",value);
            if( (value == "normal") || (value == "modaction") ) result = true;
        }

        if( result == false ) {
            vout << endl << endl;
            vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'alias' element!" << endl;
            vout << "            '" << attname << "' is unsupported attribute or its value is empty." << endl;
            vout << "            Allowed attributes are: name, value, priority." << endl;
            vout << endl;
            return(false);
        }
    }

    if( (name_set == false) || (value_set == false) ) {
        vout << endl << endl;
        vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'alias' element!" << endl;
        vout << "            Missing one or more required element/s: name, value." << endl;
        vout << endl;
        return(false);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CCache::CheckModuleScriptSyntax(CVerboseStr& vout,CXMLElement* p_script)
{
    if( p_script == NULL ) return(false);

    // check attributes of script element
    CSmallString     attname;
    CXMLIterator    I(p_script);
    bool             name_set = false;
    bool             type_set = false;

    while(  I.GetNextAttributeName(attname) == true  ) {
        bool result = false;

        if( attname == "name" ) {
            CSmallString value;
            p_script->GetAttribute("name",value);
            if( value != NULL ) {
                name_set = true;
                result = true;
            }
        }
        if( attname == "type" ) {
            CSmallString value;
            p_script->GetAttribute("type",value);
            if( (value == "inline") || (value == "child") ) {
                result = true;
                type_set = true;
            }
        }
        if( attname == "priority" ) {
            CSmallString value;
            p_script->GetAttribute("priority",value);
            if( (value == "normal") || (value == "modaction") ) result = true;
        }

        if( result == false ) {
            vout << endl << endl;
            vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'script' element!" << endl;
            vout << "            '" << attname << "' is unsupported attribute or its value is empty." << endl;
            vout << "            Allowed attributes are: name, type, priority." << endl;
            vout << endl;
            return(false);
        }

    }

    if( (name_set == false) || (type_set == false) ) {
        vout << endl << endl;
        vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'script' element!" << endl;
        vout << "            Missing one or more required element/s: name, type." << endl;
        vout << endl;
        return(false);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CCache::CheckModuleDepsSyntax(CVerboseStr& vout,CXMLElement* p_deps)
{
    if( p_deps == NULL ) return(false);

    if( p_deps->NumOfAttributes() > 0 ) {
        vout << endl << endl;
        vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'deps' element!" << endl;
        vout << "            No attrributes are supported for this element." << endl;
        vout << endl;
        return(false);
    }

    // subblocks
    CXMLIterator    I(p_deps);
    CXMLElement*     p_mele;

    while( (p_mele = I.GetNextChildElement()) != NULL ) {
        bool result = false;
        if( p_mele->GetName() == "dep" ) {
            if( p_mele->NumOfAttributes() != 2 ) {
                vout << endl << endl;
                vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'dep' element!" << endl;
                vout << "            Only two attributes (name/type) can be specified for this element." << endl;
                vout << endl;
                return(false);
            }
            CSmallString name;
            p_mele->GetAttribute("name",name);
            if( name == NULL ) {
                vout << endl << endl;
                vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'dep' element!" << endl;
                vout << "            'name' attribute is not specified or its value is empty." << endl;
                vout << endl;
                return(false);
            }
            CSmallString type;
            p_mele->GetAttribute("type",type);
            if( type == NULL ) {
                vout << endl << endl;
                vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'dep' element!" << endl;
                vout << "            'type' attribute is not specified or its value is empty." << endl;
                vout << endl;
                return(false);
            }
            if( (type != "pre") && (type != "post") && (type != "sync") && (type != "rm") && (type != "deb") ){
                vout << endl << endl;
                vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'dep' element!" << endl;
                vout << "            Specified dependency type '" << type << "' is nt supported (pre/post/sync/rm/deb)." << endl;
                vout << endl;
                return(false);
            }
            if( (type == "pre") || (type == "post") || (type == "sync") ){
                if( TestModuleByPartialName(name) == false ){
                    vout << endl << endl;
                    vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'dep' element!" << endl;
                    vout << "            Specified module dependency '" << name << "' does not exist in the cache." << endl;
                    vout << endl;
                    return(false);
                }
            }

            result = true;
        }
        if( result == false ) {
            vout << endl << endl;
            vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'deps' element!" << endl;
            vout << "            Unsupported subelement - '" << p_mele->GetName() << "'." << endl;
            vout << "            Allowed subelements are: dep." << endl;
            vout << endl;
            return(false);
        }
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CCache::CheckModuleDefaultSyntax(CVerboseStr& vout,CXMLElement* p_default)
{
    if( p_default == NULL ) return(false);

    if( p_default->NumOfAttributes() != 3 ) {
        vout << endl << endl;
        vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'default' element!" << endl;
        vout << "            Three attributes have to be provided but only '" << p_default->NumOfAttributes() << "' was/were found." << endl;
        vout << "            Allowed attributes are: ver, arch, mode." << endl;
        vout << endl;
        return(false);
    }

    // check attributes of default element
    CSmallString     attname;
    CXMLIterator    I(p_default);

    while( I.GetNextAttributeName(attname) == true ) {
        bool result = false;

        if( attname == "ver" ) {
            CSmallString value;
            p_default->GetAttribute("ver",value);
            if( value != NULL ) result = true;
        }
        if( attname == "arch" ) {
            CSmallString value;
            p_default->GetAttribute("arch",value);
            if( value != NULL ) result = true;
        }
        if( attname == "mode" ) {
            CSmallString value;
            p_default->GetAttribute("mode",value);
            if( value != NULL ) result = true;
        }

        if( result == false ) {
            vout << endl << endl;
            vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'default' element!" << endl;
            vout << "            '" << attname << "' is unsupported attribute or its value is empty." << endl;
            vout << "            Allowed attributes are: ver, arch, mode." << endl;
            vout << endl;
            return(false);
        }
    }

    CSmallString ver;
    bool ver_found = false;
    CSmallString arch;
    bool arch_found = false;
    CSmallString mode;
    bool par_found = false;

    p_default->GetAttribute("ver",ver);
    p_default->GetAttribute("arch",arch);
    p_default->GetAttribute("mode",mode);

    CXMLElement* p_module = dynamic_cast<CXMLElement*>(p_default->GetParentNode());
    CXMLElement* p_list = NULL;
    if( p_module != NULL ) p_list = p_module->GetFirstChildElement("builds");

    // check for values
    CXMLIterator R(p_list);
    CXMLElement* p_rele;

    while( (p_rele=R.GetNextChildElement("build")) != NULL) {
        CSmallString lver;
        CSmallString larch;
        CSmallString lmode;

        p_rele->GetAttribute("ver",lver);
        p_rele->GetAttribute("arch",larch);
        p_rele->GetAttribute("mode",lmode);

        if( lver == ver ) ver_found = true;
        if( larch == arch ) arch_found = true;
        if( lmode == mode ) par_found = true;
    }

    if( ver == "none" ) ver_found = true;
    if( (arch == "auto") || (arch == "none") ) arch_found = true;
    if( (mode == "auto") || (mode == "none") ) par_found = true;

    if( ver_found == false ) {
        vout << endl << endl;
        vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'default' element!" << endl;
        vout << "            'ver' attribute is not valid." << endl;
        vout << "            Allowed values are: none or any version from existing build." << endl;
        vout << endl;
        return(false);
    }
    if( arch_found == false ) {
        vout << endl << endl;
        vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'default' element!" << endl;
        vout << "            'arch' attribute is not valid." << endl;
        vout << "            Allowed values are: none, auto, or any architecture from existing build." << endl;
        vout << endl;
        return(false);
    }
    if( par_found == false ) {
        vout << endl << endl;
        vout << "<red>>>> ERROR:</red>: Syntax error in XML module specification - 'default' element!" << endl;
        vout << "            'mode' attribute is not valid." << endl;
        vout << "            Allowed values are: none, auto, or any parallel mode from existing build." << endl;
        vout << endl;
        return(false);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CXMLElement* CCache::GetModule(const CSmallString& name)
{
    CXMLElement* p_mele;

    CSmallString name_only = CUtils::GetModuleName(name);

    p_mele = Cache.GetFirstChildElement("cache");
    if( p_mele == NULL ) {
        return(NULL);
    }

    CXMLIterator    I(p_mele);
    CXMLElement*     p_ele;

    while( (p_ele = I.GetNextChildElement("module")) != NULL ) {
        CSmallString lname;
        p_ele->GetAttribute("name",lname);
        if( lname == name_only ) return(p_ele);
    }

    return(NULL);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CXMLElement* CCache::GetBuild(CXMLElement* p_module,
        const CSmallString& ver,
        const CSmallString& arch,
        const CSmallString& mode)
{
    if( p_module == NULL ) return(NULL);

    CXMLElement* p_list = p_module->GetFirstChildElement("builds");

    CXMLIterator    I(p_list);
    CXMLElement*     p_ele;

    while( (p_ele = I.GetNextChildElement("build")) != NULL ) {
        CSmallString lver;
        CSmallString larch;
        CSmallString lmode;
        p_ele->GetAttribute("ver",lver);
        p_ele->GetAttribute("arch",larch);
        p_ele->GetAttribute("mode",lmode);
        if( (lver == ver) && (larch == arch) && (lmode == mode) ) return(p_ele);
    }

    return(NULL);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CCache::GetModuleDefaults(CXMLElement* p_module,
        CSmallString& ver,
        CSmallString& arch,
        CSmallString& mode)
{
    ver = NULL;
    arch = NULL;
    mode = NULL;

    if( p_module == NULL ) return(false);

    CXMLIterator    I(p_module);
    CXMLElement*     p_sele;

    if( (p_sele = I.GetNextChildElement("default")) != NULL ) {
        p_sele->GetAttribute("ver",ver);
        p_sele->GetAttribute("arch",arch);
        p_sele->GetAttribute("mode",mode);
        return(true);
    }

    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CCache::CheckModuleVersion(CXMLElement* p_module,const CSmallString& ver)
{
    if( p_module == NULL ) return(false);

    CXMLElement* p_list = p_module->GetFirstChildElement("builds");

    CXMLIterator    I(p_list);
    CXMLElement*     p_sele;

    while( (p_sele = I.GetNextChildElement("build")) != NULL ) {
        CSmallString lver;
        p_sele->GetAttribute("ver",lver);
        if( lver == ver ) return(true);
    }

    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CCache::CheckModuleArchitecture(CXMLElement* p_module,
        const CSmallString& ver,
        const CSmallString& arch)
{
    if( p_module == NULL ) return(false);

    CXMLElement* p_list = p_module->GetFirstChildElement("builds");

    CXMLIterator    I(p_list);
    CXMLElement*     p_sele;

    while( (p_sele = I.GetNextChildElement("build")) != NULL ) {
        CSmallString lver;
        CSmallString larch;
        p_sele->GetAttribute("ver",lver);
        p_sele->GetAttribute("arch",larch);
        if( (larch == arch) && (lver == ver) ) return(true);
    }

    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CCache::CheckModuleMode(CXMLElement* p_module,
        const CSmallString& ver,
        const CSmallString& arch,
        const CSmallString& mode)
{
    if( p_module == NULL ) return(false);

    CXMLElement* p_list = p_module->GetFirstChildElement("builds");

    CXMLIterator    I(p_list);
    CXMLElement*     p_sele;

    while( (p_sele = I.GetNextChildElement("build")) != NULL ) {
        CSmallString lver;
        CSmallString larch;
        CSmallString lmode;
        p_sele->GetAttribute("ver",lver);
        p_sele->GetAttribute("arch",larch);
        p_sele->GetAttribute("mode",lmode);
        if( (lmode == mode)&&(larch == arch)&&(lver == ver) ) return(true);
    }

    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

// return length of longest module name + version + arch + mode

int CCache::GetSizeOfLongestModuleSpecification(void)
{
    CXMLElement* p_mele;

    p_mele = GetRootElementOfCache();
    if( p_mele == NULL ) return(5);

    unsigned int maxlen = 1;

    CXMLIterator    I(p_mele);
    CXMLElement*     p_ele;

    while( (p_ele = I.GetNextChildElement("module")) != NULL ) {
        CSmallString name;
        p_ele->GetAttribute("name",name);

        CXMLElement* p_list = p_ele->GetFirstChildElement("builds");

        CXMLIterator    J(p_list);
        CXMLElement*     p_sele;

        while( (p_sele = J.GetNextChildElement("build")) != NULL ) {
            CSmallString ver;
            CSmallString arch;
            CSmallString mode;
            p_sele->GetAttribute("ver",ver);
            p_sele->GetAttribute("arch",arch);
            p_sele->GetAttribute("mode",mode);
            if( maxlen < (name.GetLength() + ver.GetLength() + arch.GetLength() + mode.GetLength()) ) {
                maxlen = name.GetLength() + ver.GetLength() + arch.GetLength() + mode.GetLength();
            }
        }
    }

    return(maxlen+4); // three colon + space
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

// return length of longest module name + version

int CCache::GetSizeOfLongestModuleName(bool include_ver)
{
    CXMLElement* p_mele;
    CXMLElement* p_ele;

    p_mele = GetRootElementOfCache();
    if( p_mele == NULL ) return(5);

    unsigned int maxlen = 1;

    CXMLIterator    I(p_mele);

    while( (p_ele = I.GetNextChildElement("module")) != NULL ) {
        CSmallString name;
        p_ele->GetAttribute("name",name);

        if( maxlen < name.GetLength() ) {
            maxlen = name.GetLength();
        }
        if( include_ver == false ) continue;

        CXMLElement* p_list = p_ele->GetFirstChildElement("builds");

        CXMLIterator    J(p_list);
        CXMLElement*     p_sele;

        while( (p_sele = J.GetNextChildElement("build")) != NULL ) {
            CSmallString ver;
            p_sele->GetAttribute("ver",ver);
            if( maxlen < (name.GetLength() + ver.GetLength()) ) {
                maxlen = name.GetLength() + ver.GetLength();
            }
        }
    }

    return(maxlen+2); // one colon + space
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CCache::TestCrossDependency(const CSmallString& module,const CSmallString& depmodule)
{
    CXMLElement* p_mod = GetModule(module);
    if( p_mod == NULL ) return(false);

    CSmallString depname;
    depname = CUtils::GetModuleName(depmodule);

    // test global module deps
    CXMLElement* p_dele = p_mod->GetFirstChildElement("deps");
    if( p_dele ) p_dele = p_dele->GetFirstChildElement();

    while( p_dele != NULL ){
        CSmallString ldepmod;
        p_dele->GetAttribute("module",ldepmod);
        CSmallString ldepname = CUtils::GetModuleName(ldepmod);
        if( ldepname == depname ) return(true);
        p_dele = p_dele->GetNextSiblingElement();
    }

    // test build deps
    CXMLElement* p_build = p_mod->GetChildElementByPath("builds/build");
    while( p_build != NULL ){

        CXMLElement* p_dele = p_build->GetFirstChildElement("deps");
        if( p_dele ) p_dele = p_dele->GetFirstChildElement("dep");

        while( p_dele != NULL ){
            CSmallString ldepmod,ltype;
            p_dele->GetAttribute("name",ldepmod);
            p_dele->GetAttribute("type",ltype);
            if( (ltype == "pre") || (ltype == "post") ){
                CSmallString ldepname = CUtils::GetModuleName(ldepmod);
                if( ldepname == depname ) return(true);
            }
            p_dele = p_dele->GetNextSiblingElement("dep");
        }
        p_build = p_build->GetNextSiblingElement("build");
    }

    return(false);
}

//------------------------------------------------------------------------------

bool CCache::TestModuleByPartialName(const CSmallString& module)
{
    CXMLElement* p_mod = GetModule(module);
    if( p_mod == NULL ) return(false);

    CSmallString name;
    CSmallString ver;
    CSmallString arch;
    CSmallString mode;

    CUtils::ParseModuleName(module,name,ver,arch,mode);

    // try to find partial build
    CXMLElement* p_build = p_mod->GetChildElementByPath("builds/build");
    while( p_build != NULL ){
        CSmallString lver;
        CSmallString larch;
        CSmallString lmode;

        p_build->GetAttribute("ver",lver);
        p_build->GetAttribute("arch",larch);
        p_build->GetAttribute("mode",lmode);

        bool found = true;
        if( (ver != NULL) && (ver != "default") ) found &= ver == lver;
        if( (arch != NULL) && ((arch != "default") || (arch != "auto") ) ) found &= CUtils::AreSameTokens(arch,larch);
        if( (mode != NULL) && ((mode != "default") || (mode != "auto") ) ) found &= mode == lmode;

        if( found ) return(true);

        p_build = p_build->GetNextSiblingElement("build");
    }

    return(false);
}

//------------------------------------------------------------------------------

bool CCache::IsPermissionGrantedForModule(CXMLElement* p_module)
{
    if( p_module == NULL ){
        RUNTIME_ERROR("p_module is NULL");
    }
    CXMLElement* p_acl = p_module->GetFirstChildElement("acl");
    if( p_acl ){
        return(IsPermissionGranted(p_acl));
    }

    return(true);
}
//------------------------------------------------------------------------------

bool CCache::IsPermissionGrantedForModuleByAnyMeans(CXMLElement* p_module)
{
    if( p_module == NULL ){
        RUNTIME_ERROR("p_module is NULL");
    }
    CXMLElement* p_acl = p_module->GetFirstChildElement("acl");
    if( p_acl ){
        if( IsPermissionGranted(p_acl) == false ) return(false);
    }
    bool build_access = false;
    CXMLElement* p_build = p_module->GetChildElementByPath("builds/build");
    while( p_build != NULL ){
        build_access |= IsPermissionGrantedForBuild(p_build);
        p_build = p_build->GetNextSiblingElement("build");
    }
    return( build_access );
}

//------------------------------------------------------------------------------

bool CCache::IsPermissionGrantedForBuild(CXMLElement* p_build)
{
    if( p_build == NULL ){
        RUNTIME_ERROR("p_build is NULL");
    }
    CXMLElement* p_acl = p_build->GetFirstChildElement("acl");
    if( p_acl ){
        return(IsPermissionGranted(p_acl));
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CCache::IsPermissionGranted(CXMLElement* p_acl)
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
                if( User.IsInGroup(group) == true ) return(true);
            }
            if( p_rule->GetName() == "deny" ){
                if( User.IsInGroup(group) == true ) return(false);
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
                if( User.IsInGroup(group) == true ) return(true);
            }
            if( p_rule->GetName() == "deny" ){
                if( User.IsInGroup(group) == true ) return(false);
            }
            p_rule = p_rule->GetNextSiblingElement();
        }
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CCache::HasModuleAnyCompatibleBuild(CXMLElement* p_cmod,const CSmallString& arch)
{
    if( p_cmod == NULL ){
        RUNTIME_ERROR("p_module is NULL");
    }

    list<string> arch_tokens;
    string       sarch_arch(arch);
    split(arch_tokens,sarch_arch,is_any_of("#"));

    CXMLElement* p_build = p_cmod->GetChildElementByPath("builds/build");
    while( p_build != NULL ){
        CSmallString barch;
        p_build->GetAttribute("arch",barch);

        list<string> barch_tokens;
        string       sbarch_arch(barch);
        split(barch_tokens,sbarch_arch,is_any_of("#"));

        list<string>::iterator sit = arch_tokens.begin();
        list<string>::iterator set = arch_tokens.end();

        unsigned int count = 0;

        while( sit != set ){

            list<string>::iterator bsit = barch_tokens.begin();
            list<string>::iterator bset = barch_tokens.end();

            while( bsit != bset ){
                if( *bsit == *sit ){
                    count++;
                    break;
                }
                bsit++;
            }

            sit++;
        }

        if( count == arch_tokens.size() ) return(true);

        p_build = p_build->GetNextSiblingElement("build");
    }
    return( false );
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CXMLElement* CCache::GetRootElementOfCache(void)
{
    CXMLElement* p_mele;
    p_mele = Cache.GetFirstChildElement("cache");
    return(p_mele);
}

//-----------------------------------------------------------------------------

bool CCache::CanModuleBeExported(CXMLElement* p_module)
{
    if( p_module == NULL ) return(true);
    bool export_flag = true;
    p_module->GetAttribute("export",export_flag);
    return(export_flag);
}

//------------------------------------------------------------------------------

void CCache::GetAllModuleVersions(CXMLElement* p_module, std::list<CSmallString>& vers)
{
    if( p_module == NULL ){
        RUNTIME_ERROR("p_module is NULL");
    }

    vers.clear();

    CXMLElement* p_sele = p_module->GetChildElementByPath("builds/build");
    while( p_sele != NULL ) {
        CSmallString ver;
        p_sele->GetAttribute("ver",ver);
        vers.push_back(ver);
        p_sele = p_sele->GetNextSiblingElement("build");
    }

    vers.sort();
    vers.unique();
}

//------------------------------------------------------------------------------

void CCache::RemoveBuildsWithArchToken(const CSmallString& token)
{
    CXMLElement* p_ele = GetRootElementOfCache();
    if( p_ele == NULL ) return; // no cache loaded

    // loop over modules
    CXMLElement* p_mele = p_ele->GetFirstChildElement("module");
    while( p_mele != NULL ){

        // loop over builds
        CXMLElement* p_bele = p_mele->GetChildElementByPath("builds/build");
        while( p_bele != NULL ) {
            CSmallString arch;
            p_bele->GetAttribute("arch",arch);

            list<string> arch_tokens;
            string       sarch_arch(arch);
            split(arch_tokens,sarch_arch,is_any_of("#"));

            list<string>::iterator sit = arch_tokens.begin();
            list<string>::iterator set = arch_tokens.end();

            bool found = false;
            while( sit != set ){
                if( *sit == string(token) ){
                    found = true;
                    break;
                }
                sit++;
            }

            CXMLElement* p_oldbuild = p_bele;
            p_bele = p_bele->GetNextSiblingElement("build");

            if( found ){
                // remove build
                delete p_oldbuild;
            }
        }

        CXMLElement* p_oldmod = p_mele;
        p_mele = p_mele->GetNextSiblingElement("module");

        if( p_oldmod->GetChildElementByPath("builds/build") == NULL ){
            // if no build -> remove module
            delete p_oldmod;
        }
    }

}

//------------------------------------------------------------------------------

bool CCache::DoesItNeedGPU(const CSmallString& name)
{
    CXMLElement* p_module = GetModule(name);
    if( p_module == NULL ){
        CSmallString warning;
        warning << "module '" << name << "'' was not found in the cache";
        ES_WARNING(warning);
        return(false);
    }

    CXMLElement* p_list = p_module->GetFirstChildElement("builds");

    CXMLIterator    I(p_list);
    CXMLElement*    p_sele;

    bool gpu = false;
    bool nogpu = true;

    while( (p_sele = I.GetNextChildElement("build")) != NULL ) {
        CSmallString larch;
        p_sele->GetAttribute("arch",larch);
        if( larch.FindSubString("gpu") >= 0 ){
            gpu = true;
        } else {
            nogpu = true;
        }
    }

    if( nogpu ) return(false);
    return(gpu);
}

//------------------------------------------------------------------------------

const CSmallString CCache::GetVariableValue(CXMLElement* p_rele,
        const CSmallString& name)
{
    CXMLElement* p_setup = NULL;
    if( p_rele != NULL ) p_setup = p_rele->GetFirstChildElement("setup");

    CXMLIterator    I(p_setup);
    CXMLElement*    p_sele;

    while( (p_sele = I.GetNextChildElement("variable")) != NULL ) {
        CSmallString lname;
        CSmallString lvalue;
        p_sele->GetAttribute("name",lname);
        p_sele->GetAttribute("value",lvalue);
        if( lname == name ) return(lvalue);
    }
    return("");
}

//------------------------------------------------------------------------------

bool CCache::SetVariableValue(CXMLElement* p_rele,
        const CSmallString& name,
        const CSmallString& value,
        const CSmallString& priority
                                          )
{
    if( p_rele == NULL ) {
        ES_ERROR("p_rele is NULL");
        return(false);
    }

    CXMLElement* p_setup = NULL;
    p_setup = p_rele->GetFirstChildElement("setup");

    CXMLIterator    I(p_setup);
    CXMLElement*    p_sele;

    while( (p_sele = I.GetNextChildElement("variable")) != NULL ) {
        CSmallString lname;
        CSmallString lvalue;
        p_sele->GetAttribute("name",lname);
        if( lname == name ) {
            p_sele->SetAttribute("value",value);
            p_sele->SetAttribute("priority",priority);
            return(true);
        }
    }

    p_sele = p_setup->CreateChildElement("variable");
    if( p_sele == NULL ) {
        ES_ERROR("unable to create variable element");
        return(false);
    }

    p_sele->SetAttribute("name",name);
    p_sele->SetAttribute("value",value);
    p_sele->SetAttribute("priority",priority);

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CXMLElement* CCache::GetModuleDescription(CXMLElement* p_module)
{
    if( p_module == NULL ) return(NULL);
    CXMLElement* p_doc = p_module->GetChildElementByPath("doc");
    return(p_doc);
}

//------------------------------------------------------------------------------

bool CCache::SearchCache(const CSmallString& pattern,
                                      std::vector<CXMLElement*>& hits)
{
    hits.clear();

    CXMLElement* p_mele;

    p_mele = Cache.GetFirstChildElement("cache");
    if( p_mele == NULL ) {
        return(true);
    }

    CXMLIterator   I(p_mele);
    CXMLElement*   p_ele;

    while( (p_ele = I.GetNextChildElement("module")) != NULL ) {
        CSmallString lname;
        p_ele->GetAttribute("name",lname);

        if( fnmatch(pattern,lname,0) == 0 ) {
            hits.push_back(p_ele);
        }
    }

    return(true);

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

class CPVerRecord {
    public:
    CPVerRecord(void);
    string      version;
    int         verindx;
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

//------------------------------------------------------------------------------

bool CCache::GetSortedModuleVersions(const CSmallString& mod_name,std::vector<std::string>& versions)
{
    CSmallString name = CUtils::GetModuleName(mod_name);
    CXMLElement* p_ele = GetModule(name);

    if( p_ele == NULL ) {
        CSmallString error;
        error << "module '" << mod_name << "' was not found in AMS cache";
        ES_ERROR(error);
        return(false);
    }

    std::list<CPVerRecord>  list;
    CXMLElement*            p_list = p_ele->GetFirstChildElement("builds");
    CXMLIterator            K(p_list);
    CXMLElement*            p_tele;

    while( (p_tele = K.GetNextChildElement("build")) != NULL ) {
        CPVerRecord verrcd;
        verrcd.verindx = 0.0;
        p_tele->GetAttribute("ver",verrcd.version);
        p_tele->GetAttribute("verindx",verrcd.verindx);
        list.push_back(verrcd);
    }

    list.sort(sort_tokens);
    list.unique();

    std::list<CPVerRecord>::iterator it = list.begin();
    std::list<CPVerRecord>::iterator ie = list.end();

    versions.clear();
    while( it != ie ){
        versions.push_back((*it).version);
        it++;
    }

    return(true);
}

//------------------------------------------------------------------------------

void CCache::LoadDebReleaseFilters(const CSmallString& release)
{
    DebAliases.clear();

    CFileName    config_path;
    config_path = AMSGlobalConfig.GetETCDIR() / "debaliases.xml";

    if( ! CFileSystem::IsFile(config_path) ) return; // no aliases defined

    CXMLDocument deb_aliases;

    CXMLParser xml_parser;
    xml_parser.SetOutputXMLNode(&deb_aliases);

    if( xml_parser.Parse(config_path) == false ) {
        CSmallString error;
        error << "unable to load " << config_path;
        ES_ERROR(error);
        return;
    }

    CXMLElement* p_root = deb_aliases.GetChildElementByPath("releases/release");
    while( p_root != NULL ){
        CSmallString name;
        p_root->GetAttribute("name",name);
        if( name == release ){
            CXMLElement* p_alias = p_root->GetFirstChildElement("alias");
            while( p_alias != NULL ){
                CSmallString key;
                CSmallString deb;
                p_alias->GetAttribute("key",key);
                p_alias->GetAttribute("deb",deb);
                if( (key != NULL) && (deb != NULL) ){
                    DebAliases[key] = deb;
                }
                p_alias = p_alias->GetNextSiblingElement("alias");
            }
        }
        p_root = p_root->GetNextSiblingElement("release");
    }
}

//------------------------------------------------------------------------------

void CCache::GetDebDependencies(const CSmallString& release,std::set<CSmallString>& debdeps)
{
    // load release filters
    if( release != NULL ){
        LoadDebReleaseFilters(release);
    }

    // list debs
    CXMLElement* p_sele = GetRootElementOfCache();
    if( p_sele == NULL ){
        RUNTIME_ERROR("unable to find cache element");
    }

    CXMLIterator    I(p_sele);
    CXMLElement*     p_mele;
    while( (p_mele = I.GetNextChildElement("module")) != NULL  ) {
        CSmallString name;
        p_mele->GetAttribute("name",name);

        // traverse deps - intermodule
        CXMLElement* p_dele;

        p_dele = p_mele->GetFirstChildElement("deps");
        CXMLIterator    D(p_dele);
        CXMLElement*    p_lele;

        while( (p_lele = D.GetNextChildElement()) != NULL  ) {
            if( p_lele->GetName() == "dep" ) {
                CSmallString dname;
                CSmallString type;
                p_lele->GetAttribute("name",dname);
                p_lele->GetAttribute("type",type);
                if( type == "deb" ){
                    if( DebAliases.count(dname) > 0 ){
                        CSmallString deb = DebAliases[dname];
                        if( deb != NULL ) debdeps.insert(deb);
                    } else {
                        debdeps.insert(dname);
                    }
                }
            }
        }

        // traverse deps - interbuilds
        CXMLElement* p_bele;
        p_bele = p_mele->GetChildElementByPath("builds/build");

        while( p_bele != NULL ){

            CSmallString ver;
            CSmallString arch;
            CSmallString mode;
            p_bele->GetAttribute("ver",ver);
            p_bele->GetAttribute("arch",arch);
            p_bele->GetAttribute("mode",mode);

            CXMLElement* p_dele;

            p_dele = p_bele->GetFirstChildElement("deps");
            CXMLIterator    D(p_dele);
            CXMLElement*    p_lele;

            while( (p_lele = D.GetNextChildElement()) != NULL  ) {
                if( p_lele->GetName() == "dep" ) {
                    CSmallString dname;
                    CSmallString type;
                    p_lele->GetAttribute("name",dname);
                    p_lele->GetAttribute("type",type);
                    if( type == "deb" ){
                        if( DebAliases.count(dname) > 0 ){
                            CSmallString deb = DebAliases[dname];
                            if( deb != NULL ) debdeps.insert(deb);
                        } else {
                            debdeps.insert(dname);
                        }
                    }
                }
            }

            p_bele = p_bele->GetNextSiblingElement("build");
        }

    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
