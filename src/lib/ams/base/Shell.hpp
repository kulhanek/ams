#ifndef ShellH
#define ShellH
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
#include <SmallString.hpp>

//------------------------------------------------------------------------------

class AMS_PACKAGE CShell {
public:
    /// return value of variable with 'name' from system
    static const CSmallString GetSystemVariable(const CSmallString& name);

    /// set variable with 'name' to 'value'
    static bool SetSystemVariable(const CSmallString& name,const CSmallString& value);

    /// remove value from list of values separated by delimiter
    static const CSmallString RemoveValue(const CSmallString& variable_list,
            const CSmallString& variable,
            const CSmallString& delimiter);

    /// append value to list of values separated by delimiter
    static const CSmallString AppendValue(const CSmallString& value_list,
            const CSmallString& value,
            const CSmallString& delimiter);

    /// prepend value to list of values separated by delimiter
    static const CSmallString PrependValue(const CSmallString& value_list,
            const CSmallString& value,
            const CSmallString& delimiter);

// umask methods ---------------------------------------------------------------

    /// get user name
    static const CSmallString GetUserName(void);

    /// get umask
    static const CSmallString GetUMask(void);

    /// get umask permission bits
    static const CSmallString GetUMaskPermissions(void);

    /// get umask
    static const CSmallString GetUMask(mode_t mumask);

    /// get umask permission bits
    static mode_t GetUMaskMode(const CSmallString& smask);

    /// get umask permission bits
    static const CSmallString GetUMaskPermissions(mode_t mumask);
};

//------------------------------------------------------------------------------

#endif
