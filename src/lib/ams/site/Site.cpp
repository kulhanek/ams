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

void CSite::GetAutoLoadedModules(std::list<CSmallString>& modules,bool withorigin)
{
    CSmallString flavor = AMSRegistry.GetSiteFlavor();

    CXMLElement* p_ele = Config.GetChildElementByPath("site/autoload");
    if( p_ele ){
        p_ele = p_ele->GetFirstChildElement("module");
    }
    while( p_ele ){
        CSmallString mname,mflavor;
        p_ele->GetAttribute("name",mname);
        p_ele->GetAttribute("flavor",mflavor);
        if( (mname != NULL) && ((mflavor == NULL) || (mflavor == flavor))){
            if( withorigin ){
                mname << "[site:" << GetName();
                if( mflavor != NULL ) mname << "@" << mflavor;
                mname << "]";
                modules.push_back(mname);
            } else {
                modules.push_back(mname);
            }
        }
        p_ele = p_ele->GetNextSiblingElement("module");
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CSite::PrintShortSiteInfo(CVerboseStr& vout)
{
    CSmallString name = GetName();
    CSmallString status;

    if( ! IsSiteActive() ) {
        status =  " (not active)";
    }

    vout << endl;
    vout << "# ~~~ <b>Site Info</b> ";
    CSmallString version;
    version << " [AMS " << LibBuildVersion_AMS_NoDate << "] ";

    for(unsigned int n=18; n < 80 - version.GetLength(); n++) vout << "~";
    vout << version << "~~" << endl;

    vout << "  Site name  : <b><green>" << name << "</green></b>" << status << endl;
    vout << "  Site ID    : " << GetID() << endl;

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
    vout << "# Site config   : " << ConfigFile << endl;
    vout << "# Site flavor   : " << AMSRegistry.GetSiteFlavor() << endl;

    vout << endl;
    vout << "# ~~~ <b>Environment Variables</b> ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    CShellProcessor::PrintBuild(vout,GetSiteEnvironment());

    vout << endl;
    vout << "# ~~~ <b>Autoloaded Modules</b> ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    PrintAutoLoadedModules(vout);
}

//------------------------------------------------------------------------------

void CSite::PrintAutoLoadedModules(CVerboseStr& vout)
{
    CSmallString flavor = AMSRegistry.GetSiteFlavor();

    vout << "# Action Module                                                       Flavor    " << endl;
    vout << "# ------ ------------------------------------------------------------ ----------" << endl;
    CXMLElement* p_ele = Config.GetChildElementByPath("site/autoload");
    if( p_ele ){
        p_ele = p_ele->GetFirstChildElement("module");
    }
    while( p_ele ){
        CSmallString mname,mflavor;
        p_ele->GetAttribute("name",mname);
        p_ele->GetAttribute("flavor",mflavor);
        if( mname != NULL ){
            if( mflavor == NULL ){
                vout << setw(8) << "regular";
            } else if ( mflavor == flavor ){
                vout << setw(8) << "flavor";
            } else {
                vout << setw(8) << "ignored";
            }
            vout << " " << setw(60) << mname;
            vout << " " << setw(10) << mflavor;
            vout << endl;
        }

        p_ele = p_ele->GetNextSiblingElement("module");
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

    // install hooks from site config ---------------
    HostGroup.ExecuteModAction("activate",GetName(),~0);

    // boot host environments -----------------------
    CXMLElement* p_env_ele = HostGroup.GetHostGroupEnvironment();
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



