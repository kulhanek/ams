// =============================================================================
//  AMS - Advanced Module System
// -----------------------------------------------------------------------------
//     Copyright (C) 2012 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2011      Petr Kulhanek, kulhanek@chemi.muni.cz
//     Copyright (C) 2001-2008 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <TorqueAttr.hpp>
#include <ErrorSystem.hpp>
#include <pbs_ifl.h>
#include <string.h>

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool get_attribute(struct attrl* p_first,const char* p_name,const char* p_res,
                   bool& value,bool emit_error)
{
    if( p_first == NULL ){
        ES_ERROR("p_first is NULL");
        return(false);
    }
    struct attrl* p_a = FindAttr(p_first,p_name,p_res,emit_error);
    if( p_a == NULL ) return(!emit_error);

    // convert
    CSmallString svalue(p_a->value);
    if( (svalue == "True") || (svalue == "true") ){
        value = true;
        return(true);
    }

    if( (svalue == "False") || (svalue == "false") ){
        value = false;
        return(true);
    }

    CSmallString error;
    error << "incompatible bool type for attribute '" << p_name << "' (" << p_res;
    error << ") got '" << value << "'" ;
    ES_ERROR(error);

    return(false);
}

//------------------------------------------------------------------------------

bool get_attribute(struct attrl* p_first,const char* p_name,const char* p_res,
                   int& value,bool emit_error)
{
    if( p_first == NULL ){
        ES_ERROR("p_first is NULL");
        return(false);
    }
    struct attrl* p_a = FindAttr(p_first,p_name,p_res,emit_error);
    if( p_a == NULL ) return(!emit_error);

    // convert
    CSmallString svalue(p_a->value);
    if( svalue.IsInt() ) {
        value = svalue.ToInt();
        return(true);
    }

    CSmallString error;
    error << "incompatible bool type for attribute '" << p_name << "' (" << p_res;
    error << ") got '" << value << "'" ;
    ES_ERROR(error);

    return(false);
}

//------------------------------------------------------------------------------

bool get_attribute(struct attrl* p_first,const char* p_name,const char* p_res,
                   CSmallString& value,bool emit_error)
{
    if( p_first == NULL ){
        ES_ERROR("p_first is NULL");
        return(false);
    }
    struct attrl* p_a = FindAttr(p_first,p_name,p_res,emit_error);
    if( p_a == NULL ) return(!emit_error);

    // convert
    CSmallString svalue(p_a->value);
    value = svalue;
    return(true);
}

//------------------------------------------------------------------------------

bool get_attribute(struct attrl* p_first,const char* p_name,const char* p_res,
                   CSmallTime& value,bool emit_error)
{
    CSmallString stime;
    if( get_attribute(p_first,p_name,p_res,stime,emit_error) == false ){
        return(false);
    }
    int len = stime.GetLength();
    if( len == 0 ) return(!emit_error);
    if( len < 7 ){
        ES_ERROR("converison error - insufficient length");
        return(false);
    }
    if( (stime[len-3] != ':') || (stime[len-6] != ':') ){
        ES_ERROR("converison error - wrong delimiter");
        return(false);
    }

    int total = 0;
    total += stime[len-1] - '0';
    total += (stime[len-2] - '0')*10;

    int m;
    m  =  stime[len-4] - '0';
    m += (stime[len-5] - '0')*10;
    total += m*60;

    int h;
    h  =  stime[len-7] - '0';
    if( len > 7 ){
        h += (stime[len-8] - '0')*10;
    }
    if( len > 8 ){
        h += (stime[len-9] - '0')*100;
    }
    total += h*3600;

    CSmallTime lvalue(total);
    value = lvalue;

    return(true);
}

//------------------------------------------------------------------------------

bool get_attribute(struct attrl* p_first,const char* p_name,const char* p_res,
                   std::vector<CSmallString>& values,const char* p_delim,bool emit_error)
{
    CSmallString list;
    if( get_attribute(p_first,p_name,p_res,list,emit_error) == false ){
        return(false);
    }

    char* p_save = NULL;
    char* p_item = strtok_r(list.GetBuffer(),p_delim,&p_save);
    while( p_item ){
        values.push_back(p_item);
        p_item = strtok_r(NULL,p_delim,&p_save);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

struct attrl* FindAttr(struct attrl* p_list,const char* p_name,const char* p_res,
                       bool emit_error)
{
    while( p_list != NULL ){
        if( strcmp(p_list->name,p_name) == 0 ){
            if( p_res == NULL ) return(p_list);
            if( (p_list->resource != NULL) && (p_res != NULL) && (strcmp(p_list->resource,p_res) == 0) ){
                return(p_list);
            }
        }
        p_list = p_list->next;
    }

    if( emit_error ){
        CSmallString error;
        error << "unable to find attribute '" << p_name << "' (" << p_res << ")";
        ES_ERROR(error);
    }

    return(NULL);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

