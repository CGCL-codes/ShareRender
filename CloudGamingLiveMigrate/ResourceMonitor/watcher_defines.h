#pragma once
//常用的头文件
#include "d3d9.h"
#define _STRSAFE_H_INCLUDED_
#define VC_EXTRALEAN        // Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT 0x0501

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "windows.h"
#include "fstream"
#include "ostream"
#include "vector"
#include "NtDDNdis.h"
#include "winioctl.h"
#include "adl_sdk\adl_sdk.h"
#include "nv_sdk\nvapi.h"

//#include <strsafe.h>

#pragma comment(lib, "nvapi.lib")
#pragma comment(lib, "d3d9.lib")

#define MAXSIZE 255
#define Freq_Interval 10
#define s_unit 1000

//设置AMD显卡相关的函数指针
typedef int (*ADL_MAIN_CONTROL_CREATE)(ADL_MAIN_MALLOC_CALLBACK callback,int iEnumConnectedAdapters);
typedef int (*ADL_MAIN_CONTROL_REFRESH)();
typedef int (*ADL_OVERDRIVE5_TEMPERATURE_GET)(int iAdapterIndex,int iThermalControllerIndex,ADLTemperature *lpTemperature);
typedef int (*ADL_OVERDRIVE5_CURRENTACTIVITY_GET)(int iAdapterIndex,ADLPMActivity *lpActivity);

typedef long long int64_t;



