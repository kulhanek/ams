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

#include "RepoIndexCreateFiles.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <ErrorSystem.hpp>
#include <FileSystem.hpp>
#include <FSIndex.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;

//------------------------------------------------------------------------------

MAIN_ENTRY(CRepoIndexCreateFiles)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRepoIndexCreateFiles::CRepoIndexCreateFiles(void)
{
    NumOfAllFiles           = 0;
    NumOfUniqueFiles        = 0;

    NumOfAllDirectories     = 0;
    NumOfUniqueDirectories  = 0;

    NumOfAllBuilds          = 0;
    NumOfUniqueBuilds       = 0;
    NumOfNonSoftRepoBuilds  = 0;
    NumOfSharedBuilds       = 0;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CRepoIndexCreateFiles::Init(int argc, char* argv[])
{
    // encode program options
    int result = Options.ParseCmdLine(argc,argv);

    // should we exit or was it error?
    if( result != SO_CONTINUE ) return(result);

    // output must be directed to stderr
    // stdout can be used to save index
    Console.Attach(stderr);

    // attach verbose stream to terminal stream and set desired verbosity level
    vout.Attach(Console);
    if( Options.GetOptVerbose() ) {
        vout.Verbosity(CVerboseStr::high);
    } else {
        vout.Verbosity(CVerboseStr::low);
    }

    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << low;
    vout << endl;
    vout << "# ==============================================================================" << endl;
    vout << "# ams-index-create (AMS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CRepoIndexCreateFiles::Run(void)
{
    // create list of items to index

    if( Options.GetArgMode() == "files" ){
        if( ListFiles() == false ){
            CSmallString error;
            error << "unable to read source file";
            ES_ERROR(error);
            return(false);
        }
    } else if( Options.GetArgMode() == "directories" ){
        if( ListDirectories() == false ){
            CSmallString error;
            error << "unable to read source file";
            ES_ERROR(error);
            return(false);
        }
    } else if( Options.GetArgMode() == "builds" ){
        if( ListBuilds() == false ){
            CSmallString error;
            error << "unable to read source file";
            ES_ERROR(error);
            return(false);
        }
    } else {
        CSmallString error;
        error << "unsupported mode: " << Options.GetArgMode();
        ES_ERROR(error);
        return(false);
    }

    // calculate index
    vout << endl;
    vout << "# Calculating index ..." << endl;

    CFSIndex index;
    index.RootDir = Options.GetArgSourcePath();
    index.PersonalBundle = Options.GetOptIsPersonalBundle();

    map<CSmallString,CFileName>::iterator it = NewIndex.Paths.begin();
    map<CSmallString,CFileName>::iterator ie = NewIndex.Paths.end();

    while( it != ie ){
        CSmallString    build_id    = it->first;
        CFileName       build_path  = it->second;
        string sha1;
        if( Options.GetArgMode() == "files" ){
            sha1 = index.CalculateFileHash(build_path);
        } else if( Options.GetArgMode() == "directories" ){
            sha1 = index.CalculateDirHash(build_path);
        } else if( Options.GetArgMode() == "builds" ){
            sha1 = index.CalculateBuildHash(build_path);
        }

        NewIndex.Hashes[build_id] = sha1;
        vout << sha1 << " " << build_id << endl;
        it++;
    }

    vout << endl;
    vout << "# Statistics ..." << endl;
    vout << "  > Number of stat objects  = " << index.NumOfStats << endl;

    vout << endl;
    vout << "# Saving index ..." << endl;

    if( NewIndex.SaveIndex(Options.GetArgIndexName()) == false ){
        CSmallString error;
        error << "unable to save the index into the '" << Options.GetArgIndexName() << "' file";
        ES_ERROR(error);
        return(false);
    }
    vout << "   > Saved as: "  << Options.GetArgIndexName() << endl;

    return(true);
}

//------------------------------------------------------------------------------

bool CRepoIndexCreateFiles::ListFiles(void)
{
    vout << endl;
    vout << "# Assembling list of files ..." << endl;
    vout << "  > Source path                = " << Options.GetArgSourcePath() << endl;
    vout << "  > Source list                = " << Options.GetArgSourceList() << endl;
    vout << "  > Personal bundle            = " << bool_to_str(Options.GetOptIsPersonalBundle()) << endl;

    NumOfAllFiles           = 0;
    NumOfUniqueFiles        = 0;

    if( Options.GetArgSourceList() == "-" ){
        ListFilesRead(cin);
    } else {
        ifstream ifs;
        ifs.open(Options.GetArgSourceList());
        if( ! ifs ){
            CSmallString error;
            error << "unable to open for reading the source file with file names: '" << Options.GetArgSourceList() << "'";
            ES_ERROR(error);
            return(false);
        }
        ListFilesRead(ifs);
        ifs.close();
    }

    vout << "  > Number of files            = " << NumOfAllFiles << endl;
    vout << "  > Number of unique files     = " << NumOfUniqueFiles << endl;
    vout << "  > Number of files for index  = " << NewIndex.Paths.size() << endl;

    return(true);
}

//------------------------------------------------------------------------------

void CRepoIndexCreateFiles::ListFilesRead(std::istream& ifs)
{
    std::string line;
    while( (ifs.eof() == false ) && getline(ifs,line) ){
        CFileName file = line;
        // DEBUG: cout << "f: " << file << endl;

        CFileName path;
        if( file[0] == '/' ){
            path = file;
        } else {
            path = Options.GetArgSourcePath() / file;
        }

        if( ! Options.GetOptIsPersonalBundle() ){
            // ignore this test for personal site as the build might not be synchronized yet
            if( CFileSystem::IsFile(path) == false ){
                CSmallString error;
                error << file << " -> AMS_PACKAGE_DIR: " << path << " does not exist!";
                RUNTIME_ERROR(error);
            }
        }

        // register fake build for index
        if( NewIndex.Paths.count(file) == 0 ){
            NumOfUniqueFiles++;
        }
        NewIndex.Paths[file] = file;
        NumOfAllFiles++;
    }
}

//------------------------------------------------------------------------------

bool CRepoIndexCreateFiles::ListDirectories(void)
{
    vout << endl;
    vout << "# Assembling list of directories ..." << endl;
    vout << "  > Source path                     = " << Options.GetArgSourcePath() << endl;
    vout << "  > Source list                     = " << Options.GetArgSourceList() << endl;
    vout << "  > Personal bundle                 = " << bool_to_str(Options.GetOptIsPersonalBundle()) << endl;

    NumOfAllDirectories = 0;
    NumOfUniqueDirectories = 0;

    if( Options.GetArgSourceList() == "-" ){
        ListDirectoriesRead(cin);
    } else {
        ifstream ifs;
        ifs.open(Options.GetArgSourceList());
        if( ! ifs ){
            CSmallString error;
            error << "unable to open for reading the source file with directory names: '" << Options.GetArgSourceList() << "'";
            ES_ERROR(error);
            return(false);
        }
        ListDirectoriesRead(ifs);
        ifs.close();
    }

    vout << "  > Number of directories           = " << NumOfAllDirectories << endl;
    vout << "  > Number of unique directories    = " << NumOfUniqueDirectories << endl;
    vout << "  > Number of directories for index = " << NewIndex.Paths.size() << endl;

    return(true);
}

//------------------------------------------------------------------------------

void CRepoIndexCreateFiles::ListDirectoriesRead(std::istream& ifs)
{
    std::string line;
    while( (ifs.eof() == false ) && getline(ifs,line) ){
        CFileName dir = line;
        // DEBUG: cout << "f: " << file << endl;

        CFileName path;
        if( dir[0] == '/' ){
            path = dir;
        } else {
            path = Options.GetArgSourcePath() / dir;
        }

        if( ! Options.GetOptIsPersonalBundle() ){
            // ignore this test for personal site as the build might not be synchronized yet
            if( CFileSystem::IsDirectory(path) == false ){
                CSmallString error;
                error << dir << " -> AMS_PACKAGE_DIR: " << path << " does not exist!";
                RUNTIME_ERROR(error);
            }
        }

        // register fake build for index
        if( NewIndex.Paths.count(path) == 0 ){
            NumOfUniqueDirectories++;
        }
        NewIndex.Paths[dir] = dir;
        NumOfAllDirectories++;
    }
}

//------------------------------------------------------------------------------

bool CRepoIndexCreateFiles::ListBuilds(void)
{
    // create list of builds
    vout << endl;
    vout << "# Assembling list of builds ..." << endl;
    vout << "  > Source path                                          = " << Options.GetArgSourcePath() << endl;
    vout << "  > Source list                                          = " << Options.GetArgSourceList() << endl;
    vout << "  > Personal bundle                                      = " << bool_to_str(Options.GetOptIsPersonalBundle()) << endl;

    NumOfAllBuilds          = 0;
    NumOfUniqueBuilds       = 0;
    NumOfNonSoftRepoBuilds  = 0;
    NumOfSharedBuilds       = 0;

    UniqueBuilds.clear();
    UniqueBuildPaths.clear();

    if( Options.GetArgSourceList() == "-" ){
        if( ListBuildsRead(cin) == false ){
            CSmallString error;
            error << "an error occured during reading the source file with builds: '" << Options.GetArgSourceList() << "'";
            ES_ERROR(error);
            return(false);
        }
    } else {
        ifstream ifs;
        ifs.open(Options.GetArgSourceList());
        if( ! ifs ){
            CSmallString error;
            error << "unable to open for reading the source file with builds: '" << Options.GetArgSourceList() << "'";
            ES_ERROR(error);
            return(false);
        }
        if( ListBuildsRead(ifs) == false ){
            CSmallString error;
            error << "an error occured during reading the source file with builds: '" << Options.GetArgSourceList() << "'";
            ES_ERROR(error);
            return(false);
        }
        ifs.close();
    }

    vout << "  > Number of module builds                              = " << NumOfAllBuilds << endl;
    vout << "  > Number of unique builds                              = " << NumOfUniqueBuilds << endl;
    vout << "  > Number of builds (no AMS_PACKAGE_DIR)                = " << NumOfNonSoftRepoBuilds << endl;
    vout << "  > Number of shared builds (the same AMS_PACKAGE_DIR)   = " << NumOfSharedBuilds << endl;
    vout << "  > Number of builds for index (AMS_PACKAGE_DIR and dir) = " << NewIndex.Paths.size() << endl;

    return(true);
}

//------------------------------------------------------------------------------

bool CRepoIndexCreateFiles::ListBuildsRead(std::istream& ifs)
{
    std::string line;
    int         nline=0;
    while( getline(ifs,line) ){
        nline++;
        std::vector<std::string> items;
        split(items,line,is_any_of("\t "),boost::token_compress_on);
        if( items.size() != 2 ){
            CSmallString error;
            error << "incorrect number if items on the line " << nline << " (" << line << "), expecting two words: build and path";
            ES_ERROR(error);
            return(false);
        }

        CSmallString build_id = items[0];

        if( UniqueBuilds.count(build_id) == 1 ) continue;
        UniqueBuilds.insert(build_id);
        NumOfUniqueBuilds++;

        CFileName package_dir = items[1];
        if( package_dir == NULL ){
            NumOfNonSoftRepoBuilds++;
            continue;
        }

        CFileName path;
        if( package_dir[0] == '/' ){
            path = package_dir;
        } else {
            path = Options.GetArgSourcePath() / package_dir;
        }

        if( ! Options.GetOptIsPersonalBundle() ){
            // ignore this test for personal site as the build might not be synchronized yet
            if( CFileSystem::IsDirectory(path) == false ){
                CSmallString error;
                error << build_id << " -> AMS_PACKAGE_DIR: " << package_dir << " does not exist!";
                ES_ERROR(error);
                return(false);
            }
        }

        // register build
        if( UniqueBuildPaths.count(path) == 1 ){
            // already registered
            NumOfSharedBuilds++;
            continue;
        }
        UniqueBuildPaths.insert(path);

        // register build for index
        NewIndex.Paths[build_id] = package_dir;
    }

    return(true);
}

//------------------------------------------------------------------------------

void CRepoIndexCreateFiles::Finalize(void)
{
    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << endl;
    vout << "# ==============================================================================" << endl;
    vout << "# ams-index-create (AMS utility) terminated at " << dt.GetSDateAndTime() << endl;
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


