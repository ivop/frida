#ifndef PLATFORM_H
#define PLATFORM_H

#if defined(__linux__)
    #if defined(__ANDROID__)
        #define PLATFORM "Android"
    #else
        #define PLATFORM "Linux"
    #endif
#endif

#if defined(_WIN32)
    #if defined(_WIN64)
        #define PLATFORM "Windows 64-bit"
    #else
        #define PLATFORM "Windows 32-bit"
    #endif
#endif

#if defined(__APPLE__) && defined(__MACH__)
    #define PLATFORM "macOS"
#endif

#if defined(sun) || defined(__sun)
    #if defined(__SVR4) || defined(__svr4__)
        #define PLATFORM "Solaris"
    #else
        #define PLATFORM "SunOS"
    #endif
#endif

#if defined(__OpenBSD__)
    #define PLATFORM "OpenBSD"
#endif

#if defined(__NetBSD__)
    #define PLATFORM "NetBSD"
#endif

#if defined(__FreeBSD__)
    #define "FreeBSD"
#endif

#if defined(__HAIKU__)
    #define "Haiku"
#endif

#if !defined(PLATFORM)
    #define PLATFORM "Unknown"
#endif

#endif // PLATFORM_H
