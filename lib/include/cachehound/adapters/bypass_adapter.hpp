#ifndef CACHEHOUND_ADAPTERS_BYPASS_ADAPTER_HPP
#define CACHEHOUND_ADAPTERS_BYPASS_ADAPTER_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

#include "../concepts/instrumented_memory.hpp"
#include "../concepts/placement_policy.hpp"
#include "../concepts/resettable_memory.hpp"
#include "../concepts/flushable_memory.hpp"
#include "../concepts/stats_memory.hpp"
#include "../concepts/switch_channel_memory.hpp"
#include "../concepts/x86_memory.hpp"
#include "../util/basic_extended_memory_region.hpp"

#include "./detail/bypass_adapter_base.hpp"

namespace cachehound {

// TODO: Does not seem to work as I wanted to. But could also be a bug with memory_hierarchy.
// Works like 19 our of 20 times on non_inclusive/inclusive cache. But I need to figure out how to deal with exclusive caches.
template <instrumented_memory Memory, placement_policy PlacementPolicy>
class bypass_adapter : public detail::bypass_adapter_base<Memory> {
public:
    using region_type = std::conditional_t<
        extended_memory_region<typename Memory::region_type>,
        basic_extended_memory_region, basic_memory_region
    >;

private:
    Memory& memory_;
    PlacementPolicy& placement_policy_;
    std::vector<region_type> regions_;
    std::vector<std::vector<std::uintptr_t>> evsets_;

    void access_eviction_set(std::size_t set);

public:

    bypass_adapter(Memory& memory, PlacementPolicy& placement_policy);

    decltype(auto) regions() const noexcept;

    std::uint8_t offset_bits() const noexcept;

    std::uint8_t index_bits(unsigned level) const noexcept;

    std::size_t ways(unsigned level) const noexcept;

    void access(std::uintptr_t address);

    unsigned instrumented_access(std::uintptr_t address);

    unsigned levels() const noexcept;

    void clflush(std::uintptr_t address)
        requires x86_memory<Memory>;

    void wbinvd()
        requires x86_memory<Memory>;

    void reset() noexcept
        requires resettable_memory<Memory>;

    void switch_channel() noexcept
        requires switch_channel_memory<Memory>;

    void flush() noexcept
        requires flushable_memory<Memory>;

    auto stats() const noexcept
        requires stats_memory<Memory>;
};

}

#include "./impl/bypass_adapter.hpp"

#endif /* CACHEHOUND_ADAPTERS_BYPASS_ADAPTER_HPP */
