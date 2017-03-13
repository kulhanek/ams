// =============================================================================
//  AMS - Advanced Module System
// -----------------------------------------------------------------------------
//     Copyright (C) 2012 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2011 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2004,2005,2008,2010 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include <SoftStat.hpp>
#include <ErrorSystem.hpp>
#include <string.h>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CAddStatDatagram::CAddStatDatagram(void)
{
    memset(Magic,0,sizeof(Magic));
    memset(Control,0,sizeof(Control));
    memset(Site,0,sizeof(Site));
    memset(ModuleName,0,sizeof(ModuleName));
    memset(ModuleVers,0,sizeof(ModuleVers));
    memset(ModuleArch,0,sizeof(ModuleArch));
    memset(ModuleMode,0,sizeof(ModuleMode));
    memset(User,0,sizeof(User));
    memset(HostName,0,sizeof(HostName));
    memset(NCPUs,0,sizeof(NCPUs));
    memset(NumOfHostCPUs,0,sizeof(NumOfHostCPUs));
    memset(NGPUs,0,sizeof(NGPUs));
    memset(NumOfHostGPUs,0,sizeof(NumOfHostGPUs));
    memset(NumOfNodes,0,sizeof(NumOfNodes));
    memset(Flags,0,sizeof(Flags));
    memset(Time,0,sizeof(Time));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CAddStatDatagram::Finish(void)
{
    // magic header
    strncpy(Magic,"AMS6",4);

    // controll sum
    int control_sum = 0;
    for(unsigned int i=0; i < sizeof(Magic); i++) control_sum += Magic[i];
    for(unsigned int i=0; i < sizeof(Site); i++) control_sum += Site[i];
    for(unsigned int i=0; i < sizeof(ModuleName); i++) control_sum += ModuleName[i];
    for(unsigned int i=0; i < sizeof(ModuleVers); i++) control_sum += ModuleVers[i];
    for(unsigned int i=0; i < sizeof(ModuleArch); i++) control_sum += ModuleArch[i];
    for(unsigned int i=0; i < sizeof(ModuleMode); i++) control_sum += ModuleMode[i];
    for(unsigned int i=0; i < sizeof(User); i++) control_sum += User[i];
    for(unsigned int i=0; i < sizeof(HostName); i++) control_sum += HostName[i];
    for(unsigned int i=0; i < sizeof(NCPUs); i++) control_sum += NCPUs[i];
    for(unsigned int i=0; i < sizeof(NumOfHostCPUs); i++) control_sum += NumOfHostCPUs[i];
    for(unsigned int i=0; i < sizeof(NGPUs); i++) control_sum += NGPUs[i];
    for(unsigned int i=0; i < sizeof(NumOfHostGPUs); i++) control_sum += NumOfHostGPUs[i];
    for(unsigned int i=0; i < sizeof(NumOfNodes); i++) control_sum += NumOfNodes[i];
    for(unsigned int i=0; i < sizeof(Flags); i++) control_sum += Flags[i];
    for(unsigned int i=0; i < sizeof(Time); i++) control_sum += Time[i];

    Control[0] = (unsigned char) ((control_sum >> 24) & 0xFF);
    Control[1] = (unsigned char) ((control_sum >> 16) & 0xFF);
    Control[2] = (unsigned char) ((control_sum >>  8) & 0xFF);
    Control[3] = (unsigned char) ((control_sum      ) & 0xFF);
}

//------------------------------------------------------------------------------

void CAddStatDatagram::SetSite(const CSmallString& site)
{
    // -1 for \0 termination character
    strncpy(Site,site,sizeof(Site)-1);
}

//------------------------------------------------------------------------------

void CAddStatDatagram::SetModuleName(const CSmallString& name)
{
    strncpy(ModuleName,name,sizeof(ModuleName)-1);
}

//------------------------------------------------------------------------------

void CAddStatDatagram::SetModuleVers(const CSmallString& vers)
{
    strncpy(ModuleVers,vers,sizeof(ModuleVers)-1);
}

//------------------------------------------------------------------------------

void CAddStatDatagram::SetModuleArch(const CSmallString& arch)
{
    strncpy(ModuleArch,arch,sizeof(ModuleArch)-1);
}

//------------------------------------------------------------------------------

void CAddStatDatagram::SetModuleMode(const CSmallString& mode)
{
    strncpy(ModuleMode,mode,sizeof(ModuleMode)-1);
}

//------------------------------------------------------------------------------

void CAddStatDatagram::SetUser(const CSmallString& user)
{
    strncpy(User,user,sizeof(User)-1);
}

//------------------------------------------------------------------------------

void CAddStatDatagram::SetHostName(const CSmallString& name)
{
    strncpy(HostName,name,sizeof(HostName)-1);
}

//------------------------------------------------------------------------------

void CAddStatDatagram::SetNCPUs(int ncpus)
{
    NCPUs[0] = (unsigned char) ((ncpus >> 24) & 0xFF);
    NCPUs[1] = (unsigned char) ((ncpus >> 16) & 0xFF);
    NCPUs[2] = (unsigned char) ((ncpus >>  8) & 0xFF);
    NCPUs[3] = (unsigned char) ((ncpus      ) & 0xFF);
}

//------------------------------------------------------------------------------

void CAddStatDatagram::SetNumOfHostCPUs(int ncpus)
{
    NumOfHostCPUs[0] = (unsigned char) ((ncpus >> 24) & 0xFF);
    NumOfHostCPUs[1] = (unsigned char) ((ncpus >> 16) & 0xFF);
    NumOfHostCPUs[2] = (unsigned char) ((ncpus >>  8) & 0xFF);
    NumOfHostCPUs[3] = (unsigned char) ((ncpus      ) & 0xFF);
}

//------------------------------------------------------------------------------

void CAddStatDatagram::SetNGPUs(int ngpus)
{
    NGPUs[0] = (unsigned char) ((ngpus >> 24) & 0xFF);
    NGPUs[1] = (unsigned char) ((ngpus >> 16) & 0xFF);
    NGPUs[2] = (unsigned char) ((ngpus >>  8) & 0xFF);
    NGPUs[3] = (unsigned char) ((ngpus      ) & 0xFF);
}

//------------------------------------------------------------------------------

void CAddStatDatagram::SetNumOfHostGPUs(int ngpus)
{
    NumOfHostGPUs[0] = (unsigned char) ((ngpus >> 24) & 0xFF);
    NumOfHostGPUs[1] = (unsigned char) ((ngpus >> 16) & 0xFF);
    NumOfHostGPUs[2] = (unsigned char) ((ngpus >>  8) & 0xFF);
    NumOfHostGPUs[3] = (unsigned char) ((ngpus      ) & 0xFF);
}

//------------------------------------------------------------------------------

void CAddStatDatagram::SetNumOfNodes(int nnodes)
{
    NumOfNodes[0] = (unsigned char) ((nnodes >> 24) & 0xFF);
    NumOfNodes[1] = (unsigned char) ((nnodes >> 16) & 0xFF);
    NumOfNodes[2] = (unsigned char) ((nnodes >>  8) & 0xFF);
    NumOfNodes[3] = (unsigned char) ((nnodes      ) & 0xFF);
}

//------------------------------------------------------------------------------

void CAddStatDatagram::SetFlags(int flags)
{
    Flags[0] = (unsigned char) ((flags >> 24) & 0xFF);
    Flags[1] = (unsigned char) ((flags >> 16) & 0xFF);
    Flags[2] = (unsigned char) ((flags >>  8) & 0xFF);
    Flags[3] = (unsigned char) ((flags      ) & 0xFF);
}

//------------------------------------------------------------------------------

void CAddStatDatagram::SetTimeAndDate(const CSmallTimeAndDate& dt)
{
    int seconds = dt.GetSecondsFromBeginning();

    Time[0] = (unsigned char) ((seconds >> 24) & 0xFF);
    Time[1] = (unsigned char) ((seconds >> 16) & 0xFF);
    Time[2] = (unsigned char) ((seconds >>  8) & 0xFF);
    Time[3] = (unsigned char) ((seconds      ) & 0xFF);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CAddStatDatagram::IsValid(void)
{
    // magic header
    if( strncmp(Magic,"AMS6",4) != 0 ) {
        return(false);
    }

    // controll sum
    int control_sum = 0;
    for(unsigned int i=0; i < sizeof(Magic); i++) control_sum += Magic[i];
    for(unsigned int i=0; i < sizeof(Site); i++) control_sum += Site[i];
    for(unsigned int i=0; i < sizeof(ModuleName); i++) control_sum += ModuleName[i];
    for(unsigned int i=0; i < sizeof(ModuleVers); i++) control_sum += ModuleVers[i];
    for(unsigned int i=0; i < sizeof(ModuleArch); i++) control_sum += ModuleArch[i];
    for(unsigned int i=0; i < sizeof(ModuleMode); i++) control_sum += ModuleMode[i];
    for(unsigned int i=0; i < sizeof(User); i++) control_sum += User[i];
    for(unsigned int i=0; i < sizeof(HostName); i++) control_sum += HostName[i];
    for(unsigned int i=0; i < sizeof(NCPUs); i++) control_sum += NCPUs[i];
    for(unsigned int i=0; i < sizeof(NumOfHostCPUs); i++) control_sum += NumOfHostCPUs[i];
    for(unsigned int i=0; i < sizeof(NGPUs); i++) control_sum += NGPUs[i];
    for(unsigned int i=0; i < sizeof(NumOfHostGPUs); i++) control_sum += NumOfHostGPUs[i];
    for(unsigned int i=0; i < sizeof(NumOfNodes); i++) control_sum += NumOfNodes[i];
    for(unsigned int i=0; i < sizeof(Flags); i++) control_sum += Flags[i];
    for(unsigned int i=0; i < sizeof(Time); i++) control_sum += Time[i];

    int rec_control_sum;
    rec_control_sum = (Control[0] << 24) + (Control[1] << 16)
                      + (Control[2] << 8) + Control[3];

    if( rec_control_sum != control_sum ) {
        CSmallString error;
        error << "sum: " << CSmallString(rec_control_sum) << " rec: " << CSmallString(control_sum);
        ES_ERROR(error);
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

const CSmallString CAddStatDatagram::GetSite(void) const
{
    return(Site);
}

//------------------------------------------------------------------------------

const CSmallString CAddStatDatagram::GetModuleName(void) const
{
    return(ModuleName);
}

//------------------------------------------------------------------------------

const CSmallString CAddStatDatagram::GetModuleVers(void) const
{
    return(ModuleVers);
}

//------------------------------------------------------------------------------

const CSmallString CAddStatDatagram::GetModuleArch(void) const
{
    return(ModuleArch);
}

//------------------------------------------------------------------------------

const CSmallString CAddStatDatagram::GetModuleMode(void) const
{
    return(ModuleMode);
}

//------------------------------------------------------------------------------

const CSmallString CAddStatDatagram::GetUser(void) const
{
    return(User);
}

//------------------------------------------------------------------------------

const CSmallString CAddStatDatagram::GetHostName(void) const
{
    return(HostName);
}

//------------------------------------------------------------------------------

int CAddStatDatagram::GetNCPUs(void) const
{
    return( (NCPUs[0] << 24) + (NCPUs[1] << 16) + (NCPUs[2] << 8) + NCPUs[3] );
}

//------------------------------------------------------------------------------

int CAddStatDatagram::GetNumOfHostCPUs(void) const
{
    return( (NumOfHostCPUs[0] << 24) + (NumOfHostCPUs[1] << 16)
            + (NumOfHostCPUs[2] << 8) + NumOfHostCPUs[3] );
}

//------------------------------------------------------------------------------

int CAddStatDatagram::GetNGPUs(void) const
{
    return( (NGPUs[0] << 24) + (NGPUs[1] << 16) + (NGPUs[2] << 8) + NGPUs[3] );
}

//------------------------------------------------------------------------------

int CAddStatDatagram::GetNumOfHostGPUs(void) const
{
    return( (NumOfHostGPUs[0] << 24) + (NumOfHostGPUs[1] << 16)
            + (NumOfHostGPUs[2] << 8) + NumOfHostGPUs[3] );
}

//------------------------------------------------------------------------------

int CAddStatDatagram::GetNumOfNodes(void) const
{
    return( (NumOfNodes[0] << 24) + (NumOfNodes[1] << 16)
          + (NumOfNodes[2] << 8) + NumOfNodes[3] );
}

//------------------------------------------------------------------------------

int CAddStatDatagram::GetFlags(void) const
{
    return( (Flags[0] << 24) + (Flags[1] << 16) + (Flags[2] << 8) + Flags[3] );
}

//------------------------------------------------------------------------------

const CSmallTimeAndDate CAddStatDatagram::GetTimeAndDate(void) const
{
    int seconds_from_beginning = (Time[0] << 24) + (Time[1] << 16) + (Time[2] << 8) + Time[3];

    CSmallTimeAndDate dt(seconds_from_beginning);
    return(dt);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
