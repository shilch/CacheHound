#ifndef CACHEHOUND_ALGO_IMPL_FIND_EVICTION_SET_HPP
#define CACHEHOUND_ALGO_IMPL_FIND_EVICTION_SET_HPP

#include "../find_eviction_set.hpp"

#include "../is_eviction_set.hpp"
#include "../../concepts/address_distribution.hpp"
#include "../../util/concatenated_address_distribution.hpp"

std::vector<std::uintptr_t> cachehound::find_eviction_set(
    is_eviction_set auto&& is_eviction_set,
    instrumented_memory auto& memory,
    std::uintptr_t target,
    std::uniform_random_bit_generator auto& urbg,
    address_distribution auto& distribution
) {
    static constexpr std::size_t attempts = 5;

    std::vector<std::uintptr_t> out;
    std::unordered_set<std::uintptr_t> seen;

    std::size_t size = memory.ways(0);
    std::size_t attempt = 0;

    while (true) {
        out.resize(size);

        for(std::size_t attempt = 0; attempt < attempts; attempt++) {
            seen.clear();
            seen.emplace(target);

            distribution.reset();

            auto it = out.begin();
            while(it != out.end()) {
                auto address = distribution(urbg);
                if(!seen.contains(address)) {
                    seen.emplace(address);
                    *it++ = address;
                }
            }

            bool is_evset = is_eviction_set(memory, target, std::ranges::subrange(out.cbegin(), out.cend()));
            if(is_evset) {
                return out;
            }
        }

        size += (size / 2);
    }
}

std::vector<std::uintptr_t> cachehound::find_eviction_set(
    is_eviction_set auto&& is_eviction_set,
    instrumented_memory auto& memory,
    std::uintptr_t target,
    std::uniform_random_bit_generator auto& urbg
) {
    std::uint8_t optimistic_same_set_alignment = memory.offset_bits() + memory.index_bits(0);
    cachehound::uniform_address_distribution optimistic_distribution{
        memory, optimistic_same_set_alignment,
        target % (1 << optimistic_same_set_alignment)
    };
    cachehound::uniform_address_distribution pessimistic_distribution{memory};

    cachehound::concatenated_address_distribution distribution{
        std::move(optimistic_distribution),
        memory.index_bits(0) * memory.ways(0),
        std::move(pessimistic_distribution)
    };
    return cachehound::find_eviction_set(std::forward<decltype(is_eviction_set)>(is_eviction_set), memory, target, urbg, distribution);
}


#endif /* CACHEHOUND_ALGO_IMPL_FIND_EVICTION_SET_HPP */
