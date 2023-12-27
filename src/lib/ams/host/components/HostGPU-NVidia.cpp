// =============================================================================
//  AMS - Advanced Module System
// -----------------------------------------------------------------------------
//     Copyright (C) 2023 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2020 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2012 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2011 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2004,2005,2008,2010 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include <HostGPU-NVidia.hpp>
#include <ErrorSystem.hpp>
#include <FileSystem.hpp>
#include <CudaRT.hpp>
#include <Utils.hpp>
#include <Host.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iomanip>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/classification.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CHostSubSystemGPUNVidia::CHostSubSystemGPUNVidia(const CFileName& config_file)
    : CHostSubSystem(config_file)
{
    NumOfHostGPUs = 0;
    UseCapaTokens = false;
}

//------------------------------------------------------------------------------

CHostSubSystemGPUNVidia::~CHostSubSystemGPUNVidia(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CHostSubSystemGPUNVidia::Init(void)
{
    CXMLElement* p_ele = GetConfig("gpu-nvidia");
    if( p_ele == NULL ){
        INVALID_ARGUMENT("config element 'cuda' is NULL");
    }

    CudaDev = "-none-";
    CudaLib = "-none-";

    CXMLElement* p_fele = p_ele->GetFirstChildElement();
    while( p_fele != NULL ){
        string          stokens;
        CSmallString    cudalib,cudadev;

        if ( p_fele->GetName() == "dev" ) {
            // load config
            bool success = true;

            success &= p_fele->GetAttribute("name",cudadev);
            success &= p_fele->GetAttribute("lib",cudalib);
            success &= p_fele->GetAttribute("tokens",stokens);

            // move to next record
            p_fele = p_fele->GetNextSiblingElement();

            if( success == false ){
                ES_WARNING("undefined name, lib, or tokens attributes for dev element");
                continue;
            }

            struct stat dev_stat;

            // is device present?
            if( stat(cudadev,&dev_stat) != 0 ){
                CSmallString warning;
                warning << "cuda: host does not have device '" << cudadev << "'";
                ES_WARNING(warning);
                continue;
            }

        } else {
            ES_WARNING("unsupported element");
            // move to next record
            p_fele = p_fele->GetNextSiblingElement();
            continue;
        }

        // try to load cuda lib
        CCudaRT cuda;
        if( cuda.Init(cudalib) == false ){
            CSmallString warning;
            warning << "unable to init cuda lib '" << cudalib << "'";
            ES_WARNING(warning);
            continue;
        }

        CudaDev = cudadev;
        CudaLib = cudalib;

        // get number of GPU devices
        NumOfHostGPUs = cuda.GetNumOfGPUs();
        // get list of GPU devices
        cuda.GetGPUInfo(GPURawModelName,GPUModels,CapaTokens);

        if( NumOfHostGPUs > 0 ){
            // add gpu tokens if available and ngpus > 0
            std::vector<string> tokens;
            split(tokens,stokens,is_any_of("#"));

            vector<string>::iterator  it = tokens.begin();
            vector<string>::iterator  ie = tokens.end();
            while( it != ie ){
                ArchTokens.push_back(*it);
                it++;
            }
        }
        break;
    }
    p_ele->GetAttribute("capatokens",UseCapaTokens);

    CachedData = false;
}

//------------------------------------------------------------------------------

void CHostSubSystemGPUNVidia::Apply(void)
{
    if( UseCapaTokens ) {
        for(CSmallString token : CapaTokens){
            Host.AddArchToken(token);
        }
    }
    for(CSmallString token : ArchTokens){
        Host.AddArchToken(token);
    }
    Host.SetNumOfHostGPUs(NumOfHostGPUs);
}

//------------------------------------------------------------------------------

EHostCacheMode CHostSubSystemGPUNVidia::LoadFromCache(CXMLElement* p_ele)
{
    CXMLElement* p_cele = p_ele->GetFirstChildElement("gpu-nvidia");
    if( p_cele == NULL ) return(EHC_REININT);

    bool result = true;
    string slist;
    result &= p_cele->GetAttribute("dev",CudaDev);
    result &= p_cele->GetAttribute("lib",CudaLib);
    result &= p_cele->GetAttribute("ngpus",NumOfHostGPUs);
    result &= p_cele->GetAttribute("raw",GPURawModelName);
    result &= p_cele->GetAttribute("ucap",UseCapaTokens);

    slist.clear();
    result &= p_cele->GetAttribute("mods",slist);
    if( result && (! slist.empty()) ) split(GPUModels,slist,is_any_of("|"));

    slist.clear();
    result &= p_cele->GetAttribute("ctk",slist);
    if( result && (! slist.empty()) ) split(CapaTokens,slist,is_any_of("#"));

    slist.clear();
    result &= p_cele->GetAttribute("atk",slist);
    if( result && (! slist.empty()) ) split(ArchTokens,slist,is_any_of("#"));

    CachedData = result;
    if( CachedData ){
        return(EHC_LOADED);
    }
    return(EHC_REININT);
}

//------------------------------------------------------------------------------

void CHostSubSystemGPUNVidia::SaveToCache(CXMLElement* p_ele)
{
    CXMLElement* p_cele = p_ele->CreateChildElement("gpu-nvidia");

    p_cele->SetAttribute("dev",CudaDev);
    p_cele->SetAttribute("lib",CudaLib);
    p_cele->SetAttribute("ngpus",NumOfHostGPUs);
    p_cele->SetAttribute("raw",GPURawModelName);
    p_cele->SetAttribute("ucap",UseCapaTokens);
    p_cele->SetAttribute("mods",GetTokenList(GPUModels,"|"));
    p_cele->SetAttribute("ctk",GetTokenList(CapaTokens,"#"));
    p_cele->SetAttribute("atk",GetTokenList(ArchTokens,"#"));
}

//------------------------------------------------------------------------------

void CHostSubSystemGPUNVidia::PrintSubSystemInfo(CVerboseStr& vout)
{
    if( CachedData ){
    vout << ">>> gpu-nvidia ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ [cached] ~~" << endl;
    } else {
    vout << ">>> gpu-nvidia ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    }

    vout <<                  "    Configuration  : " << GetConfigFile() <<  endl;
    vout <<                  "    CUDA device    : " << CudaDev << endl;
    if( CudaDev != "-none-" ){
    vout <<                  "    CUDA library   : " << CudaLib << endl;
    vout <<                  "    Host GPUs      : " << NumOfHostGPUs << endl;
    if( NumOfHostGPUs > 0 ){
    if( IsGPUModelSMP() == false ){
    int i=1;
    for(CSmallString model : GPUModels){
    vout <<                  "    GPU model #" << setw(1) << i << "   : " << model << endl;
    i++;
    }
    } else {
    vout <<                  "    SMP GPU model  : " << GPUModels.front() << endl;
    }
    CUtils::PrintTokens(vout,"    Capabilities   : ",GetTokenList(CapaTokens), 80, ' ');
    }
    }
    CUtils::PrintTokens(vout,"    CUDA tokens    : ",GetTokenList(ArchTokens), 80, ' ');
}

//------------------------------------------------------------------------------

void CHostSubSystemGPUNVidia::PrintNodeResources(CVerboseStr& vout)
{
    vout << "gpu_model " << GPURawModelName << endl;

    // FIXME - maybe here we want also compatible cuda capabilities?
    vout << "gpu_cap " << GetTokenList(CapaTokens,",") << endl;
}

//------------------------------------------------------------------------------

void CHostSubSystemGPUNVidia::PrintHostInfoFor(CVerboseStr& vout,EPrintHostInfo mode)
{
    if( mode == EPHI_SITE ) {
        if( NumOfHostGPUs > 0 ){
        vout << "  Num of host GPUs   : " << NumOfHostGPUs << endl;
        if( IsGPUModelSMP() == false ){
        int i=1;
        for(CSmallString model : GPUModels){
        vout << "  Host GPU model #" << setw(1) << i << "  : " << model << endl;
        i++;
        }
        } else {
        vout << "  Host SMP GPU model : " << GPUModels.front() << endl;
        }
        }
    }
    if( mode == EPHI_MODULE ) {
        if( NumOfHostGPUs > 0 ){
        if( IsGPUModelSMP() == false ){
        int i=1;
        for(CSmallString model : GPUModels){
        vout << "  Host GPU model #" << setw(1) << i << "  : " << model << endl;
        i++;
        }
        } else {
        vout << "  Host SMP GPU model : " << GPUModels.front() << endl;
        }
        }
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CHostSubSystemGPUNVidia::IsGPUModelSMP(void)
{
    if( NumOfHostGPUs <= 1 ) return(false);
    CSmallString first = GPUModels.front();
    for(CSmallString model : GPUModels){
        if( model != first ) return(false);
    }
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

