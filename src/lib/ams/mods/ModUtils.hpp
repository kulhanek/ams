#ifndef ModUtilsH
#define ModUtilsH
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
#include <SmallString.hpp>
#include <XMLElement.hpp>

//------------------------------------------------------------------------------

class AMS_PACKAGE CModUtils {
public:
// module name operation -------------------------------------------------------
    /// divide module name to its parts (name,ver,arch,mode)
    static bool ParseModuleName(const CSmallString& module,
            CSmallString& name,
            CSmallString& ver,
            CSmallString& arch,
            CSmallString& mode);

    /// divide module name to its parts (name,ver,arch)
    static bool ParseModuleName(const CSmallString& module,
            CSmallString& name,
            CSmallString& ver,
            CSmallString& arch);

    /// divide module name to its parts (name,ver)
    static bool ParseModuleName(const CSmallString& module,
            CSmallString& name,
            CSmallString& ver);

    /// divide module name to its parts (name)
    static bool ParseModuleName(const CSmallString& module,
            CSmallString& name);

// architectures ---------------------------------------------------------------
    /// compare two architectures
    static bool AreSameTokens(const CSmallString& user_arch,const CSmallString& build_arch);

    /// compare two architectures
    static bool AreSameTokens(const CSmallString& user_arch,const CSmallString& build_arch,int& matches,int& maxmatches);

// module name operation -------------------------------------------------------
    /// extract name part from full or incomplete module name
    static const CSmallString GetModuleName(const CSmallString& module);

    /// extract version part from full or incomplete module name
    static const CSmallString GetModuleVer(const CSmallString& module);

    /// extract architecture part from full or incomplete module name
    static const CSmallString GetModuleArch(const CSmallString& module);

    /// extract parascheme part from full or incomplete module name
    static const CSmallString GetModuleMode(const CSmallString& module);

// common methods for printing builds and site setups --------------------------
    /// print info about builds
    static void PrintBuild(std::ostream& vout,CXMLElement* p_build);

    /// get sizes for build print
    static void GetMaxSizesForBuild(CXMLElement* p_ele,
            unsigned int& col1,unsigned int& col2,
            unsigned int& col3,unsigned int& col4);
};

//------------------------------------------------------------------------------

#endif
