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

#include <Torque.hpp>
#include <ErrorSystem.hpp>
#include <iostream>
#include <pbs_ifl.h>
#include <stdlib.h>
#include <TorqueAttr.hpp>
#include <FileName.hpp>
#include <Shell.hpp>

using namespace std;

// -----------------------------------------------------------------------------

CTorque Torque;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CTorque::CTorque(void)
{
    ServerID = 0;
}

//------------------------------------------------------------------------------

CTorque::~CTorque(void)
{
    DisconnectFromServer();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CTorque::Init(const CSmallString& tlib,const CSmallString& tsrv)
{
    TorqueLibName = tlib;
    ServerName = tsrv;

    if( InitSymbols() == false ){
        ES_ERROR("unable to init symbols");
        return(false);
    }
    if( ConnectToServer() == false ){
        ES_ERROR("unable to connect to server");
        return(false);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CTorque::InitSymbols(void)
{
    if( TorqueLib.Open(TorqueLibName) == false ){
        ES_ERROR("unable to load torque library");
        return(false);
    }

    // load symbols
    bool status = true;
    pbs_connect     = (PBS_CONNECT)TorqueLib.GetProcAddress("pbs_connect");
    if( pbs_connect == NULL ){
        ES_ERROR("unable to bind to pbs_connect");
        status = false;
    }
    pbs_disconnect  = (PBS_DISCONNECT)TorqueLib.GetProcAddress("pbs_disconnect");
    if( pbs_disconnect == NULL ){
        ES_ERROR("unable to bind to pbs_disconnect");
        status = false;
    }
    pbs_statserver  = (PBS_STATSERVER)TorqueLib.GetProcAddress("pbs_statserver");
    if( pbs_statserver == NULL ){
        ES_ERROR("unable to bind to pbs_statserver");
        status = false;
    }
    pbs_statnode  = (PBS_STATNODE)TorqueLib.GetProcAddress("pbs_statnode");
    if( pbs_statnode == NULL ){
        ES_ERROR("unable to bind to pbs_statnode");
        status = false;
    }
    pbs_statfree  = (PBS_STATFREE)TorqueLib.GetProcAddress("pbs_statfree");
    if( pbs_statfree == NULL ){
        ES_ERROR("unable to bind to pbs_statfree");
        status = false;
    }
    pbs_geterrmsg  = (PBS_GETERRMSG)TorqueLib.GetProcAddress("pbs_geterrmsg");
    if( pbs_geterrmsg == NULL ){
        ES_ERROR("unable to bind to pbs_geterrmsg");
        status = false;
    }

    return(status);
}

//------------------------------------------------------------------------------

bool CTorque::ConnectToServer(void)
{
    ServerID = pbs_connect(ServerName);
    if( ServerID == 0 ){
        ES_ERROR("unable to connect to server");
        return(false);
    }
    return(true);
}

//------------------------------------------------------------------------------

bool CTorque::DisconnectFromServer(void)
{
    if( ServerID <= 0 ) return(true);

    int error = pbs_disconnect(ServerID);
    ServerID = 0;
    if( error != 0 ){
        ES_ERROR("unable to disconnect from server");
        return(false);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CTorque::GetNodeInfo(const CSmallString& node,int& ncpus,CSmallString& props)
{
    struct batch_status* p_nodes = pbs_statnode(ServerID,(char*)node.GetBuffer(),NULL,NULL);
    if( p_nodes == NULL ){
        CSmallString error;
        error << "unable to get info about node '" << node << "'";
        ES_TRACE_ERROR(error);
        return(false);
    }

    ncpus = 1;
    get_attribute(p_nodes->attribs,ATTR_NODE_NP,NULL,ncpus);
    props = 1;
    get_attribute(p_nodes->attribs,ATTR_NODE_PROPS,NULL,props);

    pbs_statfree(p_nodes);

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

