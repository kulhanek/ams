#ifndef AddDatagramSenderH
#define AddDatagramSenderH
// =============================================================================
//  AMS - Advanced Module System
// -----------------------------------------------------------------------------
//     Copyright (C) 2024 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include <AMSMainHeader.hpp>
#include <SmallString.hpp>
#include <SoftStat.hpp>
#include <StatDatagramSender.hpp>

//------------------------------------------------------------------------------

class CAddDatagramSender : public CStatDatagramSender {
public:
    /// send datagram
    virtual bool SendDataToServer(const CSmallString& servername,int port);

    /// get datagram flags
    virtual int GetFlags(void);

// section of public data ------------------------------------------------------
public:
    CAddStatDatagram   Datagram;
};

//------------------------------------------------------------------------------

#endif
