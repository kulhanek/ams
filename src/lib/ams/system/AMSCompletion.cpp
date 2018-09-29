// =============================================================================
//  AMS - Advanced Module System
// -----------------------------------------------------------------------------
//     Copyright (C) 2012 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include <AMSCompletion.hpp>
#include <ctype.h>
#include <ErrorSystem.hpp>
#include <XMLParser.hpp>
#include <XMLElement.hpp>
#include <XMLIterator.hpp>
#include <Shell.hpp>
#include <fnmatch.h>
#include <AmsUUID.hpp>
#include <DirectoryEnum.hpp>
#include <Site.hpp>
#include <Cache.hpp>
#include <PrintEngine.hpp>
#include <string.h>
#include <AMSGlobalConfig.hpp>

//------------------------------------------------------------------------------

CAMSCompletion AMSCompletion;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CAMSCompletion::CAMSCompletion(void)
{
    RelPartCompleted = 0;
    CGenPosition = 0;
    CWord = 0;
    ModuleNameComp = false;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CAMSCompletion::InitCompletion(void)
{
    // get completion data --------------------------
    CommandLine = CShell::GetSystemVariable("COMP_LINE");
    CSmallString tmp;
    tmp = CShell::GetSystemVariable("COMP_POINT");
    CGenPosition = tmp.ToInt();
    bool debug = CShell::GetSystemVariable("COMP_DEBUG") == "ON";

    // extract all words from command line ----------
    tmp = CommandLine;
    char* p_saveptr = NULL;
    char* p_beg = tmp.GetBuffer();
    char* p_word;

    CWord = 0; // command cannot be completed by this program
    p_word = strtok_r(p_beg," ",&p_saveptr);
    while(p_word != NULL) {
        if( (strlen(p_word) >= 1) && (p_word[0] == '-') ){
            // option - ignore
        } else {
            Words.push_back(p_word);
            unsigned int pos = p_word - p_beg;
            if( (pos <= CGenPosition) &&
                    ((pos + strlen(p_word)) >= CGenPosition) ) CWord = Words.size() - 1;
        }
        p_word = strtok_r(NULL," ",&p_saveptr);
    }
    if( CWord == 0 ) CWord = Words.size();

    RelPartCompleted = 2; // only name and version is completed by default

    // get command
    if( Words.size() >= 1 ){
        Command = Words[0];
    }

    // get action
    if( Words.size() >= 2 ){
        Action = Words[1];
    }

    if( debug ){
        cerr << "line:  " << CommandLine << endl;
        cerr << "point: " << CGenPosition << end;
        cerr << "cmd:   " << Command << endl;
        cerr << "act:   " << Action << endl;
        cerr << "cword: " << CWord << endl;
        cerr << "words: " << endl;
        for(int i=0; i < Words.size(); i++){
            cerr << " " << Words[i] << endl;
        }
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CAMSCompletion::GetSuggestions(void)
{
    // get suggestions according to command ---------
    if( GetCommand() == "site" ) {
        // what part should be completed?
        switch(CWord) {
        case 1:
            AddSuggestions("activate info disp");
            FilterSuggestions();
            PrintSuggestions();
            return(true);
        case 2:
            AddSiteSuggestions();
            FilterSuggestions();
            PrintSuggestions();
            return(true);
        default:
            // out of command requirements -> no suggestions
            return(true);
        }
    }
    // ----------------------------------------------
    else if( (GetCommand() == "module") || (GetCommand() == "amsmodule") ) {
        // what part should be completed?
        switch(CWord) {
        case 1:
            AddSuggestions("add builds remove versions disp list help");
            FilterSuggestions();
            PrintSuggestions();
            return(true);
        default:
            if( (GetAction() == "add" ) ||
                    (GetAction() == "remove" ) ||
                    (GetAction() == "help" ) ||
                    (GetAction() == "activate" ) ||
                    (GetAction() == "versions" ) ||
                    (GetAction() == "builds" ) ||
                    (GetAction() == "disp" ) ||
                    (GetAction() == "isactive" ) ||
                    (GetAction() == "getactver" ) ) {
                ModuleNameComp = true;
                AddModuleSuggestions();
                FilterSuggestions();
                PrintSuggestions();
            }
            // out of command requirements -> no suggestions
            return(true);
        }
        return(true);
    }
    // ----------------------------------------------
    else if( GetCommand() == "ams-cache" ) {
        // what part should be completed?
        switch(CWord) {
        case 1:
            AddSuggestions("rebuild rebuildall deps syntax syntaxall");
            FilterSuggestions();
            PrintSuggestions();
            return(true);
        default:
            // out of command requirements -> no suggestions
            return(true);
        }
        return(true);
    }
    // ----------------------------------------------
    else if( GetCommand() == "ams-map-manip" ) {
        // what part should be completed?
        switch(CWord) {
        case 1:
            AddSuggestions("addbuilds rmbuilds distribute aliases");
            FilterSuggestions();
            PrintSuggestions();
            return(true);
        default:
            // out of command requirements -> no suggestions
            return(true);
        }
        return(true);
    }    
    // ----------------------------------------------
    else {
        // unsupported command return
        return(false);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CSmallString CAMSCompletion::GetCommand(void)
{
    return(Command);
}

//------------------------------------------------------------------------------

CSmallString CAMSCompletion::GetAction(void)
{
    return(Action);
}

//------------------------------------------------------------------------------

bool CAMSCompletion::FilterSuggestions(void)
{
    // build filter ---------------------------------
    CSmallString filter;

    if( CWord < Words.size() ) {
        filter = Words[CWord] + "*";
    } else {
        filter = "*";
    }

    // filter suggestions ---------------------------
    for(unsigned int i=0; i < Suggestions.size(); i++) {
        if( fnmatch(filter,Suggestions[i],0) != 0 ) {
            // does not match - remove suggestion
            Suggestions[i] = NULL;
        }
    }

    // keep only the last word after ":"
    for(unsigned int i=0; i < Suggestions.size(); i++) {
        CSmallString tmp = Suggestions[i];
        char* p_saveptr = NULL;
        char* p_word;

        p_word = strtok_r(tmp.GetBuffer(),":",&p_saveptr);
        while(p_word != NULL) {
            Suggestions[i] = p_word;
            p_word = strtok_r(NULL,":",&p_saveptr);
        }
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CAMSCompletion::PrintSuggestions(void)
{
    // count number of suggestions
    int scount = 0;
    for(unsigned int i=0; i < Suggestions.size(); i++) {
        if( Suggestions[i] != NULL) scount++;
    }

    // print suggestions
    for(unsigned int i=0; i < Suggestions.size(); i++) {
        if( scount == 1 ) {
            if( ModuleNameComp == false ) {
                // only one suggestion - put extra space to move to next argument
                if( Suggestions[i] != NULL) printf("%s \n",(const char*)Suggestions[i]);
            } else {
                if( (WhatBuildPart() + 1 == RelPartCompleted) ||
                        (WhatBuildPart() == 3) ) {
                    // end of suggestion
                    if( Suggestions[i] != NULL) printf("%s \n",(const char*)Suggestions[i]);
                } else {
                    // continue with next build part
                    if( Suggestions[i] != NULL) printf("%s:\n",(const char*)Suggestions[i]);
                }
            }
        } else {
            // print only suggestion
            if( Suggestions[i] != NULL) printf("%s\n",(const char*)Suggestions[i]);
        }
    }
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CAMSCompletion::AddSuggestions(const CSmallString& list)
{
    CSmallString tmp = list;
    char* p_saveptr = NULL;
    char* p_word;

    p_word = strtok_r(tmp.GetBuffer()," ",&p_saveptr);
    while(p_word != NULL) {
        Suggestions.push_back(p_word);
        p_word = strtok_r(NULL," ",&p_saveptr);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CAMSCompletion::AddSiteSuggestions(void)
{
    // make list of all available sites -------------
    CDirectoryEnum dir_enum(AMSGlobalConfig.GetETCDIR() / "sites");

    dir_enum.StartFindFile("*");
    CFileName site_sid;
    while( dir_enum.FindFile(site_sid) ) {
        CAmsUUID    site_id;
        CSite       site;
        if( site_id.LoadFromString(site_sid) == false ) continue;
        if( site.LoadConfig(site_sid) == false ) continue;

        Suggestions.push_back(site.GetName());
    }
    dir_enum.EndFindFile();

    return(true);
}

//------------------------------------------------------------------------------

bool CAMSCompletion::AddModuleSuggestions(void)
{
    CXMLElement*     p_mele = Cache.GetRootElementOfCache();
    CXMLElement*     p_ele;
    CXMLElement*     p_sele;

    CXMLIterator    I(p_mele);

    int numparts = WhatBuildPart();

    while( (p_ele = I.GetNextChildElement("module")) != NULL ) {
        CSmallString name;
        p_ele->GetAttribute("name",name);
        if( Cache.IsPermissionGrantedForModuleByAnyMeans(p_ele) == false ) continue;

        CXMLElement*    p_rele = p_ele->GetFirstChildElement("builds");
        CXMLIterator    J(p_rele);

        while( (p_sele = J.GetNextChildElement("build")) != NULL ) {
            CSmallString ver;
            CSmallString arch;
            CSmallString mode;
            p_sele->GetAttribute("ver",ver);
            p_sele->GetAttribute("arch",arch);
            p_sele->GetAttribute("mode",mode);

            CSmallString suggestion;

            // how many items from name should be printed?
            switch(numparts) {
            case 0:
                suggestion = name;
                break;
            case 1:
                suggestion = name + ":" + ver;
                break;
            case 2:
                suggestion = name + ":" + ver + ":" + arch;
                break;
            case 3:
                suggestion = name + ":" + ver + ":" + arch + ":" + mode;
                break;
            default:
                break;
            }

            // is already in the list?
            bool found = false;
            for(unsigned int i=0; i < Suggestions.size(); i++) {
                if( Suggestions[i] == suggestion ) {
                    found = true;
                    break;
                }
            }

            if( found == false ) Suggestions.push_back(suggestion);
        }

    }

    return(true);
}

//------------------------------------------------------------------------------

unsigned int CAMSCompletion::WhatBuildPart(void)
{
    unsigned int numsem = 0;

    if( CWord < Words.size() ) {
        for(unsigned int i=0; i < Words[CWord].GetLength(); i++) {
            if( Words[CWord][i] == ':' ) numsem++;
        }
    }

    return(numsem);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

