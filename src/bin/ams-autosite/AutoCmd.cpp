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
#include <Utils.hpp>
#include <XMLElement.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

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

bool CAutoCmd::Run(void)
{
    // init global host and user data
    Host.InitGlobalSetup();
    User.InitGlobalSetup();

    // initialize hosts -----------------------------
    Host.InitHostFile();
    Host.InitHost();

    vout << high;
        vout << "Host               : " << Host.GetHostName() << endl;

    CSmallString primary,transferable,others;

    CXMLElement* p_gele = Host.FindGroup();
    if( p_gele != NULL ){
        CSmallString gname;
        p_gele->GetAttribute("name",gname);
        vout << "Group              : " << gname << endl;
        CXMLElement* p_sele = p_gele->GetFirstChildElement("sites");
        if( p_sele != NULL ){
            p_sele->GetAttribute("primary",primary);
            p_sele->GetAttribute("transferable",transferable);
            if( transferable == NULL ) transferable="-none-";
            p_sele->GetAttribute("others",others);
            if( others == NULL ) others="-none-";
        vout << "Primary site       : " << primary << endl;
        vout << "Transferable sites : " << transferable << endl;
        vout << "Other sites        : " << others << endl;
        } else{
        vout << ">>> INFO: <sites></sites> element not found in the hosts.xml file!" << endl;
        }
    } else {
        vout << "Group              : - not found-" << endl;
    }

    CSmallString best_site;

    if( Options.IsOptTransferSiteSet() == true ){
        CSmallString tname;
        if( ! CUtils::IsSiteIDValid(Options.GetOptTransferSite()) ){
            tname = Options.GetOptTransferSite();
        vout << "Transferred site   : " << tname << endl;
        } else {
            tname = CUtils::GetSiteName(Options.GetOptTransferSite());
        vout << "Transferred site   : " << Options.GetOptTransferSite() << " (" << tname << ")" << endl;
        }

        string stransferrable(transferable);
        // split string into tokens
        std::vector<std::string> transferrables;
        split(transferrables,stransferrable,is_any_of(","));

        // is it allowed?
        if(std::find(transferrables.begin(), transferrables.end(), string(tname)) != transferrables.end()) {
            if( Options.GetOptIsTransferable() == true ){
                vout << low;
                vout << ">>> INFO: The source site '" << tname << "' is allowed to be transfered to this host ..." << endl;
            }
            best_site =  tname;
        } else {
            if( Options.GetOptIsTransferable() == false ){
                vout << ">>> INFO: The transferred site is not allowed - using default!" << endl;
            } else {
                vout << low;
                vout << ">>> INFO: The source site '" << tname << "' is not allowed to be transfered to this host, using default ..." << endl;
            }
        }
    }

    if( best_site == NULL ){
        // default site
        best_site = primary;
    }

    if( best_site == NULL ){
        best_site = "none";     // return something that is recognized by the site command as a no-site case
    }

    // print best site
    if( Options.GetOptIsTransferable() == false ){
        vout << "Selected site      : ";
        vout << low;
        vout << best_site;
        vout << high;
    }

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
    } else {
        vout << endl;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


