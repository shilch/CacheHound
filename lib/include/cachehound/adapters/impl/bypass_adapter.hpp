#ifndef CACHEHOUND_ADAPTERS_IMPL_BYPASS_ADAPTER_HPP
#define CACHEHOUND_ADAPTERS_IMPL_BYPASS_ADAPTER_HPP

#include "../bypass_adapter.hpp"

#include <cassert>

template <cachehound::instrumented_memory Memory, cachehound::placement_policy PlacementPolicy>
void cachehound::bypass_adapter<Memory, PlacementPolicy>::access_eviction_set(std::size_t set) {
    for(std::uintptr_t address : evsets_[set]) {
        memory_.access(address);
    }
}

template <cachehound::instrumented_memory Memory, cachehound::placement_policy PlacementPolicy>
cachehound::bypass_adapter<Memory, PlacementPolicy>::bypass_adapter(Memory& memory, PlacementPolicy& placement_policy)
    : memory_(memory)
    , placement_policy_(placement_policy)
    , evsets_(1 << memory.index_bits(0))
{
    std::size_t ways = memory.ways(0)
              , sets = 1 << memory.index_bits(0)
              , increment = (1 << memory.offset_bits());

    std::size_t found_evsets = 0;

    for(auto& region : memory.regions()) {
        std::uintptr_t address = region.base();
        std::uintptr_t last_address = region.base() + region.size();

        if(found_evsets < sets) {
            for (; address < last_address; address += increment) {
                auto set = placement_policy_(address);
                if (evsets_[set].size() < ways) {
                    evsets_[set].emplace_back(address);
                    found_evsets += (evsets_[set].size() == ways);
                    if(found_evsets == sets) {
                        auto new_base = address + increment;
                        if (new_base < last_address) {
                            auto reserved_space = new_base - region.base();
                            auto new_size = region.size() - reserved_space;

                            if constexpr(extended_memory_region<region_type>) {
                                auto new_physical_base = region.physical_base() + reserved_space;
                                regions_.emplace_back(new_base, new_physical_base, new_size);
                            } else {
                                regions_.emplace_back(new_base, new_size);
                            }
                        }

                        goto done;
                    }
                }
            }
        } else {
            if constexpr(extended_memory_region<region_type>) {
                regions_.emplace_back(region.base(), region.physical_base(), region.size());
            } else {
                regions_.emplace_back(region.base(), region.size());
            }
        }
    
    done:;
    }
}

template<cachehound::instrumented_memory Memory, cachehound::placement_policy PlacementPolicy>
decltype(auto) cachehound::bypass_adapter<Memory, PlacementPolicy>::regions() const noexcept
{
    return regions_;
}

template<cachehound::instrumented_memory Memory, cachehound::placement_policy PlacementPolicy>
std::uint8_t cachehound::bypass_adapter<Memory, PlacementPolicy>::offset_bits() const noexcept
{
    return memory_.offset_bits();
}

template<cachehound::instrumented_memory Memory, cachehound::placement_policy PlacementPolicy>
std::uint8_t cachehound::bypass_adapter<Memory, PlacementPolicy>::index_bits(unsigned level) const noexcept
{
    return memory_.index_bits(level + 1);
}

template<cachehound::instrumented_memory Memory, cachehound::placement_policy PlacementPolicy>
std::size_t cachehound::bypass_adapter<Memory, PlacementPolicy>::ways(unsigned level) const noexcept
{
    return memory_.ways(level + 1);
}

template <cachehound::instrumented_memory Memory, cachehound::placement_policy PlacementPolicy>
void cachehound::bypass_adapter<Memory, PlacementPolicy>::access(std::uintptr_t address) 
{
    instrumented_access(address);
}

template <cachehound::instrumented_memory Memory, cachehound::placement_policy PlacementPolicy>
unsigned cachehound::bypass_adapter<Memory, PlacementPolicy>::instrumented_access(std::uintptr_t address)
{
    auto level = memory_.instrumented_access(address);
    if (level > 0)
        return level - 1;

    const auto set = placement_policy_(address);

retry:
    access_eviction_set(set);

    level = memory_.instrumented_access(address);
    if (level == 0)
        goto retry;

    return 0;


//     const auto set = placement_policy_(address);

// retry:
//     access_eviction_set(set);

//     auto level = memory_.instrumented_access(address);
//     if(level == 0) {
//         goto retry;
//     }

//     access_eviction_set(set);
//     return level - 1;
}

template<cachehound::instrumented_memory Memory, cachehound::placement_policy PlacementPolicy>
unsigned cachehound::bypass_adapter<Memory, PlacementPolicy>::levels() const noexcept
{
    return memory_.levels() - 1;
}

template<cachehound::instrumented_memory Memory, cachehound::placement_policy PlacementPolicy>
void cachehound::bypass_adapter<Memory, PlacementPolicy>::clflush(std::uintptr_t address)
    requires cachehound::x86_memory<Memory>
{
    memory_.clflush(address);
}

template<cachehound::instrumented_memory Memory, cachehound::placement_policy PlacementPolicy>
void cachehound::bypass_adapter<Memory, PlacementPolicy>::wbinvd()
    requires x86_memory<Memory>
{
    memory_.wbinvd();
}

template<cachehound::instrumented_memory Memory, cachehound::placement_policy PlacementPolicy>
void cachehound::bypass_adapter<Memory, PlacementPolicy>::reset() noexcept
    requires resettable_memory<Memory>
{
    memory_.reset();
}

template<cachehound::instrumented_memory Memory, cachehound::placement_policy PlacementPolicy>
void cachehound::bypass_adapter<Memory, PlacementPolicy>::switch_channel() noexcept
    requires switch_channel_memory<Memory>
{
    memory_.switch_channel();
}

template<cachehound::instrumented_memory Memory, cachehound::placement_policy PlacementPolicy>
void cachehound::bypass_adapter<Memory, PlacementPolicy>::flush() noexcept
    requires flushable_memory<Memory>
{
    memory_.flush();
}

template<cachehound::instrumented_memory Memory, cachehound::placement_policy PlacementPolicy>
auto cachehound::bypass_adapter<Memory, PlacementPolicy>::stats() const noexcept
    requires stats_memory<Memory>
{
    return memory_.stats();
}


#endif /* CACHEHOUND_ADAPTERS_IMPL_BYPASS_ADAPTER_HPP */
