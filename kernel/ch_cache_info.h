#ifndef CH_CACHE_INFO_H
#define CH_CACHE_INFO_H

#include "ch_plat.h"

inline unsigned ch_cache_levels(void)
#if defined(__x86_64__) || defined(_M_X64)
    {
        if(ch_is_intel()) {
            unsigned levels = 0;
            unsigned long long i;
            
            for(i = 0 ;; i++) {
                unsigned long long eax;
                asm volatile ("cpuid" : "=a"(eax) : "a"(4), "c"(i) : "ebx", "edx");
                if((eax & 0b1111) == 0) break;
                levels = ((eax >> 5) & 0b111);
            }

            return levels;
	} else {
            return 3;
	}
    }
#elif defined(__aarch64__) || defined(_M_ARM64)
    {
        unsigned long long clidr;
        asm volatile("MRS %[out], CLIDR_EL1" : [out] "=r"(clidr) :: "memory");

        unsigned levels = 0;
        for(; levels < 7 && ((clidr >> (3 * levels)) & 0b111) != 0; levels++);
        return levels;
    }
#else
    ;
#endif

inline unsigned ch_cache_offset_bits(void)
#if defined(__x86_64__) || defined(_M_X64)
    {
        return 6;
    }
#elif defined(__aarch64__) || defined(_M_ARM64)
    {
        asm volatile("MSR CSSELR_EL1, %[in]" :: [in] "r"(0) : "memory");

        unsigned long long ccsidr;
        asm volatile("MRS %[out], CCSIDR_EL1" : [out] "=r"(ccsidr) :: "memory");

        return (ccsidr & 0b11) + 4;
    }
#else
    ;
#endif

inline unsigned ch_dcache_sets(unsigned level)
#if defined(__x86_64__) || defined(_M_X64)
    {
        if(ch_is_intel()) {
            unsigned long long i;
            for(i = 0 ;; i++) {
                unsigned long long eax, ecx;
                asm volatile ("cpuid" : "=a"(eax), "=c"(ecx) : "a"(4), "c"(i) : "ebx", "edx");
                if((eax & 0b1111) == 0) break;
                if((eax & 0b1111) == 2) continue;
                if(((eax >> 5) & 0b111) != level + 1) continue;
                return ecx + 1;
            }
	} else {
	    // TODO: Hardcoded for the test system for now, but the AMD API to retrieve cache params is cumbersome
	    if(level == 0) return 64;
	    if(level == 1) return 1024;
	    if(level == 2) return 16384;
	}
        return 0;
    }
#elif defined(__aarch64__) || defined(_M_ARM64)
    {
        asm volatile("MSR CSSELR_EL1, %[in]" :: [in] "r"(level << 1) : "memory");

        unsigned long long ccsidr;
        asm volatile("MRS %[out], CCSIDR_EL1" : [out] "=r"(ccsidr) :: "memory");

        return ((ccsidr >> 13) & ((1 << (27 - 12)) - 1)) + 1;
    }
#else
    ;
#endif

inline unsigned ch_dcache_ways(unsigned level)
#if defined(__x86_64__) || defined(_M_X64)
    {
	if(ch_is_intel()) {
            unsigned long long i;
            for(i = 0 ;; i++) {
                unsigned long long eax, ebx;
                asm volatile ("cpuid" : "=a"(eax), "=b"(ebx) : "a"(4), "c"(i) : "edx");
                if((eax & 0b1111) == 0) break;
                if((eax & 0b1111) == 2) continue;
	        if(((eax >> 5) & 0b111) != level + 1) continue;
	        return ((ebx >> 22) & 0b1111111111) + 1;
	    }
	} else {
	    // TODO: Hardcoded for the test system for now, but the AMD API to retrieve cache params is cumbersome
	    if(level == 0) return 8;
	    if(level == 1) return 8;
	    if(level == 2) return 16;
	}

        return 0;	
    }
#elif defined(__aarch64__) || defined(_M_ARM64)
    {
        asm volatile("MSR CSSELR_EL1, %[in]" :: [in] "r"(level << 1) : "memory");

        unsigned long long ccsidr;
        asm volatile("MRS %[out], CCSIDR_EL1" : [out] "=r"(ccsidr) :: "memory");

        return ((ccsidr >> 3) & ((1 << (12 - 3)) - 1)) + 1;
    }
#else
    ;
#endif


#endif
