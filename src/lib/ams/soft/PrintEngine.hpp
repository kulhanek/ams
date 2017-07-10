#ifndef ModulePrintEngineH
#define ModulePrintEngineH
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
#include <VerboseStr.hpp>
#include <Terminal.hpp>
#include <TemplateParams.hpp>

//------------------------------------------------------------------------------

class AMS_PACKAGE CPrintEngine {
public:
// constructor and destructor --------------------------------------------------
    CPrintEngine(void);

// input/output methods --------------------------------------------------------
    /// load print engine config
    bool LoadConfig(void);

    /// load system wide configuration
    bool LoadSystemConfig(void);

    /// load user wide configuration
    bool LoadUserConfig(void);

    /// save user wide configuration
    bool SaveUserConfig(void);

    /// set user config from system one
    bool SetUserConfigFromSystem(void);

    /// clear user config
    void ClearUserConfig(void);

    /// get root element of print config
    CXMLElement* GetRootElementOfConfig(void);

    /// set output stream
    void SetOutputStream(CVerboseStr& fout);

    /// print comma delimited tokens
    static void PrintTokens(std::ostream& sout,const CSmallString& title, const CSmallString& res_list);

// raw print output -----------------------------------------------------------
    void PrintRawAllModules(void);
    void PrintRawAllBuilds(void);
    void PrintRawBuilds(const CSmallString& filter);
    void PrintRawCategories(void);
    bool PrintRawModulesInCategory(const CSmallString& cat_name,bool include_vers);
    bool PrintRawModuleVersions(const CSmallString& mod_name);   
    bool PrintRawModuleBuilds(const CSmallString& mod_name);
    bool PrintRawModuleDisp(const CSmallString& mod_name);
    bool PrintRawModuleDefault(const CSmallString& mod_name);
    bool PrintRawVariable(const CSmallString& build,const CSmallString& var_name);
    void PrintRawSites(void);
    void PrintRawSystemVersion(void);
    void PrintRawDependencies(void);
    void PrintRawArchs(void);
    void PrintRawModes(void);

// help print output -----------------------------------------------------------
    void StartHelp(void);
    bool AddHelp(const CSmallString& mod_name);
    bool ShowHelp(void);

// color print output ----------------------------------------------------------
    bool PrintModModuleVersions(const CSmallString& mod_name);
    bool PrintModModuleBuilds(const CSmallString& mod_name);
    bool PrintModModuleInfo(const CSmallString& mod_name);
    void PrintModAvailableModules(CTerminal& terminal,bool include_system=true,bool include_version=false);
    void PrintModActiveModules(CTerminal& terminal);
    void PrintModExportedModules(CTerminal& terminal);

    void PrintAMSAvailHeader(CTerminal& terminal);
    void PrintSYSAvailHeader(CTerminal& terminal);

// color print output ----------------------------------------------------------
    void PrintAvailableSites(CTerminal& terminal,bool print_all,bool plain);

// web data providers
    void ListModAvailableModules(CTemplateParams& params,bool include_vers);

// configuration ---------------------------------------------------------------
    // setup methods -----------------
    bool AreColorsEnabled(void);
    char GetSiteSectionDelimiter(void);
    int  GetSiteSectionBgColor(void);
    int  GetSiteSectionFgColor(void);
    char GetSectionDelimiter(void);
    int  GetSectionBgColor(void);
    int  GetSectionFgColor(void);
    char GetCategoryDelimiter(void);
    int  GetCategoryBgColor(void);
    int  GetCategoryFgColor(void);
    int  GetModuleBgColor(void);
    int  GetModuleFgColor(void);
    bool IncludeVersion(void);

    bool AreColorsEnabled(bool& usersetup);
    char GetSiteSectionDelimiter(bool& usersetup);
    int  GetSiteSectionBgColor(bool& usersetup);
    int  GetSiteSectionFgColor(bool& usersetup);
    char GetSectionDelimiter(bool& usersetup);
    int  GetSectionBgColor(bool& usersetup);
    int  GetSectionFgColor(bool& usersetup);
    char GetCategoryDelimiter(bool& usersetup);
    int  GetCategoryBgColor(bool& usersetup);
    int  GetCategoryFgColor(bool& usersetup);
    int  GetModuleBgColor(bool& usersetup);
    int  GetModuleFgColor(bool& usersetup);
    bool IncludeVersion(bool& usersetup);

    // setup methods -----------------
    void EnableColors(bool set);
    void SetSiteSectionDelimiter(char delim);
    void SetSiteSectionBgColor(int color);
    void SetSiteSectionFgColor(int color);
    void SetSectionDelimiter(char delim);
    void SetSectionBgColor(int color);
    void SetSectionFgColor(int color);
    void SetCategoryDelimiter(char delim);
    void SetCategoryBgColor(int color);
    void SetCategoryFgColor(int color);
    void SetModuleBgColor(int color);
    void SetModuleFgColor(int color);
    void SetIncludeVersion(bool set);

// section of private data -----------------------------------------------------
private:
    CVerboseStr     vout;
    CXMLDocument    SystemConfig;   // print engine configuration
    CXMLDocument    UserConfig;     // user print configuration
    CXMLDocument    HTMLHelp;       // module help document
};

//-----------------------------------------------------------------------------

extern CPrintEngine PrintEngine;

//-----------------------------------------------------------------------------

#endif
