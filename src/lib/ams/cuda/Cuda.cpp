// =============================================================================
//  AMS - Advanced Module System
// -----------------------------------------------------------------------------
//     Copyright (C) 2012 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include <Cuda.hpp>
#include <ErrorSystem.hpp>
#include <iostream>
#include <stdlib.h>
#include <FileSystem.hpp>
#include <iomanip>
#include <sstream>
#include <Host.hpp>
#include <XMLElement.hpp>

using namespace std;

// -----------------------------------------------------------------------------

CCuda Cuda;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CCuda::CCuda(void)
{
}

//------------------------------------------------------------------------------

CCuda::~CCuda(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CCuda::Init(const CSmallString& cudalibname)
{
    CudaRTLibName = cudalibname;

    // test if file exist
    if( CFileSystem::IsFile(CudaRTLibName) == false ){
        CSmallString warning;
        warning << "'" << CudaRTLibName << "' does not exist";
        ES_WARNING(warning);
        return(false);
    }

    if( InitSymbols() == false ){
        ES_WARNING("unable to init symbols");
        return(false);
    }
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CCuda::InitSymbols(void)
{
    if( CudaRTLib.Open(CudaRTLibName) == false ){
        ES_ERROR("unable to load cuda library");
        return(false);
    }

    // load symbols
    bool status = true;

    cudaGetDeviceCount = (CUDAGetDeviceCount)CudaRTLib.GetProcAddress("cudaGetDeviceCount");
    if( cudaGetDeviceCount == NULL ){
        ES_ERROR("unable to bind to cudaGetDeviceCount");
        status = false;
    }

    cudaSetDevice = (CUDASetDevice)CudaRTLib.GetProcAddress("cudaSetDevice");
    if( cudaSetDevice == NULL ){
        ES_ERROR("unable to bind to cudaSetDevice");
        status = false;
    }

    cudaGetDeviceProperties = (CUDAGetDeviceProperties)CudaRTLib.GetProcAddress("cudaGetDeviceProperties");
    if( cudaGetDeviceProperties == NULL ){
        ES_ERROR("unable to bind to cudaGetDeviceProperties");
        status = false;
    }

    cudaGetErrorString = (CUDAGetErrorString)CudaRTLib.GetProcAddress("cudaGetErrorString");
    if( cudaGetErrorString == NULL ){
        ES_ERROR("unable to bind to cudaGetErrorString");
        status = false;
    }

    cudaGetLastError = (CUDAGetLastError)CudaRTLib.GetProcAddress("cudaGetLastError");
    if( cudaGetLastError == NULL ){
        ES_ERROR("unable to bind to cudaGetLastError");
        status = false;
    }

    return(status);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CCuda::GetNumOfGPUs(void)
{
    if( cudaGetDeviceCount == NULL ) return(0);
    int ngpus = 0;
    if( cudaGetDeviceCount(&ngpus) == CUDA_SUCCESS ){
        return(ngpus);
    }
    return(false);
}

//------------------------------------------------------------------------------

void CCuda::GetGPUInfo(CSmallString& raw_model,std::vector<std::string>& list,std::vector<std::string>& capas)
{
    list.clear();
    capas.clear();

    if( cudaGetDeviceCount == NULL ) return;
    if( cudaSetDevice == NULL ) return;
    if( cudaGetDeviceProperties == NULL ) return;
    if( cudaGetErrorString == NULL ) return;
    if( cudaGetLastError == NULL ) return;

    int ngpus = 0;
    if( cudaGetDeviceCount(&ngpus) != CUDA_SUCCESS ){
        CSmallString error;
        error << "unable to call cudaGetDeviceCount (" << cudaGetErrorString(cudaGetLastError()) << ")";
        ES_ERROR(error);
        return;
    }

    std::set<std::string> capabilities;

    for(int devid=0; devid < ngpus; devid++){

        if( cudaSetDevice(devid) != CUDA_SUCCESS ) {
            CSmallString error;
            error << "unable to call cudaSetDevice (" << cudaGetErrorString(cudaGetLastError()) << ")";
            ES_ERROR(error);
            return;
        }

        cudaDeviceProp deviceProp;
        if( cudaGetDeviceProperties(&deviceProp, devid) != CUDA_SUCCESS ) {
            CSmallString error;
            error << "unable to call cudaGetDeviceProperties (" << cudaGetErrorString(cudaGetLastError()) << ")";
            ES_ERROR(error);
            return;
        }

        deviceProp.name[255] = '\0';
        CSmallString gpudev_name(deviceProp.name);

        CSmallString final_name;
        // name
        final_name << gpudev_name;
        // FIXME - it takes the last model, how to handle heteregenous situation?
        raw_model = gpudev_name;
        // memory
        double dmem = (double)deviceProp.totalGlobalMem;
        stringstream str;
        dmem = dmem / 1024 / 1024 ; // bytes -> MB
        if( dmem < 1024 ){
            str << " [Memory: " << fixed << setprecision(1) << dmem << " MB/GPU" << "]";
        } else {
            dmem = dmem / 1024;
            if( dmem < 1024 ){
                str << " [Memory: " << fixed << setprecision(1) << dmem << " GB/GPU" << "]";
            } else {
                dmem = dmem / 1024;
                str << " [Memory: " << fixed << setprecision(1) << dmem << " TB/GPU" << "]";
            }
        }

        final_name << str.str();

        list.push_back(string(final_name));

        // decode capability
        DecodeCapability(deviceProp,capabilities);
    }

    // copy list of capabilities
    capas.insert(capas.begin(),capabilities.begin(),capabilities.end());
}

//------------------------------------------------------------------------------

void CCuda::DecodeCapability(cudaDeviceProp& prop,std::set<std::string>& capabilities)
{
    std::stringstream capa;
    capa << "cuda" << prop.major << prop.minor;

    // GPU capability
    capabilities.insert(capa.str());

    // process compatible capabilities
    CXMLElement* p_ele = Host.GetCUDACapabilities();
    if( p_ele == NULL ) return;

    std::set<std::string> compat;
    CXMLElement* p_sele = p_ele->GetFirstChildElement("cap");
    while( p_sele != NULL ){
        std::string cap;
        p_sele->GetAttribute("name",cap);
        if( cap == capa.str() ){
            capabilities.insert(compat.begin(),compat.end());
            return;
        }
        compat.insert(cap);
        p_sele = p_sele->GetNextSiblingElement("cap");
    }

    return;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

