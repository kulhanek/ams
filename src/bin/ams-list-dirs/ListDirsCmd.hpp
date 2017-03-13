#ifndef ListDirsCmdH
#define ListDirsCmdH
// =============================================================================
// AMS - Advanced Module System
// -----------------------------------------------------------------------------
//    Copyright (C) 2012      Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include "ListDirsCmdOptions.hpp"
#include <VerboseStr.hpp>
#include <TerminalStr.hpp>
#include "NodeItem.hpp"
#include <Site.hpp>

// -----------------------------------------------------------------------------

class CListDirsCmd {
public:
// constructor -----------------------------------------------------------------
        CListDirsCmd(void);

// main methods ----------------------------------------------------------------
    /// init options
    int Init(int argc,char* argv[]);

    /// main part of program
    bool Run(void);

    /// finalize
    void Finalize(void);

// section of private data -----------------------------------------------------
private:
    CListDirsCmdOptions     Options;
    CTerminalStr            Console;
    CVerboseStr             vout;
    CNodeItemPtr            DirectoryTree;

    void ScanAllSiteBuilds(void);
    bool ScanUserSiteBuilds(const CSmallString& sites);
    void ScanSiteBuilds(const CSite& site);
};

// -----------------------------------------------------------------------------

#endif
