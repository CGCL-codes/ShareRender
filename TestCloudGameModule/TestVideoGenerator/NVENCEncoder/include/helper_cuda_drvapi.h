/*
 * Copyright 1993-2014 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO LICENSEE:
 *
 * This source code and/or documentation ("Licensed Deliverables") are
 * subject to NVIDIA intellectual property rights under U.S. and
 * international Copyright laws.
 *
 * These Licensed Deliverables contained herein is PROPRIETARY and
 * CONFIDENTIAL to NVIDIA and is being provided under the terms and
 * conditions of a form of NVIDIA software license agreement by and
 * between NVIDIA and Licensee ("License Agreement") or electronically
 * accepted by Licensee.  Notwithstanding any terms or conditions to
 * the contrary in the License Agreement, reproduction or disclosure
 * of the Licensed Deliverables to any third party without the express
 * written consent of NVIDIA is prohibited.
 *
 * ALL NVIDIA DESIGN SPECIFICATIONS, REFERENCE BOARDS, FILES, DRAWINGS,
 * DIAGNOSTICS, LISTS, AND OTHER DOCUMENTS (TOGETHER AND SEPARATELY,
 * �MATERIALS? ARE BEING PROVIDED �AS IS.?WITHOUT EXPRESS OR IMPLIED
 * WARRANTY OF ANY KIND.  NVIDIA DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THESE LICENSED DELIVERABLES, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE LICENSE
 * AGREEMENT, IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT,
 * INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THESE LICENSED DELIVERABLES.
 *
 * Information furnished is believed to be accurate and reliable. However,
 * NVIDIA assumes no responsibility for the consequences of use of such
 * information nor for any infringement of patents or other rights of
 * third parties, which may result from its use.  No License is granted
 * by implication or otherwise under any patent or patent rights of NVIDIA
 * Corporation.  Specifications mentioned in the software are subject to
 * change without notice. This publication supersedes and replaces all
 * other information previously supplied.
 *
 * NVIDIA Corporation products are not authorized for use as critical
 * components in life support devices or systems without express written
 * approval of NVIDIA Corporation.
 *
 * U.S. Government End Users.  These Licensed Deliverables are a
 * "commercial item" as that term is defined at 48 C.F.R. 2.101 (OCT
 * 1995), consisting of "commercial computer software" and "commercial
 * computer software documentation" as such terms are used in 48
 * C.F.R. 12.212 (SEPT 1995) and is provided to the U.S. Government
 * only as a commercial end item.  Consistent with 48 C.F.R.12.212 and
 * 48 C.F.R. 227.7202-1 through 227.7202-4 (JUNE 1995), all
 * U.S. Government End Users acquire the Licensed Deliverables with
 * only those rights set forth herein.
 *
 * Any use of the Licensed Deliverables in individual and commercial
 * software must include, in the user documentation and internal
 * comments to the code, the above Disclaimer and U.S. Government End
 * Users Notice.
 */

// Helper functions for CUDA Driver API error handling (make sure that CUDA_H is included in your projects)
#ifndef HELPER_CUDA_DRVAPI_H
#define HELPER_CUDA_DRVAPI_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "./helper_string.h"
#include "./drvapi_error_string.h"
//#include <drvapi_error_string.h>

#ifndef MAX
#define MAX(a,b) (a > b ? a : b)
#endif

#ifndef EXIT_WAIVED
#define EXIT_WAIVED 2
#endif

////////////////////////////////////////////////////////////////////////////////
// These are CUDA Helper functions

// add a level of protection to the CUDA SDK samples, let's force samples to explicitly include CUDA.H
#ifdef  __cuda_cuda_h__
// This will output the proper CUDA error strings in the event that a CUDA host call returns an error
#ifndef checkCudaErrors
#define checkCudaErrors(err)  __checkCudaErrors (err, __FILE__, __LINE__)

// These are the inline versions for all of the SDK helper functions
inline void __checkCudaErrors(CUresult err, const char *file, const int line)
{
    if (CUDA_SUCCESS != err)
    {
        fprintf(stderr, "checkCudaErrors() Driver API error = %04d \"%s\" from file <%s>, line %i.\n",
                err, getCudaDrvErrorString(err), file, line);
        exit(EXIT_FAILURE);
    }
}
#endif

#ifdef getLastCudaDrvErrorMsg
#undef getLastCudaDrvErrorMsg
#endif

#define getLastCudaDrvErrorMsg(msg)           __getLastCudaDrvErrorMsg  (msg, __FILE__, __LINE__)

inline void __getLastCudaDrvErrorMsg(const char *msg, const char *file, const int line)
{
    CUresult err = cuCtxSynchronize();

    if (CUDA_SUCCESS != err)
    {
        fprintf(stderr, "getLastCudaDrvErrorMsg -> %s", msg);
        fprintf(stderr, "getLastCudaDrvErrorMsg -> cuCtxSynchronize API error = %04d \"%s\" in file <%s>, line %i.\n",
                err, getCudaDrvErrorString(err), file, line);
        exit(EXIT_FAILURE);
    }
}

// This function wraps the CUDA Driver API into a template function
template <class T>
inline void getCudaAttribute(T *attribute, CUdevice_attribute device_attribute, int device)
{
    CUresult error_result = cuDeviceGetAttribute(attribute, device_attribute, device);

    if (error_result != CUDA_SUCCESS)
    {
        printf("cuDeviceGetAttribute returned %d\n-> %s\n", (int)error_result, getCudaDrvErrorString(error_result));
        exit(EXIT_SUCCESS);
    }
}
#endif

// Beginning of GPU Architecture definitions
inline int _ConvertSMVer2CoresDRV(int major, int minor)
{
    // Defines for GPU Architecture types (using the SM version to determine the # of cores per SM
    typedef struct
    {
        int SM; // 0xMm (hexidecimal notation), M = SM Major version, and m = SM minor version
        int Cores;
    } sSMtoCores;

    sSMtoCores nGpuArchCoresPerSM[] =
    {
        { 0x10,  8 }, // Tesla Generation (SM 1.0) G80 class
        { 0x11,  8 }, // Tesla Generation (SM 1.1) G8x class
        { 0x12,  8 }, // Tesla Generation (SM 1.2) G9x class
        { 0x13,  8 }, // Tesla Generation (SM 1.3) GT200 class
        { 0x20, 32 }, // Fermi Generation (SM 2.0) GF100 class
        { 0x21, 48 }, // Fermi Generation (SM 2.1) GF10x class
        { 0x30, 192}, // Kepler Generation (SM 3.0) GK10x class
        { 0x35, 192}, // Kepler Generation (SM 3.5) GK11x class
        {   -1, -1 }
    };

    int index = 0;

    while (nGpuArchCoresPerSM[index].SM != -1)
    {
        if (nGpuArchCoresPerSM[index].SM == ((major << 4) + minor))
        {
            return nGpuArchCoresPerSM[index].Cores;
        }

        index++;
    }

    // If we don't find the values, we default use the previous one to run properly
    printf("MapSMtoCores for SM %d.%d is undefined.  Default to use %d Cores/SM\n", major, minor, nGpuArchCoresPerSM[7].Cores);
    return nGpuArchCoresPerSM[7].Cores;
}
// end of GPU Architecture definitions

#ifdef __cuda_cuda_h__
// General GPU Device CUDA Initialization
inline int gpuDeviceInitDRV(int ARGC, const char **ARGV)
{
    int cuDevice = 0;
    int deviceCount = 0;
    CUresult err = cuInit(0);

    if (CUDA_SUCCESS == err)
    {
        checkCudaErrors(cuDeviceGetCount(&deviceCount));
    }

    if (deviceCount == 0)
    {
        fprintf(stderr, "cudaDeviceInit error: no devices supporting CUDA\n");
        exit(EXIT_FAILURE);
    }

    int dev = 0;
    dev = getCmdLineArgumentInt(ARGC, (const char **) ARGV, "device=");

    if (dev < 0)
    {
        dev = 0;
    }

    if (dev > deviceCount-1)
    {
        fprintf(stderr, "\n");
        fprintf(stderr, ">> %d CUDA capable GPU device(s) detected. <<\n", deviceCount);
        fprintf(stderr, ">> cudaDeviceInit (-device=%d) is not a valid GPU device. <<\n", dev);
        fprintf(stderr, "\n");
        return -dev;
    }

    checkCudaErrors(cuDeviceGet(&cuDevice, dev));
    char name[100];
    cuDeviceGetName(name, 100, cuDevice);

    int computeMode;
    getCudaAttribute<int>(&computeMode, CU_DEVICE_ATTRIBUTE_COMPUTE_MODE, dev);

    if (computeMode == CU_COMPUTEMODE_PROHIBITED)
    {
        fprintf(stderr, "Error: device is running in <CU_COMPUTEMODE_PROHIBITED>, no threads can use this CUDA Device.\n");
        return -1;
    }

    if (checkCmdLineFlag(ARGC, (const char **) ARGV, "quiet") == false)
    {
        printf("gpuDeviceInitDRV() Using CUDA Device [%d]: %s\n", dev, name);
    }

    return dev;
}

// This function returns the best GPU based on performance
inline int gpuGetMaxGflopsDeviceIdDRV()
{
    CUdevice current_device = 0, max_perf_device = 0;
    int device_count        = 0, sm_per_multiproc = 0;
    int max_compute_perf    = 0, best_SM_arch     = 0;
    int major = 0, minor = 0   , multiProcessorCount, clockRate;

    cuInit(0);
    checkCudaErrors(cuDeviceGetCount(&device_count));

    if (device_count == 0)
    {
        fprintf(stderr, "gpuGetMaxGflopsDeviceIdDRV error: no devices supporting CUDA\n");
        exit(EXIT_FAILURE);
    }

    // Find the best major SM Architecture GPU device
    while (current_device < device_count)
    {
        checkCudaErrors(cuDeviceComputeCapability(&major, &minor, current_device));

        if (major > 0 && major < 9999)
        {
            best_SM_arch = MAX(best_SM_arch, major);
        }

        current_device++;
    }

    // Find the best CUDA capable GPU device
    current_device = 0;

    while (current_device < device_count)
    {
        checkCudaErrors(cuDeviceGetAttribute(&multiProcessorCount,
                                             CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT,
                                             current_device));
        checkCudaErrors(cuDeviceGetAttribute(&clockRate,
                                             CU_DEVICE_ATTRIBUTE_CLOCK_RATE,
                                             current_device));
        checkCudaErrors(cuDeviceComputeCapability(&major, &minor, current_device));

        int computeMode;
        getCudaAttribute<int>(&computeMode, CU_DEVICE_ATTRIBUTE_COMPUTE_MODE, current_device);

        if (computeMode != CU_COMPUTEMODE_PROHIBITED)
        {
            if (major == 9999 && minor == 9999)
            {
                sm_per_multiproc = 1;
            }
            else
            {
                sm_per_multiproc = _ConvertSMVer2CoresDRV(major, minor);
            }

            int compute_perf  = multiProcessorCount * sm_per_multiproc * clockRate;

            if (compute_perf  > max_compute_perf)
            {
                // If we find GPU with SM major > 2, search only these
                if (best_SM_arch > 2)
                {
                    // If our device==dest_SM_arch, choose this, or else pass
                    if (major == best_SM_arch)
                    {
                        max_compute_perf  = compute_perf;
                        max_perf_device   = current_device;
                    }
                }
                else
                {
                    max_compute_perf  = compute_perf;
                    max_perf_device   = current_device;
                }
            }
        }

        ++current_device;
    }

    return max_perf_device;
}

// This function returns the best Graphics GPU based on performance
inline int gpuGetMaxGflopsGLDeviceIdDRV()
{
    CUdevice current_device = 0, max_perf_device = 0;
    int device_count     = 0, sm_per_multiproc = 0;
    int max_compute_perf = 0, best_SM_arch     = 0;
    int major = 0, minor = 0, multiProcessorCount, clockRate;
    int bTCC = 0;
    char deviceName[256];

    cuInit(0);
    checkCudaErrors(cuDeviceGetCount(&device_count));

    if (device_count == 0)
    {
        fprintf(stderr, "gpuGetMaxGflopsGLDeviceIdDRV error: no devices supporting CUDA\n");
        exit(EXIT_FAILURE);
    }

    // Find the best major SM Architecture GPU device that are graphics devices
    while (current_device < device_count)
    {
        checkCudaErrors(cuDeviceGetName(deviceName, 256, current_device));
        checkCudaErrors(cuDeviceComputeCapability(&major, &minor, current_device));

#if CUDA_VERSION >= 3020
        checkCudaErrors(cuDeviceGetAttribute(&bTCC,  CU_DEVICE_ATTRIBUTE_TCC_DRIVER, current_device));
#else

        // Assume a Tesla GPU is running in TCC if we are running CUDA 3.1
        if (deviceName[0] == 'T')
        {
            bTCC = 1;
        }

#endif

        int computeMode;
        getCudaAttribute<int>(&computeMode, CU_DEVICE_ATTRIBUTE_COMPUTE_MODE, current_device);

        if (computeMode != CU_COMPUTEMODE_PROHIBITED)
        {
            if (!bTCC)
            {
                if (major > 0 && major < 9999)
                {
                    best_SM_arch = MAX(best_SM_arch, major);
                }
            }
        }

        current_device++;
    }

    // Find the best CUDA capable GPU device
    current_device = 0;

    while (current_device < device_count)
    {
        checkCudaErrors(cuDeviceGetAttribute(&multiProcessorCount,
                                             CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT,
                                             current_device));
        checkCudaErrors(cuDeviceGetAttribute(&clockRate,
                                             CU_DEVICE_ATTRIBUTE_CLOCK_RATE,
                                             current_device));
        checkCudaErrors(cuDeviceComputeCapability(&major, &minor, current_device));

#if CUDA_VERSION >= 3020
        checkCudaErrors(cuDeviceGetAttribute(&bTCC,  CU_DEVICE_ATTRIBUTE_TCC_DRIVER, current_device));
#else

        // Assume a Tesla GPU is running in TCC if we are running CUDA 3.1
        if (deviceName[0] == 'T')
        {
            bTCC = 1;
        }

#endif

        int computeMode;
        getCudaAttribute<int>(&computeMode, CU_DEVICE_ATTRIBUTE_COMPUTE_MODE, current_device);

        if (computeMode != CU_COMPUTEMODE_PROHIBITED)
        {
            if (major == 9999 && minor == 9999)
            {
                sm_per_multiproc = 1;
            }
            else
            {
                sm_per_multiproc = _ConvertSMVer2CoresDRV(major, minor);
            }

            // If this is a Tesla based GPU and SM 2.0, and TCC is disabled, this is a contendor
            if (!bTCC)   // Is this GPU running the TCC driver?  If so we pass on this
            {
                int compute_perf  = multiProcessorCount * sm_per_multiproc * clockRate;

                if (compute_perf  > max_compute_perf)
                {
                    // If we find GPU with SM major > 2, search only these
                    if (best_SM_arch > 2)
                    {
                        // If our device = dest_SM_arch, then we pick this one
                        if (major == best_SM_arch)
                        {
                            max_compute_perf  = compute_perf;
                            max_perf_device   = current_device;
                        }
                    }
                    else
                    {
                        max_compute_perf  = compute_perf;
                        max_perf_device   = current_device;
                    }
                }
            }
        }

        ++current_device;
    }

    return max_perf_device;
}

// General initialization call to pick the best CUDA Device
inline CUdevice findCudaDeviceDRV(int argc, const char **argv)
{
    CUdevice cuDevice;
    int devID = 0;

    // If the command-line has a device number specified, use it
    if (checkCmdLineFlag(argc, (const char **)argv, "device"))
    {
        devID = gpuDeviceInitDRV(argc, argv);

        if (devID < 0)
        {
            printf("exiting...\n");
            exit(EXIT_SUCCESS);
        }
    }
    else
    {
        // Otherwise pick the device with highest Gflops/s
        char name[100];
        devID = gpuGetMaxGflopsDeviceIdDRV();
        checkCudaErrors(cuDeviceGet(&cuDevice, devID));
        cuDeviceGetName(name, 100, cuDevice);
        printf("> Using CUDA Device [%d]: %s\n", devID, name);
    }

    cuDeviceGet(&cuDevice, devID);

    return cuDevice;
}

// This function will pick the best CUDA device available with OpenGL interop
inline CUdevice findCudaGLDeviceDRV(int argc, const char **argv)
{
    CUdevice cuDevice;
    int devID = 0;

    // If the command-line has a device number specified, use it
    if (checkCmdLineFlag(argc, (const char **)argv, "device"))
    {
        devID = gpuDeviceInitDRV(argc, (const char **)argv);

        if (devID < 0)
        {
            printf("no CUDA capable devices found, exiting...\n");
            exit(EXIT_SUCCESS);
        }
    }
    else
    {
        char name[100];
        // Otherwise pick the device with highest Gflops/s
        devID = gpuGetMaxGflopsGLDeviceIdDRV();
        checkCudaErrors(cuDeviceGet(&cuDevice, devID));
        cuDeviceGetName(name, 100, cuDevice);
        printf("> Using CUDA/GL Device [%d]: %s\n", devID, name);
    }

    return devID;
}

// General check for CUDA GPU SM Capabilities
inline bool checkCudaCapabilitiesDRV(int major_version, int minor_version, int devID)
{
    CUdevice cuDevice;
    char name[256];
    int major = 0, minor = 0;

    checkCudaErrors(cuDeviceGet(&cuDevice, devID));
    checkCudaErrors(cuDeviceGetName(name, 100, cuDevice));
    checkCudaErrors(cuDeviceComputeCapability(&major, &minor, devID));

    if ((major > major_version) ||
        (major == major_version && minor >= minor_version))
    {
        printf("> Device %d: <%16s >, Compute SM %d.%d detected\n", devID, name, major, minor);
        return true;
    }
    else
    {
        printf("No GPU device was found that can support CUDA compute capability %d.%d.\n", major_version, minor_version);
        return false;
    }
}
#endif

// end of CUDA Helper Functions

#endif
