// =============================================================================
// AMS
// -----------------------------------------------------------------------------
//    Copyright (C) 2023      Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2011      Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2001-2008 Petr Kulhanek, kulhanek@chemi.muni.cz
//
//     This program is free software; you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation; either version 2 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License along
//     with this program; if not, write to the Free Software Foundation, Inc.,
//     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// =============================================================================

#include "Old2NewCmd.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <XMLParser.hpp>
#include <XMLPrinter.hpp>

//------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------

MAIN_ENTRY(COld2NewCmd)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int COld2NewCmd::Init(int argc, char* argv[])
{
    // encode program options, all check procedures are done inside of Options
    int result = Options.ParseCmdLine(argc,argv);

    // should we exit or was it error?
    if( result != SO_CONTINUE ) return(result);

    // output must be directed to stderr
    // stdout is used for shell processor
    Console.Attach(stderr);

    // attach verbose stream to terminal stream and set desired verbosity level
    vout.Attach(Console);
    if( Options.GetOptVerbose() ) {
        vout.Verbosity(CVerboseStr::high);
    } else {
        vout.Verbosity(CVerboseStr::low);
    }

    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << endl;
    vout << "# ==============================================================================" << endl;
    vout << "# doc-old2new (AMS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    vout << low;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool COld2NewCmd::Run(void)
{   
    vout << low;

    CXMLDocument    cat;
    CXMLDocument    doc;

// -----------------------------------------------------------------------------
    CXMLParser   cat_xml_parser;

    cat_xml_parser.SetOutputXMLNode(&cat);

    if( cat_xml_parser.Parse(Options.GetArgCatFile()) == false ) {
        CSmallString error;
        error << "unable to parse module categories file '" << Options.GetArgCatFile() << "'";
        ES_ERROR(error);
        return(false);
    }

    CXMLParser   doc_xml_parser;

    doc_xml_parser.EnableWhiteCharacters(true);
    doc_xml_parser.SetOutputXMLNode(&doc);

    if( doc_xml_parser.Parse(Options.GetArgDocFile()) == false ) {
        CSmallString error;
        error << "unable to parse module docu file '" << Options.GetArgDocFile() << "'";
        ES_ERROR(error);
        return(false);
    }

// -----------------------------------------------------------------------------

    CXMLElement* p_mele = doc.GetChildElementByPath("module");
    if( p_mele ==  NULL ){
        RUNTIME_ERROR("no module element");
    }

    CSmallString mname;
    p_mele->GetAttribute("name",mname);

    vout << "> module '" << mname << endl;

    CXMLElement* p_cele = cat.GetChildElementByPath("print/categories/category");
    while( p_cele != NULL ){
        CSmallString cname;
        p_cele->GetAttribute("name",cname);
        CXMLElement* p_cmele = p_cele->GetFirstChildElement("module");
        while( p_cmele != NULL ){
            CSmallString cmname;
            p_cmele->GetAttribute("name",cmname);
            if( cmname == mname ){
                CXMLElement* p_ele = doc.GetChildElementByPath("module/categories",true);
                CXMLElement* p_nc = p_ele->GetFirstChildElement("category");
                bool found = false;
                while( p_nc != NULL ){
                    CSmallString mcname;
                    p_nc->GetAttribute("name",mcname);
                    if( mcname == cname ) found = true;
                    p_nc = p_nc->GetNextSiblingElement("category");
                }
                if( ! found ){
                    p_nc = p_ele->CreateChildElement("category");
                    p_nc->SetAttribute("name",cname);
                    vout << "  category '" << cname << "' added" << endl;
                } else {
                    vout << "  category '" << cname << "' already assigned" << endl;
                }
            }
            p_cmele = p_cmele->GetNextSiblingElement("module");
        }
        p_cele = p_cele->GetNextSiblingElement("category");
    }

// -----------------------------------------------------------------------------
    CXMLPrinter doc_xml_printer;

    doc_xml_printer.SetPrintedXMLNode(&doc);
    doc_xml_printer.SetPrintAsItIs(true);

    if( doc_xml_printer.Print(Options.GetArgDocFile()) == false ) {
        CSmallString error;
        error << "unable to save module docu file '" << Options.GetArgDocFile() << "'";
        ES_ERROR(error);
        return(false);
    }

    return(false);
}

//------------------------------------------------------------------------------

void COld2NewCmd::Finalize(void)
{
    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << "# ==============================================================================" << endl;
    vout << "# doc-old2new (AMS utility) terminated at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    vout << endl;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



