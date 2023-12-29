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

#include "ServerWatcher.hpp"
#include <ErrorSystem.hpp>
#include <unistd.h>
#include <errno.h>
#include <SmallTimeAndDate.hpp>
#include <string.h>
#include <iomanip>
#include <XMLElement.hpp>
#include <FileSystem.hpp>

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CServerWatcher::CServerWatcher(void)
{
    Enabled = true;
    FlushErrorStackInterval  = 30;
    ErrorLogFileName         = "errors.log";
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CServerWatcher::ProcessWatcherControl(CVerboseStr& vout,CXMLElement* p_config)
{
    vout << "# === [watcher] ================================================================" << endl;

    if( p_config == NULL ){
        vout << "# Watcher service is enabled (enabled)           = " << setw(6) << bool_to_str(Enabled) << "              (default)" << endl;
        vout << "# Flush error stack interval (errorinterval) [s] = " << setw(6) << FlushErrorStackInterval << "              (default)" << endl;
        vout << "# Error log file (logname)                       = " << setw(15) << ErrorLogFileName << "     (default)" << endl;
        return(true);
    }

    if( p_config->GetAttribute("enabled",Enabled) == true ) {
        vout << "# Watcher service is enabled (enabled)           = " << setw(6) << bool_to_str(Enabled) << endl;
    } else {
        vout << "# Watcher service is enabled (enabled)           = " << setw(6) << bool_to_str(Enabled) << "              (default)" << endl;
    }

    if( p_config->GetAttribute("errorinterval",FlushErrorStackInterval) == true ) {
        vout << "# Flush error stack interval (errorinterval) [s] = " << setw(6) << FlushErrorStackInterval << endl;
    } else {
        vout << "# Flush error stack interval (errorinterval) [s] = " << setw(6) << FlushErrorStackInterval << "              (default)" << endl;
    }

    if( p_config->GetAttribute("logname",ErrorLogFileName) == true ) {
        vout << "# Error log file (logname)                       = " << setw(15) << ErrorLogFileName << endl;
    } else {
        vout << "# Error log file (logname)                       = " << setw(15) << ErrorLogFileName << "     (default)" << endl;
    }

    // create log directory
    CFileName log_dir = CFileName(ErrorLogFileName).GetFileDirectory();
    if( log_dir != NULL ){
        if( CFileSystem::IsDirectory(log_dir) == false ){
            CFileSystem::CreateDir(log_dir);
        }

    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CServerWatcher::ExecuteThread(void)
{
    if( FlushErrorStackInterval > 0 ) {
        FILE* p_fout = fopen(ErrorLogFileName,"w");
        if( p_fout == NULL ) {
            CSmallString error;
            error << "unable to open log file '" << ErrorLogFileName << "' (" << strerror(errno) << ")";
            ES_ERROR(error);
            return;
        }
        CSmallTimeAndDate current;
        current.GetActualTimeAndDate();

        fprintf(p_fout,"# === ERROR Stack ==============================================================\n");
        fprintf(p_fout,"# Server started at:    %s\n",(const char*)current.GetSDateAndTime());
        fflush(p_fout);
        fclose(p_fout);
    }

    LastFlushErrorStackTime.GetActualTime();

    while( ! ThreadTerminated ) {

        CSmallTime currtime;
        currtime.GetActualTime();
        if( currtime - LastFlushErrorStackTime > FlushErrorStackInterval ) {
            LastFlushErrorStackTime = currtime;
            // append to error log and clear error stack
            ErrorSystem.AppendToErrLog(ErrorLogFileName,true);
        }

        usleep(500000); // sleep for 500 ms
    }

    if( FlushErrorStackInterval > 0 ) {
        FILE* p_fout = fopen(ErrorLogFileName,"a");
        if( p_fout == NULL ) {
            CSmallString error;
            error << "unable to open log file '" << ErrorLogFileName << "' (" << strerror(errno) << ")";
            ES_ERROR(error);
            return;
        }

        ErrorSystem.AppendToErrLog(ErrorLogFileName,true);

        CSmallTimeAndDate current;
        current.GetActualTimeAndDate();

        fprintf(p_fout,"# Server terminated at: %s\n",(const char*)current.GetSDateAndTime());
        fflush(p_fout);
        fclose(p_fout);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
