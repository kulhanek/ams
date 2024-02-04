#ifndef ModBundleIndexH
#define ModBundleIndexH
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
#include <FileName.hpp>
#include <VerboseStr.hpp>
#include <map>

//------------------------------------------------------------------------------

class AMS_PACKAGE CModBundleIndex {
public:
    /// load index
    bool LoadIndex(const CFileName& index_name);

    /// load index
    bool LoadIndex(std::istream& ifs);

    /// save index
    bool SaveIndex(const CFileName& index_name);

    /// save index
    bool SaveIndex(std::ostream& ofs);

    /// diff two indexes
    void Diff(CModBundleIndex& old_index, CVerboseStr& vout, bool skip_removed,
              bool skip_added, bool verbose);

    /// clear index
    void Clear(void);

// section of public data ------------------------------------------------------
public:
    std::map<CSmallString,CFileName>    Paths;
    std::map<CSmallString,std::string>  Hashes;
    CFileName                           IndexFile;
};

//-----------------------------------------------------------------------------

#endif
