// =============================================================================
//  AMS - Advanced Module System
// -----------------------------------------------------------------------------
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

#include <ModBundleIndex.hpp>
#include <FileSystem.hpp>
#include <DirectoryEnum.hpp>
#include <ErrorSystem.hpp>
#include <iomanip>
#include <FSIndex.hpp>

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CModBundleIndex::LoadIndex(const CFileName& index_name)
{
    bool result = false;
    if( index_name == "-" ){
        IndexFile = "stdin";
        result = LoadIndex(cin);
    } else {
        IndexFile = index_name;
        ifstream ifs(IndexFile);
        if( ! ifs ){
            CSmallString error;
            error << "Unable to open the index file '" << IndexFile << "'";
            ES_ERROR(error);
            return(false);
        }
        result = LoadIndex(ifs);
        ifs.close();
    }

    return(result);
}

//------------------------------------------------------------------------------

bool CModBundleIndex::LoadIndex(std::istream& ifs)
{
    string  line;
    int     lino = 0;
    while( getline(ifs,line) ){
        lino++;
        string flag,sha1,build,path;
        stringstream str(line);
        str >> flag >> sha1 >> build >> path;
        if( ! str ){
            CSmallString error;
            error << "Corrupted index file '" << IndexFile << "' at line " << lino;
            ES_ERROR(error);
            return(false);
        }

        if( Hashes.count(build) == 1 ){
            CSmallString error;
            error << "SHA1 collision in index file '" << IndexFile << "' at line " << lino;
            ES_ERROR(error);
            return(false);
        }
        // flag is ignored, set only sha1,build,path
        Hashes[build] = sha1;
        Paths[build] = path;
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CModBundleIndex::SaveIndex(const CFileName& index_name)
{
    bool result = false;
    if( index_name == "-" ){
        IndexFile = "stdout";
        result = SaveIndex(cout);
    } else {
        IndexFile = index_name;
        ofstream ofs(IndexFile);
        if( ! ofs ){
            CSmallString error;
            error << "unable to open the index file '" << IndexFile << "' for writing!";
            ES_ERROR(error);
            return(false);
        }
        result = SaveIndex(ofs);
        ofs.close();
    }
    return(result);
}

//------------------------------------------------------------------------------

bool CModBundleIndex::SaveIndex(std::ostream& ofs)
{
    map<CSmallString,CFileName>::iterator it = Paths.begin();
    map<CSmallString,CFileName>::iterator ie = Paths.end();

    while( it != ie ){
        CSmallString build_id = it->first;
        string sha1 = Hashes[build_id];
        ofs << "* " << sha1 << " " << build_id << " " << it->second << endl;
        it++;
    }

    if( ! ofs ){
        ES_ERROR("The index was not saved due to error!");
        return(false);
    }
    return(true);
}

//------------------------------------------------------------------------------

void CModBundleIndex::Diff(CModBundleIndex& old_index, CVerboseStr& vout,
                           bool skip_removed, bool skip_added, bool verbose)
{
    if( verbose ) {
        vout << endl;
        vout << "# Diffing two indexes ..." << endl;
    }

    vout << low;

    map<CSmallString,string>::iterator it;
    map<CSmallString,string>::iterator ie;

    if( skip_removed == false ){

        // determine removed entries (-)
        it = old_index.Hashes.begin();
        ie = old_index.Hashes.end();

        while( it != ie ){
            CSmallString build = it->first;
            if( Hashes.count(build) == 0 ){
                vout << "- " << old_index.Hashes[build] << " " << left << setw(50) << build << " " << old_index.Paths[build] <<  endl;
            }
            it++;
        }
    }

    // determine new entries (+) or modified (M)
    it = Hashes.begin();
    ie = Hashes.end();

    while( it != ie ){
        CSmallString build = it->first;
        if( old_index.Hashes.count(build) == 0 ){
            if( skip_added == false ){
                vout << "+ " << Hashes[build] << " " << left << setw(50) << build << " " << Paths[build] <<  endl;
            }
        } else {
            if( Hashes[build] != old_index.Hashes[build] ){
                vout << "M " << Hashes[build] << " " << left << setw(50) << build << " " << Paths[build] <<  endl;
            }
        }
        it++;
    }
}

//------------------------------------------------------------------------------

void CModBundleIndex::Clear(void)
{
    Paths.clear();
    Hashes.clear();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
