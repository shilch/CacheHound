#ifndef CACHEHOUND_SIM_CACHE_SET_ASSOCIATIVE_CACHE_HPP
#define CACHEHOUND_SIM_CACHE_SET_ASSOCIATIVE_CACHE_HPP

#include <cassert>
#include <cstddef>
#include <concepts>
#include <limits>
#include <optional>
#include <span>
#include <type_traits>
#include <vector>

#include "../concepts/cache.hpp"
#include "../concepts/set_associative_policy.hpp"
#include "../util/any_set_associative_policy.hpp"

namespace cachehound::sim {

class set_associative_cache {
    struct cache_line {
        // All cache lines are invalid initially
        bool           valid = false;
        std::uintptr_t tag {};
    };

private:
    any_set_associative_policy policy_;
    std::vector<cache_line> data_;

    std::size_t get_set_index(std::uintptr_t aligned_address) const noexcept {
        std::size_t set_index = policy_.get_set(aligned_address);
        return set_index;
    }

    std::pair<std::size_t, std::size_t> get_cache_line_set_way(const cache_line& line) const noexcept {
        auto offset = std::distance(data_.data(), &line);
        auto way = offset % ways();
        auto set = offset / ways();
        return {set, way};
    }

    cache_line& get_cache_line_at_set_way(std::size_t set, std::size_t way) noexcept {
        return data_[set * ways() + way];
    }

public:
    template<set_associative_policy CachePolicy>
    set_associative_cache(CachePolicy&& policy)
        : policy_(std::move(policy))
        , data_(sets() * ways()) {
    }

    std::size_t ways() const noexcept {
        return policy_.ways();
    }

    std::size_t sets() const noexcept {
        return 1 << policy_.index_bits();
    }

    const void* lookup(std::uintptr_t aligned_address) const noexcept {
        std::size_t set_index = get_set_index(aligned_address);
        std::span<const cache_line> set {
            &data_[set_index * ways()],
            ways()
        };

        // Find matching line in set
        auto tag = aligned_address;
        auto it = std::find_if(set.begin(), set.end(), [tag](auto& l){
            return l.valid && l.tag == tag;
        });
        auto way_index = std::distance(set.begin(), it);

        if(way_index == ways()) {
            return nullptr;
        }

        return &set[way_index];
    }

    void hit(const void* handle) noexcept {
        assert(handle);

        auto& line = *static_cast<const cache_line*>(handle);
        assert(line.valid);
        auto [set, way] = get_cache_line_set_way(line);
        policy_.register_hit(set, way);
    }

    void invalidate(const void* handle) noexcept {
        assert(handle);
        auto& line = *static_cast<const cache_line*>(handle);
        assert(line.valid);
        auto [set, way] = get_cache_line_set_way(line);
        get_cache_line_at_set_way(set, way).valid = false;
    }

    std::optional<std::uintptr_t> fill(std::uintptr_t aligned_address) noexcept {
        auto handle = lookup(aligned_address);
        std::optional<std::uintptr_t> victim = std::nullopt;

        if(!handle) {
            auto set_index = get_set_index(aligned_address);
            std::span<cache_line> set {
                &data_[set_index * ways()],
                ways()
            };

            auto victim_way = policy_.register_miss(set_index);
            if (set[victim_way].valid) {
                victim = set[victim_way].tag;
            }
            set[victim_way].valid = true;
            set[victim_way].tag = aligned_address;
        }

        return victim;
    }
};
static_assert(cache<set_associative_cache>);

}

#endif /* CACHEHOUND_SIM_CACHE_SET_ASSOCIATIVE_CACHE_HPP */
