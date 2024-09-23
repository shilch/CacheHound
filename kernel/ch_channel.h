#ifndef CH_CHANNEL_H
#define CH_CHANNEL_H

#ifdef __KERNEL__
#    include <linux/types.h>
#    include <linux/atomic.h>
#    include <linux/string.h>
#else
#    include <stdint.h>
#    include <stddef.h>
#    include <atomic>
#    include <sched.h>
#    include <cassert>
#endif

enum {
    _CH_CHANNEL_IDLE   = 0,
    _CH_CHANNEL_ACCESS = 1,
};

struct ch_channel {
    // least-significant bit: IDLE/ACCESS
    // higher bits: size
#ifdef __KERNEL__
    atomic64_t control;
#else
    std::atomic<uint64_t> control;
#endif
    uintptr_t data[7];
};
#ifdef __cplusplus
    static_assert(sizeof(ch_channel) <= 64, "Channel must fit into a cache line");

extern "C" {
#endif

#ifdef __KERNEL__
inline void ch_channel_init(struct ch_channel* ch) {
    memset(ch->data, 0, sizeof(ch->data));
    atomic64_set(&ch->control, _CH_CHANNEL_IDLE);
}

inline void _ch_pause(void)
#if defined(__x86_64__) || defined(_M_X64)
    {
        asm volatile ("pause");
    }
#elif defined(__aarch64__) || defined(_M_ARM64)
    {
        asm volatile ("isb sy");
    }
#else
    ;
#endif

inline void _ch_channel_terminate(struct ch_channel* ch) {
    while(atomic64_cmpxchg(&ch->control, _CH_CHANNEL_IDLE, _CH_CHANNEL_ACCESS) != _CH_CHANNEL_IDLE) {
#ifdef __KERNEL__
        _ch_pause();
#else
        sched_yield();
#endif
    }
}

#endif

inline uint64_t _ch_channel_read_control(
    struct ch_channel* ch,
    size_t* count /* Only valid in the case of ACCESS */
) {
    uint64_t control;
#ifdef __KERNEL__
    control = atomic64_read_acquire(&ch->control);
#else
    control = ch->control.load(std::memory_order_acquire);
#endif
    if(count) {
        *count = (control >> 1) & 0b111;
    }
    return control&1;
}

inline size_t _ch_channel_spin(
    struct ch_channel* ch,
    uint64_t desired
) {
    size_t count;
    while(_ch_channel_read_control(ch, &count) != desired) {
#ifdef __KERNEL__
        _ch_pause();
#else
        sched_yield();
#endif
    }
    return count;
}

inline void _ch_channel_write_control(
    struct ch_channel* ch,
    uint64_t control,
    size_t count /* only in the case of ACCESS */
) {
#ifdef __KERNEL__
    atomic64_set_release(&ch->control, control|(count << 1));
#else
    ch->control.store(control|(count << 1), std::memory_order_release);
#endif
}

inline void _ch_channel_set_idle(struct ch_channel* ch) {
    _ch_channel_write_control(ch, _CH_CHANNEL_IDLE, 0);
}

inline size_t _ch_channel_wait_for_access(struct ch_channel* ch) {
    return _ch_channel_spin(ch, _CH_CHANNEL_ACCESS);
}

inline void ch_channel_wait_for_idle(struct ch_channel* ch) {
    _ch_channel_spin(ch, _CH_CHANNEL_IDLE);
}

inline void ch_channel_set_access(struct ch_channel* ch, size_t count) {
#ifndef __KERNEL__
    // assert(count > 0);
#endif
    _ch_channel_write_control(ch, _CH_CHANNEL_ACCESS, count);
}

#ifdef __cplusplus
}
#endif

#endif
