#ifndef ShellProcessorH
#define ShellProcessorH
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
#include <FileName.hpp>

//-----------------------------------------------------------------------------

enum EScriptType {
    EST_INLINE,
    EST_CHILD
};

enum EModuleAction {
    EMA_ADD_MODULE,
    EMA_REMOVE_MODULE,
};

//-----------------------------------------------------------------------------

class AMS_PACKAGE CShellProcessor {
public:
// constructor and destructor --------------------------------------------------
    CShellProcessor(void);

// common methods for printing builds and site setups --------------------------
    /// print info about builds
    static void PrintBuild(std::ostream& vout,CXMLElement* p_build);

// executive methods -----------------------------------------------------------
    /// set exit code
    void SetExitCode(int exitcode);

    /// clear shell processor
    bool RollBack(void);

    /// build shell environment
    void BuildEnvironment(void);

// setup methods ---------------------------------------------------------------
    /// update environment according to module specification and variable priority
    bool PrepareModuleEnvironmentForDeps(CXMLElement* p_build);
    bool PrepareModuleEnvironmentForModActionI(CXMLElement* p_build);
    bool PrepareModuleEnvironmentForModActionII(CXMLElement* p_build);
    bool PrepareModuleEnvironmentForLowPriority(CXMLElement* p_build, EModuleAction action);

    /// prepend value to variable
    void PrependValueToVariable(const CSmallString& name,
                                const CSmallString& value,
                                const CSmallString& delimiter);

    /// append value to variable
    void AppendValueToVariable(const CSmallString& name,
                                const CSmallString& value,
                                const CSmallString& delimiter);

    /// remove value from variable
    void RemoveValueFromVariable(const CSmallString& name,
                                const CSmallString& value,
                                const CSmallString& delimiter);

    /// set umask
    void SetUMask(const CSmallString& umask);

    /// get current umask set
    const CSmallString GetCurrentUMask(void);

    /// set variable
    void SetVariable(const CSmallString& name,const CSmallString& value);

    /// unset variable
    void UnsetVariable(const CSmallString& name);

    /// register script
    void RegisterScript(const CSmallString& name,
                                     const CSmallString& args,EScriptType type);

    /// set alias
    void SetAlias(const CSmallString& name,const CSmallString& value);

    /// unset alias
    void UnsetAlias(const CSmallString& name);

    /// begin subshell via "("
    void BeginSubshell(void);

    /// put exit escape condition
    void ExitIfError(void);

    /// capture PWD into AMS_PWD_BACKUP
    void CapturePWD(void);

    /// restore PWD from AMS_PWD_BACKUP
    void RestorePWD(void);

    /// change current directory
    void ChangeCurrentDir(const CFileName& path);

    /// execute command
    void ExecuteCMD(const CSmallString& cmd);

    /// end subshell via ")"
    void EndSubshell(void);

// section of private data -----------------------------------------------------
private:
    /// list of all actions that has to executed by shell to activate/deactivate module
    CXMLDocument        ShellActions;
    CSmallString        CurrentUMask;

    /// final exit code set by module system as _MODULE_EXIT_CODE
    int            ExitCode;

    /// get sizes for build print
    static void GetMaxSizesForBuild(CXMLElement* p_ele,
            unsigned int& col1,unsigned int& col2,
            unsigned int& col3,unsigned int& col4);
};

//------------------------------------------------------------------------------

extern CShellProcessor ShellProcessor;

//------------------------------------------------------------------------------

#endif
