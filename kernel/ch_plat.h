#ifndef CH_PLAT_H
#define CH_PLAT_H

#if defined(__x86_64__) || defined(_M_X64)
inline bool ch_is_intel(void)
{
    unsigned long long eax, ebx, ecx, edx;
    asm volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a" (0));
    return ecx == 0x6c65746e;
}
#endif

#endif

