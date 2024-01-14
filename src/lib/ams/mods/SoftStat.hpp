#ifndef SoftStatH
#define SoftStatH
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

#include <AMSMainHeader.hpp>
#include <XMLDocument.hpp>
#include <SmallTimeAndDate.hpp>

//------------------------------------------------------------------------------

struct CAddStatDatagram {
    // constructor and destructors ------------------------------------------------
    CAddStatDatagram(void);

    // set methods ----------------------------------------------------------------
    void Finish(void);

    void SetSite(const CSmallString& site);
    void SetModuleName(const CSmallString& name);
    void SetModuleVers(const CSmallString& vers);
    void SetModuleArch(const CSmallString& arch);
    void SetModuleMode(const CSmallString& mode);
    void SetBundleName(const CSmallString& name);
    void SetUser(const CSmallString& user);
    void SetHostName(const CSmallString& name);
    void SetNCPUs(int ncpus);
    void SetNumOfHostCPUs(int ncpus);
    void SetNGPUs(int ngpus);
    void SetNumOfHostGPUs(int ngpus);
    void SetNumOfNodes(int nnodes);
    void SetFlags(int flags);
    void SetTimeAndDate(const CSmallTimeAndDate& dt);

    // get methods ----------------------------------------------------------------
    bool IsValid(void);

    const CSmallString      GetSite(void) const;
    const CSmallString      GetModuleName(void) const;
    const CSmallString      GetModuleVers(void) const;
    const CSmallString      GetModuleArch(void) const;
    const CSmallString      GetModuleMode(void) const;
    const CSmallString      GetBundleName(void) const;
    const CSmallString      GetUser(void) const;
    const CSmallString      GetHostName(void) const;
    int                     GetNCPUs(void) const;
    int                     GetNumOfHostCPUs(void) const;
    int                     GetNGPUs(void) const;
    int                     GetNumOfHostGPUs(void) const;
    int                     GetNumOfNodes(void) const;
    int                     GetFlags(void) const;
    const CSmallTimeAndDate GetTimeAndDate(void) const;

    // section of private data ----------------------------------------------------
private:
    char            Magic[4];               // magic word
    unsigned char   Control[4];             // control sum
    char            Site[128];              // site name
    char            ModuleName[32];         // module build
    char            ModuleVers[32];
    char            ModuleArch[32];
    char            ModuleMode[32];
    char            BundleName[32];
    char            User[32];               // user name
    char            HostName[32];           // hostname
    unsigned char   NCPUs[4];               // ncpus
    unsigned char   NumOfHostCPUs[4];       // ncpus on node
    unsigned char   NGPUs[4];               // ngpus
    unsigned char   NumOfHostGPUs[4];       // ngpus on node
    unsigned char   NumOfNodes[4];          // number of nodes
    unsigned char   Flags[4];               // 32 bit flags
    unsigned char   Time[4];                // time in seconds from 00:00:00 UTC, January 1, 1970
};

//------------------------------------------------------------------------------

#endif
