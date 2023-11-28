// =============================================================================
//  AMS - Advanced Module System
// -----------------------------------------------------------------------------
//     Copyright (C) 2023 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include <Utils.hpp>
#include <string.h>
#include <Site.hpp>
#include <DirectoryEnum.hpp>
#include <FileName.hpp>
#include <FileSystem.hpp>
#include <AmsUUID.hpp>
#include <errno.h>
#include <ErrorSystem.hpp>
#include <XMLIterator.hpp>
#include <AMSRegistry.hpp>
#include <iomanip>
#include <list>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <XMLElement.hpp>
#include <fnmatch.h>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CSmallString CUtils::GenerateUUID(void)
{
    CUUID my_uuid;
    my_uuid.CreateUUID();
    return(my_uuid.GetStringForm());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CUtils::PrintTokens(std::ostream& sout, const CSmallString& title, const CSmallString& res_list,
                         int ncolumns, char firstchar)
{
    string                      svalue = string(res_list);
    std::list<CSmallString>   list;

    // split to items
    if( res_list != NULL ) split(list,svalue,is_any_of(","));

    CUtils::PrintTokens(sout,title,list,ncolumns,firstchar);
}

//------------------------------------------------------------------------------

void CUtils::PrintTokens(std::ostream& sout, const CSmallString& title,
                        const CSmallString& res_list, const CSmallString& delimiters,
                        int ncolumns, char firstchar)
{
    string                      svalue = string(res_list);
    std::list<CSmallString>   list;

    // split to items
    std::string sdelims(delimiters);
    if( res_list != NULL ) split(list,svalue,is_any_of(sdelims));

    CUtils::PrintTokens(sout,title,list,ncolumns,firstchar);
}

//------------------------------------------------------------------------------

void CUtils::PrintTokens(std::ostream& sout, const CSmallString& title,
                        std::list<CSmallString>& list, int ncolumns,char firstchar)
{
    if( ncolumns < 0 ) {
        int nrow;
        ncolumns = 80;
        CTerminal::GetSize(nrow,ncolumns);
    }
    std::list<CSmallString>::iterator it = list.begin();
    std::list<CSmallString>::iterator ie = list.end();

    sout << title;

    if( list.empty() ){
        sout << "-none-" << endl;
        return;
    }

    int len = title.GetLength();

    while( it != ie ){
        CSmallString sres = *it;
        sout << sres;
        len += sres.GetLength();
        len++;
        it++;
        if( it != ie ){
            CSmallString sres = *it;
            int tlen = len;
            tlen += sres.GetLength();
            tlen++;
            if( tlen > ncolumns ){
                sout << "," << endl;
                if( firstchar != 0 ){
                      sout << firstchar;
                      for(unsigned int i=1; i < title.GetLength(); i++){
                          sout << " ";
                      }
                } else {
                    for(unsigned int i=0; i < title.GetLength(); i++){
                        sout << " ";
                    }
                }
                len = title.GetLength();
            } else {
                sout << ", ";
                len += 2;
            }
        }
    }
    sout << endl;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CUtils::FindFile(const CFileName& search_paths,const CFileName& module,const CFileName& ext,CFileName& output_file)
{
    std::vector<CFileName> paths;
    std::string            spaths = string(search_paths);
    split(paths,spaths,is_any_of(":"),boost::token_compress_on);

    for(CFileName path : paths){

        CFileName full_name = path / module + ext;
        // DEBUG: cout << full_name << endl;
        if( CFileSystem::IsFile(full_name) ){
            output_file = full_name;
            return(true);
        }
    }

    return(false);
}

//------------------------------------------------------------------------------

void CUtils::FindAllFilesInPaths(const CFileName& search_paths, const CFileName& pattern,
                     std::list<CFileName>& list)
{
    std::vector<CFileName> paths;
    std::string            spaths = string(search_paths);
    split(paths,spaths,is_any_of(":"),boost::token_compress_on);

    for(CFileName path : paths){
        FindAllFiles(path,pattern,list);
    }
}

//------------------------------------------------------------------------------

void CUtils::FindAllFiles(const CFileName& path, const CFileName& pattern,
                     std::list<CFileName>& list)
{
    CDirectoryEnum penum(path);

    if( penum.StartFindFile("*") == false ) {
        return;
    }

    CFileName file;
    while( penum.FindFile(file) == true ) {
        if( file == "." ) continue;
        if( file == ".." ) continue;

        CFileName full_name = path / file;
        if( CFileSystem::IsDirectory(full_name) ){
            FindAllFiles(full_name,pattern,list);
        } else {
            if( fnmatch(pattern,file,0) == 0 ){
                list.push_back(full_name);
            }
        }
    }

    penum.EndFindFile();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

