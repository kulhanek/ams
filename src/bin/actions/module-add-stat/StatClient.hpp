#ifndef StatClientH
#define StatClientH
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

#include <SmallTimeAndDate.hpp>
#include <SoftStat.hpp>
#include <VerboseStr.hpp>
#include <TerminalStr.hpp>
#include "StatClientOptions.hpp"

// -----------------------------------------------------------------------------

class CStatClient {
public:
// constructor and destructors -------------------------------------------------
    CStatClient(void);

// main methods ----------------------------------------------------------------
    /// init options
    int Init(int argc,char* argv[]);

    /// main part of program
    bool Run(void);

    /// finalize
    void Finalize(void);

// executive methods -----------------------------------------------------------
    /// populate datagram with statistics data
    bool PopulateDatagram(const CSmallString& build,int flags);

    /// send data to server
    bool SendDataToServer(const CSmallString& servername,int port);

// section of private data -----------------------------------------------------
private:
    CStatClientOptions  Options;
    CAddStatDatagram    Datagram;
    CTerminalStr        Console;
    CVerboseStr         vout;
};

// -----------------------------------------------------------------------------

#endif
