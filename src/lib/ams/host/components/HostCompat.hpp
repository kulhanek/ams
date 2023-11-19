#ifndef HostSubSystemCompatH
#define HostSubSystemCompatH
// =============================================================================
//  AMS - Advanced Module System
// -----------------------------------------------------------------------------
//     Copyright (C) 2023 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2020 Petr Kulhanek (kulhanek@chemi.muni.cz)
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
#include <HostSubSystem.hpp>
#include <FileName.hpp>
#include <XMLElement.hpp>
#include <VerboseStr.hpp>
#include <list>

// -----------------------------------------------------------------------------

class AMS_PACKAGE CHostSubSystemCompat : public CHostSubSystem {
public:
// constructor -----------------------------------------------------------------
    CHostSubSystemCompat(const CFileName& config_file);
    ~CHostSubSystemCompat(void);

// input methods ---------------------------------------------------------------
    /// apply setup to Host
    virtual void Apply(void);

    /// print subsystem info
    virtual void PrintSubSystemInfo(CVerboseStr& vout);

// section of private data -----------------------------------------------------
private:
    std::list<CSmallString> ArchTokens;
};

//------------------------------------------------------------------------------

#endif
