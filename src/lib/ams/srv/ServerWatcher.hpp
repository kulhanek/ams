#ifndef ServerWatcherH
#define ServerWatcherH
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

#include <SmartThread.hpp>
#include <SmallTime.hpp>
#include <VerboseStr.hpp>

//------------------------------------------------------------------------------

class CXMLElement;

//------------------------------------------------------------------------------

//! Server watcher
/*! \ingroup eserver

  [watcher]
  enabled           (on/off) - determine is the watcher is started or not
  errorinterval     (int)    - time in seconds to flush CErrorSystem log into logfile
  logname           (string) - file name of logfile

*/
class CServerWatcher : public CSmartThread {
public:
// constructor -----------------------------------------------------------------
    CServerWatcher(void);

    //! read watcher common setup
    bool ProcessWatcherControl(CVerboseStr& vout,CXMLElement* p_config);

// section of private data -----------------------------------------------------
private:
    bool            Enabled;

    int             FlushErrorStackInterval;    // in seconds
    CSmallTime      LastFlushErrorStackTime;
    CSmallString    ErrorLogFileName;

    // main loop
    virtual void ExecuteThread(void);
};

//------------------------------------------------------------------------------

#endif
