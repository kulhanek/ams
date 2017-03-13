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
#include <pbs_ifl.h>
#include <stdlib.h>
#include <FileSystem.hpp>
#include <iomanip>

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
    CudaLibName = cudalibname;

    // test if file exist
    if( CFileSystem::IsFile(CudaLibName) == false ){
        CSmallString warning;
        warning << "'" << CudaLibName << "' does not exist";
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
    if( CudaLib.Open(CudaLibName) == false ){
        ES_ERROR("unable to load cuda library");
        return(false);
    }

    // load symbols
    bool status = true;

    cuInit = (CUInit)CudaLib.GetProcAddress("cuInit");
    if( cuInit == NULL ){
        ES_ERROR("unable to bind to cuInit");
        status = false;
    }

    cuDeviceGetCount = (CUDeviceGetCount)CudaLib.GetProcAddress("cuDeviceGetCount");
    if( cuDeviceGetCount == NULL ){
        ES_ERROR("unable to bind to cuDeviceGetCount");
        status = false;
    }

    cuDeviceGet = (CUDeviceGet)CudaLib.GetProcAddress("cuDeviceGet");
    if( cuDeviceGet == NULL ){
        ES_ERROR("unable to bind to cuDeviceGet");
        status = false;
    }

    cuDeviceGetName = (CUDeviceGetName)CudaLib.GetProcAddress("cuDeviceGetName");
    if( cuDeviceGetName == NULL ){
        ES_ERROR("unable to bind to cuDeviceGetName");
        status = false;
    }

    cuDeviceTotalMem = (CUDeviceTotalMem)CudaLib.GetProcAddress("cuDeviceTotalMem");
    if( cuDeviceTotalMem == NULL ){
        ES_ERROR("unable to bind to cuDeviceTotalMem");
        status = false;
    }

    // init driver
    if( cuInit ){
        if( cuInit(0) != CUDA_SUCCESS ){
            ES_WARNING("cuInit(0) failed");
            status = false;
        }
    }

    return(status);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CCuda::GetNumOfGPUs(void)
{
    if( cuDeviceGetCount == NULL ) return(0);
    int ngpus = 0;
    if( cuDeviceGetCount(&ngpus) == CUDA_SUCCESS ){
        return(ngpus);
    }
    return(false);
}

//------------------------------------------------------------------------------

void CCuda::GetGPUInfo(std::vector<std::string>& list)
{
    list.clear();
    if( cuDeviceGetCount == NULL ) return;
    if( cuDeviceGet == NULL ) return;
    if( cuDeviceGetName == NULL ) return;
    if( cuDeviceTotalMem == NULL ) return;

    int ngpus = 0;
    if( cuDeviceGetCount(&ngpus) != CUDA_SUCCESS ){
        ES_ERROR("unable to get number of devices");
        return;
    }

    for(int devid=0; devid < ngpus; devid++){
        CUdevice dev;
        // get device
        if( cuDeviceGet(&dev,devid) != CUDA_SUCCESS ){
            CSmallString error;
            error << "unable to get device #" << devid;
            ES_ERROR(error);
            continue;
        }

        // get device name
        char buffer[255];
        if( cuDeviceGetName(buffer,255,dev) != CUDA_SUCCESS ){
            CSmallString error;
            error << "unable to get name of device #" << devid;
            ES_ERROR(error);
            continue;
        }
        buffer[254] = '\0';
        CSmallString gpudev_name(buffer);

        // get device memory
        size_t mem = 0;
        if( cuDeviceTotalMem(&mem,dev) != CUDA_SUCCESS ){
            CSmallString warning;
            warning << "unable to get total memory of device #" << devid;
            ES_WARNING(warning);
        }

        CSmallString final_name;
        final_name << gpudev_name;

        double dmem = mem;
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
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

