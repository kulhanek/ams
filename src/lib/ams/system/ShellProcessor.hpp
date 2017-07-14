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

//-----------------------------------------------------------------------------

enum EScriptType {
    EST_INLINE,
    EST_CHILD
};

//-----------------------------------------------------------------------------

class AMS_PACKAGE CShellProcessor {
public:
    // constructor and destructor ------------------------------------------------
    CShellProcessor(void);

    // executive methods --------------------------------------------------------
    /// set exit code
    void SetExitCode(int exitcode);

    /// clear shell processor
    bool RollBack(void);

    /// build shell environment
    void BuildEnvironment(void);

    // setup methods -------------------------------------------------------------
    /// update environment according to module specification and variable priority
    bool PrepareModuleEnvironmentForDeps(CXMLElement* p_build);
    bool PrepareModuleEnvironmentForModActionI(CXMLElement* p_build);
    bool PrepareModuleEnvironmentForModActionII(CXMLElement* p_build);
    bool PrepareModuleEnvironmentForLowPriority(CXMLElement* p_build,
            bool add_module);

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

    // section of private data ---------------------------------------------------
private:
    /// list of all actions that has to executed by shell to activate/deactivate module
    CXMLDocument        ShellActions;

    /// final exit code set by module system as _MODULE_EXIT_CODE
    int            ExitCode;
};

//-----------------------------------------------------------------------------

extern CShellProcessor ShellProcessor;

//-----------------------------------------------------------------------------

#endif
