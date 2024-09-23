#ifndef CH_IOC_H
#define CH_IOC_H

#include "ch_channel.h"

#ifdef __KERNEL__
#  include <asm/ioctl.h>
#else
#  include <sys/ioctl.h>
#endif

#define CH_IOCTL_TYPE  0xD0661E5
#define CH_DEVICE_NAME "cachehound"

struct ch_ioc_alloc_config {
    /* in */ unsigned order;
    /* out */ uintptr_t virtual_base;
    /* out */ uintptr_t physical_base;
};

struct ch_ioc_cache_info {
    /* out */ unsigned levels;
    /* out */ unsigned offset_bits;
    /* out */ unsigned sets[3];
    /* out */ unsigned ways[3];
};

enum {
    CH_ISOLATION_OFF         = 0,
    CH_ISOLATION_NO_PREEMPT  = 1,
    CH_ISOLATION_DISABLE_IRQ = 2,
    NR_CH_ISOLATION = 3
};
struct ch_ioc_start_config {
    /* in */  unsigned cpu;
    /* in */  unsigned long long evts[3];
    /* in */  unsigned isolation_level;
    /* out */ uintptr_t channels_base; /* Address of the channels inside kernel address space */
    /* out */ size_t active_channel;
    /* out */ size_t channel_count; /* Number of channels allocated in the kernel */
};

enum {
    CH_IOC_ALLOC_MEMORY = _IOWR(CH_IOCTL_TYPE, 0, struct ch_ioc_alloc_config),
    CH_IOC_CACHE_INFO   = _IOR(CH_IOCTL_TYPE, 1, struct ch_ioc_cache_info),
    CH_IOC_START_AGENT  = _IOWR(CH_IOCTL_TYPE, 2, struct ch_ioc_start_config),
};

#endif
