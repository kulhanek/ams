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

#include "RepoIndexCreate.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <AMSGlobalConfig.hpp>
#include <Cache.hpp>
#include <AmsUUID.hpp>
#include <DirectoryEnum.hpp>
#include <Site.hpp>
#include <ErrorSystem.hpp>
#include <Shell.hpp>
#include <Utils.hpp>
#include "sha1.hpp"
#include <XMLIterator.hpp>
#include <XMLParser.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <vector>
#include <list>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//------------------------------------------------------------------------------

MAIN_ENTRY(CRepoIndexCreate)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRepoIndexCreate::CRepoIndexCreate(void)
{
    NumOfAllBuilds = 0;
    NumOfStats = 0;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CRepoIndexCreate::Init(int argc, char* argv[])
{
    // encode program options
    int result = Options.ParseCmdLine(argc,argv);

    // should we exit or was it error?
    if( result != SO_CONTINUE ) return(result);

    // attach verbose stream to terminal stream and set desired verbosity level
    vout.Attach(Console);
    if( Options.GetOptVerbose() ) {
        vout.Verbosity(CVerboseStr::high);
    } else {
        vout.Verbosity(CVerboseStr::low);
    }

    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << endl;
    vout << "# ==============================================================================" << endl;
    vout << "# ams-repoindex-fdirs (AMS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    vout << high;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CRepoIndexCreate::Run(void)
{
    // create list of builds
    vout << endl;
    vout << "# Assembling list of subdirectories ..." << endl;
    if( ListDirs() == false ) return(false);

    vout << "  > Number of subdirectories                             = " << NumOfAllBuilds << endl;

    // calculate index
    vout << endl;
    vout << "# Calculating index ..." << endl;

    map<CBuildId,CFileName>::iterator it = BuildPaths.begin();
    map<CBuildId,CFileName>::iterator ie = BuildPaths.end();

    while( it != ie ){
        CBuildId  build_id = it->first;
        CFileName build_path(it->second);
        string sha1 = CalculateBuildHash(build_path);
        BuildIndexes[build_id] = sha1;
        vout << sha1 << " " << build_id.Name << endl;
        it++;
    }

    vout << endl;
    vout << "# Statistics ..." << endl;
    vout << "  > Number of stat objects  = " << NumOfStats << endl;

    vout << endl;
    vout << "# Saving index ..." << endl;

    ofstream ofs(Options.GetArgOutputFile());
    if( ! ofs ){
        ES_ERROR("Unable to open the index file for writing!");
        return(false);
    }

    it = BuildPaths.begin();

    while( it != ie ){
        CBuildId  build_id = it->first;
        string sha1 = BuildIndexes[build_id];
        ofs << "* " << sha1 << " " << build_id.Name << " " << it->second << endl;
        it++;
    }

    if( ! ofs ){
        ES_ERROR("The index was not saved due to error!");
        return(false);
    }

    ofs.close();

    return(true);
}

//------------------------------------------------------------------------------

bool CRepoIndexCreate::ListDirs(void)
{
    CDirectoryEnum enum_dir(Options.GetArgScannedDir());

    CFileName   file;

    enum_dir.StartFindFile("*");
    while( enum_dir.FindFile(file) ){
        if( file == "." ) continue;
        if( file == ".." ) continue;
        if( CFileSystem::IsDirectory( CFileName(Options.GetArgScannedDir()) / file ) ){
            NumOfAllBuilds++;

            // register build
            CBuildId build_id;
            build_id.Name = file;

            // register build for index
            BuildPaths[build_id] = file;
        }
    }

    enum_dir.EndFindFile();

    return(true);
}

//------------------------------------------------------------------------------

string CRepoIndexCreate::CalculateBuildHash(const CFileName& build_path)
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
                full_path = Options.GetArgScannedDir();
            }
        }
        it++;
        if( dir == NULL ) continue;         // if absolute path is provided in AMS_PACKAGE_DIR
        full_path = full_path / dir;

        struct stat my_stat;
        if( lstat(full_path,&my_stat) != 0 ) continue; // silently skip
        HashNode(dir,my_stat,it == dirs.end(),sha1);
    }

    // scan the build directory
    HashDir(full_path,sha1);

    // final hash
    return( sha1.final() );
}

//------------------------------------------------------------------------------

void CRepoIndexCreate::HashDir(const CFileName& full_path,SHA1& sha1)
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

void CRepoIndexCreate::HashNode(const CFileName& name,struct stat& my_stat,bool build_node,SHA1& sha1)
{
    // sum up monitored values
    stringstream str;

// core data
    str << name;
    str << my_stat.st_size;

    if( Options.GetOptFullIndex() == true ){
        str << my_stat.st_ino;
        str << my_stat.st_dev;
    }

// permisssion data
    if( Options.GetOptFullIndex() == true ){
        str << my_stat.st_uid;
        str << my_stat.st_gid;
    }
    str << my_stat.st_mode;

// time data
    if( build_node ){
        str << my_stat.st_mtime;
        if( Options.GetOptFullIndex() == true ){
            str << my_stat.st_ctime;
        }
    } else {
        // str << my_stat.st_mtime; this prevent to mark several unchanged builds by modification of their parent directories
        // str << my_stat.st_ctime;
    }

    sha1.update(str.str());

    NumOfStats++;
}

//------------------------------------------------------------------------------

void CRepoIndexCreate::Finalize(void)
{
    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << endl;
    vout << "# ==============================================================================" << endl;
    vout << "# ams-repoindex-fdirs (AMS utility) terminated at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    if( ErrorSystem.IsError() || (ErrorSystem.IsAnyRecord() && Options.GetOptVerbose()) ){
        vout << low;
        ErrorSystem.PrintErrors(vout);
    }

    vout << endl;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CBuildId::operator < (const CBuildId& bp_id) const
{
    // Otherwise a are equal
    if (Name < bp_id.Name)  return(true);
    if (Name > bp_id.Name)  return(false);
    // Otherwise both are equal
    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


