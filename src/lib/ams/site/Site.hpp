#ifndef SiteH
#define SiteH
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

#include <AMSMainHeader.hpp>
#include <SmallString.hpp>
#include <FileName.hpp>
#include <VerboseStr.hpp>
#include <XMLDocument.hpp>
#include <Module.hpp>
#include <boost/shared_ptr.hpp>
#include <list>

//------------------------------------------------------------------------------

class AMS_PACKAGE CSite {
public:
// constructor and destructor --------------------------------------------------
    CSite(void);
    ~CSite(void);

// setup methods ---------------------------------------------------------------
    /// load site configuration
    bool LoadConfig(const CSmallString& config_file);

// information methods ---------------------------------------------------------
    /// return name of the site
    const CSmallString GetName(void) const;

    /// return ID of the site
    const CSmallString GetID(void) const;

    /// return documentation
    const CSmallString GetDocumentationURL(void);

    /// return support contact email
    const CSmallString GetSupportEMail(bool incname);

    /// is site active?
    bool IsSiteActive(void);

    /// return short site description, these module are not exported
    void GetAutoLoadedModules(std::list<CSmallString>& modules,
                              bool withorigin=false,bool personal=false);

    /// get autoloaded modules
    CXMLElement* GetAutoLoadedModules(void);

    /// get site environment block
    CXMLElement* GetSiteEnvironment(void);

// print information about site -----------------------------------------------
    /// print short info about site
    void PrintShortSiteInfo(CVerboseStr& vout);

    /// print full info about site
    void PrintFullSiteInfo(CVerboseStr& vout);

    /// print list of autoloaded modules
    void PrintAutoLoadedModules(CVerboseStr& vout);

// actions ---------------------------------------------------------------------
    /// activate site - autoloaded modules must be handled later
    bool ActivateSite(void);

    /// deactivate site - autoloaded module must be handled sooner
    bool DeactivateSite(void);

// section of private data -----------------------------------------------------
private:
    CFileName       ConfigFile;     // file name with the site configuration
    CXMLDocument    Config;         // site specific configuration

    /// prepare site environment
    bool PrepareSiteEnvironment(CXMLElement* p_build, EModuleAction action);

    /// print autoloaded modules
    void PrintAutoLoadedModules(CVerboseStr& vout,CXMLElement* p_ele,const CSmallString& origin);
};

//------------------------------------------------------------------------------

typedef boost::shared_ptr<CSite>   CSitePtr;

//------------------------------------------------------------------------------

#endif
