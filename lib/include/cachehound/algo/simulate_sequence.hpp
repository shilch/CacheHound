#ifndef CACHEHOUND_ALGO_SIMULATE_SEQUENCE_HPP
#define CACHEHOUND_ALGO_SIMULATE_SEQUENCE_HPP

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <vector>

#include "../concepts/replacement_policy.hpp"
#include "../concepts/sequence.hpp"

namespace cachehound {

std::size_t simulate_sequence(
    sequence auto&& sequence,
    replacement_policy auto&& policy
) {
    static constexpr auto dirty = std::numeric_limits<std::size_t>::max();
    std::vector<bool> seen;
    std::vector<std::size_t> addr_way;
    std::vector<std::size_t> way_addr(policy.ways(), dirty);
    std::size_t hit_counter = 0;

    for(auto addr : std::forward<decltype(sequence)>(sequence)) {
        if (addr >= seen.size()) {
            seen.resize(1 + addr);
            addr_way.resize(1 + addr);
        }

        auto way = addr_way[addr];
        bool is_hit = way_addr[way] == addr;
        if (is_hit) {
            policy.hit(way);
        } else {
            way = policy.miss();
            addr_way[addr] = way;
            way_addr[way] = addr;
        }

        if (seen[addr]) hit_counter += is_hit;
        else seen[addr] = true;
    }

    return hit_counter;
}

}

#endif /* CACHEHOUND_ALGO_SIMULATE_SEQUENCE_HPP */
