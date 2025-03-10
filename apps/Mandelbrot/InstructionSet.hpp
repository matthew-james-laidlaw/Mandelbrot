#pragma once
#include <stdint.h>

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
    #if defined(__SSE__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 1)
        #define __SUPPORTS_SSE__ 1
    #else
        #define __SUPPORTS_SSE__ 0
    #endif
#else
    #define __SUPPORTS_SSE__ 0
#endif

#if defined(__ARM_NEON) || defined(__ARM_NEON__) || \
    (defined(_M_ARM64) || defined(__aarch64__))
    #define __SUPPORTS_NEON__ 1
#else
    #define __SUPPORTS_NEON__ 0
#endif

#if defined(__APPLE__)

#include <sys/sysctl.h>
#include <unistd.h>

#if __SUPPORTS_SSE__
    #include <emmintrin.h>
#endif

bool SupportsSSE_Apple()
{
    int cpu_type;
    size_t size = sizeof(cpu_type);
    sysctlbyname("hw.cputype", &cpu_type, &size, nullptr, 0);
    return (cpu_type == 7);
}

bool SupportsNEON_Apple()
{
    int cpu_type;
    size_t size = sizeof(cpu_type);
    sysctlbyname("hw.cputype", &cpu_type, &size, nullptr, 0);
    return (cpu_type == 16777228);
}

#elif defined(__linux__)

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#if __SUPPORTS_SSE__
    #include <emmintrin.h>
#elif __SUPPORTS_NEON__
    #include <arm_neon.h>
#endif

bool SupportsSSE_Linux()
{
    FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
    if (!cpuinfo)
    {
        return false;
    }

    char line[512];
    bool has_sse = false;

    while (fgets(line, sizeof(line), cpuinfo) != nullptr)
    {
        if (strncmp(line, "flags", 5) == 0)
        {
            has_sse = (strstr(line, " sse ") != nullptr);
            break;
        }
    }

    fclose(cpuinfo);
    return has_sse;
}

bool SupportsNEON_Linux()
{
    FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
    if (!cpuinfo)
    {
        return false;
    }

    char line[512];
    bool has_neon = false;

    while (fgets(line, sizeof(line), cpuinfo) != nullptr)
    {
        if (strncmp(line, "Features", 8) == 0)
        {
            has_neon = (strstr(line, " neon ") != nullptr);
            break;
        }
    }

    fclose(cpuinfo);
    return has_neon;
}

#elif defined(_WIN32)

#include <intrin.h>
#include <windows.h>

bool SupportsSSE_Windows()
{
    int cpuInfo[4] = {0};
    __cpuid(cpuInfo, 1);
    return (cpuInfo[3] & (1 << 25)) != 0;
}

bool SupportsNEON_Windows()
{
    SYSTEM_INFO sysInfo;
    GetNativeSystemInfo(&sysInfo);
    return sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM64;
}

#endif

bool SupportsSSE()
{
#if defined(__APPLE__)
    return SupportsSSE_Apple();
#elif defined(__linux__)
    return SupportsSSE_Linux();
#elif defined(_WIN32)
    return SupportsSSE_Windows();
#else
    return false;
#endif
}

bool SupportsNEON()
{
#if defined(__APPLE__)
    return SupportsNEON_Apple();
#elif defined(__linux__)
    return SupportsNEON_Linux();
#elif defined(_WIN32)
    return SupportsNEON_Windows();
#else
    return false;
#endif
}
