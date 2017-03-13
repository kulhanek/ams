#ifndef TorqueH
#define TorqueH
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

#include <AMSMainHeader.hpp>
#include <DynamicPackage.hpp>
#include <iostream>

// -----------------------------------------------------------------------------

class CNodeList;

// -----------------------------------------------------------------------------

typedef int (*PBS_CONNECT)(const char*);
typedef int (*PBS_DISCONNECT)(int);
typedef struct batch_status* (*PBS_STATSERVER)(int,struct attrl*,char*);
typedef struct batch_status* (*PBS_STATNODE)(int,char*,struct attrl*,char*);
typedef void (*PBS_STATFREE)(struct batch_status *);
typedef char* (*PBS_GETERRMSG)(int connect);


// -----------------------------------------------------------------------------

class AMS_PACKAGE CTorque {
public:
// constructor -----------------------------------------------------------------
        CTorque(void);
        ~CTorque(void);

// init torque subsystem -------------------------------------------------------
    /// load symbols and connect to server
    bool Init(const CSmallString& tlib,const CSmallString& tsrv);

// enumeration -----------------------------------------------------------------
    /// get node info
    bool GetNodeInfo(const CSmallString& node,int& ncpus,CSmallString& props);

// section of private data -----------------------------------------------------
private:
    CSmallString    TorqueLibName;
    CSmallString    ServerName;
    CDynamicPackage TorqueLib;
    int             ServerID;

    // init symbols
    bool InitSymbols(void);

    // connect to server
    bool ConnectToServer(void);

    // disconnect from server
    bool DisconnectFromServer(void);

    // torque api symbols
    PBS_CONNECT     pbs_connect;
    PBS_DISCONNECT  pbs_disconnect;
    PBS_STATSERVER  pbs_statserver;
    PBS_STATNODE    pbs_statnode;
    PBS_STATFREE    pbs_statfree;
    PBS_GETERRMSG   pbs_geterrmsg;
};

// -----------------------------------------------------------------------------

extern CTorque Torque;

// -----------------------------------------------------------------------------

#endif
