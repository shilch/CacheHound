#ifndef CACHEHOUND_ADAPTERS_IMPL_ARMV8_BYPASS_ADAPTER_HPP
#define CACHEHOUND_ADAPTERS_IMPL_ARMV8_BYPASS_ADAPTER_HPP

#include "../armv8_bypass_adapter.hpp"
#include "../../concepts/armv8_memory.hpp"
#include "../../concepts/placement_policy.hpp"

template<cachehound::armv8_memory Memory, cachehound::placement_policy PlacementPolicy>
void cachehound::armv8_bypass_adapter<Memory, PlacementPolicy>:: invalidate_set(std::size_t set)
{
    for (int way = 0; way < memory_.ways(0); way++)
        memory_.isw(0, set, way);
    memory_.flush();
}

template<cachehound::armv8_memory Memory, cachehound::placement_policy PlacementPolicy>
cachehound::armv8_bypass_adapter<Memory, PlacementPolicy>::armv8_bypass_adapter(Memory& memory, PlacementPolicy& placement_policy)
    : memory_(memory)
    , placement_policy_(placement_policy)
{
}

template<cachehound::armv8_memory Memory, cachehound::placement_policy PlacementPolicy>
decltype(auto) cachehound::armv8_bypass_adapter<Memory, PlacementPolicy>::regions() const noexcept
{
    return memory_.regions();
}

template<cachehound::armv8_memory Memory, cachehound::placement_policy PlacementPolicy>
std::uint8_t cachehound::armv8_bypass_adapter<Memory, PlacementPolicy>::offset_bits() const noexcept
{
    return memory_.offset_bits();
}

template<cachehound::armv8_memory Memory, cachehound::placement_policy PlacementPolicy>
std::uint8_t cachehound::armv8_bypass_adapter<Memory, PlacementPolicy>::index_bits(unsigned level) const noexcept
{
    return memory_.index_bits(level + 1);
}

template<cachehound::armv8_memory Memory, cachehound::placement_policy PlacementPolicy>
std::size_t cachehound::armv8_bypass_adapter<Memory, PlacementPolicy>::ways(unsigned level) const noexcept
{
    return memory_.ways(level + 1);
}

template<cachehound::armv8_memory Memory, cachehound::placement_policy PlacementPolicy>
void cachehound::armv8_bypass_adapter<Memory, PlacementPolicy>::access(std::uintptr_t address)
{
    instrumented_access(address);
}

template<cachehound::armv8_memory Memory, cachehound::placement_policy PlacementPolicy>
unsigned cachehound::armv8_bypass_adapter<Memory, PlacementPolicy>::instrumented_access(std::uintptr_t address)
{
    std::size_t set = placement_policy_(address);
    invalidate_set(set);
    auto level = memory_.instrumented_access(address);
    assert(level != 0);
    invalidate_set(set);
    return level - 1;
}

template<cachehound::armv8_memory Memory, cachehound::placement_policy PlacementPolicy>
unsigned cachehound::armv8_bypass_adapter<Memory, PlacementPolicy>::levels() const noexcept
{
    return memory_.levels() - 1;
}

template<cachehound::armv8_memory Memory, cachehound::placement_policy PlacementPolicy>
void cachehound::armv8_bypass_adapter<Memory, PlacementPolicy>::cisw(unsigned level, std::size_t set, std::size_t way) {
    memory_.cisw(level + 1, set, way);
}

template<cachehound::armv8_memory Memory, cachehound::placement_policy PlacementPolicy>
void cachehound::armv8_bypass_adapter<Memory, PlacementPolicy>::csw(unsigned level, std::size_t set, std::size_t way) {
    memory_.csw(level + 1, set, way);
}

template<cachehound::armv8_memory Memory, cachehound::placement_policy PlacementPolicy>
void cachehound::armv8_bypass_adapter<Memory, PlacementPolicy>::isw(unsigned level, std::size_t set, std::size_t way) {
    memory_.isw(level + 1, set, way);
}

template<cachehound::armv8_memory Memory, cachehound::placement_policy PlacementPolicy>
void cachehound::armv8_bypass_adapter<Memory, PlacementPolicy>::reset() noexcept
    requires resettable_memory<Memory>
{
    memory_.reset();
}

template<cachehound::armv8_memory Memory, cachehound::placement_policy PlacementPolicy>
void cachehound::armv8_bypass_adapter<Memory, PlacementPolicy>::switch_channel() noexcept
    requires switch_channel_memory<Memory>
{
    memory_.switch_channel();
}

template<cachehound::armv8_memory Memory, cachehound::placement_policy PlacementPolicy>
void cachehound::armv8_bypass_adapter<Memory, PlacementPolicy>::flush() noexcept
    requires flushable_memory<Memory>
{
    memory_.flush();
}

template<cachehound::armv8_memory Memory, cachehound::placement_policy PlacementPolicy>
typename cachehound::armv8_bypass_adapter<Memory, PlacementPolicy>::stats_type
cachehound::armv8_bypass_adapter<Memory, PlacementPolicy>::stats() const noexcept
    requires stats_memory<Memory>
{
    return memory_.stats();
}

#endif /* CACHEHOUND_ADAPTERS_IMPL_ARMV8_BYPASS_ADAPTER_HPP */
