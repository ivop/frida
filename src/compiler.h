#ifndef COMPILER_H
#define COMPILER_H

#if defined(__GNUC__)
    #if defined(__clang__)
        #define COMPILER "Clang"
        #define COMPILER_VERSION __clang_major__
    #elif defined(__INTEL_COMPILER)
        #define COMPILER "ICC"
        #define COMPILER_VERSION __INTEL_COMPILER
    #elif defined(__MINGW64__)
        #define COMPILER "MinGW 64-bit"
        #define COMPILER_VERSION __GNUC__
    #elif defined(__MINGW32__)
        #define COMPILER "MinGW 32-bit"
        #define COMPILER_VERSION __GNUC__
    #elif defined(__EMSCRIPTEN__)
        #define COMPILER "emscripten"
        #define COMPILER_VERSION __EMSCRIPTEN_major__
    #else
        #define COMPILER "GCC"
        #define COMPILER_VERSION __GNUC__
    #endif
#endif

#if defined(_MSC_VER)
    #define COMPILER "MSVC"
    #define COMPILER_VERSION _MSC_VER
#endif

#if !defined(COMPILER)
    #define COMPILER "Unknown"
    #define COMPILER_VERSION 0
#endif

#endif // COMPILER_H
