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
#include <iostream>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

using namespace std;
using namespace boost;
using namespace boost::algorithm;

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
    Debug = false;
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
    Debug = CShell::GetSystemVariable("COMP_DEBUG") == "ON";

    // extract all words from command line ----------
    tmp = CommandLine;
    char* p_saveptr = NULL;
    char* p_beg = tmp.GetBuffer();
    char* p_word = NULL;

    CWord = 0; // command cannot be completed by this program
    if( p_beg != NULL ){
        p_word = strtok_r(p_beg," ",&p_saveptr);
    }
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

    if( Debug ){
        cerr << "line:  " << CommandLine << endl;
        cerr << "point: " << CGenPosition << endl;
        cerr << "cmd:   " << Command << endl;
        cerr << "act:   " << Action << endl;
        cerr << "cword: " << CWord << endl;
        cerr << "words: " << endl;
        for(unsigned int i=0; i < Words.size(); i++){
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

    if( Debug ){
        cerr << "filter: " << filter << endl;
    }

    // filter suggestions ---------------------------
    std::vector<std::string>::iterator it = Suggestions.begin();
    while( it != Suggestions.end() ){
        string sgt = *it;
        if( Debug ){
            cerr << "sgt: " << sgt << endl;
        }
        if( fnmatch(filter,sgt.c_str(),0) != 0 ) {
            it = Suggestions.erase(it); // does not match - remove suggestion
        } else {
            it++;
        }
    }

    // keep only the last word after ":"
    for(unsigned int i=0; i < Suggestions.size(); i++) {
        string tmp = Suggestions[i];
        vector<string> items;
        split(items,tmp,is_any_of(":"));
        if( items.size() > 0 ){
            Suggestions[i] = items[items.size()-1];
        }
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CAMSCompletion::PrintSuggestions(void)
{
    // count number of suggestions
    int scount = Suggestions.size();

    // print suggestions
    for(unsigned int i=0; i < Suggestions.size(); i++) {
        if( scount == 1 ) {
            if( ModuleNameComp == false ) {
                // only one suggestion - put extra space to move to the next argument
                printf("%s \n",Suggestions[i].c_str());
            } else {
                if( (WhatBuildPart() + 1 == RelPartCompleted) ||
                        (WhatBuildPart() == 3) ) {
                    // end of suggestion
                    printf("%s \n",Suggestions[i].c_str());
                } else {
                    // continue with next build part
                    printf("%s:\n",Suggestions[i].c_str());
                }
            }
        } else {
            // print only suggestion
            printf("%s\n",Suggestions[i].c_str());
        }
    }
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CAMSCompletion::AddSuggestions(const CSmallString& list)
{
    string tmp = string(list);
    split(Suggestions,tmp,is_any_of(" "),token_compress_on);

    if( Debug ){
        cerr << "suggestions: " << endl;
        for(unsigned int i=0; i < Suggestions.size(); i++){
            cerr << " " << Suggestions[i] << endl;
        }
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

        Suggestions.push_back(string(site.GetName()));
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
                if( Suggestions[i] == string(suggestion) ) {
                    found = true;
                    break;
                }
            }

            if( found == false ) Suggestions.push_back(string(suggestion));
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

