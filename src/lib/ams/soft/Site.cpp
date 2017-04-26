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

#include <Site.hpp>
#include <FileName.hpp>
#include <XMLParser.hpp>
#include <XMLElement.hpp>
#include <ErrorSystem.hpp>
#include <Shell.hpp>
#include <AMSGlobalConfig.hpp>
#include <XMLIterator.hpp>
#include <fnmatch.h>
#include <XMLText.hpp>
#include <ShellProcessor.hpp>
#include <Actions.hpp>
#include <Cache.hpp>
#include <string.h>
#include <Terminal.hpp>
#include <Utils.hpp>
#include <AMSUserConfig.hpp>
#include <User.hpp>
#include <Host.hpp>
#include <iomanip>
#include <boost/format.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <PrintEngine.hpp>

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

bool CSite::LoadConfig(void)
{
    if( AMSGlobalConfig.GetActiveSiteID() == NULL ) {
        ES_ERROR("no site is active");
        return(false);
    }

    return(LoadConfig(AMSGlobalConfig.GetActiveSiteID()));
}

//------------------------------------------------------------------------------

bool CSite::LoadConfig(const CSmallString& site_id)
{
    if( site_id == NULL ){
        ES_WARNING("site_id is NULL");
        return(false);
    }

    CFileName    config_path;
    SiteConfig.RemoveAllChildNodes();

    config_path = AMSGlobalConfig.GetETCDIR();
    config_path = config_path / "sites" / site_id / "site.xml";

    CXMLParser xml_parser;
    xml_parser.SetOutputXMLNode(&SiteConfig);

    if( xml_parser.Parse(config_path) == false ) {
        CSmallString error;
        error << "unable to load site '" << site_id << "' config";
        ES_ERROR(error);
        return(false);
    }

    SiteID = site_id;

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CSite::GetName(void) const
{
    CSmallString name;
    CXMLElement* p_ele = SiteConfig.GetChildElementByPath("site");
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

const CSmallString CSite::GetGroupDesc(void) const
{
    CSmallString name;
    CXMLElement* p_ele = SiteConfig.GetChildElementByPath("site/description/group");
    if( p_ele == NULL ) {
        ES_ERROR("unable to open site/description/group");
        return(name);
    }
    CXMLText* p_text = p_ele->GetFirstChildText();
    if( p_text == NULL ) {
        ES_ERROR("unable to get text from element");
        return(name);
    }
    name = p_text->GetText();
    return(name);
}

//------------------------------------------------------------------------------

const CSmallString CSite::GetID(void) const
{
    CSmallString id;
    CXMLElement* p_ele = SiteConfig.GetChildElementByPath("site");
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

const CSmallString CSite::GetOrganizationName(void)
{
    CSmallString name;
    CXMLElement* p_ele = SiteConfig.GetChildElementByPath("site/description/organization");
    if( p_ele == NULL ) {
        ES_ERROR("unable to open site/description/organization path");
        return(name);
    }
    CXMLText* p_text = p_ele->GetFirstChildText();
    if( p_text == NULL ) {
        ES_ERROR("unable to get text from element");
        return(name);
    }
    name = p_text->GetText();
    return(name);
}

//------------------------------------------------------------------------------

const CSmallString CSite::GetOrganizationURL(void)
{
    CSmallString name;
    CXMLElement* p_ele = SiteConfig.GetChildElementByPath("site/description/organization");
    if( p_ele == NULL ) {
        ES_ERROR("unable to open site/description/organization path");
        return(name);
    }

    if( p_ele->GetAttribute("url",name) ) {
        ES_ERROR("unable to get url attribute");
        return(name);
    }
    return(name);
}

//------------------------------------------------------------------------------

const CSmallString CSite::GetSupportName(void)
{
    CSmallString name;
    CXMLElement* p_ele = SiteConfig.GetChildElementByPath("site/description/support");
    if( p_ele == NULL ) {
        return(name);
    }
    CXMLText* p_text = p_ele->GetFirstChildText();
    if( p_text == NULL ) {
        return(name);
    }
    name = p_text->GetText();
    return(name);
}

//------------------------------------------------------------------------------

const CSmallString CSite::GetSupportEMail(void)
{
    CSmallString name;
    CXMLElement* p_ele = SiteConfig.GetChildElementByPath("site/description/support");
    if( p_ele == NULL ) {
        return(name);
    }

    if( p_ele->GetAttribute("email",name) ) {
        return(name);
    }
    return(name);
}

//------------------------------------------------------------------------------

const CSmallString CSite::GetMailingList(void)
{
    CSmallString name;
    CXMLElement* p_ele = SiteConfig.GetChildElementByPath("site/description/mailinglist");
    if( p_ele == NULL ) {
        return(name);
    }

    if( p_ele->GetAttribute("email",name) ) {
        return(name);
    }
    return(name);
}

//------------------------------------------------------------------------------

const CSmallString CSite::GetMailingListDesc(void)
{
    CSmallString name;
    CXMLElement* p_ele = SiteConfig.GetChildElementByPath("site/description/mailinglist");
    if( p_ele == NULL ) {
        return(name);
    }
    CXMLText* p_text = p_ele->GetFirstChildText();
    if( p_text == NULL ) {
        return(name);
    }
    name = p_text->GetText();
    return(name);
}

//------------------------------------------------------------------------------

const CSmallString CSite::GetDocumentationText(void)
{
    CSmallString name;
    CXMLElement* p_ele = SiteConfig.GetChildElementByPath("site/description/documentation");
    if( p_ele == NULL ) {
        return(name);
    }
    CXMLText* p_text = p_ele->GetFirstChildText();
    if( p_text == NULL ) {
        return(name);
    }
    name = p_text->GetText();
    return(name);
}

//------------------------------------------------------------------------------

const CSmallString CSite::GetDocumentationURL(void)
{
    CSmallString name;
    CXMLElement* p_ele = SiteConfig.GetChildElementByPath("site/description/documentation");
    if( p_ele == NULL ) {
        return(name);
    }
    if( p_ele->GetAttribute("url",name) ) {
        return(name);
    }
    return(name);
}

//------------------------------------------------------------------------------

CXMLNode* CSite::GetSiteDescrXML(void)
{
    CXMLElement* p_ele = SiteConfig.GetChildElementByPath("site/description/text");
    if( p_ele == NULL ) {
        return(NULL);
    }
    return(p_ele);
}

//------------------------------------------------------------------------------

bool CSite::IsSiteVisible(void)
{
    CXMLElement* p_ele = SiteConfig.GetChildElementByPath("site");
    if( p_ele == NULL ) {
        return(true);
    }
    bool visible = true;
    p_ele->GetAttribute("visible",visible);
    return(visible);
}

//------------------------------------------------------------------------------

bool CSite::IsActive(void)
{
    // get active site id
    CSmallString active_site = AMSGlobalConfig.GetActiveSiteID();

    // compare with site id
    return((active_site == GetID()) && (active_site.GetLength() > 0));
}

//------------------------------------------------------------------------------

bool CSite::IsSiteAdaptive(void)
{
    CXMLElement* p_ele = SiteConfig.GetChildElementByPath("site");
    if( p_ele == NULL ) {
        return(true);
    }
    bool adaptive = false;
    p_ele->GetAttribute("adaptive",adaptive);
    return(adaptive);
}

//------------------------------------------------------------------------------

bool CSite::CanBeActivated(void)
{
    CSmallString host_name = Host.GetHostName();
    CXMLElement* p_ele = Host.FindGroup();
    if( p_ele == NULL ){
        CSmallString error;
        error << "no host group found - site '" << GetName() << "' is not allowed on host '" << host_name <<"'";
        ES_TRACE_ERROR(error);
        return(false);
    }
    CSmallString name;
    p_ele->GetAttribute("name",name);

    CXMLElement* p_sele = p_ele->GetFirstChildElement("sites");
    if( p_sele == NULL ){
        CSmallString error;
        error << "no 'sites'' element found for the group '" << name << "' - site '" << GetName() << "' is not allowed on host '" << host_name <<"'";
        ES_TRACE_ERROR(error);
        return(false);
    }

    string primary,transferable,others;
    p_sele->GetAttribute("primary",primary);
    p_sele->GetAttribute("transferable",transferable);
    p_sele->GetAttribute("others",others);

    // split into tokens
    std::vector<std::string> sites;
    std::vector<std::string> splits;
    split(splits,primary,is_any_of(","));
    sites.insert(sites.end(),splits.begin(),splits.end());

    split(splits,transferable,is_any_of(","));
    sites.insert(sites.end(),splits.begin(),splits.end());

    split(splits,others,is_any_of(","));
    sites.insert(sites.end(),splits.begin(),splits.end());

    // is allowed?
    if(std::find(sites.begin(), sites.end(), string(GetName())) == sites.end()) {
        CSmallString error;
        error << "site '" << GetName() << "' is not allowed on host '" << host_name <<"'";
        ES_TRACE_ERROR(error);
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

CXMLElement* CSite::GetAutoloadedModules(void)
{
    return(SiteConfig.GetChildElementByPath("site/autoload"));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CSite::PrintShortSiteInfo(ostream& vout)
{
    CSmallString name = GetName();
    CSmallString status;

    if( IsActive() ) {
        if( IsSiteAdaptive() ) {
            status << " [adaptive]";
        }
    } else {
        status =  "is not active";
    }

    CSmallString version;
    version << " [AMS " << LibBuildVersion_AMS_NoDate << "] ";

    vout << endl;
    vout << "# ";
    for(unsigned int n=4; n < 80 - version.GetLength(); n++) vout << "~";
    vout << version << "~~" << endl;

    int  n1 = (80 - 8 - status.GetLength() - name.GetLength())/2;
    vout << "# Site info ";
    for(int i=12; i < n1; i++) vout << " ";
    vout << "||| <b><green>" << name << "</green></b>" << status << " |||";


    // user and host info
    vout << endl;
    vout << "# ~~~ User identification";
    for(int n=25; n < 80;n++) vout << "~";
    vout << endl;

    vout << "# User name  : " << User.GetName() << endl;
    vout << "# User group : " << User.GetEGroup() << " [umask: " << User.GetUMask() << " " << User.GetUMaskPermissions() << "]" << endl;
    CPrintEngine::PrintTokens(vout,"# ACL groups : ",User.GetGroups());

    if( IsActive() ) {
    vout << "#" << endl;
    if( Host.IsLoadedFromCache() ){
    vout << "# ~~~ Host info ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ [cached] ~~" << endl;
    } else {
    vout << "# ~~~ Host info ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    }
    vout << "# Full host name     : " << Host.GetHostName() << endl;
    vout << "# Num of host CPUs   : " << Host.GetNumOfHostCPUs() << endl;
    vout << "# Host SMP CPU model : " << Host.GetCPUModel() << endl;
        if( Host.GetNumOfHostGPUs() > 0 ){
    vout << "# Num of host GPUs   : " << Host.GetNumOfHostGPUs() << endl;
    if( Host.IsGPUModelSMP() == false ){
    for(size_t i=0; i < Host.GetGPUModels().size(); i++){
    vout << "# Host GPU model #" << setw(1) << i+1 << "  : " << Host.GetGPUModels()[i] << endl;
    }
    } else {
    vout << "# Host SMP GPU model : " << Host.GetGPUModels()[0] << endl;
    }
        }
    CPrintEngine::PrintTokens(vout,"# Host arch tokens   : ",Host.GetArchTokens());
    }

    if( (GetDocumentationURL() != NULL) ||
            (GetSupportName() != NULL) ||
            (GetSupportEMail() != NULL) ) {
        vout << "#" << endl;
        vout << "# ~~~ Site documentation and support ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
        if( GetDocumentationURL() != NULL ) {
            vout << "# Documentation  : " << GetDocumentationURL() << endl;
        }
        if( (GetMailingList() != NULL) && (GetMailingListDesc() != NULL) ) {
            vout << "# Mailing list   : " << GetMailingList() << " [" << GetMailingListDesc() << "]" << endl;
        }
        if( (GetMailingList() != NULL) && (GetMailingListDesc() == NULL) ) {
            vout << "# Mailing list   : " << GetMailingList() << endl;
        }
        if( (GetSupportEMail() != NULL) && (GetSupportName() != NULL) ) {
            vout << "# Support e-mail : <b><green>" << GetSupportEMail() << "</green></b> [" << GetSupportName() << "]" << endl;
        }
        if( (GetSupportEMail() != NULL) && (GetSupportName() == NULL) ) {
            vout << "# Support e-mail : <b><green>" << GetSupportEMail() << "</green></b>" << endl;
        }
    }

    vout << "# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
    vout << endl;
}

//------------------------------------------------------------------------------

void CSite::PrintFullSiteInfo(std::ostream& vout)
{
    PrintShortSiteInfo(vout);
    vout << endl;

    // print further info -----------------------
    CXMLElement* p_env_ele = SiteConfig.GetChildElementByPath("site/environment");

    CUtils::PrintBuild(vout,p_env_ele);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CSite::ActivateSite(void)
{
    if( CanBeActivated() == false ) {
        CSmallString error;
        error << "site (" << GetName() << ") is not allowed to be activated on this host (";
        error << Host.GetHostName() << ")";
        ES_TRACE_ERROR(error);
        return(false);
    }

    // set active site id ---------------------------
    AMSGlobalConfig.SetActiveSiteID(SiteID);
    ShellProcessor.SetVariable("AMS_SITE",SiteID);

    if( GetSupportEMail() != NULL ) {
        ShellProcessor.SetVariable("AMS_SITE_SUPPORT",GetSupportEMail());
    }

    // initialize hosts -----------------------------
    Host.ClearAll();
    Host.InitGlobalSetup();

    User.ClearAll();
    User.InitGlobalSetup();

    Host.InitHostFile();
    Host.InitHost();

    User.InitUserFile(SiteID);
    User.InitUser();

    // boot site environments -----------------------
    CXMLElement* p_env_ele = SiteConfig.GetChildElementByPath("site/environment");
    if( p_env_ele != NULL ) {
        // beginning
        if( ShellProcessor.PrepareModuleEnvironmentForModActionI(p_env_ele) == false ) {
            ES_ERROR("unable to build new environment I");
            return(false);
        }
        // install hooks from site config ------------
        ExecuteModaction("activate","\""+SiteID+"\"");
        // main
        if( ShellProcessor.PrepareModuleEnvironmentForLowPriority(p_env_ele,true) == false ) {
            ES_ERROR("unable to build new environment LP");
            return(false);
        }
        // finalization
        if( ShellProcessor.PrepareModuleEnvironmentForModActionII(p_env_ele) == false ) {
            ES_ERROR("unable to build new environment II");
            return(false);
        }
    }

    // setup host resources
    ShellProcessor.SetVariable("AMS_NHOSTCPUS",Host.GetNumOfHostCPUs());
    ShellProcessor.SetVariable("AMS_NHOSTGPUS",Host.GetNumOfHostGPUs());

    CSmallString groupns;
    CXMLElement* p_gele = Host.FindGroup();
    if( p_gele != NULL ){
        p_gele->GetAttribute("groupns",groupns);
    }
    ShellProcessor.SetVariable("AMS_GROUPNS",groupns);

    // reactivate cache, actions, and user config -----------------

    // initialze AMS cache
    if( Cache.LoadCache() == false) {
        ES_ERROR("unable to load AMS cache");
        return(false);
    }

    // initialize user config
    // it is not possible to load user config because paths to the setup might be set
    // by created above and thus not yet available
    // AMSUserConfig.LoadUserConfig();

    // initial setup - no module is active
    ShellProcessor.UnsetVariable("AMS_ACTIVE_MODULES");
    ShellProcessor.UnsetVariable("AMS_EXPORTED_MODULES");

    // initial setup - destroy info about ABS collections
    ShellProcessor.UnsetVariable("INF_COLLECTION_NAME");
    ShellProcessor.UnsetVariable("INF_COLLECTION_PATH");
    ShellProcessor.UnsetVariable("INF_COLLECTION_ID");

    // load automodules -----------------------------
    CXMLElement* p_mod_ele;
    // this can be performed here - setup is in site config file
    if( AMSUserConfig.AreSystemAutoloadedModulesDisabled() == false ) {
        p_mod_ele = SiteConfig.GetChildElementByPath("site/autoload");
        Actions.SetFlags(Actions.GetFlags() | MFB_SYS_AUTOLOADED);
        if( ActivateAutoloadedModules(p_mod_ele) == false ) {
            ES_ERROR("unable to load system auto-loaded modules");
            return(false);
        }
    }

//    p_mod_ele = AMSUserConfig.GetAutoloadedModules();
//    Actions.SetFlags(Actions.GetFlags() ^ MFB_SYS_AUTOLOADED);
//    Actions.SetFlags(Actions.GetFlags() | MFB_USER_AUTOLOADED);
//    if( ActivateAutoloadedModules(p_mod_ele) == false ) {
//        ES_WARNING("unable to load user auto-loaded modules");
//        return(true);
//    }
    // this must be done by calling "amsmodule autoload"
    ShellProcessor.RegisterScript("amsmodule","autoload",EST_CHILD);

    return(true);
}

//------------------------------------------------------------------------------

bool CSite::DeactivateSite(void)
{
    if( SiteID != AMSGlobalConfig.GetActiveSiteID() ) {
        ES_ERROR("only active site can be deactivated");
        return(false);
    }

    // initialze AMS cache
    if( Cache.LoadCache() == false) {
        ES_ERROR("unable to load AMS cache");
        return(false);
    }

    // unload active modules ------------------------
    CSmallString tmp(AMSGlobalConfig.GetActiveModules());

    Actions.SetActionPrintLevel(EAPL_NONE);

    CVerboseStr vout;

    std::list<CSmallString> modules;

    char* p_str;
    char* p_strtok = NULL;

    p_str = strtok_r(tmp.GetBuffer(),"|",&p_strtok);
    while( p_str != NULL ) {
        modules.push_back(CSmallString(p_str));
        p_str = strtok_r(NULL,"|",&p_strtok);
    }

    modules.reverse();

    std::list<CSmallString>::iterator it = modules.begin();
    std::list<CSmallString>::iterator ie = modules.end();

    while( it != ie ){
        CSmallString module = *it;
        if( Actions.RemoveModule(vout,module) != EAE_STATUS_OK ) {
            CSmallString error;
            error << "unable to remove module '" << module << "'";
            ES_WARNING(error);
        }
        it++;
    }

    // just be sure
    ShellProcessor.UnsetVariable("AMS_ACTIVE_MODULES");
    ShellProcessor.UnsetVariable("AMS_EXPORTED_MODULES");


    // unload site environment ----------------------
    CXMLElement* p_env_ele = SiteConfig.GetChildElementByPath("site/environment");
    if( p_env_ele != NULL ) {
        // beginning
        if( ShellProcessor.PrepareModuleEnvironmentForModActionI(p_env_ele) == false ) {
            ES_ERROR("unable to build new environment I");
            return(false);
        }
        // install hooks from site config ------------
        ExecuteModaction("deactivate","\""+SiteID+"\"");
        // main
        if( ShellProcessor.PrepareModuleEnvironmentForLowPriority(p_env_ele,false) == false ) {
            ES_ERROR("unable to build new environment LP");
            return(false);
        }
        // finalization
        if( ShellProcessor.PrepareModuleEnvironmentForModActionII(p_env_ele) == false ) {
            ES_ERROR("unable to build new environment II");
            return(false);
        }
    }

    AMSGlobalConfig.SetActiveSiteID("");
    ShellProcessor.UnsetVariable("AMS_SITE");

    return(true);
}

//------------------------------------------------------------------------------

bool CSite::ExecuteModaction(const CSmallString& action,
                             const CSmallString& args)
{
    CXMLElement* p_ele = SiteConfig.GetChildElementByPath("site/actions");
    if( p_ele == NULL ) {
        // no actions -> return
        ES_WARNING("no actions are define");
        return(true);
    }

    CXMLIterator I(p_ele);
    CXMLElement* p_cele;

    while((p_cele = I.GetNextChildElement("action")) != NULL ) {
        CSmallString laction;
        if( p_cele->GetAttribute("name",laction) == false ) continue;
        if( laction != action ) continue;

        // action found - get the remaining specification
        CSmallString lcommand,ltype,largs;
        if( p_cele->GetAttribute("command",lcommand) == false ) {
            CSmallString error;
            error << "action '" << action << "' found but command is not provided";
            ES_ERROR(error);
            return(false);
        }
        p_cele->GetAttribute("type",ltype);
        p_cele->GetAttribute("args",largs);

        // complete entire comand
        CFileName full_command;
        full_command = AMSGlobalConfig.GetAMSRootDir() / "bin" / "actions" / lcommand;

        CFileName full_arguments;
        full_arguments = largs + " " + args;

        if( ltype == "inline" ) {
            ShellProcessor.RegisterScript(full_command,full_arguments,EST_INLINE);
        } else if( ltype == "child" ) {
            ShellProcessor.RegisterScript(full_command,full_arguments,EST_CHILD);
        } else {
            CSmallString error;
            error << "unsupported script type '" << ltype << "'";
            ES_ERROR(error);
            return(false);
        }
    }

// remove false warnings for remove action
//    if( found == false ) {
//        CSmallString warning;
//        warning << "action (" << action << ") was not processed";
//        ES_WARNING(warning);
//    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CSite::PrintAutoloadedModules(FILE* fout,CXMLElement* p_mod_ele)
{
    int mcount = 0;
    CXMLIterator    I(p_mod_ele);
    CXMLElement*    p_mod;

    while((p_mod = I.GetNextChildElement("module")) != NULL) {
        CSmallString mod_name;
        bool         mod_exp = false;

        p_mod->GetAttribute("name",mod_name);
        p_mod->GetAttribute("export",mod_exp);

        if( mcount > 0 ) fprintf(fout,"|");

        if( mod_exp ) {
            fprintf(fout,"%s*",(const char*)mod_name);
        } else {
            fprintf(fout,"%s",(const char*)mod_name);
        }
        mcount++;
    }

    if( mcount == 0 ) fprintf(fout,"-none-");
    fprintf(fout,"\n");
}

//------------------------------------------------------------------------------

bool CSite::ActivateAutoloadedModules(CXMLElement* p_mod_ele)
{
    CXMLIterator    I(p_mod_ele);
    CXMLElement*    p_mod;

    Actions.SetActionPrintLevel(EAPL_NONE);

    CVerboseStr vout;
    vout << high;

    while((p_mod = I.GetNextChildElement("module")) != NULL) {
        CSmallString mod_name;
        bool         mod_exp = true;

        p_mod->GetAttribute("name",mod_name);
        p_mod->GetAttribute("export",mod_exp);

        Actions.SetModuleExportFlag(mod_exp);

        if( Actions.AddModule(vout,mod_name) != EAE_STATUS_OK ) {
            CSmallString error;
            error << "unable to add module '" << mod_name << "'";
            ES_WARNING(error);
        }
    }

    return(true);
}

//------------------------------------------------------------------------------

void CSite::RemoveIncompatibleBuilds(void)
{
    CXMLElement* p_ele = SiteConfig.GetChildElementByPath("site/adaptive");
    if( p_ele == NULL ) {
        // no filters - exit
        return;
    }

    CXMLIterator    I(p_ele);
    CXMLElement*    p_filter;

    while((p_filter = I.GetNextChildElement("filter")) != NULL) {
        CSmallString token;
        p_filter->GetAttribute("token",token);

        // is token present for node host
        if( Host.HasToken(token) == true ) continue;

        // if not remove all builds that contains it
        Cache.RemoveBuildsWithArchToken(token);
    }

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



