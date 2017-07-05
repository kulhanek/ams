#ifndef SiteH
#define SiteH
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

#include <AMSMainHeader.hpp>
#include <SmallString.hpp>
#include <XMLDocument.hpp>
#include <XMLNode.hpp>

//------------------------------------------------------------------------------

// module is actived by the site command
#define MFB_SYSTEM          (1 << 0)

// module is activated by an user
#define MFB_USER            (1 << 1)

// module is activated within infinity job
#define MFB_INFINITY        (1 << 2)

// module is reactivated
#define MFB_REACTIVATED     (1 << 16)

// module is reexported
#define MFB_REEXPORTED      (1 << 17)

// module was autoloaded by system
#define MFB_SYS_AUTOLOADED  (1 << 18)

// module was autoloaded by user
#define MFB_USER_AUTOLOADED (1 << 19)

//------------------------------------------------------------------------------

class AMS_PACKAGE CSite {
public:
    // constructor and destructor ------------------------------------------------
    CSite(void);
    ~CSite(void);

    // input methods --------------------------------------------------------------
    /// load active site configuration
    bool LoadConfig(void);

    /// load site configuration
    bool LoadConfig(const CSmallString& site_sid);

    // information methods --------------------------------------------------------
    /// return name of the site
    const CSmallString GetName(void) const;

    /// return group desc of the site
    const CSmallString GetGroupDesc(void) const;

    /// return ID of the site
    const CSmallString GetID(void) const;

    /// return organization
    const CSmallString GetOrganizationName(void);

    /// return organization
    const CSmallString GetOrganizationURL(void);

    /// return name of site support name
    const CSmallString GetSupportName(void);

    /// return support contact email
    const CSmallString GetSupportEMail(void);

    /// return name of mailing list
    const CSmallString GetMailingList(void);

    /// return mailing list description
    const CSmallString GetMailingListDesc(void);

    /// return documentation text
    const CSmallString GetDocumentationText(void);

    /// return documentation url
    const CSmallString GetDocumentationURL(void);

    /// return long site description
    CXMLNode* GetSiteDescrXML(void);

    /// is site vidible?
    bool IsSiteVisible(void);

    /// is site adaptive?
    bool IsSiteAdaptive(void);

    /// return short site description
    CXMLElement* GetAutoloadedModules(void);

    /// is site active?
    bool IsActive(void);

    /// can be site activated on this host?
    bool CanBeActivated(void);

    // print information about site -----------------------------------------------
    /// print short info about site
    void PrintShortSiteInfo(std::ostream& vout);

    /// print full info about site
    void PrintFullSiteInfo(std::ostream& vout);

    /// print list of autoloaded modules
    void PrintAutoloadedModules(std::ostream& vout);

    /// print list of autoloaded modules
    void PrintAutoloadedModules(CXMLElement* p_mod_ele,std::ostream& vout);

    // executive methods ----------------------------------------------------------
    /// activate site
    bool ActivateSite(void);

    /// deactivate site
    bool DeactivateSite(void);

    /// do postactions
    bool ExecuteModaction(const CSmallString& action,const CSmallString& args);

    /// activate autoloaded modules
    static bool ActivateAutoloadedModules(CXMLElement* p_mod_ele);

    /// remove incompatible builds for single node site mode
    void RemoveIncompatibleBuilds(void);

    // section of private data ----------------------------------------------------
private:
    CSmallString    SiteID;             // site id
    CXMLDocument    SiteConfig;         // site specific configuration
    CSmallString    ActiveModules;      // active modules
    CSmallString    ExportedModules;    // exported modules

    void PrintAutoloadedModules(FILE* fout,CXMLElement* p_mod_ele);
};

//------------------------------------------------------------------------------

extern CSite    Site;

//------------------------------------------------------------------------------

#endif
