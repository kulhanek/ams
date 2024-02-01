#ifndef FSIndexH
#define FSIndexH
// =============================================================================
// AMS - Advanced Module System
// -----------------------------------------------------------------------------
//    Copyright (C) 2016,2017      Petr Kulhanek, kulhanek@chemi.muni.cz
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
#include <FileName.hpp>

class SHA1;

// -----------------------------------------------------------------------------

class AMS_PACKAGE CFSIndex {
public:

    CFSIndex(void);

// main methods ----------------------------------------------------------------

    std::string CalculateBuildHash(const CFileName& build_path);
    std::string CalculateDirHash(const CFileName& dir_path);
    std::string CalculateFileHash(const CFileName& file_path);

    void HashDir(const CFileName& full_path,SHA1& sha1);
    void HashNode(const CFileName& name,struct stat& my_stat,bool build_node,SHA1& sha1);

public:
    CFileName   RootDir;
    bool        PersonalBundle;
    int         NumOfStats;
};

// -----------------------------------------------------------------------------

#endif
