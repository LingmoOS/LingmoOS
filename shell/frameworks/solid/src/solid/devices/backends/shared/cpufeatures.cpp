/*
    SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2013 Patrick Spendrin <ps_ml@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

// clang-format off

#include "cpufeatures.h"

#ifndef _MSC_VER
//for cpuFeatures
#include <csignal>
#include <csetjmp>
#else
#include <intrin.h>
#endif
#include <config-processor.h>

#if defined(__GNUC__) || defined(__INTEL_COMPILER)
#  define HAVE_GNU_INLINE_ASM
#endif

namespace Solid
{
namespace Backends
{
namespace Shared
{

#ifndef _MSC_VER
typedef void (*kde_sighandler_t)(int);

#if defined( __i386__ ) || defined( __x86_64__ )
static jmp_buf env;

#if HAVE_X86_SSE
// Sighandler for the SSE OS support check
static void sighandler(int)
{
    std::longjmp(env, 1);
}
#endif
#endif

#ifdef __i386__
#define ASM_REG(reg)              "%e" reg
#define ASM_POP(reg)              "popl   %%e" reg "             \n\t"
#define ASM_PUSH(reg)             "pushl  %%e" reg "             \n\t"
#define ASM_XOR_REG(reg1, reg2)   "xorl   %%e" reg1 ", %%e" reg2 " \n\t"
#define ASM_XOR_VAR(var, reg)     "xorl   " var ",     %%e" reg "  \n\t"
#define ASM_CMP_REG(reg1, reg2)   "cmpl   %%e" reg1 ", %%e" reg2 " \n\t"
#define ASM_MOV_REG(reg1, reg2)   "movl   %%e" reg1 ", %%e" reg2 " \n\t"
#define ASM_MOV_VAR(var, reg)     "movl   " var ",     %%e" reg "  \n\t"
#elif defined(__x86_64__)
#define ASM_REG(reg)              "%r" reg
#define ASM_POP(reg)              "popq   %%r" reg "             \n\t"
#define ASM_PUSH(reg)             "pushq  %%r" reg "             \n\t"
#define ASM_XOR_REG(reg1, reg2)   "xorq   %%r" reg1 ", %%r" reg2 " \n\t"
#define ASM_XOR_VAR(var, reg)     "xorq   " var ",     %%r" reg "  \n\t"
#define ASM_CMP_REG(reg1, reg2)   "cmpq   %%r" reg1 ", %%r" reg2 " \n\t"
#define ASM_MOV_REG(reg1, reg2)   "movq   %%r" reg1 ", %%r" reg2 " \n\t"
#define ASM_MOV_VAR(var, reg)     "movq   " var ",     %%r" reg "  \n\t"
#endif
#endif

#ifdef __PPC__
static sigjmp_buf jmpbuf;
static sig_atomic_t canjump = 0;

static void sigill_handler(int sig)
{
    if (!canjump) {
        signal(sig, SIG_DFL);
        raise(sig);
    }
    canjump = 0;
    siglongjmp(jmpbuf, 1);
}
#endif

Solid::Processor::InstructionSets cpuFeatures()
{
    volatile unsigned int features = 0;

#if defined( HAVE_GNU_INLINE_ASM )
#if defined( __i386__ ) || defined( __x86_64__ )
    bool haveCPUID = false;
    unsigned int result = 0;
    unsigned int result2 = 0;

    // First check if the CPU supports the CPUID instruction
    __asm__ __volatile__(
        // Try to toggle the CPUID bit in the EFLAGS register
        "pushf                      \n\t"   // Push the EFLAGS register onto the stack
        ASM_POP("cx")                       // Pop the value into ECX
        ASM_MOV_REG("cx", "dx")             // Copy ECX to EDX
        ASM_XOR_VAR("$0x00200000", "cx")    // Toggle bit 21 (CPUID) in ECX
        ASM_PUSH("cx")                      // Push the modified value onto the stack
        "popf                       \n\t"   // Pop it back into EFLAGS

        // Check if the CPUID bit was successfully toggled
        "pushf                      \n\t"   // Push EFLAGS back onto the stack
        ASM_POP("cx")                       // Pop the value into ECX
        ASM_XOR_REG("ax", "ax")             // Zero out the EAX register
        ASM_CMP_REG("cx", "dx")             // Compare ECX with EDX
        "je    .Lno_cpuid_support%= \n\t"   // Jump if they're identical
        ASM_MOV_VAR("$1", "ax")             // Set EAX to true
        ".Lno_cpuid_support%=:      \n\t"
        : "=a"(haveCPUID) : : ASM_REG("cx"), ASM_REG("dx"));

    // If we don't have CPUID we won't have the other extensions either
    if (haveCPUID) {
        // Execute CPUID with the feature request bit set
        __asm__ __volatile__(
            ASM_PUSH("bx")                      // Save EBX
            ASM_MOV_VAR("$1", "ax")             // Set EAX to 1 (features request)
            "cpuid                      \n\t"   // Call CPUID
            ASM_POP("bx")                       // Restore EBX
            : "=d"(result), "=c"(result2) : : ASM_REG("ax"));

        features = result & 0x06800000; //copy the mmx and sse & sse2 bits to features
        features |= result2 & 0x00180101; //copy the ssse3, sse3 and sse4.1, sse4.2 bits to features

        __asm__ __volatile__(
            ASM_PUSH("bx")
            ASM_PUSH("dx")
            ASM_MOV_VAR("$0x80000000", "ax")
            ASM_MOV_VAR("$0x80000000", "dx")
            "cpuid                   \n\t"
            ASM_CMP_REG("dx", "ax")
            "jbe .Lno_extended%=     \n\t"
            ASM_MOV_VAR("$0x80000001", "ax")
            "cpuid                   \n\t"
            ".Lno_extended%=:        \n\t"
            ASM_POP("dx")
            ASM_POP("bx")                      // Restore EBX
            : "=d"(result) : : ASM_REG("ax"), ASM_REG("cx"));

        if (result & 0x80000000) {
            features |= 0x80000000;
        }

#if HAVE_X86_SSE
        // Test bit 25 (SSE support)
        if (features & 0x02000000) {
            // OS support test for SSE.
            // Install our own sighandler for SIGILL.
            kde_sighandler_t oldhandler = std::signal(SIGILL, sighandler);

            // Try executing an SSE insn to see if we get a SIGILL
            if (setjmp(env)) {
                features &= ~0x06080001;    // The OS support test failed
            } else {
                __asm__ __volatile__("xorps %xmm0, %xmm0");
            }

            // Restore the default sighandler
            std::signal(SIGILL, oldhandler);

            // Note: The OS requirements for SSE2 are the same as for SSE
            //       so we don't have to do any additional tests for that.
        }
#endif // HAVE_X86_SSE
    }
#elif defined __PPC__ && HAVE_PPC_ALTIVEC
    signal(SIGILL, sigill_handler);
    if (sigsetjmp(jmpbuf, 1)) {
        signal(SIGILL, SIG_DFL);
    } else {
        canjump = 1;
        __asm__ __volatile__("mtspr 256, %0\n\t"
                             "vand %%v0, %%v0, %%v0"
                             : /* none */
                             : "r"(-1));
        signal(SIGILL, SIG_DFL);
        features = 0x2;
    }
#endif // __i386__ || __x86_64__
#elif defined(_MSC_VER)
    int array[4], ft = 1;
    __cpuid(array, ft);

    features = array[3] & 0x06800000; //copy the mmx and sse & sse2 bits to features
    features |= array[2] & 0x00180101; //copy the ssse3, sse3 and sse4.1, sse4.2 bits to features

    if (array[3] & 0x80000000) {
        features |= 0x80000000;
    }
#endif //HAVE_GNU_INLINE_ASM
    Solid::Processor::InstructionSets featureflags;

    if (features & 0x80000000) {
        featureflags |= Solid::Processor::Amd3DNow;
    }
    if (features & 0x00800000) {
        featureflags |= Solid::Processor::IntelMmx;
    }
    if (features & 0x02000000) {
        featureflags |= Solid::Processor::IntelSse;
    }
    if (features & 0x04000000) {
        featureflags |= Solid::Processor::IntelSse2;
    }
    if (features & 0x00000001) {
        featureflags |= Solid::Processor::IntelSse3;
    }
    if (features & 0x00000100) {
        featureflags |= Solid::Processor::IntelSsse3;
    }
    if (features & 0x00080000) {
        featureflags |= Solid::Processor::IntelSse41;
    }
    if (features & 0x00100000) {
        featureflags |= Solid::Processor::IntelSse42;
    }

    if (features & 0x2) {
        featureflags |= Solid::Processor::AltiVec;
    }

    return featureflags;
}

}
}
}
