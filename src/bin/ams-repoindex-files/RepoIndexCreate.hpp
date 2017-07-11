#ifndef RepoIndexCreateH
#define RepoIndexCreateH
// =============================================================================
// AMS - Advanced Module System
// -----------------------------------------------------------------------------
//    Copyright (C) 2016      Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include "RepoIndexCreateOptions.hpp"
#include <VerboseStr.hpp>
#include <TerminalStr.hpp>
#include <Site.hpp>
#include <FileName.cpp>
#include <map>
#include <string>
#include <set>
#include <list>

// -----------------------------------------------------------------------------

class CBuildId {
public:
    std::string Name;
    bool operator < (const CBuildId& bp_id) const;
};

class SHA1;

// -----------------------------------------------------------------------------

class CRepoIndexCreate {
public:
// constructor -----------------------------------------------------------------
        CRepoIndexCreate(void);

// main methods ----------------------------------------------------------------
    /// init options
    int Init(int argc,char* argv[]);

    /// main part of program
    bool Run(void);

    /// finalize
    void Finalize(void);

// section of private data -----------------------------------------------------
private:
    CRepoIndexCreateOptions     Options;
    CTerminalStr                Console;
    CVerboseStr                 vout;

    int                             NumOfAllBuilds;
    int                             NumOfStats;
    std::map<CBuildId,CFileName>    BuildPaths;
    std::map<CBuildId,std::string>  BuildIndexes;
    CFileName                       CurrentDir;

    bool ListFiles(void);
    std::string CalculateBuildHash(const CFileName& build_path);
    void HashDir(const CFileName& full_path,SHA1& sha1);
    void HashNode(const CFileName& name,struct stat& my_stat,bool build_node,SHA1& sha1);
};

// -----------------------------------------------------------------------------

#endif
