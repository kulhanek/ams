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

#include <AddDatagramSender.hpp>
#include <ErrorSystem.hpp>

#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//------------------------------------------------------------------------------

#define MAX_NET_NAME 255

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CAddDatagramSender::GetFlags(void)
{
    return(Datagram.GetFlags());
}

//------------------------------------------------------------------------------

bool CAddDatagramSender::SendDataToServer(const CSmallString& servername,int port)
{
    // get IP address of server ---------------------
    addrinfo*      p_addrinfo;
    int            nerr;

    // get hostname and IP
    if( (nerr = getaddrinfo(servername,NULL,NULL,&p_addrinfo)) != 0 ) {
        CSmallString error;
        error << "unable to decode server name '" << servername
              << "' (" <<  gai_strerror(nerr) << ")";
        ES_WARNING(error);
        return(false);
    }

    // get server name
    char s_name[MAX_NET_NAME];
    memset(s_name,0,MAX_NET_NAME);

    if( (nerr = getnameinfo(p_addrinfo->ai_addr,p_addrinfo->ai_addrlen,s_name,
                            MAX_NET_NAME-1,NULL,0,0)) != 0 ) {
        CSmallString error;
        error << "unable to get server name (" <<  gai_strerror(nerr) << ")";
        ES_WARNING(error);
        return(false);
    }
    CSmallString name = s_name;

    // get server IP
    if( (nerr = getnameinfo(p_addrinfo->ai_addr,p_addrinfo->ai_addrlen,s_name,
                            MAX_NET_NAME-1,NULL,0,NI_NUMERICHOST)) != 0 ) {
        CSmallString error;
        error << "unable to get server IP (" <<  gai_strerror(nerr) << ")";
        ES_WARNING(error);
        return(false);
    }
    CSmallString ip = s_name;

    freeaddrinfo(p_addrinfo);

    // Obtain address(es) matching host/port
    sockaddr_in    server_addr;

    server_addr.sin_port = htons(port);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    int sfd;

    sfd = socket(AF_INET,SOCK_DGRAM,0);
    if( sfd == -1 ) {
        CSmallString error;
        error << "unable to create socket (" << strerror(errno) << ")";
        ES_WARNING(error);
        return(false);
    }

    int result = connect(sfd,(sockaddr*)&server_addr,sizeof(server_addr));

    if( result == -1 ) {
        CSmallString error;
        error << "unable to connect to server " << name
              << "[" << ip << "] (" << strerror(errno) << ")";
        ES_WARNING(error);
        return(false);
    }

    // send module datagram to server

    if (send(sfd, &Datagram, sizeof(Datagram), MSG_NOSIGNAL) != sizeof(Datagram)) {
        ES_WARNING("unable to send datagram");
        exit(EXIT_FAILURE);
    }

    // close stream
    close(sfd);

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
