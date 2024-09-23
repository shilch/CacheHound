#ifndef CH_PMC_H
#define CH_PMC_H

#include "ch_plat.h"

#define CH_IA32_PERFEVTSEL0 0x186
#define CH_PERF_LEGACY_CTL0 0xc0010000

struct ch_local_pmc {
#if defined(__x86_64__) || defined(_M_X64)
    unsigned counters[8];
#elif defined(__aarch64__) || defined(_M_ARM64)
    unsigned long long counters[4];
#endif
};

static unsigned long long CH_PMC_MSR0;

inline void ch_local_pmc_init(void)
#if defined(__x86_64__) || defined(_M_X64)
{
    if (ch_is_intel()) {
        CH_PMC_MSR0 = CH_IA32_PERFEVTSEL0;
    } else {
        CH_PMC_MSR0 = CH_PERF_LEGACY_CTL0;
    }
}
#elif defined(__aarch64__) || defined(_M_ARM64)
{}
#else
;
#endif

/*
* Save the configuration of the first four custom performance counters
* and resets them to zero.
*/
inline void ch_local_pmc_save(struct ch_local_pmc* local_pmc)
#if defined(__x86_64__) || defined(_M_X64)
{
    unsigned* it = local_pmc->counters;
    unsigned* const end = local_pmc->counters + (sizeof(local_pmc->counters) / sizeof(local_pmc->counters[0]));
    int i = 0;
    while(it != end) {
        unsigned* lo = it++;
        unsigned* hi = it++;
        asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(CH_PMC_MSR0 + i) : "memory");
        asm volatile("wrmsr" : : "a"(0), "d"(0), "c"(CH_PMC_MSR0 + i) : "memory");
        i++;
    }
}
#elif defined(__aarch64__) || defined(_M_ARM64)
{
    asm volatile("MRS %[out], PMEVTYPER0_EL0" : [out] "=r"(local_pmc->counters[0]) :: "memory");
    asm volatile("MRS %[out], PMEVTYPER1_EL0" : [out] "=r"(local_pmc->counters[1]) :: "memory");
    asm volatile("MRS %[out], PMEVTYPER2_EL0" : [out] "=r"(local_pmc->counters[2]) :: "memory");
    asm volatile("MRS %[out], PMEVTYPER3_EL0" : [out] "=r"(local_pmc->counters[3]) :: "memory");
}
#endif

/*
* Restore the previously saved configuration of the first four custom performance counters.
*/
inline void ch_local_pmc_restore(const struct ch_local_pmc* local_pmc)
#if defined(__x86_64__) || defined(_M_X64)
{
    const unsigned* it = local_pmc->counters;
    const unsigned* const end = local_pmc->counters + (sizeof(local_pmc->counters) / sizeof(local_pmc->counters[0]));
    int i = 0;
    while(it != end) {
        unsigned lo = *it++;
        unsigned hi = *it++;
        asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(CH_PMC_MSR0 + i) : "memory");
        i++;
    }
}
#elif defined(__aarch64__) || defined(_M_ARM64)
{
    asm volatile("MSR PMEVTYPER0_EL0, %[in]" :: [in] "r"(local_pmc->counters[0]) : "memory");
    asm volatile("MSR PMEVTYPER1_EL0, %[in]" :: [in] "r"(local_pmc->counters[1]) : "memory");
    asm volatile("MSR PMEVTYPER2_EL0, %[in]" :: [in] "r"(local_pmc->counters[2]) : "memory");
    asm volatile("MSR PMEVTYPER3_EL0, %[in]" :: [in] "r"(local_pmc->counters[3]) : "memory");
}
#endif

inline void ch_local_pmc_configure(
    unsigned reg,
    unsigned long long evt
)
#if defined(__x86_64__) || defined(_M_X64)
{
    unsigned int lo, hi;
    lo = evt & ((1UL << 32) - 1);
    hi = evt >> 32;
    asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(CH_PMC_MSR0 + reg) : "memory");
}
#elif defined(__aarch64__) || defined(_M_ARM64)
{
    evt &= 0x3ff;
    evt |= (1 << 27); // Enable counting in EL2

    asm volatile(
        /* Select event */
        "MSR PMSELR_EL0, %[reg]\n"
        "MSR PMXEVTYPER_EL0, %[evt]\n"

        /* Enable PMU */
        "MOV x0, #1\n"
        "MSR PMCR_EL0, x0\n"

        /* Let that register start counting */
        "MRS x0, PMCNTENSET_EL0\n"
        "MOV x1, #1\n"
        "LSL x1, x1, %[reg]\n"
        "ORR x0, x0, x1\n"
        "MSR PMCNTENSET_EL0, x0\n"

        /* Clear filter */
        "MSR PMCCFILTR_EL0, xzr\n"

        /* Instruction serialization */
        "ISB SY\n"

        :
        : [reg] "r"(reg)
        , [evt] "r"(evt)

        : "x0", "x1", "memory"
    );
}
#else
;
#endif

inline unsigned ch_local_pmc_counters(void)
#if defined(__x86_64__) || defined(_M_X64)
{
    return 4;
}
#elif defined(__aarch64__) || defined(_M_ARM64)
{
    unsigned long long pmcr;
    asm volatile("MRS %[pmcr], PMCR_EL0" : [pmcr] "=r"(pmcr));
    return (pmcr >> 11) & ((1 << (15 - 11)) - 1);
}
#else
{
    return 0;
}
#endif

#endif /* CH_PMC_H */
