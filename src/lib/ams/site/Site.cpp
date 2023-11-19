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
#include <iomanip>
#include <string.h>
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

bool CSite::LoadConfig(const CSmallString& site_id)
{
    if( site_id == NULL ){
        ES_WARNING("site_id is NULL");
        return(false);
    }

    CFileName    config_path;
    Config.RemoveAllChildNodes();

    config_path = AMSRegistry.GetETCDIR();
    config_path = config_path / "sites" / site_id / "site.xml";

    CXMLParser xml_parser;
    xml_parser.SetOutputXMLNode(&Config);

    if( xml_parser.Parse(config_path) == false ) {
        CSmallString error;
        error << "unable to load site '" << site_id << "' config";
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
    if( p_ele->GetAttribute("url",name) ) {
        return(name);
    }
    return(name);
}

//------------------------------------------------------------------------------

const CSmallString CSite::GetSupportEMail(void)
{
    CSmallString name;
    CXMLElement* p_ele = Config.GetChildElementByPath("site/description/support");
    if( p_ele == NULL ) {
        return(name);
    }

    if( p_ele->GetAttribute("email",name) ) {
        return(name);
    }
    return(name);
}

//------------------------------------------------------------------------------

bool CSite::IsSiteActive(void)
{
    // FIXME
   return(false);
}

//------------------------------------------------------------------------------

bool CSite::IsSiteAdaptive(void)
{
    CXMLElement* p_ele = Config.GetChildElementByPath("site");
    if( p_ele == NULL ) {
        return(true);
    }
    bool adaptive = false;
    p_ele->GetAttribute("adaptive",adaptive);
    return(adaptive);
}

//------------------------------------------------------------------------------

CXMLElement* CSite::GetSiteEnvoronment(void)
{
    return( Config.GetChildElementByPath("site/environment") );
}

//------------------------------------------------------------------------------

void CSite::GetAutoloadedModules(std::list<CSmallString>& modules, const CSmallString& flavor)
{
    // FIXME
   // return(Config.GetChildElementByPath("site/autoload"));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CSite::PrintShortSiteInfo(CVerboseStr& vout)
{
    CSmallString name = GetName();
    CSmallString status;

    if( IsSiteActive() ) {
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

    User.PrintUserInfoForSite(vout);
    Host.PrintHostInfoForSite(vout);

    if( (GetDocumentationURL() != NULL) || (GetSupportEMail() != NULL) ) {
        vout << "#" << endl;
        vout << "# ~~~ Site documentation and support ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
        if( GetDocumentationURL() != NULL ) {
            vout << "# Documentation  : " << GetDocumentationURL() << endl;
        }
        if( GetSupportEMail() != NULL ) {
            vout << "# Support e-mail : <b><green>" << GetSupportEMail() << "</green></b>" << endl;
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
    CShellProcessor::PrintBuild(vout,GetSiteEnvoronment());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



