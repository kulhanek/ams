#ifndef UserUtilsH
#define UserUtilsH
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
#include <FileName.hpp>

//------------------------------------------------------------------------------

class AMS_PACKAGE CUserUtils {
public:

// information methods ---------------------------------------------------------
    /// get group ID
    static gid_t GetGroupID(const CSmallString& name,bool trynobody=true);

// umask methods ---------------------------------------------------------------

    /// get user name
    static const CSmallString GetUserName(void);

    /// get umask mode
    static mode_t GetUMaskMode(void);

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
