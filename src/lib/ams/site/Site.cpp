// =============================================================================
//  AMS - Advanced Module System
// -----------------------------------------------------------------------------
//     Copyright (C) 2023 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include <Site.hpp>
#include <FileName.hpp>
#include <XMLParser.hpp>
#include <XMLElement.hpp>
#include <XMLIterator.hpp>
#include <XMLText.hpp>
#include <ErrorSystem.hpp>
#include <AMSRegistry.hpp>
#include <Shell.hpp>
#include <ShellProcessor.hpp>
#include <Terminal.hpp>
#include <Utils.hpp>
#include <User.hpp>
#include <Host.hpp>
#include <HostGroup.hpp>
#include <iomanip>
#include <SiteController.hpp>
#include <ModCache.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//------------------------------------------------------------------------------

CSite    Site;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CSite::CSite(void)
{

}

//------------------------------------------------------------------------------

CSite::~CSite(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CSite::LoadConfig(const CSmallString& config_file)
{
    if( config_file == NULL ){
        ES_WARNING("config_file is NULL");
        return(false);
    }

    ConfigFile = config_file;

    CXMLParser xml_parser;
    xml_parser.SetOutputXMLNode(&Config);

    if( xml_parser.Parse(ConfigFile) == false ) {
        CSmallString error;
        error << "unable to load site '" << ConfigFile << "' config";
        ES_ERROR(error);
        return(false);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CSite::GetName(void) const
{
    CSmallString name;
    CXMLElement* p_ele = Config.GetChildElementByPath("site");
    if( p_ele == NULL ) {
        ES_ERROR("unable to open site path");
        return(name);
    }
    if( p_ele->GetAttribute("name",name) == false ) {
        ES_ERROR("unable to get setup item");
        return(name);
    }
    return(name);
}

//------------------------------------------------------------------------------

const CSmallString CSite::GetID(void) const
{
    CSmallString id;
    CXMLElement* p_ele = Config.GetChildElementByPath("site");
    if( p_ele == NULL ) {
        ES_ERROR("unable to open site path");
        return(id);
    }
    if( p_ele->GetAttribute("id",id) == false ) {
        ES_ERROR("unable to get setup item");
        return(id);
    }
    return(id);
}

//------------------------------------------------------------------------------

const CSmallString CSite::GetDocumentationURL(void)
{
    CSmallString name;
    CXMLElement* p_ele = Config.GetChildElementByPath("site/description/documentation");
    if( p_ele == NULL ) {
        return(name);
    }
    p_ele->GetAttribute("url",name);
    return(name);
}

//------------------------------------------------------------------------------

const CSmallString CSite::GetSupportEMail(bool incname)
{
    CSmallString email,name;
    CXMLElement* p_ele = Config.GetChildElementByPath("site/description/support");
    if( p_ele == NULL ) {
        return(email);
    }

    p_ele->GetAttribute("email",email);
    p_ele->GetAttribute("name",name);
    if( (name != NULL) && incname ){
        email << " (" << name << ")";
    }
    return(email);
}

//------------------------------------------------------------------------------

bool CSite::IsSiteActive(void)
{
   return(SiteController.GetActiveSite() == GetName());
}

//------------------------------------------------------------------------------

CXMLElement* CSite::GetSiteEnvironment(void)
{
    return( Config.GetChildElementByPath("site/environment") );
}

//------------------------------------------------------------------------------

void CSite::GetAutoLoadedModules(std::list<CSmallString>& modules,
                                 bool withorigin,bool personal)
{
     CSmallString flavour = AMSRegistry.GetUserSiteFlavour();

    CXMLElement* p_ele = GetAutoLoadedModules();
    if( p_ele ){
        p_ele = p_ele->GetFirstChildElement("module");
    }
    while( p_ele ){
        CSmallString mname,mflavour;
        p_ele->GetAttribute("name",mname);
        bool enabled = ModCache.IsAutoloadEnabled(mname);

        p_ele->GetAttribute("flavour",mflavour);
        if( (mname != NULL) && ((mflavour == NULL) || (mflavour == flavour))){
            if( withorigin ){
                mname << "[site:" << GetName();
                if( mflavour != NULL ) mname << "@" << mflavour;
                mname << "]";
                if( ! enabled ){
                    mname << " - disabled by admin";
                }
                modules.push_back(mname);
            } else {
                if( enabled || personal ) modules.push_back(mname);
            }
        }
        p_ele = p_ele->GetNextSiblingElement("module");
    }
}

//------------------------------------------------------------------------------

CXMLElement* CSite::GetAutoLoadedModules(void)
{
    CXMLElement* p_ele = Config.GetChildElementByPath("site/autoload");
    return(p_ele);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CSite::PrintShortSiteInfo(CVerboseStr& vout)
{
    CSmallString name = GetName();
    CSmallString title;

    title << "<b><green>" << name << "</green></b>";
    if( ! IsSiteActive() ) {
        title <<  " (not active)";
    }

    vout << endl;
    vout << "# ~~~ <b>Site Info</b> ";
    CSmallString version;
    version << " [AMS " << LibBuildVersion_AMS_NoDate << "] ";

    for(unsigned int n=18; n < 80 - version.GetLength(); n++) vout << "~";
    vout << version << "~~" << endl;

    vout << "  Site name  : " << title << endl;
    vout << "  Host group : " << setw(27) << left << HostGroup.GetHostGroupName();
    vout << " | User site flavour : " << setw(15) << right << AMSRegistry.GetUserSiteFlavour() << endl;

    User.PrintUserInfoForSite(vout);
    Host.PrintHostInfoForSite(vout);

    if( (GetDocumentationURL() != NULL) || (GetSupportEMail(true) != NULL) ) {
        vout << "# ~~~ <b>Site documentation and support</b> ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
        if( GetDocumentationURL() != NULL ) {
            vout << "  Documentation  : " << GetDocumentationURL() << endl;
        }
        if( GetSupportEMail(true) != NULL ) {
            vout << "  Support e-mail : <b><green>" << GetSupportEMail(true) << "</green></b>" << endl;
        }
    }

    vout << "# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
    vout << endl;
}

//------------------------------------------------------------------------------

void CSite::PrintFullSiteInfo(CVerboseStr& vout)
{
    PrintShortSiteInfo(vout);

    vout << endl;
    vout << "# ~~~ <b>Site Attributes</b> ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    vout << "# Site config : " << ConfigFile << endl;
    vout << "# Site ID     : " << GetID() << endl;

    vout << endl;
    vout << "# ~~~ <b>Environment Variables (hosts-config)</b> ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    CShellProcessor::PrintBuild(vout,HostGroup.GetHostsConfigEnvironment());

    vout << endl;
    vout << "# ~~~ <b>Environment Variables (host-group)</b> ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    CShellProcessor::PrintBuild(vout,HostGroup.GetHostGroupEnvironment());

    vout << endl;
    vout << "# ~~~ <b>Environment Variables (site)</b> ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    CShellProcessor::PrintBuild(vout,GetSiteEnvironment());

    vout << endl;
    vout << "# ~~~ <b>AutoLoaded Modules</b> ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    PrintAutoLoadedModules(vout);
}

//------------------------------------------------------------------------------

void CSite::PrintAutoLoadedModules(CVerboseStr& vout)
{
    vout << "# Origin Action   Module                                              Flavour   " << endl;
    vout << "# ------ -------- --------------------------------------------------- ----------" << endl;

    CXMLElement* p_ele;
    p_ele = HostGroup.GetHostsConfigAutoLoadedModules();
    PrintAutoLoadedModules(vout,p_ele,"hostcfg");

    p_ele = HostGroup.GetHostGroupAutoLoadedModules();
    PrintAutoLoadedModules(vout,p_ele,"hostgrp");

    p_ele = GetAutoLoadedModules();
    PrintAutoLoadedModules(vout,p_ele,"site");

    p_ele = AMSRegistry.GetUserAutoLoadedModules();
    PrintAutoLoadedModules(vout,p_ele,"user");
}

//------------------------------------------------------------------------------

void CSite::PrintAutoLoadedModules(CVerboseStr& vout,CXMLElement* p_ele,const CSmallString& origin)
{
    if( p_ele == NULL ) return;
    p_ele = p_ele->GetFirstChildElement("module");

    CSmallString flavour = AMSRegistry.GetUserSiteFlavour();

//  vout << "# Origin Action   Module                                              Flavour   " << endl;
//  vout << "# ------ -------- --------------------------------------------------- ----------" << endl;

    while( p_ele ){
        CSmallString mname,mflavour;
        p_ele->GetAttribute("name",mname);
        p_ele->GetAttribute("flavour",mflavour);
        p_ele = p_ele->GetNextSiblingElement("module");
        if( mname == NULL ) continue;

        vout << setw(8) << origin << " ";

        bool enabled = ModCache.IsAutoloadEnabled(mname);

        if( enabled ){
            if( (mflavour == NULL) || (mflavour == flavour) ){
                vout << setw(8) << "add";
            } else {
                vout << setw(8) << "ignored";
            }
        } else {
            vout << setw(8) << "disabled";
        }

        vout << " " << setw(51) << mname;
        vout << " " << setw(10) << mflavour;
        vout << endl;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CSite::ActivateSite(void)
{
    if( HostGroup.IsSiteAllowed(GetName()) == false ) {
        CSmallString error;
        error << "site (" << GetName()
              << ") is not allowed to be activated on this host group ("
              << HostGroup.GetHostGroupName() << ")";
        ES_TRACE_ERROR(error);
        return(false);
    }

    // set active site ------------------------------
    ShellProcessor.SetVariable("AMS_SITE",GetName());
    SiteController.SetActiveSite(GetName());

    if( GetSupportEMail(false) != NULL ) {
        ShellProcessor.SetVariable("AMS_SITE_SUPPORT",GetSupportEMail(false));
    }

    // boot host environments -----------------------
    CXMLElement* p_env_ele;

    p_env_ele = HostGroup.GetHostsConfigEnvironment();
    PrepareSiteEnvironment(p_env_ele,EMA_ADD_MODULE);

    p_env_ele = HostGroup.GetHostGroupEnvironment();
    PrepareSiteEnvironment(p_env_ele,EMA_ADD_MODULE);

    // boot site environments -----------------------
    p_env_ele = GetSiteEnvironment();
    PrepareSiteEnvironment(p_env_ele,EMA_ADD_MODULE);

    // setup host resources -------------------------
    ShellProcessor.SetVariable("AMS_NHOSTCPUS",Host.GetNumOfHostCPUs());
    ShellProcessor.SetVariable("AMS_NHOSTGPUS",Host.GetNumOfHostGPUs());
    ShellProcessor.SetVariable("AMS_GROUPNS",HostGroup.GetGroupNS());

    // reininitialization ---------------------------

    // initial setup - destroy info about ABS collections
    ShellProcessor.UnsetVariable("INF_COLLECTION_NAME");
    ShellProcessor.UnsetVariable("INF_COLLECTION_PATH");
    ShellProcessor.UnsetVariable("INF_COLLECTION_ID");

    return(true);
}

//------------------------------------------------------------------------------

bool CSite::DeactivateSite(void)
{
    if( IsSiteActive() == false ) {
        ES_ERROR("only active site can be deactivated");
        return(false);
    }

    // unload site environment ----------------------
    CXMLElement* p_env_ele = GetSiteEnvironment();
    PrepareSiteEnvironment(p_env_ele,EMA_REMOVE_MODULE);

    // boot site environments -----------------------
    p_env_ele = HostGroup.GetHostGroupEnvironment();
    PrepareSiteEnvironment(p_env_ele,EMA_REMOVE_MODULE);

    SiteController.SetActiveSite("");
    ShellProcessor.UnsetVariable("AMS_SITE");

    return(true);
}

//------------------------------------------------------------------------------

bool CSite::PrepareSiteEnvironment(CXMLElement* p_build, EModuleAction action)
{
    if( p_build == NULL ) return(true); // nothing to do

    // beginning
    if( ShellProcessor.PrepareModuleEnvironmentForModActionI(p_build) == false ) {
        ES_ERROR("unable to build new environment I");
        return(false);
    }
    // main
    if( ShellProcessor.PrepareModuleEnvironmentForLowPriority(p_build,action) == false ) {
        ES_ERROR("unable to build new environment LP");
        return(false);
    }
    // finalization
    if( ShellProcessor.PrepareModuleEnvironmentForModActionII(p_build) == false ) {
        ES_ERROR("unable to build new environment II");
        return(false);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



