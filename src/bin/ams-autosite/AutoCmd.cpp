// =============================================================================
// AMS
// -----------------------------------------------------------------------------
//    Copyright (C) 2011      Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2001-2008 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include "AutoCmd.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <GlobalConfig.hpp>
#include <Site.hpp>
#include <Host.hpp>
#include <DirectoryEnum.hpp>
#include <AmsUUID.hpp>
#include "prefix.h"
#include <iomanip>
#include <User.hpp>

//------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------

MAIN_ENTRY(CAutoCmd)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CAutoCmd::Init(int argc, char* argv[])
{
    // encode program options, all check procedures are done inside of Options
    int result = Options.ParseCmdLine(argc,argv);

    // should we exit or was it error?
    if( result != SO_CONTINUE ) return(result);

    // output must be directed to stderr
    // stdout is used for shell processor
    Console.Attach(stdout);

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
    vout << "# ams-autosite (AMS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    vout << low;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

class CSiteRec {
    public:
    CSmallString    Name;
    int             Priority;
};

//------------------------------------------------------------------------------

bool SitePriorityCompare(const CSiteRec& left,const CSiteRec& right)
{
    return( left.Priority >= right.Priority );
}

//------------------------------------------------------------------------------

bool CAutoCmd::Run(void)
{
    // init global host and user data
    Host.InitGlobalSetup();
    User.InitGlobalSetup();

    if( GlobalConfig.GetActiveSiteID() != NULL ){
        // initialize hosts -----------------------------
        Host.InitHostFile(GlobalConfig.GetActiveSiteID());
        Host.InitHost();
    }

    vout << high;

    // make list of all available sites -------------
    CDirectoryEnum      dir_enum(BR_ETCDIR("/sites"));
    CFileName           site_sid;
    std::list<CSiteRec> sites;

    dir_enum.StartFindFile("{*}");

    while( dir_enum.FindFile(site_sid) ) {
        CAmsUUID    site_id;
        vout << endl;
        if( site_id.LoadFromString(site_sid) == false ) continue;
        vout << ">>> site: " << site_id.GetFullStringForm() << endl;

        CSite site;
        if( site.LoadConfig(site_sid) == false ) {
            vout << "    unable load config" << endl;
            continue;
        }
            vout << "    Name     : " << site.GetName() << endl;
        int priority = 0;
        if( site.CanBeActivated(priority,true) ){
            vout << "    Priority : " << priority << endl;
            CSiteRec srec;
            srec.Name = site.GetName();
            srec.Priority = priority;
            sites.push_back(srec);
        } else {
            vout << "    Priority : -not allowed on this host-" << endl;
        }
    }
    dir_enum.EndFindFile();
    vout << endl;

    // sort allowed sites by their priorities
    sites.sort(SitePriorityCompare);

    std::list<CSiteRec>::iterator it = sites.begin();
    std::list<CSiteRec>::iterator ie = sites.end();

    vout << "*** Allowed sites ***" << endl;
    while( it != ie ){
        vout << setw(5) << (*it).Priority << " " << (*it).Name << endl;
        it++;
    }
    vout << endl;

    // print best site
    vout << low;
    if( ! sites.empty() ){
        vout << sites.front().Name;
    }

    vout << high;
    vout << endl;

    return(true);
}

//------------------------------------------------------------------------------

void CAutoCmd::Finalize(void)
{
    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << endl;
    vout << "# ==============================================================================" << endl;
    vout << "# ams-autosite (AMS utility) terminated at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    if( ErrorSystem.IsError() || (ErrorSystem.IsAnyRecord() && Options.GetOptVerbose()) ){
        ErrorSystem.PrintErrors(vout);       
        vout << endl;
    }   
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


