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

#include <XMLParser.hpp>
#include <ErrorSystem.hpp>
#include <FileName.hpp>
#include <Shell.hpp>
#include <Utils.hpp>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fnmatch.h>
#include "StatClient.hpp"
#include <ModUtils.hpp>

//------------------------------------------------------------------------------

#define MAX_NET_NAME 255

//------------------------------------------------------------------------------

MAIN_ENTRY(CStatClient)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CStatClient::CStatClient(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CStatClient::Init(int argc, char* argv[])
{
    // encode program options, all check procedures are done inside of CABFIntOpts
    int result = Options.ParseCmdLine(argc,argv);

    // should we exit or was it error?
    if( result != SO_CONTINUE ) return(result);

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CStatClient::Run(void)
{
    if( PopulateDatagram(Options.GetArgBuild(),Options.GetArgBundle(),Options.GetArgFlags()) == false ) {
        ES_ERROR("unable to populate datagram");
        return(false);
    }

    if( SendDataToServer(Options.GetArgServerName(),Options.GetOptPort()) == false ) {
        ES_ERROR("unable to send datagram");
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

void CStatClient::Finalize(void)
{
    if( Options.GetOptVerbose() ) {
        ErrorSystem.PrintErrors(stderr);
        fprintf(stderr,"\n");
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CStatClient::PopulateDatagram(const CSmallString& build,const CSmallString& bname,int flags)
{
    CSmallString site_sid;

    site_sid = CShell::GetSystemVariable("AMS_SITE");
    if( site_sid == NULL ) {
        ES_ERROR("no active site");
        return(false);
    }

    Datagram.SetSite(site_sid);

    CSmallString name,vers,arch,mode;
    CModUtils::ParseModuleName(build,name,vers,arch,mode);

    Datagram.SetModuleName(name);
    Datagram.SetModuleVers(vers);
    Datagram.SetModuleArch(arch);
    Datagram.SetModuleMode(mode);

    if( bname != NULL ){
        Datagram.SetBundleName(bname);
    } else {
        Datagram.SetBundleName("unknown");
    }

    CSmallString user = CShell::GetSystemVariable("USER");
    if( user == NULL ){
        // fix reporting in various cron deamons
        user = CShell::GetSystemVariable("LOGNAME");
    }

    // trim user name - due to pam_ldap that sometime leaves spaces
    // around to user name
    user.Trim();
    // it also passes names as they are typed by users
    user.ToLowerCase();

    Datagram.SetUser(user);
    Datagram.SetHostName(CShell::GetSystemVariable("HOSTNAME"));

    int ncpus = CShell::GetSystemVariable("AMS_NCPUS").ToInt();
    if( ncpus <= 0 ) ncpus = 1;
    Datagram.SetNCPUs(ncpus);
    ncpus = CShell::GetSystemVariable("AMS_NHOSTCPUS").ToInt();
    Datagram.SetNumOfHostCPUs(ncpus);

    int ngpus = CShell::GetSystemVariable("AMS_NGPUS").ToInt();
    Datagram.SetNGPUs(ngpus);
    ngpus = CShell::GetSystemVariable("AMS_NHOSTGPUS").ToInt();
    Datagram.SetNumOfHostGPUs(ngpus);

    int nnodes = CShell::GetSystemVariable("AMS_NNODES").ToInt();
    if( nnodes <= 0 ) nnodes = 1;
    Datagram.SetNumOfNodes(nnodes);

    Datagram.SetFlags(flags);

    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();
    Datagram.SetTimeAndDate(dt);

    Datagram.Finish();

    return(true);
}

//------------------------------------------------------------------------------

bool CStatClient::SendDataToServer(const CSmallString& servername,int port)
{
    // get IP address of server ---------------------
    addrinfo*      p_addrinfo;
    int            nerr;

    // get hostname and IP
    if( (nerr = getaddrinfo(servername,NULL,NULL,&p_addrinfo)) != 0 ) {
        CSmallString error;
        error << "unable to decode server name '" << servername
              << "' (" <<  gai_strerror(nerr) << ")";
        ES_ERROR(error);
        return(false);
    }

    // get server name
    char s_name[MAX_NET_NAME];
    memset(s_name,0,MAX_NET_NAME);

    if( (nerr = getnameinfo(p_addrinfo->ai_addr,p_addrinfo->ai_addrlen,s_name,
                            MAX_NET_NAME-1,NULL,0,0)) != 0 ) {
        CSmallString error;
        error << "unable to get server name (" <<  gai_strerror(nerr) << ")";
        ES_ERROR(error);
        return(false);
    }
    CSmallString name = s_name;

    // get server IP
    if( (nerr = getnameinfo(p_addrinfo->ai_addr,p_addrinfo->ai_addrlen,s_name,
                            MAX_NET_NAME-1,NULL,0,NI_NUMERICHOST)) != 0 ) {
        CSmallString error;
        error << "unable to get server IP (" <<  gai_strerror(nerr) << ")";
        ES_ERROR(error);
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
        ES_ERROR(error);
        return(false);
    }

    int result = connect(sfd,(sockaddr*)&server_addr,sizeof(server_addr));

    if( result == -1 ) {
        CSmallString error;
        error << "unable to connect to server " << name
              << "[" << ip << "] (" << strerror(errno) << ")";
        ES_ERROR(error);
        return(false);
    }

    // send module datagram to server

    if (send(sfd, &Datagram, sizeof(Datagram), MSG_NOSIGNAL) != sizeof(Datagram)) {
        ES_ERROR("unable to send datagram");
        exit(EXIT_FAILURE);
    }

    // close stream
    close(sfd);

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
