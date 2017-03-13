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

#include "BuildItem.hpp"
#include <DirectoryEnum.hpp>
#include <FileSystem.hpp>
#include <vector>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <iomanip>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CBuildItem::CBuildItem(void)
{
    Orphaned = false;
    Exists = false;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CBuildItem::AddPackageDir(const CFileName& path,const CSmallString& build)
{
    string sdirs = string(path);

    vector<string> dir_list;
    split(dir_list,sdirs,is_any_of("/"));

    vector<string>::iterator    it = dir_list.begin();
    vector<string>::iterator    ie = dir_list.end();

    CBuildItemPtr p_parent;
    CBuildItemPtr p_node;

    while( it != ie ){
        if( p_parent == NULL ){
            p_node = SubItems[*it];
        } else {
            p_node = p_parent->SubItems[*it];
        }
        if( p_node == NULL ){
            p_node = CBuildItemPtr(new CBuildItem);
            p_node->Name = *it;
            if( p_parent == NULL ){
                SubItems[string(*it)] = p_node;
            } else {
                p_parent->SubItems[string(*it)] = p_node;
            }
        }
        p_parent = p_node;
        it++;
    }

    if( p_node != NULL ){
        p_node->Build = build;
    }
}

//------------------------------------------------------------------------------

void CBuildItem::ScanSoftRepoTree(const CFileName& path,int level)
{
    CDirectoryEnum  dir_enum(path);
    CFileName       child_name;

    dir_enum.StartFindFile("*");
    while( dir_enum.FindFile(child_name) ) {
        if( child_name == "." ) continue;
        if( child_name == ".." ) continue;
        if( CFileSystem::IsDirectory(path / child_name) == true ){
            CBuildItemPtr child_item = SubItems[string(child_name)];
            if( child_item == NULL ) {
                child_item = CBuildItemPtr(new CBuildItem);
                child_item->Name = child_name;
                SubItems[string(child_name)] = child_item;
            }
            child_item->Exists = true;
            if( level != 0 ){
                child_item->ScanSoftRepoTree(path / child_name, level-1);
            }
        }
    }
    dir_enum.EndFindFile();
}

//------------------------------------------------------------------------------

void CBuildItem::PrintTree(std::ostream& vout,const CFileName& path,int level)
{
    if( level > 0 ){
        for(int i=1; i < level; i++){
            vout << "    ";
        }
        if( level > 1 ){
            vout << "|-";
        }
        if( Exists == false ){
            vout << "<red>" << Name << "</red>";
        } else {
            vout << Name;
        }
        if( SubItems.size() == 0 ){
            vout << " " << Build;
        }
        vout << endl;
    }

    std::map<std::string,CBuildItemPtr>::iterator    it = SubItems.begin();
    std::map<std::string,CBuildItemPtr>::iterator    ie = SubItems.end();

    while( it != ie ){
        CBuildItemPtr p_child = it->second;
        p_child->PrintTree(vout,path/Name,level+1);
        it++;
    }
}

//------------------------------------------------------------------------------

void CBuildItem::PrintMissing(std::ostream& vout,const CFileName& path)
{
    std::map<std::string,CBuildItemPtr>::iterator    it = SubItems.begin();
    std::map<std::string,CBuildItemPtr>::iterator    ie = SubItems.end();

    CFileName new_path = path;
    if( ! Name.empty() ){
        new_path = new_path / Name;
    }

    while( it != ie ){
        CBuildItemPtr p_child = it->second;
        p_child->PrintMissing(vout,new_path);
        it++;
    }

    if( (SubItems.size() == 0) && (Exists == false) && (Build.empty() == false) ){
        vout << left << setw(40) << Build << "" << new_path << endl;
    }
}

//------------------------------------------------------------------------------

void CBuildItem::PrintExisting(std::ostream& vout,const CFileName& path)
{
    std::map<std::string,CBuildItemPtr>::iterator    it = SubItems.begin();
    std::map<std::string,CBuildItemPtr>::iterator    ie = SubItems.end();

    CFileName new_path = path;
    if( ! Name.empty() ){
        new_path = new_path / Name;
    }

    while( it != ie ){
        CBuildItemPtr p_child = it->second;
        p_child->PrintExisting(vout,new_path);
        it++;
    }

    if( (SubItems.size() == 0) && (Exists == true) && (Build.empty() == false) ){
        vout << left << setw(40) << Build << "" << new_path << endl;
    }
}

//------------------------------------------------------------------------------

void CBuildItem::PrintOrphans(std::ostream& vout,const CFileName& path)
{
    std::map<std::string,CBuildItemPtr>::iterator    it = SubItems.begin();
    std::map<std::string,CBuildItemPtr>::iterator    ie = SubItems.end();

    CFileName new_path = path;
    if( ! Name.empty() ){
        new_path = new_path / Name;
    }

    while( it != ie ){
        CBuildItemPtr p_child = it->second;
        p_child->PrintOrphans(vout, new_path);
        it++;
    }

    if( Orphaned ){
        vout << new_path << endl;
    }
}

//------------------------------------------------------------------------------

void CBuildItem::CountOrphanedChilds(void)
{
    std::map<std::string,CBuildItemPtr>::iterator    it = SubItems.begin();
    std::map<std::string,CBuildItemPtr>::iterator    ie = SubItems.end();

    unsigned int count = 0;
    while( it != ie ){
        CBuildItemPtr p_child = it->second;
        p_child->CountOrphanedChilds();
        if( p_child->Orphaned == true ) count++;
        it++;
    }
    if( (count == SubItems.size()) && (SubItems.size() > 0) ){
        Orphaned = true;
    }
    if( SubItems.size() == 0 ){
        if( (Build.size() == 0) && (Exists == true) ){
            Orphaned = true;
        }
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


