#ifndef ARCHITECTURE_H
#define ARCHITECTURE_H

#if defined(__amd64__)  \
 || defined(__amd64)    \
 || defined(__x86_64__) \
 || defined(__x86_64)   \
 || defined(_M_X64)     \
 || defined(_M_AMD64)
    #define ARCHITECTURE "AMD64/x86_64"
#endif

#if defined(__i386__) \
 || defined(i386)     \
 || defined(_M_IX86)  \
 || defined(_X86_)    \
 || defined(__THW_INTEL)
    #define ARCHITECTURE "i386/x86"
#endif

#if defined(__arm__)
    #if defined(__ARM_ARCH_7__)  \
     || defined(__ARM_ARCH_7A__) \
     || defined(__ARM_ARCH_7K__) \
     || defined(__ARM_ARCH_7R__) \
     || defined(__ARM_ARCH_7S__)
        #if defined(__arm64) \
         || defined(__aarch64)
            #define ARCHITECTURE "ARM7 64-bit"
        #else
            #define ARCHITECTURE "ARM7 32-bit"
        #endif
    #elif defined(__ARM_ARCH_8__)
        #if defined(__arm64) \
         || defined(__aarch64)
            #define ARCHITECTURE "ARM8 64-bit"
        #else
            #define ARCHITECTURE "ARM8 32-bit"
        #endif
    #else
        #if defined(__arm64) \
         || defined(__aarch64)
            #define ARCHITECTURE "ARM 64-bit"
        #else
            #define ARCHITECTURE "ARM 32-bit"
        #endif
    #endif
#endif

#if defined(__ppc__)     \
 || defined(__PPC__)     \
 || defined(__powerpc__) \
 || defined(__powerpc)   \
 || defined(__POWERPC__) \
 || defined(_M_PPC)      \
 || defined(__PPC)
    #define ARCHITECTURE "PowerPC"
#endif

#if (  defined(__ppc64__)      \
    || defined(__PPC64__))
    #define ARCHITECTURE "PowerPC64"
#endif

#if defined(mips)     \
 || defined(__mips__) \
 || defined(MIPS)     \
 || defined(_MIPS_)
    #if defined(__mips64))
        #define ARCHITECTURE "MIPS 32-bit"
    #else
        #define ARCHITECTURE "MIPS 64-bit"
    #endif
#endif

#if defined(__ia64__) \
 || defined(_M_IA64)
    #if !defined(__LP64__)
        #define ARCHITECTURE "Itanium 32-bit"
    #else
        #define ARCHITECTURE "Itanium 64-bit"
    #endif
#endif

#if defined(__sparc__)
    #define ARCHITECTURE "SPARC"
#endif

#if !defined(ARCHITECTURE)
    #define ARCHITECTURE "Unknown"
#endif

#endif // ARCHITECTURE_H
