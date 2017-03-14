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

enum cudaError {
    CUDA_SUCCESS = 0
};

typedef enum cudaError CUDAresult;

// -----------------------------------------------------------------------------

// CUDART 8.0

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
              int clockRate;
              size_t totalConstMem;
              int major;
              int minor;
              size_t textureAlignment;
              size_t texturePitchAlignment;
              int deviceOverlap;
              int multiProcessorCount;
              int kernelExecTimeoutEnabled;
              int integrated;
              int canMapHostMemory;
              int computeMode;
              int maxTexture1D;
              int maxTexture1DMipmap;
              int maxTexture1DLinear;
              int maxTexture2D[2];
              int maxTexture2DMipmap[2];
              int maxTexture2DLinear[3];
              int maxTexture2DGather[2];
              int maxTexture3D[3];
              int maxTexture3DAlt[3];
              int maxTextureCubemap;
              int maxTexture1DLayered[2];
              int maxTexture2DLayered[3];
              int maxTextureCubemapLayered[2];
              int maxSurface1D;
              int maxSurface2D[2];
              int maxSurface3D[3];
              int maxSurface1DLayered[2];
              int maxSurface2DLayered[3];
              int maxSurfaceCubemap;
              int maxSurfaceCubemapLayered[2];
              size_t surfaceAlignment;
              int concurrentKernels;
              int ECCEnabled;
              int pciBusID;
              int pciDeviceID;
              int pciDomainID;
              int tccDriver;
              int asyncEngineCount;
              int unifiedAddressing;
              int memoryClockRate;
              int memoryBusWidth;
              int l2CacheSize;
              int maxThreadsPerMultiProcessor;
              int streamPrioritiesSupported;
              int globalL1CacheSupported;
              int localL1CacheSupported;
              size_t sharedMemPerMultiprocessor;
              int regsPerMultiprocessor;
              int managedMemSupported;
              int isMultiGpuBoard;
              int multiGpuBoardGroupID;
              int singleToDoublePrecisionPerfRatio;
              int pageableMemoryAccess;
              int concurrentManagedAccess;
          };

// -----------------------------------------------------------------------------

typedef CUDAresult (*CUDAGetDeviceCount)(int* count);
typedef CUDAresult (*CUDASetDevice)(int  device);
typedef CUDAresult (*CUDAGetDeviceProperties)(cudaDeviceProp* prop, int  device);

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
    CSmallString    CudaRTLibName;
    CDynamicPackage CudaRTLib;

    // init symbols
    bool InitSymbols(void);

    // cuda api symbols
    CUDAGetDeviceCount      cudaGetDeviceCount;
    CUDASetDevice           cudaSetDevice;
    CUDAGetDeviceProperties cudaGetDeviceProperties;

};

// -----------------------------------------------------------------------------

#endif
