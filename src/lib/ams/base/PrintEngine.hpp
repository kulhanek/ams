#ifndef PrintEngineH
#define PrintEngineH
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
#include <FileName.hpp>
#include <XMLDocument.hpp>
#include <Terminal.hpp>
#include <list>

//------------------------------------------------------------------------------

enum EPrintEngineHeaderScope {
    EPEHS_SITE      = 0,
    EPEHS_SECTION   = 1,
    EPEHS_CATEGORY  = 2,
};

//------------------------------------------------------------------------------

class AMS_PACKAGE CPrintEngine {
public:
// constructor and destructor --------------------------------------------------
    CPrintEngine(void);

// setup methods ---------------------------------------------------------------
    /// init print profile
    void InitPrintProfile(void);

// print methods ---------------------------------------------------------------
    /// print header
    void PrintHeader(CTerminal& terminal,const CSmallString& title,EPrintEngineHeaderScope scope);

    /// print items
    void PrintItems(CTerminal& terminal,std::list<CSmallString>& list);

// get setup -------------------------------------------------------------------
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

// section of private data -----------------------------------------------------
private:
    CFileName    PrintProfileFile;
    CXMLDocument PrintProfile;      // print engine configuration
};

//-----------------------------------------------------------------------------

extern CPrintEngine PrintEngine;

//-----------------------------------------------------------------------------

#endif
