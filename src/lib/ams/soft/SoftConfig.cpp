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

#include <SoftConfig.hpp>
#include <XMLParser.hpp>
#include <XMLPrinter.hpp>
#include <prefix.h>
#include <ErrorSystem.hpp>
#include <XMLElement.hpp>
#include <Shell.hpp>
#include <Utils.hpp>
#include <string.h>
#include <vector>
#include <AMSGlobalConfig.hpp>
#include <FileSystem.hpp>
#include <PrintEngine.hpp>
#include <XMLIterator.hpp>
#include <DirectoryEnum.hpp>
#include <Site.hpp>
#include <AmsUUID.hpp>

//------------------------------------------------------------------------------

CSoftConfig SoftConfig;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CSoftConfig::CSoftConfig(void)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CSoftConfig::LoadUserConfig(void)
{
    // load common config ----------------------------------
    CFileName    config_name;

    // common config
    config_name = GlobalConfig.GetUserGlobalConfigDir() / "ams.xml";
    if( CFileSystem::IsFile(config_name) == false ) {
        // no user common config
        CommonConfig.CreateChildDeclaration();
        CommonConfig.CreateChildComment("AMS User Common Configuration");
        CommonConfig.CreateChildElement("user");
    } else {
        CXMLParser xml_parser;
        xml_parser.SetOutputXMLNode(&CommonConfig);

        if( xml_parser.Parse(config_name) == false ) {
            CSmallString error;
            error << "unable to load common user config (" << config_name <<")";
            RUNTIME_ERROR(error);
        }
    }

// FIX ME
    // load site config ---------------------------------------
//    if( GlobalConfig.GetActiveSiteID() == NULL ) {
//        // no user config
//        SiteConfig.CreateChildDeclaration();
//        SiteConfig.CreateChildComment("AMS User Configuration");
//        SiteConfig.CreateChildElement("user");
//        return;
//    }

//    // then try site config
//    config_name = GlobalConfig.GetUserSiteConfigDir() / "ams.xml";
//    if( CFileSystem::IsFile(config_name) == false ) {
//        // no user config
//        SiteConfig.CreateChildDeclaration();
//        SiteConfig.CreateChildComment("AMS User Configuration");
//        SiteConfig.CreateChildElement("user");
//        return;
//    }

//    CXMLParser xml_parser;
//    xml_parser.SetOutputXMLNode(&SiteConfig);

//    if( xml_parser.Parse(config_name) == false ) {
//        CSmallString error;
//        error << "unable to load user config (" << config_name <<")";
//        RUNTIME_ERROR(error);
//    }

}

//------------------------------------------------------------------------------

bool CSoftConfig::SaveUserConfig(void)
{
    // save config ----------------------------------
    CFileName    config_name;

    // save common config
    config_name = GlobalConfig.GetUserGlobalConfigDir() / "ams.xml";

    CXMLPrinter xml_printer;
    xml_printer.SetPrintedXMLNode(&CommonConfig);

    if( xml_printer.Print(config_name) == false ) {
        CSmallString error;
        error << "unable to save common user config (" << config_name <<")";
        ES_ERROR(error);
        return(false);
    }

// FIX ME
//    if( GlobalConfig.GetActiveSiteID() == NULL ) {
//        ES_ERROR("no site is active");
//        return(false);
//    }

//    // save site config
//    config_name = GlobalConfig.GetUserSiteConfigDir() / "ams.xml";

//    xml_printer.SetPrintedXMLNode(&SiteConfig);

//    if( xml_printer.Print(config_name) == false ) {
//        CSmallString error;
//        error << "unable to save user config (" << config_name <<")";
//        ES_ERROR(error);
//        return(false);
//    }

    return(true);
}

//------------------------------------------------------------------------------

void CSoftConfig::ClearAutorestoredConfig(void)
{
    CXMLElement* p_ele = CommonConfig.GetChildElementByPath("user/autoload");
    if( p_ele == NULL ) return;
    delete p_ele;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CSoftConfig::AreSystemAutoloadedModulesDisabled(void)
{
    // global setup
    CFileName    config_name;
    config_name = GlobalConfig.GetUserSiteConfigDir() / "_disable_system_modules";

    if( CFileSystem::IsFile(config_name) == true ) {
        // user does not want system modules for all sites
        ES_WARNING("user has _disable_system_modules config file for the active site");
        return(true);
    }

    // global setup
    config_name = GlobalConfig.GetUserGlobalConfigDir() / "_disable_system_modules";

    if( CFileSystem::IsFile(config_name) == true ) {
        // user does not want system modules for particular
        ES_WARNING("user has _disable_system_modules config file for all sites");
    }

    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CXMLElement* CSoftConfig::GetAutoloadedModules(void)
{
    CXMLElement* p_ele = CommonConfig.GetChildElementByPath("user/autoload");
    if( p_ele == NULL ) {
        ES_WARNING("unable to open user/autoload path");
        return(NULL);
    }
    return(p_ele);
}

//------------------------------------------------------------------------------

bool CSoftConfig::PrintAutorestoredModules(FILE* fout)
{
    if( fout == NULL ) {
        fout = stdout;
    }

    CXMLElement* p_ele = CommonConfig.GetChildElementByPath("user/autoload");
    if( p_ele == NULL ) {
        ES_WARNING("unable to open user/autoload path");
    }

    // print module
    CXMLIterator I(p_ele);
    CXMLElement* p_mele;

    int count = 0;
    while((p_mele = I.GetNextChildElement("module")) != NULL) {
        CSmallString lname;
        if( p_mele->GetAttribute("name",lname) == true ) {
            fprintf(fout,"%s\n",(const char*)lname);
            count++;
        }
    }

    if( count == 0 ) fprintf(fout,"[none]\n");

    return(true);
}

//------------------------------------------------------------------------------

void CSoftConfig::AddAutorestoredModule(const CSmallString& module)
{
    CXMLElement* p_ele = CommonConfig.GetChildElementByPath("user/autoload",true);
    CXMLElement* p_mele = p_ele->CreateChildElement("module");
    p_mele->SetAttribute("name",module);
}

//------------------------------------------------------------------------------

bool CSoftConfig::RemoveAutorestoredModule(const CSmallString& module)
{
    CXMLElement* p_ele = CommonConfig.GetChildElementByPath("user/autoload");
    if( p_ele == NULL ) {
        ES_ERROR("unable to open user/autoload path");
        return(false);
    }

    CSmallString name;
    CUtils::ParseModuleName(module,name);

    // find module
    CXMLIterator I(p_ele);
    CXMLElement* p_mele;

    while((p_mele = I.GetNextChildElement("module")) != NULL) {
        CSmallString lmod;
        CSmallString lname;
        p_mele->GetAttribute("name",lmod);
        CUtils::ParseModuleName(lmod,lname);

        if( lname == name ) {
            // module found -> remove it
            delete p_mele;
            return(true);
        }
    }

    CSmallString error;
    error << "module '" << name << "' was not found";
    ES_ERROR(error);
    return(false);
}

//------------------------------------------------------------------------------

bool CSoftConfig::IsAutorestoredModule(const CSmallString& module)
{
    CXMLElement* p_ele = CommonConfig.GetChildElementByPath("user/autoload");
    if( p_ele == NULL ) {
        ES_WARNING("unable to open user/autoload path");
        return(false);
    }

    CSmallString name;
    CUtils::ParseModuleName(module,name);

    // find module
    CXMLIterator I(p_ele);
    CXMLElement* p_mele;

    while((p_mele = I.GetNextChildElement("module")) != NULL) {
        CSmallString lmod;
        CSmallString lname;
        p_mele->GetAttribute("name",lmod);
        CUtils::ParseModuleName(lmod,lname);

        if( lname == name ) {
            // module found
            return(true);
        }
    }

    // not found
    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CSoftConfig::GetUserGroup(void)
{
    CXMLElement* p_ele = CommonConfig.GetChildElementByPath("user",true);
    CSmallString group;
    p_ele->GetAttribute("group",group);
    return(group);
}

//------------------------------------------------------------------------------

void CSoftConfig::SetUserGroup(const CSmallString& group)
{
    CXMLElement* p_ele = CommonConfig.GetChildElementByPath("user",true);
    p_ele->SetAttribute("group",group);
}

//------------------------------------------------------------------------------

const CSmallString CSoftConfig::GetUserUMask(void)
{
    CXMLElement* p_ele = CommonConfig.GetChildElementByPath("user",true);
    CSmallString umask;
    p_ele->GetAttribute("umask",umask);
    if( umask == NULL ){
        umask = DEFAULT_UMASK;
    }
    return(umask);
}

//------------------------------------------------------------------------------

void CSoftConfig::SetUserUMask(const CSmallString& umask)
{
    CXMLElement* p_ele = CommonConfig.GetChildElementByPath("user",true);
    p_ele->SetAttribute("umask",umask);
}


//------------------------------------------------------------------------------

const CSmallString CSoftConfig::GetSitePriorities(void)
{
    CXMLElement* p_ele = CommonConfig.GetChildElementByPath("user",true);
    CSmallString pri;
    p_ele->GetAttribute("sitepri",pri);
    return(pri);
}

//------------------------------------------------------------------------------

void CSoftConfig::SetSitePriorities(const CSmallString& pri)
{
    CXMLElement* p_ele = CommonConfig.GetChildElementByPath("user",true);
    p_ele->SetAttribute("sitepri",pri);
}

//------------------------------------------------------------------------------

const CSmallString CSoftConfig::GetDefaultModulePriorities(void)
{
    CSmallString pri;
    CShell::SetSystemVariable("AMS_DEFAULT_MODPRI",pri);
    if( pri == NULL ){
        pri = "amsmodule";
    }
    return(pri);
}

//------------------------------------------------------------------------------

const CSmallString CSoftConfig::GetModulePriorities(void)
{
    CXMLElement* p_ele = CommonConfig.GetChildElementByPath("user",true);
    CSmallString pri;
    p_ele->GetAttribute("modpri",pri);
    return(pri);
}

//------------------------------------------------------------------------------

void CSoftConfig::SetModulePriorities(const CSmallString& pri)
{
    CXMLElement* p_ele = CommonConfig.GetChildElementByPath("user",true);
    p_ele->SetAttribute("modpri",pri);
}

//------------------------------------------------------------------------------

bool CSoftConfig::IsAvailableSite(const CSmallString& name)
{
    CDirectoryEnum dir_enum(BR_ETCDIR("/sites"));

    dir_enum.StartFindFile("*");
    CFileName site_sid;

    while( dir_enum.FindFile(site_sid) ) {
        CAmsUUID    site_id;
        CSite       site;
        if( site_id.LoadFromString(site_sid) == false ) continue;
        if( site.LoadConfig(site_sid) == false ) continue;
        if( site.CanBeActivated() == false ) continue;
        if( site.GetName() == name ) return(true);
    }
    dir_enum.EndFindFile();

    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

