#ifndef AMSCompletionH
#define AMSCompletionH
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

#include <AMSMainHeader.hpp>
#include <XMLDocument.hpp>
#include <SmallString.hpp>
#include <vector>

//------------------------------------------------------------------------------

class AMS_PACKAGE CAMSCompletion {
public:
    // constructor and destructors ------------------------------------------------
    CAMSCompletion(void);

    // input methods --------------------------------------------------------------
    /// load completion configuration file and command line
    bool InitCompletion(void);

    // executive methods ----------------------------------------------------------
    /// return suggestions for completion
    bool GetSuggestions(void);

    // section of private date ----------------------------------------------------
private:
    unsigned int                RelPartCompleted;   // what parts from build should be completed
    CSmallString                CommandLine;
    unsigned int                CGenPosition;
    std::vector<CSmallString>   Words;              // command line splitted into words, options are ignored
    CSmallString                Command;
    CSmallString                Action;
    unsigned int                CWord;              // word to be completed
    std::vector<std::string>    Suggestions;        // suggestions
    bool                        ModuleNameComp;     // module name completion mode

    // return command name or empty string
    CSmallString GetCommand(void);

    // return action name or empty string
    CSmallString GetAction(void);

    // filter suggestions
    bool FilterSuggestions(void);

    // print suggestions
    bool PrintSuggestions(void);

    // suggestions list generators ------------------------------------------------
    // add initial suggestions (for actions)
    bool AddSuggestions(const CSmallString& list);

    // add site name suggestions
    bool AddSiteSuggestions(void);

    // add module name suggestions
    bool AddModuleSuggestions(void);

    // what part from build should be printed?
    unsigned int WhatBuildPart(void);
};

//------------------------------------------------------------------------------

extern  CAMSCompletion AMSCompletion;

//------------------------------------------------------------------------------

#endif
