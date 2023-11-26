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

#include <PrintEngine.hpp>
#include <XMLElement.hpp>
#include <AMSRegistry.hpp>
#include <XMLParser.hpp>
#include <FileSystem.hpp>
#include <ErrorSystem.hpp>

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CPrintEngine PrintEngine;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CPrintEngine::CPrintEngine(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CPrintEngine::InitPrintProfile(void)
{
    PrintProfileFile = AMSRegistry.GetPrintProfileFile();

    if( CFileSystem::IsFile(PrintProfileFile) == false ){
        // no print profile file
        CSmallString warning;
        warning << "no print profile file '" << PrintProfileFile << "'";
        ES_WARNING(warning);
        return;
    }

    CXMLParser xml_parser;
    xml_parser.SetOutputXMLNode(&PrintProfile);
    if( xml_parser.Parse(PrintProfileFile) == false ){
        ErrorSystem.RemoveAllErrors(); // avoid global error
        CSmallString warning;
        warning << "unable to parse print profile file '" << PrintProfileFile << "'";
        ES_WARNING(warning);
        return;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CPrintEngine::PrintHeader(CTerminal& terminal,const CSmallString& title,EPrintEngineHeaderScope scope)
{
    switch(scope){
        case(EPEHS_SITE):
            terminal.Printf("\n");
            terminal.SetColors(GetSiteSectionFgColor(),
                               GetSiteSectionBgColor());
            terminal.SetBold();
            terminal.PrintTitle(title,GetSiteSectionDelimiter(),3);
            terminal.SetDefault();
            terminal.Printf("\n");
        break;
        case(EPEHS_SECTION):
            terminal.Printf("\n");
            terminal.SetColors(GetSectionFgColor(),
                               GetSectionBgColor());
            terminal.SetBold();
            terminal.PrintTitle(title,GetSectionDelimiter(),3);
            terminal.SetDefault();
            terminal.Printf("\n");
        break;
        case(EPEHS_CATEGORY):
            terminal.Printf("\n");
            terminal.SetColors(GetCategoryFgColor(),
                               GetCategoryBgColor());
            terminal.SetBold();
            terminal.PrintTitle(title,GetCategoryDelimiter(),3);
            terminal.SetDefault();
            terminal.Printf("\n");
        break;

    }
}

//------------------------------------------------------------------------------

void CPrintEngine::PrintItems(CTerminal& terminal,std::list<CSmallString>& list)
{
    terminal.SetColors(GetModuleFgColor(),GetModuleBgColor());
    terminal.PrintColumnSortedList(list);
    terminal.SetDefault();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CPrintEngine::AreColorsEnabled(void)
{
    bool setup = false;
    CXMLElement* p_ele = PrintProfile.GetChildElementByPath("print/config");
    if( p_ele ) p_ele->GetAttribute("UseColors",setup);
    return(setup);
}

//------------------------------------------------------------------------------

char CPrintEngine::GetSiteSectionDelimiter(void)
{
    char setup = ' ';
    CXMLElement* p_ele = PrintProfile.GetChildElementByPath("print/config");
    if( p_ele ) p_ele->GetAttribute("SiteSectionDelimiter",setup);
    return(setup);
}

//------------------------------------------------------------------------------

int  CPrintEngine::GetSiteSectionBgColor(void)
{
    int setup = -1;
    CXMLElement* p_ele = PrintProfile.GetChildElementByPath("print/config");
    if( p_ele ) p_ele->GetAttribute("SiteSectionBgColor",setup);
    return(setup);
}

//------------------------------------------------------------------------------

int  CPrintEngine::GetSiteSectionFgColor(void)
{
    int setup = -1;
    CXMLElement* p_ele = PrintProfile.GetChildElementByPath("print/config");
    if( p_ele ) p_ele->GetAttribute("SiteSectionFgColor",setup);
    return(setup);
}

//------------------------------------------------------------------------------

char CPrintEngine::GetSectionDelimiter(void)
{
    char setup = ' ';
    CXMLElement* p_ele = PrintProfile.GetChildElementByPath("print/config");
    if( p_ele ) p_ele->GetAttribute("SectionDelimiter",setup);
    return(setup);
}

//------------------------------------------------------------------------------

int  CPrintEngine::GetSectionBgColor(void)
{
    int setup = -1;
    CXMLElement* p_ele = PrintProfile.GetChildElementByPath("print/config");
    if( p_ele ) p_ele->GetAttribute("SectionBgColor",setup);
    return(setup);
}

//------------------------------------------------------------------------------

int  CPrintEngine::GetSectionFgColor(void)
{
    int setup = -1;
    CXMLElement* p_ele = PrintProfile.GetChildElementByPath("print/config");
    if( p_ele ) p_ele->GetAttribute("SectionFgColor",setup);
    return(setup);
}

//------------------------------------------------------------------------------

char CPrintEngine::GetCategoryDelimiter(void)
{
    char setup = ' ';
    CXMLElement* p_ele = PrintProfile.GetChildElementByPath("print/config");
    if( p_ele ) p_ele->GetAttribute("CategoryDelimiter",setup);
    return(setup);
}

//------------------------------------------------------------------------------

int  CPrintEngine::GetCategoryBgColor(void)
{
    int setup = -1;
    CXMLElement* p_ele = PrintProfile.GetChildElementByPath("print/config");
    if( p_ele ) p_ele->GetAttribute("CategoryBgColor",setup);
    return(setup);
}

//------------------------------------------------------------------------------

int  CPrintEngine::GetCategoryFgColor(void)
{
    int setup = -1;
    CXMLElement* p_ele = PrintProfile.GetChildElementByPath("print/config");
    if( p_ele ) p_ele->GetAttribute("CategoryFgColor",setup);
    return(setup);
}

//------------------------------------------------------------------------------

int  CPrintEngine::GetModuleBgColor(void)
{
    int setup = -1;
    CXMLElement* p_ele = PrintProfile.GetChildElementByPath("print/config");
    if( p_ele ) p_ele->GetAttribute("ModuleBgColor",setup);
    return(setup);
}

//------------------------------------------------------------------------------

int  CPrintEngine::GetModuleFgColor(void)
{
    int setup = -1;
    CXMLElement* p_ele = PrintProfile.GetChildElementByPath("print/config");
    if( p_ele ) p_ele->GetAttribute("ModuleFgColor",setup);
    return(setup);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

