#ifndef CudaH
#define CudaH
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

#include <AMSMainHeader.hpp>
#include <DynamicPackage.hpp>
#include <vector>

// -----------------------------------------------------------------------------

enum cudaError_enum {
    CUDA_SUCCESS = 0
};

typedef enum cudaError_enum CUresult;
typedef int CUdevice;

struct cudaDeviceProp {
     char name[256];
     size_t totalGlobalMem;
     size_t sharedMemPerBlock;
     int regsPerBlock;
     int warpSize;
     size_t memPitch;
     int maxThreadsPerBlock;
     int maxThreadsDim[3];
     int maxGridSize[3];
     size_t totalConstMem;
     int major;
     int minor;
     int clockRate;
     size_t textureAlignment;
     int deviceOverlap;
     int multiProcessorCount;
     int kernelExecTimeoutEnabled;
     int integrated;
     int canMapHostMemory;
     int computeMode;
     int concurrentKernels;
     int ECCEnabled;
     int pciBusID;
     int pciDeviceID;
     int tccDriver;
 };

typedef CUresult (*CUInit)(unsigned int Flags);
typedef CUresult (*CUDeviceGetCount)(int* count);
typedef CUresult (*CUDeviceGet)(CUdevice* device,int ordinal);
typedef CUresult (*CudaGetDeviceProperties)(struct cudaDeviceProp* prop,CUdevice dev);

// -----------------------------------------------------------------------------

class AMS_PACKAGE CCuda {
public:
// constructor -----------------------------------------------------------------
        CCuda(void);
        ~CCuda(void);

// init torque subsystem -------------------------------------------------------
    /// load symbols
    bool Init(const CSmallString& cudalibname);

// enumeration -----------------------------------------------------------------
    /// init number of GPU devices
    int GetNumOfGPUs(void);

    /// get GPU info
    void GetGPUInfo(std::vector<std::string>& list);

// section of private data -----------------------------------------------------
private:
    CSmallString    CudaLibName;
    CDynamicPackage CudaLib;

    // init symbols
    bool InitSymbols(void);

    // cuda api symbols
    CUInit                  cuInit;
    CUDeviceGetCount        cuDeviceGetCount;
    CUDeviceGet             cuDeviceGet;
    CudaGetDeviceProperties cudaGetDeviceProperties;
};

// -----------------------------------------------------------------------------

#endif
