#ifndef CACHEHOUND_ADAPTERS_ARMV8_BYPASS_ADAPTER_HPP
#define CACHEHOUND_ADAPTERS_ARMV8_BYPASS_ADAPTER_HPP

#include "../concepts/armv8_memory.hpp"
#include "../concepts/placement_policy.hpp"
#include "../concepts/resettable_memory.hpp"
#include "../concepts/flushable_memory.hpp"
#include "../concepts/stats_memory.hpp"
#include "../concepts/switch_channel_memory.hpp"

namespace cachehound {

template <armv8_memory Memory, placement_policy PlacementPolicy>
class armv8_bypass_adapter {
    Memory& memory_;
    PlacementPolicy& placement_policy_;

    void invalidate_set(std::size_t set);

public:
    using region_type = typename Memory::region_type;
    using stats_type = typename Memory::stats_type;

    armv8_bypass_adapter(Memory& memory, PlacementPolicy& placement_policy);

    decltype(auto) regions() const noexcept;

    std::uint8_t offset_bits() const noexcept;

    std::uint8_t index_bits(unsigned level) const noexcept;

    std::size_t ways(unsigned level) const noexcept;

    void access(std::uintptr_t address);

    unsigned instrumented_access(std::uintptr_t address);

    unsigned levels() const noexcept;

    void cisw(unsigned level, std::size_t set, std::size_t way);

    void csw(unsigned level, std::size_t set, std::size_t way);

    void isw(unsigned level, std::size_t set, std::size_t way);

    void reset() noexcept
        requires resettable_memory<Memory>;

    void switch_channel() noexcept
        requires switch_channel_memory<Memory>;

    void flush() noexcept
        requires flushable_memory<Memory>;

    stats_type stats() const noexcept
        requires stats_memory<Memory>;
};

}

#include "./impl/armv8_bypass_adapter.hpp"

#endif /* CACHEHOUND_ADAPTERS_ARMV8_BYPASS_ADAPTER_HPP */
