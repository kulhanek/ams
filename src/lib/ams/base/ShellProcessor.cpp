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

#include <XMLElement.hpp>
#include <XMLIterator.hpp>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <iomanip>
#include <ShellProcessor.hpp>
#include <ErrorSystem.hpp>

//------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------

CShellProcessor ShellProcessor;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CShellProcessor::CShellProcessor(void)
{
    ExitCode = 0;
    ShellActions.CreateChildElement("actions");
    CurrentUMask = "unset";
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CShellProcessor::PrepareModuleEnvironmentForDeps(CXMLElement* p_build)
{
    CXMLElement* p_setup = NULL;
    if( p_build != NULL ) p_setup = p_build->GetFirstChildElement("setup");

    CXMLIterator CI(p_setup);
    CXMLElement* p_sele;

    while( (p_sele = CI.GetNextChildElement()) != NULL ) {
        CSmallString lpriority;
        p_sele->GetAttribute("priority",lpriority);

        if( lpriority != "dependency" ) continue;

        // variables --------------------------------
        if( p_sele->GetName() == "variable" ) {
            CSmallString name;
            CSmallString value;
            CSmallString operation;

            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("value",value);
            p_sele->GetAttribute("operation",operation);

            if( operation == "append" ) {
                AppendValueToVariable(name,value,":");
            }

            if( operation == "prepend" ) {
                PrependValueToVariable(name,value,":");
            }

            if( operation == "set" ) {
                SetVariable(name,value);
            }

            if( operation == "keep" ) {
                SetVariable(name,value);
            }

            if( operation == "unset" ) {
                UnsetVariable(name);
            }
        }

        if( p_sele->GetName() == "script" ) {
            CSmallString name;
            CSmallString type;

            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("type",type);

            EScriptType mod_type = EST_CHILD;

            if( type == "inline" ) {
                mod_type = EST_INLINE;
            }

            RegisterScript(name,"add",mod_type);
        }

        if( p_sele->GetName() == "alias" ) {
            CSmallString name;
            CSmallString value;

            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("value",value);

            SetAlias(name,value);
        }
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CShellProcessor::PrepareModuleEnvironmentForModActionI(
                            CXMLElement* p_build)
{
    CXMLElement* p_setup = NULL;
    if( p_build != NULL ) p_setup = p_build->GetFirstChildElement("setup");

    CXMLIterator CI(p_setup);
    CXMLElement* p_sele;

    while( (p_sele = CI.GetNextChildElement()) != NULL ) {
        CSmallString lpriority;
        p_sele->GetAttribute("priority",lpriority);
        if( lpriority != "modaction" ) continue;

        // variables --------------------------------
        if( p_sele->GetName() == "variable" ) {
            CSmallString name;
            CSmallString value;
            CSmallString operation;
            CSmallString mode;

            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("value",value);
            p_sele->GetAttribute("operation",operation);

            if( operation == "append" ) {
                AppendValueToVariable(name,value,":");
            }

            if( operation == "prepend" ) {
                PrependValueToVariable(name,value,":");
            }

            if( operation == "set" ) {
                SetVariable(name,value);
            }

            if( operation == "keep" ) {
                SetVariable(name,value);
            }

            if( operation == "unset" ) {
                UnsetVariable(name);
            }
        }

        if( p_sele->GetName() == "script" ) {
            CSmallString name;
            CSmallString type;

            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("type",type);

            EScriptType mod_type = EST_CHILD;

            if( type == "inline" ) {
                mod_type = EST_INLINE;
            }

            RegisterScript(name,"add",mod_type);
        }

        if( p_sele->GetName() == "alias" ) {
            CSmallString name;
            CSmallString value;

            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("value",value);

            SetAlias(name,value);
        }
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CShellProcessor::PrepareModuleEnvironmentForModActionII(
                            CXMLElement* p_build)
{
    CXMLElement* p_setup = NULL;
    if( p_build != NULL ) p_setup = p_build->GetFirstChildElement("setup");

    CXMLIterator CI(p_setup);
    CXMLElement* p_sele;

    while( (p_sele = CI.GetNextChildElement()) != NULL ) {
        CSmallString lpriority;
        p_sele->GetAttribute("priority",lpriority);
        if( (lpriority != "modaction") && (lpriority != "dependency") ) continue;

        // variables --------------------------------
        if( p_sele->GetName() == "variable" ) {
            CSmallString name;
            CSmallString value;
            CSmallString operation;

            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("value",value);
            p_sele->GetAttribute("operation",operation);

            if( operation == "append" ) {
                RemoveValueFromVariable(name,value,":");
            }

            if( operation == "prepend" ) {
                RemoveValueFromVariable(name,value,":");
            }

            if( operation == "set" ) {
                UnsetVariable(name);
            }

            // operation - keep - nothing
        }

        if( p_sele->GetName() == "script" ) {
            CSmallString name;
            CSmallString type;

            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("type",type);

            EScriptType mod_type = EST_CHILD;

            if( type == "inline" ) {
                mod_type = EST_INLINE;
            }

            RegisterScript(name,"remove",mod_type);
        }

        if( p_sele->GetName() == "alias" ) {
            CSmallString name;

            p_sele->GetAttribute("name",name);
            UnsetAlias(name);
        }
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CShellProcessor::PrepareModuleEnvironmentForLowPriority(CXMLElement* p_build,
                                                             EModuleAction action)
{
    CSimpleList<CXMLElement> CommandList;
    CXMLElement* p_sele;

    CXMLElement* p_setup = NULL;
    if( p_build != NULL ) p_setup = p_build->GetFirstChildElement("setup");

    // we need reverse order when unload !
    if( action == EMA_ADD_MODULE ) {
        CXMLIterator     I(p_setup);
        CXMLElement*     p_sele;
        while( (p_sele = I.GetNextChildElement()) != NULL ) {
            CommandList.InsertToEnd(p_sele);
        }
    } else {
        CXMLIterator     I(p_setup);
        CXMLElement*     p_sele;
        while( (p_sele = I.GetNextChildElement()) != NULL ) {
            CommandList.InsertToBegin(p_sele);
        }
    }

    CSimpleIterator<CXMLElement> CI(CommandList);

    while( (p_sele = CI.Current()) != NULL ) {
        CI++;
        CSmallString lpriority;
        p_sele->GetAttribute("priority",lpriority);

        if( lpriority == "modaction" ) continue;

        // variables --------------------------------
        if( p_sele->GetName() == "variable" ) {
            CSmallString name;
            CSmallString value;
            CSmallString operation;

            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("value",value);
            p_sele->GetAttribute("operation",operation);

            if( operation == "append" ) {
                if( action == EMA_ADD_MODULE ) {
                    AppendValueToVariable(name,value,":");
                } else {
                    RemoveValueFromVariable(name,value,":");
                }
            }

            if( operation == "prepend" ) {
                if( action == EMA_ADD_MODULE ) {
                    PrependValueToVariable(name,value,":");
                } else {
                    RemoveValueFromVariable(name,value,":");
                }
            }

            if( operation == "set" ) {
                if( action == EMA_ADD_MODULE ) {
                    SetVariable(name,value);
                } else {
                    UnsetVariable(name);
                }
            }

            if( operation == "keep" ) {
                if( action == EMA_ADD_MODULE ) {
                    SetVariable(name,value);
                }
                // keep does nothing for remove !
            }

            if( operation == "unset" ) {
                if( action == EMA_ADD_MODULE ) {
                    UnsetVariable(name);
                }
            }
        }

        if( p_sele->GetName() == "script" ) {
            CSmallString name;
            CSmallString type;

            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("type",type);

            EScriptType mod_type = EST_CHILD;

            if( type == "inline" ) {
                mod_type = EST_INLINE;
            }

            if( action == EMA_ADD_MODULE ) {
                RegisterScript(name,"add",mod_type);
            } else {
                RegisterScript(name,"remove",mod_type);
            }
        }

        if( p_sele->GetName() == "alias" ) {
            CSmallString name;
            CSmallString value;

            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("value",value);

            if( action == EMA_ADD_MODULE ) {
                SetAlias(name,value);
            } else {
                UnsetAlias(name);
            }
        }

    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CShellProcessor::PrependValueToVariable(const CSmallString& name,
                                            const CSmallString& value,
                                            const CSmallString& delimiter)
{
    CXMLElement* p_ele = ShellActions.GetFirstChildElement("actions");
    if( p_ele == NULL ){
        LOGIC_ERROR("p_ele is NULL");
    }

    // and create new variable
    CXMLElement* p_sele = p_ele->CreateChildElement("prepend");
    p_sele->SetAttribute("name",name);
    p_sele->SetAttribute("value",value);
    p_sele->SetAttribute("delimiter",delimiter);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CShellProcessor::AppendValueToVariable(const CSmallString& name,
                                            const CSmallString& value,
                                            const CSmallString& delimiter)
{
    CXMLElement* p_ele = ShellActions.GetFirstChildElement("actions");
    if( p_ele == NULL ){
        LOGIC_ERROR("p_ele is NULL");
    }

    // and create new variable
    CXMLElement* p_sele = p_ele->CreateChildElement("append");

    p_sele->SetAttribute("name",name);
    p_sele->SetAttribute("value",value);
    p_sele->SetAttribute("delimiter",delimiter);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CShellProcessor::RemoveValueFromVariable(const CSmallString& name,
                                            const CSmallString& value,
                                            const CSmallString& delimiter)
{
    CXMLElement* p_ele = ShellActions.GetFirstChildElement("actions");
    if( p_ele == NULL ){
        LOGIC_ERROR("p_ele is NULL");
    }

    // and create new variable

    CXMLElement* p_sele = p_ele->CreateChildElement("remove");

    p_sele->SetAttribute("name",name);
    p_sele->SetAttribute("value",value);
    p_sele->SetAttribute("delimiter",delimiter);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CShellProcessor::SetUMask(const CSmallString& umask)
{
    CXMLElement* p_ele = ShellActions.GetFirstChildElement("actions");
    if( p_ele == NULL ){
        LOGIC_ERROR("p_ele is NULL");
    }

    // and set new umask
    CXMLElement* p_sele = p_ele->CreateChildElement("umask");
    p_sele->SetAttribute("value",umask);

    CurrentUMask = umask;
}

//------------------------------------------------------------------------------

const CSmallString CShellProcessor::GetCurrentUMask(void)
{
    return(CurrentUMask);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CShellProcessor::SetVariable(const CSmallString& name,
                                    const CSmallString& value)
{
    CXMLElement* p_ele = ShellActions.GetFirstChildElement("actions");
    if( p_ele == NULL ){
        LOGIC_ERROR("p_ele is NULL");
    }

    // and create new variable
    CXMLElement* p_sele = p_ele->CreateChildElement("variable");

    p_sele->SetAttribute("name",name);
    p_sele->SetAttribute("remove",false);
    p_sele->SetAttribute("value",value);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CShellProcessor::UnsetVariable(const CSmallString& name)
{
    CXMLElement* p_ele = ShellActions.GetFirstChildElement("actions");
    if( p_ele == NULL ){
        LOGIC_ERROR("p_ele is NULL");
    }

    CXMLElement* p_sele = p_ele->CreateChildElement("variable");

    p_sele->SetAttribute("name",name);
    p_sele->SetAttribute("remove",true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CShellProcessor::SetAlias(const CSmallString& name,const CSmallString& value)
{
    CXMLElement* p_ele = ShellActions.GetFirstChildElement("actions");
    if( p_ele == NULL ){
        LOGIC_ERROR("p_ele is NULL");
    }

    CXMLElement* p_sele = p_ele->CreateChildElement("alias");

    p_sele->SetAttribute("name",name);
    p_sele->SetAttribute("value",value);
    p_sele->SetAttribute("remove",false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CShellProcessor::UnsetAlias(const CSmallString& name)
{
    CXMLElement* p_ele = ShellActions.GetFirstChildElement("actions");
    if( p_ele == NULL ){
        LOGIC_ERROR("p_ele is NULL");
    }

    CXMLElement* p_sele = p_ele->CreateChildElement("alias");

    p_sele->SetAttribute("name",name);
    p_sele->SetAttribute("remove",true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CShellProcessor::BeginSubshell(void)
{
    CXMLElement* p_ele = ShellActions.GetFirstChildElement("actions");
    if( p_ele == NULL ){
        LOGIC_ERROR("p_ele is NULL");
    }

    CXMLElement* p_sele = p_ele->CreateChildElement("shell");
    p_sele->SetAttribute("action","begin");
}

//------------------------------------------------------------------------------

void CShellProcessor::EndSubshell(void)
{
    CXMLElement* p_ele = ShellActions.GetFirstChildElement("actions");
    if( p_ele == NULL ){
        LOGIC_ERROR("p_ele is NULL");
    }

    CXMLElement* p_sele = p_ele->CreateChildElement("shell");
    p_sele->SetAttribute("action","end");
}

//------------------------------------------------------------------------------

void CShellProcessor::ExitIfError(void)
{
    CXMLElement* p_ele = ShellActions.GetFirstChildElement("actions");
    if( p_ele == NULL ){
        LOGIC_ERROR("p_ele is NULL");
    }

    CXMLElement* p_sele = p_ele->CreateChildElement("shell");
    p_sele->SetAttribute("action","exitiferror");
}

//------------------------------------------------------------------------------

void CShellProcessor::CapturePWD(void)
{
    CXMLElement* p_ele = ShellActions.GetFirstChildElement("actions");
    if( p_ele == NULL ){
        LOGIC_ERROR("p_ele is NULL");
    }

    CXMLElement* p_sele = p_ele->CreateChildElement("shell");
    p_sele->SetAttribute("action","capturepwd");
}

//------------------------------------------------------------------------------

void CShellProcessor::RestorePWD(void)
{
    CXMLElement* p_ele = ShellActions.GetFirstChildElement("actions");
    if( p_ele == NULL ){
        LOGIC_ERROR("p_ele is NULL");
    }

    CXMLElement* p_sele = p_ele->CreateChildElement("shell");
    p_sele->SetAttribute("action","restorepwd");
}

//------------------------------------------------------------------------------

void CShellProcessor::ChangeCurrentDir(const CFileName& path,bool silent)
{
    CXMLElement* p_ele = ShellActions.GetFirstChildElement("actions");
    if( p_ele == NULL ){
        LOGIC_ERROR("p_ele is NULL");
    }

    CXMLElement* p_sele = p_ele->CreateChildElement("shell");
    p_sele->SetAttribute("action","cd");
    p_sele->SetAttribute("path",path);
    p_sele->SetAttribute("silent",silent);
}

//------------------------------------------------------------------------------

void CShellProcessor::ExecuteCMD(const CSmallString& cmd)
{
    CXMLElement* p_ele = ShellActions.GetFirstChildElement("actions");
    if( p_ele == NULL ){
        LOGIC_ERROR("p_ele is NULL");
    }

    CXMLElement* p_sele = p_ele->CreateChildElement("shell");
    p_sele->SetAttribute("action","exec");
    p_sele->SetAttribute("cmd",cmd);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CShellProcessor::RegisterScript(const CSmallString& name,
        const CSmallString& args,
        EScriptType type)
{
    CXMLElement* p_ele = ShellActions.GetFirstChildElement("actions");
    if( p_ele == NULL ){
        LOGIC_ERROR("p_ele is NULL");
    }

    CXMLElement* p_sele = p_ele->CreateChildElement("script");

    p_sele->SetAttribute("name",name);
    p_sele->SetAttribute("type",(int)type);
    p_sele->SetAttribute("args",args);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CShellProcessor::SetExitCode(int exitcode)
{
    ExitCode = exitcode;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CShellProcessor::RollBack(void)
{
    ExitCode = 0;
    ShellActions.RemoveAllChildNodes();
    ShellActions.CreateChildElement("actions");
    return(true);
}

//------------------------------------------------------------------------------

void CShellProcessor::BuildEnvironment(void)
{
    CSmallString exit_code;

    exit_code.IntToStr(ExitCode);

    // set exit code variable
    SetVariable("AMS_EXIT_CODE",exit_code);

    // build environment
    CXMLElement* p_ele = ShellActions.GetFirstChildElement("actions");
    if( p_ele == NULL ){
        LOGIC_ERROR("p_ele is NULL");
    }

    CXMLElement*     p_sele;
    CXMLIterator    I(p_ele);

    while( (p_sele = I.GetNextChildElement()) != NULL ) {

        if( p_sele->GetName() == "umask" ) {
            CSmallString     value;
            p_sele->GetAttribute("value",value);
            if( value != NULL ) {
                printf("umask 0%s;\n",(const char*)value);
            }
        }

        if( p_sele->GetName() == "variable" ) {
            CSmallString     name;
            CSmallString     value;
            bool            remove = false;
            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("remove",remove);
            p_sele->GetAttribute("value",value);
            if( remove == false ) {
                if( name != NULL ) {
                    if( value != NULL ) {
                        printf("export %s=\"%s\";\n",(const char*)name,(const char*)value);
                    } else {
                        printf("export %s=\"\";\n",(const char*)name);
                    }
                }
            } else {
                if( name != NULL ) {
                    printf("unset %s;\n",(const char*)name);
                }
            }
        }

        if( p_sele->GetName() == "prepend" ) {
            CSmallString     name;
            CSmallString     value;
            CSmallString    delimiter;
            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("value",value);
            p_sele->GetAttribute("delimiter",delimiter);
            printf("export %s=`$AMS_ROOT/bin/_ams-module-var prepend \"$%s\" \"%s\" \"%s\"`;\n",
                   (const char*)name,(const char*)name,
                   (const char*)delimiter,(const char*)value);
        }

        if( p_sele->GetName() == "append" ) {
            CSmallString     name;
            CSmallString     value;
            CSmallString    delimiter;
            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("value",value);
            p_sele->GetAttribute("delimiter",delimiter);
            printf("export %s=`$AMS_ROOT/bin/_ams-module-var append \"$%s\" \"%s\" \"%s\"`;\n",
                   (const char*)name,(const char*)name,
                   (const char*)delimiter,(const char*)value);
        }

        if( p_sele->GetName() == "remove" ) {
            CSmallString     name;
            CSmallString     value;
            CSmallString    delimiter;
            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("value",value);
            p_sele->GetAttribute("delimiter",delimiter);
            printf("export %s=`$AMS_ROOT/bin/_ams-module-var remove \"$%s\" \"%s\" \"%s\"`;\n",
                   (const char*)name,(const char*)name,
                   (const char*)delimiter,(const char*)value);
        }

        if( p_sele->GetName() == "script" ) {
            CSmallString     name;
            CSmallString    args;
            int             type;
            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("type",type);
            p_sele->GetAttribute("args",args);
            if( name != NULL ) {
                switch((EScriptType)type) {
                case EST_INLINE:
                    if( args != NULL ) {
                        printf("source \"%s\" %s;\n",(const char*)name,(const char*)args);
                    } else {
                        printf("source \"%s\";\n",(const char*)name);
                    }
                    break;
                case EST_CHILD:
                default:
                    if( args != NULL ) {
                        printf("\"%s\" %s;\n",(const char*)name,(const char*)args);
                    } else {
                        printf("\"%s\";\n",(const char*)name);
                    }
                    break;
                }
            }
        }

        if( p_sele->GetName() == "alias" ) {
            CSmallString     name;
            CSmallString     value;
            bool            remove = false;
            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("remove",remove);
            p_sele->GetAttribute("value",value);
            if( remove == false ) {
                if( (name != NULL) && (value != NULL) ) {
                    if( value != NULL ) {
                        printf("alias %s=\"%s\";\n",(const char*)name,(const char*)value);
                    }
                }
            } else {
                if( name != NULL ) {
                    printf("unalias %s &> /dev/null;\n",(const char*)name);
                }
            }
        }

        if( p_sele->GetName() == "shell" ) {
            CSmallString     action;
            p_sele->GetAttribute("action",action);
            if( action == "begin" ){
                printf("(\n");
            }
            if( action == "end" ){
                printf(")\n");
            }
            if( action == "capturepwd" ){
                printf("export AMS_PWD_BACKUP=\"$PWD\";\n");
            }
            if( action == "restorepwd" ){
                printf("cd \"$AMS_PWD_BACKUP\";\n");
            }
            if( action == "cd" ){
                CSmallString path;
                bool silent = false;
                p_sele->GetAttribute("path",path);
                p_sele->GetAttribute("silent",silent);
                if( silent ){
                    printf("cd \"%s\" 2> /dev/null;\n",(const char*)path);
                } else {
                    printf("cd \"%s\";\n",(const char*)path);
                }
            }
            if( action == "exec" ){
                CSmallString cmd;
                p_sele->GetAttribute("cmd",cmd);
                printf("%s;\n",(const char*)cmd);
            }
            if( action == "exitiferror" ){
                printf("if [ $? -ne 0 ]; then exit 1; fi;\n");
            }
        }

    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CShellProcessor::PrintBuild(std::ostream& vout,CXMLElement* p_build)
{
    // info out ----------------------------------------------------------------------------
    unsigned int col1 = 1;
    unsigned int col2 = 1;
    unsigned int col3 = 1;
    unsigned int col4 = 1;

    CShellProcessor::GetMaxSizesForBuild(p_build,col1,col2,col3,col4);

    if( col1 < 4 ) col1 = 4;
    if( col2 < 10 ) col2 = 10;
    if( col3 < 14 ) col3 = 14;
    if( col4 < 8 ) col4 = 8;

    vout << "# What";
    vout << " Name";
    for(unsigned int i = 4; i < col1; i++) vout << " ";
    vout << " Value/Name";
    for(unsigned int i = 10; i < col2; i++) vout << " ";
    vout << " Operation/Type";
    for(unsigned int i = 14; i < col3; i++) vout << " ";
    vout << " Priority" << endl;
    vout << "# ";
    for(unsigned int i = 0; i < 4; i++) vout << "-";
    vout << " ";
    for(unsigned int i = 0; i < col1; i++) vout << "-";
    vout << " ";
    for(unsigned int i = 0; i < col2; i++) vout << "-";
    vout << " ";
    for(unsigned int i = 0; i < col3; i++) vout << "-";
    vout << " ";
    for(unsigned int i = 0; i < col4; i++) vout << "-";
    vout << endl;

    CXMLElement*    p_setup = NULL;
    if( p_build != NULL ) p_setup = p_build->GetFirstChildElement("setup");

    CXMLIterator    I(p_setup);
    CXMLElement*    p_sele;

    while( (p_sele = I.GetNextChildElement()) != NULL ) {
        if( p_sele->GetName() == "variable" ) {
            CSmallString name;
            CSmallString value;
            CSmallString operation;
            CSmallString priority;
            bool         secret = false;
            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("value",value);
            p_sele->GetAttribute("operation",operation);
            p_sele->GetAttribute("priority",priority);
            p_sele->GetAttribute("secret",secret);
            vout << left << "     V ";
            vout << setw(col1) << name << " ";
            if( secret ){
                value = "*******";
            }
            vout << setw(col2) << value << " ";
            vout << setw(col3) << operation << " ";
            vout << setw(col4) << priority << endl;
        }
        if( p_sele->GetName() == "script" ) {
            CSmallString name;
            CSmallString type;
            CSmallString priority;
            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("type",type);
            p_sele->GetAttribute("priority",priority);
            vout << "     S ";
            vout << setw(col1) << " " << " ";
            vout << setw(col2) << name << " ";
            vout << setw(col3) << type << " ";
            vout << setw(col4) << priority << endl;
        }
        if( p_sele->GetName() == "alias" ) {
            CSmallString name;
            CSmallString value;
            CSmallString priority;
            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("value",value);
            p_sele->GetAttribute("priority",priority);
            vout << "     A ";
            vout << setw(col1) << name << " ";
            vout << setw(col2) << value << " ";
            vout << setw(col3) << " " << " ";
            vout << setw(col4) << priority << endl;
        }
    }
}

//------------------------------------------------------------------------------

void CShellProcessor::GetMaxSizesForBuild(CXMLElement* p_ele,
                                        unsigned int& col1,unsigned int& col2,
                                        unsigned int& col3,unsigned int& col4)
{
    col1 = 1;
    col2 = 1;
    col3 = 1;
    col4 = 1;

    CXMLElement*    p_setup = NULL;
    if( p_ele != NULL ) p_setup = p_ele->GetFirstChildElement("setup");

    CXMLIterator    I(p_setup);
    CXMLElement*    p_sele;

    while( (p_sele = I.GetNextChildElement()) != NULL ) {
        if( p_sele->GetName() == "variable" ) {
            CSmallString name;
            CSmallString value;
            CSmallString operation;
            CSmallString priority;
            p_sele->GetAttribute("name",name);
            if( col1 < name.GetLength() ) col1 = name.GetLength();
            p_sele->GetAttribute("value",value);
            if( col2 < value.GetLength() ) col2 = value.GetLength();
            p_sele->GetAttribute("operation",operation);
            if( col3 < operation.GetLength() ) col3 = operation.GetLength();
            p_sele->GetAttribute("priority",priority);
            if( col4 < priority.GetLength() ) col4 = priority.GetLength();
        }
        if( p_sele->GetName() == "script" ) {
            CSmallString name;
            CSmallString value;
            CSmallString type;
            CSmallString priority;
            p_sele->GetAttribute("name",name);
            if( col2 < name.GetLength() ) col2 = name.GetLength();
            p_sele->GetAttribute("type",type);
            if( col3 < type.GetLength() ) col3 = type.GetLength();
            p_sele->GetAttribute("priority",priority);
            if( col4 < priority.GetLength() ) col4 = priority.GetLength();
        }
        if( p_sele->GetName() == "alias" ) {
            CSmallString name;
            CSmallString value;
            CSmallString priority;
            p_sele->GetAttribute("name",name);
            if( col1 < name.GetLength() ) col1 = name.GetLength();
            p_sele->GetAttribute("value",value);
            if( col2 < value.GetLength() ) col2 = value.GetLength();
            p_sele->GetAttribute("priority",priority);
            if( col4 < priority.GetLength() ) col4 = priority.GetLength();
        }
    }
}


//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================






