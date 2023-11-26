#ifndef UtilsH
#define UtilsH
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
#include <list>

//------------------------------------------------------------------------------

class AMS_PACKAGE CUtils {
public:
// miscelaneous ----------------------------------------------------------------
    /// generate unique ID
    static CSmallString GenerateUUID(void);

// some common print techniques
    /// print comma delimited tokens
    static void PrintTokens(std::ostream& sout, const CSmallString& title,
                            const CSmallString& res_list, int ncolumns=-1,char firstchar='#');

    /// print comma delimited tokens
    static void PrintTokens(std::ostream& sout, const CSmallString& title,
                            std::list<CSmallString>& list, int ncolumns=-1,char firstchar='#');

    /// find file in paths
    static bool FindFile(const CFileName& search_paths, const CFileName& module,
                         const CFileName& ext, CFileName& output_file);

    /// find all files in paths
    static void FindAllFilesInPaths(const CFileName& search_paths, const CFileName& pattern,
                         std::list<CFileName>& list);

    /// find all files in given path
    static void FindAllFiles(const CFileName& path, const CFileName& pattern,
                         std::list<CFileName>& list);
};

//------------------------------------------------------------------------------

#endif
