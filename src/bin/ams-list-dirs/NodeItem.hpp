#ifndef NodeItemH
#define NodeItemH
// =============================================================================
// AMS
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

#include <map>
#include <string>
#include <boost/shared_ptr.hpp>
#include <SmallString.hpp>
#include <FileName.hpp>

// -----------------------------------------------------------------------------

class CNodeItem;
typedef boost::shared_ptr<CNodeItem>    CNodeItemPtr;

// -----------------------------------------------------------------------------

class CNodeItem {
public:
// constructor -----------------------------------------------------------------
        CNodeItem(void);

// main methods ----------------------------------------------------------------
    /// add build path
    void AddPackageDir(const CFileName& path,const CSmallString& build);

    /// scan softrepo filesystem
    void ScanSoftRepoTree(const CFileName& path,int level);

    /// print tree with various flags
    void PrintTree(std::ostream& vout,const CFileName& path,int level);

    /// print missing records
    void PrintMissing(std::ostream& vout,const CFileName& path);

    /// print existing records
    void PrintExisting(std::ostream& vout,const CFileName& path);

    /// count orphaned childs
    void CountOrphanedChilds(void);

    /// print orphaned directories
    void PrintOrphans(std::ostream& vout,const CFileName& path);

// section of public data ------------------------------------------------------
public:
    std::string                         Name;
    std::map<std::string,CNodeItemPtr>  SubItems;
    std::string                         Build;          // associated build
    bool                                Exists;         // exists on FS
    bool                                Orphaned;       // node is orphaned
};

// -----------------------------------------------------------------------------

#endif
