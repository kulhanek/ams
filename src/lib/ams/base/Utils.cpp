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
#include <sys/types.h>
#include <grp.h>

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

void CUtils::PrintTokens(std::ostream& sout,const CSmallString& title, const CSmallString& res_list,
                         int ncolumns)
{
    string          svalue = string(res_list);
    vector<string>  items;
    if( ncolumns < 0 ) {
        int nrow;
        ncolumns = 80;
        CTerminal::GetSize(nrow,ncolumns);
    }

    // split to items
    split(items,svalue,is_any_of(","));

    std::vector<string>::iterator it = items.begin();
    std::vector<string>::iterator ie = items.end();

    sout << title;

    if(res_list == NULL ){
        sout << "-none-" << endl;
        return;
    }

    int len = title.GetLength();

    while( it != ie ){
        string sres = *it;
        sout << sres;
        len += sres.size();
        len++;
        it++;
        if( it != ie ){
            string sres = *it;
            int tlen = len;
            tlen += sres.size();
            tlen++;
            if( tlen > ncolumns ){
                sout << "," << endl;
                for(unsigned int i=0; i < title.GetLength(); i++){
                    sout << " ";
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

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

gid_t CUtils::GetGroupID(const CSmallString& name,bool trynobody)
{
    struct group *p_grp = getgrnam(name);
    if( p_grp == NULL ){
        if( trynobody ){
            CSmallString error;
            error << "no gid for '" << name << "' - trying to use nogroup as bypass";
            ES_ERROR(error);
            p_grp = getgrnam("nogroup");
        }
        if( p_grp == NULL ) return(-1);
    }
    return(p_grp->gr_gid);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

