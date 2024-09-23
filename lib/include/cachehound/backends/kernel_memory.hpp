#ifndef CACHEHOUND_BACKENDS_KERNEL_MEMORY_HPP
#define CACHEHOUND_BACKENDS_KERNEL_MEMORY_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

#include "ch_channel.h"
#include "ch_ioc.h"

#include "../util/address_checker.hpp"
#include "../util/basic_extended_memory_region.hpp"
#include "../concepts/memory.hpp"
#include "../concepts/stats_memory.hpp"

namespace cachehound {

class kernel_memory {
public:
    enum class isolation_level : decltype(ch_ioc_start_config::isolation_level) {
        off = CH_ISOLATION_OFF,
        no_preempt = CH_ISOLATION_NO_PREEMPT,
        disable_irq = CH_ISOLATION_DISABLE_IRQ
    };

    using region_type = basic_extended_memory_region;

    using pmu_handler_type = unsigned(std::uint64_t, std::uint64_t, std::uint64_t
                               , std::uint64_t, std::uint64_t, std::uint64_t);

    struct stats_type {
        std::size_t accesses              = 0
                  , instrumented_accesses = 0
                  , flushes               = 0
                  , channel_switches      = 0;
#if defined(__x86_64__) || defined(_M_X64)
        std::size_t clflushes = 0
                  , wbinvds   = 0;
#elif defined(__aarch64__) || defined(_M_ARM64)
        std::size_t csws  = 0
                  , isws  = 0
                  , cisws = 0;
#endif
    };

private:
    static constexpr std::size_t reserved_upper_bits = 2;

    static int obtain_ch_fd();
    static ch_channel* mmap_channels(int fd, int page_size);
    static bool alloc_region(int fd, ch_ioc_alloc_config& config);
    static void read_cache_info(int fd, ch_ioc_cache_info& cache_info);
    static void start_agent(int fd, ch_ioc_start_config& config);
    ch_channel* active_channel() const;
    void switch_channel(std::size_t channel) noexcept;
    void internal_access(std::uintptr_t address);
    void defragment_regions();

    ch_channel* channels_;
    std::size_t active_channel_;
    std::size_t channel_count_;

    std::vector<region_type> regions_;
    std::array<std::uintptr_t, 7> buffer_ {};
    std::size_t buffered_ = 0;
    address_checker address_checker_;

    stats_type stats_{};

    std::function<pmu_handler_type> pmu_handler_;
    ch_ioc_cache_info cache_info_;

public:
    // TODO: Wrap handler and events in a common class
    kernel_memory(
        std::size_t min_size,
        unsigned cpu,
        auto&& pmu_events,
        std::function<pmu_handler_type> pmu_handler,
        isolation_level isolation = isolation_level::no_preempt,
        unsigned max_order = 6);

    ~kernel_memory() noexcept;
    [[nodiscard]] const std::vector<region_type>& regions() const noexcept;
    [[nodiscard]] unsigned levels() const noexcept;

    std::uint8_t offset_bits() const noexcept;
    std::uint8_t index_bits(unsigned level) const noexcept;
    std::size_t ways(unsigned level) const noexcept;

    void access(std::uintptr_t address) noexcept;
    unsigned instrumented_access(std::uintptr_t address);
    inline void flush() noexcept;

    stats_type stats() const noexcept;


#if defined(__x86_64__) || defined(_M_X64)

    void clflush(std::uintptr_t address) noexcept;
    void wbinvd() noexcept;

#elif defined(__aarch64__) || defined(_M_ARM64)

private:
    void internal_cisw(bool clean, bool invalidate, unsigned level, std::size_t set, std::size_t way);

public:
    void csw(unsigned level, std::size_t set, std::size_t way);
    void isw(unsigned level, std::size_t set, std::size_t way);
    void cisw(unsigned level, std::size_t set, std::size_t way);
    void reset();

#endif

    void switch_channel() noexcept;
};

static_assert(memory<kernel_memory>);
static_assert(stats_memory<kernel_memory>);

}

#if defined(CACHEHOUND_HEADER_ONLY)
#include "./impl/kernel_memory.ipp"
#endif
#include "./impl/kernel_memory.hpp"

#endif /* CACHEHOUND_BACKENDS_KERNEL_MEMORY_HPP */
