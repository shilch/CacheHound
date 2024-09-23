#ifndef CACHEHOUND_ADAPTERS_PHYSICAL_ADAPTER_HPP
#define CACHEHOUND_ADAPTERS_PHYSICAL_ADAPTER_HPP

#include <cassert>
#include <map>

#include "../concepts/memory.hpp"
#include "../concepts/armv8_memory.hpp"
#include "../concepts/extended_memory_region_range.hpp"
#include "../concepts/instrumented_memory.hpp"
#include "../util/basic_memory_region.hpp"
#include "cachehound/concepts/flushable_memory.hpp"
#include "cachehound/concepts/resettable_memory.hpp"
#include "cachehound/concepts/switch_channel_memory.hpp"
#include "cachehound/concepts/x86_memory.hpp"

namespace cachehound {

template<memory Memory>
    requires requires(Memory m) {
        { m.regions() } noexcept -> extended_memory_region_range;
    }
class physical_adapter {
public:
    using region_type = basic_memory_region;
    using stats_type = typename Memory::stats_type;

private:
    Memory& memory_;
    std::vector<region_type> regions_;
    std::map<std::uintptr_t, std::uintptr_t> address_translation_;

    std::uintptr_t translate(std::uintptr_t physical_address) {
        auto it = address_translation_.upper_bound(physical_address);
        if(it == address_translation_.begin()) return false;
        std::advance(it, -1);
        assert(physical_address >= it->first);
        return it->second + (physical_address - it->first);
    }

public:
    physical_adapter(Memory& memory) : memory_(memory) {
        for(auto& region : memory_.regions()) {
            regions_.emplace_back(region.physical_base(), region.size());
            address_translation_[region.physical_base()] = region.base();
        }
    }

    void access(std::uintptr_t address) {
        memory_.access(translate(address));
    }

    auto regions() const noexcept {
        return regions_;
    }

    std::uint8_t offset_bits() const noexcept {
        return memory_.offset_bits();
    }

    unsigned instrumented_access(std::uintptr_t address)
        requires instrumented_memory<Memory>
    {
        return memory_.instrumented_access(translate(address));
    }

    unsigned levels() const noexcept
        requires instrumented_memory<Memory>
    {
        return memory_.levels();
    }

    std::uint8_t index_bits(unsigned level) const noexcept
        requires instrumented_memory<Memory>
    {
        return memory_.index_bits(level);
    }

    std::size_t ways(unsigned level) const noexcept
        requires instrumented_memory<Memory>
    {
        return memory_.ways(level);
    }

    void cisw(unsigned level, std::size_t set, std::size_t way)
        requires armv8_memory<Memory>
    {
        memory_.cisw(level, set, way);
    }

    void csw(unsigned level, std::size_t set, std::size_t way)
        requires armv8_memory<Memory>
    {
        memory_.csw(level, set, way);
    }

    void isw(unsigned level, std::size_t set, std::size_t way)
        requires armv8_memory<Memory>
    {
        memory_.isw(level, set, way);
    }

    void flush()
        requires flushable_memory<Memory>
    {
        memory_.flush();
    }

    void reset()
        requires resettable_memory<Memory>
    {
        memory_.reset();
    }

    void switch_channel()
        requires switch_channel_memory<Memory>
    {
        memory_.switch_channel();
    }

    void clflush(std::uintptr_t address)
        requires x86_memory<Memory>
    {
        memory_.clflush(translate(address));
    }

    void wbinvd()
        requires x86_memory<Memory>
    {
        memory_.wbinvd();
    }

    stats_type stats() const noexcept {
        return memory_.stats();
    }
};

}

#endif /* CACHEHOUND_ADAPTERS_PHYSICAL_ADAPTER_HPP */
