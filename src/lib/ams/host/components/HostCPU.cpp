// =============================================================================
//  AMS - Advanced Module System
// -----------------------------------------------------------------------------
//     Copyright (C) 2023 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2020 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include <HostCPU.hpp>
#include <ErrorSystem.hpp>
#include <Utils.hpp>
#include <Host.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <hwloc.h>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <fnmatch.h>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CHostSubSystemCPU::CHostSubSystemCPU(const CFileName& config_file)
    : CHostSubSystem(config_file)
{
    CachedData          = false;
    NumOfHostCPUs       = 1;
    HTEnabled           = false;
    NumOfHostThreads    = 1;
}

//------------------------------------------------------------------------------

CHostSubSystemCPU::~CHostSubSystemCPU(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CHostSubSystemCPU::Init(void)
{
    CXMLElement* p_ele = GetConfig("cpu");
    if( p_ele == NULL ){
        INVALID_ARGUMENT("config element 'cpu' is NULL");
    }

    int              err;
    hwloc_topology_t topology;

    err = hwloc_topology_init(&topology);
    if( err ){
        ES_TRACE_ERROR("unable to init hwloc topology");
        return;
    }

    err = hwloc_topology_load(topology);
    if( err ){
        ES_TRACE_ERROR("unable to load hwloc topology");
        return;
    }

    // CPU core data
    NumOfHostCPUs       = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_CORE);
    NumOfHostThreads    = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_PU);

    // CPU MODEL
    GetCPUModelAndFlags();
    CPURawModelName = CPUModelName;

    // Machine Memory
    hwloc_obj_t machine = hwloc_get_obj_by_type(topology, HWLOC_OBJ_MACHINE, 0);
    double mem = 0;
    if( machine ){
        mem = machine->total_memory;
    }

    // get total mem
    if( mem > 0 ){
        stringstream str;
        mem = mem / 1024 ; // bytes to KB
        mem = mem / 1024 ; // kbytes to MB
        if( mem < 1024 ){
            str << " [Total memory: " << fixed << setprecision(1) << mem << " MB" << "]";
        } else {
            mem = mem / 1024;
            if( mem < 1024 ){
                str << " [Total memory: " << fixed << setprecision(1) << mem << " GB" << "]";
            } else {
                mem = mem / 1024;
                str << " [Total memory: " << fixed << setprecision(1) << mem << " TB" << "]";
            }
        }
        CPUModelName << str.str();
    }

    // filter CPU tokens
    bool tenable = false;
    p_ele->GetAttribute("tokens",tenable);
    if( tenable ) {
        CXMLElement* p_fele = p_ele->GetFirstChildElement("filter");
        while( p_fele != NULL ){
            CSmallString filter;
            p_fele->GetAttribute("value",filter);
            std::list<CSmallString>::iterator  it = CPUFlags.begin();
            std::list<CSmallString>::iterator  ie = CPUFlags.end();
            while( it != ie ){
                if( fnmatch(filter,(*it),0) == 0 ){
                    ArchTokens.push_back(*it);
                }
                it++;
            }
            p_fele = p_fele->GetNextSiblingElement("filter");
        }
    }
    p_ele->GetAttribute("ht",HTEnabled);

    if( HTEnabled == false ){
        NumOfHostThreads = NumOfHostCPUs;
    }

    CachedData = false;
}

//------------------------------------------------------------------------------

void CHostSubSystemCPU::GetCPUModelAndFlags(void)
{
    ifstream cpuinfo;

    // open /proc/cpuinfo file
    cpuinfo.open("/proc/cpuinfo");
    if( ! cpuinfo ){
        RUNTIME_ERROR("unable to open /proc/cpuinfo");
    }

    CPUModelName = NULL;
    CPUFlags.clear();

    // parse CPU attributes
    string line;
    bool mod_found = false;
    bool flags_found = false;

    while( getline(cpuinfo,line) ){

        // all found
        if( flags_found && mod_found ) break;

        vector<string>  key_and_value;
        split(key_and_value,line,is_any_of(":"));
        if( key_and_value.size() != 2 ) continue;

        string key = key_and_value[0];
        string values = key_and_value[1];

        trim(key);
        trim(values);

        if( (key == "model name") && (mod_found == false) ){
            vector<string> words;
            split(words,values,is_any_of(" "),token_compress_on);
            CPUModelName =  join(words," ");
            mod_found = true;
            continue;
        }

        if( (key == "flags") && (flags_found == false) ){
            split(CPUFlags,values,is_any_of(" "),token_compress_on);
            flags_found = true;
            continue;
        }
    }

    // close file
    cpuinfo.close();
}

//------------------------------------------------------------------------------

void CHostSubSystemCPU::Apply(void)
{
    Host.SetNumOfHostCPUs(NumOfHostCPUs);
    Host.SetNumOfHostThreads(NumOfHostThreads);
    for(CSmallString token : ArchTokens){
        Host.AddArchToken(token);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

EHostCacheMode CHostSubSystemCPU::LoadFromCache(CXMLElement* p_ele)
{
    CXMLElement* p_cele = p_ele->GetFirstChildElement("cpu");
    if( p_cele == NULL ) return(EHC_REININT);

    bool result = true;
    std::string slist;

    result &= p_cele->GetAttribute("ncpus",NumOfHostCPUs);
    result &= p_cele->GetAttribute("nthrs",NumOfHostThreads);
    result &= p_cele->GetAttribute("cmodr",CPURawModelName);
    result &= p_cele->GetAttribute("cmod",CPUModelName);

    slist.clear();
    result &= p_cele->GetAttribute("fl",slist);
    if( result && (! slist.empty()) ) split(CPUFlags,slist,is_any_of(","));

    slist.clear();
    result &= p_cele->GetAttribute("tks",slist);
    if( result && (! slist.empty()) ) split(ArchTokens,slist,is_any_of("#"));

    CachedData = result;
    if( CachedData ){
        return(EHC_LOADED);
    }
    return(EHC_REININT);
}

//------------------------------------------------------------------------------

void CHostSubSystemCPU::SaveToCache(CXMLElement* p_ele)
{
    CXMLElement* p_cele = p_ele->CreateChildElement("cpu");

    p_cele->SetAttribute("ncpus",NumOfHostCPUs);
    p_cele->SetAttribute("nthrs",NumOfHostThreads);
    p_cele->SetAttribute("cmodr",CPURawModelName);
    p_cele->SetAttribute("cmod",CPUModelName);
    p_cele->SetAttribute("fl",GetTokenList(CPUFlags,","));
    p_cele->SetAttribute("tks",GetTokenList(ArchTokens,"#"));
}

//------------------------------------------------------------------------------

void CHostSubSystemCPU::PrintSubSystemInfo(CVerboseStr& vout)
{
    if( CachedData ){
    vout << ">>> cpu ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ [cached] ~~" << endl;
    } else {
    vout << ">>> cpu ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    }

    vout <<                  "    Configuration  : " << GetConfigFile() <<  endl;
    vout <<                  "    Host CPU cores : " << NumOfHostCPUs << endl;
    vout <<                  "    HT enabled     : " << bool_to_str(HTEnabled) << endl;
    vout <<                  "    Host threads   : " << NumOfHostThreads << endl;
    int tpc = 1;
    if( NumOfHostCPUs > 0 ){
        tpc = NumOfHostThreads / NumOfHostCPUs;
    }
    vout <<                  "    Threads/Core   : " << tpc << endl;
    vout <<                  "    SMP CPU model  : " << CPUModelName << endl;
    CUtils::PrintTokens(vout,"    CPU flags      : ",GetTokenList(CPUFlags), 80, ' ');
    CUtils::PrintTokens(vout,"    CPU tokens     : ",GetTokenList(ArchTokens), 80, ' ');
}

//------------------------------------------------------------------------------

void CHostSubSystemCPU::PrintNodeResources(CVerboseStr& vout)
{
    vout << "cpu_model " << CPURawModelName << endl;
    vout << "cpu_flag " << GetTokenList(CPUFlags,",") << endl;
    bool HTDetected = NumOfHostThreads > NumOfHostCPUs;
    vout << "hyperthreading " << HTDetected << endl;
}

//------------------------------------------------------------------------------

void CHostSubSystemCPU::PrintHostInfoFor(CVerboseStr& vout,EPrintHostInfo mode)
{
    if( mode == EPHI_SITE ){
        vout << "  Num of host CPUs   : " << setw(4) << left << Host.GetNumOfHostCPUs();
        vout << " / Num of host threads : " << setw(4) << left << Host.GetNumOfHostThreads() << endl;
        vout << "  Host SMP CPU model : " << CPUModelName << endl;
    }
    if( mode == EPHI_MODULE ){
        vout << "  Host SMP CPU model : " << CPUModelName << endl;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

