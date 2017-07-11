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

#include "RepoIndexCreateFDirs.hpp"
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
#include <FSIndex.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//------------------------------------------------------------------------------

MAIN_ENTRY(CRepoIndexCreateFDirs)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRepoIndexCreateFDirs::CRepoIndexCreateFDirs(void)
{
    NumOfAllBuilds = 0;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CRepoIndexCreateFDirs::Init(int argc, char* argv[])
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
    vout << "# ams-repoindex-create-fdirs (AMS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    vout << high;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CRepoIndexCreateFDirs::Run(void)
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

    CFSIndex index;
    index.RootDir = Options.GetArgScannedDir();
    index.PersonalSite = false;

    while( it != ie ){
        CBuildId  build_id = it->first;
        CFileName build_path(it->second);
        string sha1 = index.CalculateBuildHash(build_path);
        BuildIndexes[build_id] = sha1;
        vout << sha1 << " " << build_id.Name << endl;
        it++;
    }

    vout << endl;
    vout << "# Statistics ..." << endl;
    vout << "  > Number of stat objects  = " << index.NumOfStats << endl;

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

bool CRepoIndexCreateFDirs::ListDirs(void)
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

void CRepoIndexCreateFDirs::Finalize(void)
{
    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << endl;
    vout << "# ==============================================================================" << endl;
    vout << "# ams-repoindex-create-fdirs (AMS utility) terminated at " << dt.GetSDateAndTime() << endl;
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


