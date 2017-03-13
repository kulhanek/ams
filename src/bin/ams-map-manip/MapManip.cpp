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

#include "MapManip.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <GlobalConfig.hpp>
#include <Map.hpp>
#include <Site.hpp>
#include <Cache.hpp>
#include <DirectoryEnum.hpp>
#include <prefix.h>
#include <AmsUUID.hpp>

//------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------

MAIN_ENTRY(CMapManip)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CMapManip::CMapManip(void)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CMapManip::Init(int argc, char* argv[])
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
    vout << "# ams-map-manip (AMS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    vout << low;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CMapManip::Run(void)
{
    Map.LoadSiteAliases();

    // ----------------------------------------------
    if( Options.GetArgAction() == "addbuilds" ) {
        if( Map.LoadMap() == false ) return(false);
        if( Options.IsOptPrefixSet() ){
            if( Map.LoadAutoPrefixes(Options.GetOptPrefix()) == false ) return(false);
        } else {
            if( Map.LoadAutoPrefixes(GlobalConfig.GetActiveSiteName()) == false ) return(false);
        }
        if( Map.AddBuildsForSites(vout,Options.GetProgArg(1),Options.GetProgArg(2)) == false ) {
            ES_TRACE_ERROR("unable to distribute builds");
            return(false);
        }
        vout << endl;
        if( Map.SaveMap() == false ) return(false);
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "rmbuilds" ) {
        if( Map.LoadMap() == false ) return(false);
        Map.RemoveBuildsForSites(vout,Options.GetProgArg(1),Options.GetProgArg(2));
        vout << endl;
        if( Map.SaveMap() == false ) return(false);
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "showprefixes" ) {
        Map.ShowPrefixes(vout);
        vout << endl;
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "showallbuilds" ) {
        Map.ShowAllBuilds(vout);
        vout << endl;
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "showbuilds" ) {
        Map.ShowBuilds(vout,Options.GetProgArg(1),Options.GetProgArg(2));
        vout << endl;
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "setdefault" ) {
        if( Map.LoadMap() == false ) return(false);
        if( Map.SetDefaultForSites(Options.GetProgArg(1),Options.GetProgArg(2)) == false ) {
            ES_TRACE_ERROR("unable to set default");
            return(false);
        }
        if( Map.SaveMap() == false ) return(false);
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "rmdefault" ) {
        if( Map.LoadMap() == false ) return(false);
        if( Map.RemoveDefaultForSites(Options.GetProgArg(1),Options.GetProgArg(2)) == false ) {
            ES_TRACE_ERROR("unable to remove default");
            return(false);
        }
        if( Map.SaveMap() == false ) return(false);
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "rmmodule" ) {
        if( Map.LoadMap() == false ) return(false);
        if( Map.RemoveModuleForSites(Options.GetProgArg(1),Options.GetProgArg(2)) == false ) {
            ES_TRACE_ERROR("unable to remove module");
            return(false);
        }
        if( Map.SaveMap() == false ) return(false);
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "showmap" ) {
        if( Map.LoadMap() == false ) return(false);
        Map.ShowMapForSites(vout,Options.GetProgArg(1));
        vout << endl;
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "rmmap" ) {
        if( Map.LoadMap() == false ) return(false);
        if( Map.RemoveMapForSites(vout,Options.GetProgArg(1)) == false ) return(false);
        if( Map.SaveMap() == false ) return(false);
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "cpmap" ) {
        if( Map.LoadMap() == false ) return(false);
        if( Map.CopyMap(Options.GetProgArg(1),Options.GetProgArg(2)) == false ) return(false);
        if( Map.SaveMap() == false ) return(false);
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "rmorphansites" ) {
        if( Map.LoadMap() == false ) return(false);
        Map.RemoveOrphanSites(vout);
        if( Map.SaveMap() == false ) return(false);
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "newverindex" ) {
        if( Map.LoadMap() == false ) return(false);
        double verindex = Map.GetNewVerIndex(Options.GetProgArg(1));
        vout << verindex;
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "updateverindexes" ) {
        if( Map.LoadMap() == false ) return(false);
        Map.UpdateVerIndexes(vout);
        if( Map.SaveMap() == false ) return(false);
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "distribute" ) {
        if( Map.LoadMap() == false ) return(false);
        if( Map.DistributeAll(vout) == false ) {
            ES_TRACE_ERROR("unable to distribute builds");
            return(false);
        }
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "numofundos" ) {
        vout << Map.GetNumOfUndoMapChanges() << endl;
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "undo" ) {
        if( Map.GetNumOfUndoMapChanges() == 0 ){
            vout << endl;
            vout << "<b><red> ERROR: no changes can be undone!</red></b>" << endl;
            vout << endl;
            ES_TRACE_ERROR("no changes can be undone");
            return(false);
        }
        Map.UndoMapChange();
        vout << "Remaining number of changes that can be undone : " << Map.GetNumOfUndoMapChanges() << endl;
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "aliases" ) {
        Map.PrintSiteAliases(vout);
        vout << endl;
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "isbuild" ) {
        return( Map.IsBuild(Options.GetProgArg(1),Options.GetProgArg(2),Options.GetOptPrefix()) );
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "showsyncdeps" ) {
        Map.ShowSyncDeps(vout,Options.GetProgArg(1),Options.GetProgArg(2),Options.GetOptPrefix(),false);
        return(true);
    }
    // ----------------------------------------------
    else if( Options.GetArgAction() == "deepsyncdeps" ) {
        Map.ShowSyncDeps(vout,Options.GetProgArg(1),Options.GetProgArg(2),Options.GetOptPrefix(),true);
        return(true);
    }
    // ----------------------------------------------
    else {
        ES_ERROR("not implemented action");
        return(false);
    }
}

//------------------------------------------------------------------------------

void CMapManip::Finalize(void)
{
    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << "# ==============================================================================" << endl;
    vout << "# ams-map-manip (AMS utility) terminated at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    vout << low;
    if( ErrorSystem.IsError() || (ErrorSystem.IsAnyRecord() && Options.GetOptVerbose()) ){
        ErrorSystem.PrintErrors(vout);
        vout << endl;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================




