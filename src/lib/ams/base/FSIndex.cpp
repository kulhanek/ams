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

#include <FSIndex.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <unistd.h>
#include <errno.h>
#include <vector>
#include <list>
#include <sstream>
#include <sha1.hpp>
#include <FileSystem.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/classification.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CFSIndex::CFSIndex(void)
{
    PersonalBundle = false;
    NumOfStats = 0;
}

//------------------------------------------------------------------------------

std::string CFSIndex::CalculateBuildHash(const CFileName& build_path)
{
    SHA1 sha1;

    // split build_path into individual directories and hash them
    string sbuildp = string(build_path);
    vector<string> dirs;
    split(dirs,sbuildp,is_any_of("/"));

    vector<string>::iterator it = dirs.begin();
    vector<string>::iterator ie = dirs.end();

    CFileName full_path;
    CFileName dir;
    while (it != ie ){
        dir = CFileName(*it);
        if( it == dirs.begin() ) {
            if( dir == NULL ){
                full_path = "/";
            } else {
                full_path = RootDir;
            }
        }
        it++;
        if( dir == NULL ) continue;         // if absolute path is provided in AMS_PACKAGE_DIR
        full_path = full_path / dir;

        if( PersonalBundle == true ){
            // the build might not be synchronized yet
            if( CFileSystem::IsDirectory(full_path) == false ){
                // return null sha1
                return("0000000000000000000000000000000000000000");
            }
        }

        struct stat my_stat;
        if( lstat(full_path,&my_stat) != 0 ) continue; // silently skip
        HashNode(dir,my_stat,it == dirs.end(),sha1);
    }

    if( PersonalBundle == true ){
        // the build might not be synchronized yet
        if( CFileSystem::IsDirectory(full_path) == false ){
            // return null sha1
            return("0000000000000000000000000000000000000000");
        }
    }

    // scan the build directory
    HashDir(full_path,sha1);

    // final hash
    return( sha1.final() );
}

//------------------------------------------------------------------------------

void CFSIndex::HashDir(const CFileName& full_path,SHA1& sha1)
{   
    DIR* p_dir = opendir(full_path);
    if( p_dir == NULL ) return;  // silently skip

    struct dirent*  p_subdir;
    list<string>    nodes;

    while( (p_subdir = readdir(p_dir)) != NULL ){
        if( strcmp(p_subdir->d_name,".") == 0 ) continue;
        if( strcmp(p_subdir->d_name,"..") == 0 ) continue;
        nodes.push_back(p_subdir->d_name);
    }
    closedir(p_dir);

    // sort it
    nodes.sort();

    // calculate hash
    list<string>::iterator it = nodes.begin();
    list<string>::iterator ie = nodes.end();

    while( it != ie ){
        CFileName   name(*it);
        it++;
        CFileName   sub_node = full_path / name;

        struct stat my_stat;
        if( lstat(sub_node,&my_stat) != 0 ) continue; // silently skip
        HashNode(name,my_stat,true,sha1);

        if (S_ISLNK(my_stat.st_mode)) continue;

        if (S_ISDIR(my_stat.st_mode)) {
            HashDir(sub_node,sha1);
        }
    }
}

//------------------------------------------------------------------------------

void CFSIndex::HashNode(const CFileName& name,struct stat& my_stat,bool build_node,SHA1& sha1)
{
    // sum up monitored values
    stringstream str;

// core data
    str << name;

    // non-regular files (for example directories can have different sizes on different FSs)
    if( S_ISREG(my_stat.st_mode) ){
        str << my_stat.st_size;
    }
    str << my_stat.st_mode;

// time data
    if( build_node ){
        str << my_stat.st_mtime;
    } else {
        // str << my_stat.st_mtime; this prevent to mark several unchanged builds by modification of their parent directories
        // str << my_stat.st_ctime;
    }

    sha1.update(str.str());

    NumOfStats++;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


