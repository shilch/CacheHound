#ifndef CACHEHOUND_SIM_BACKENDS_SIMULATED_MEMORY_HPP
#define CACHEHOUND_SIM_BACKENDS_SIMULATED_MEMORY_HPP

#include <cachehound/cachehound.hpp>
#include <bit>
#include <type_traits>

#include "../concepts/cache.hpp"
#include "../util/any_cache.hpp"
#include "../cache/set_associative_cache.hpp"

namespace cachehound::sim {

enum class inclusiveness {
    non_inclusive = 0,
    inclusive = 1,
    exclusive = 2
};

// TODO: Verify cache hierarchy logic.
// - How does the L2 replacement policy work when L2 is exclusive?
template<cache CommonCache = set_associative_cache>
class simulated_memory {
public:
    using region_type = basic_memory_region;

private:
    std::vector<region_type> regions_;
    std::uint8_t offset_bits_;
    std::vector<CommonCache> caches_;
    // std::array<inclusiveness, levels> inclusiveness_;

    inclusiveness get_inclusiveness(unsigned level) noexcept {
        // TODO: Need to figure out a nice way to parameterize the inclusion policy.
        return inclusiveness::non_inclusive;
    }

    unsigned do_access(std::uintptr_t address, unsigned level = 0) noexcept {
        if (level < levels()) {
            // Non-main-memory access
            auto& cache = caches_[level];
            auto incl = get_inclusiveness(level);

            // Lookup in this cache
            auto handle = cache.lookup(address);
            if(!handle) {
                // Continue search in next level
                return do_access(address, level + 1);
            }

            // Register hit
            cache.hit(handle);

            if (level > 0 && incl == inclusiveness::exclusive) {
                // invalidate this copy
                cache.invalidate(handle);
            }
        }

        if (level > 0) {
            // Propagate entry to lower level caches
            back_fill(address, level - 1);
        }

        return level;
    }

    void back_fill(std::uintptr_t address, unsigned level) noexcept {
        assert(level < levels());

        auto& cache = caches_[level];
        auto incl = get_inclusiveness(level);

        if (incl != inclusiveness::exclusive) {
            fill(address, level, true);
        }

        if (level > 0) {
            back_fill(address, level - 1);
        }
    }

    void fill(std::uintptr_t address, unsigned level, bool should_back_invalidate) noexcept {
        auto& cache = caches_[level];
        auto incl = get_inclusiveness(level);

        auto victim = cache.fill(address);
        if (victim) {
            if (should_back_invalidate && level > 0 && incl == inclusiveness::inclusive) {
                back_invalidate(*victim, level - 1);
            }
            if (level + 1 < caches_.size()) {
                fill(*victim, level + 1, false);
            }
        }
    }

    void back_invalidate(std::uintptr_t address, unsigned level) noexcept {
        assert(level < levels());

        auto& cache = caches_[level];
        auto incl = get_inclusiveness(level);

        auto handle = cache.lookup(address);
        if(handle) {
            cache.invalidate(handle);
        }

        if(level > 0) {
            back_invalidate(address, level - 1);
        }
    }

public:
    template<typename... Caches>
    simulated_memory(std::uintptr_t base, std::size_t size, std::uint8_t offset_bits, Caches&&... caches)
        : regions_({region_type{base, size}})
        , offset_bits_(offset_bits)
        , caches_{std::forward<Caches>(caches)...} {}

    unsigned levels() const noexcept {
        return caches_.size();
    }

    std::uint8_t offset_bits() const noexcept {
        return offset_bits_;
    }

    std::uint8_t index_bits(unsigned level) const noexcept
        requires std::is_base_of_v<set_associative_cache, CommonCache>
    {
        return std::countr_zero(caches_[level].sets());
    }

    std::size_t ways(unsigned level) const noexcept
        requires std::is_base_of_v<set_associative_cache, CommonCache>
    {
        return caches_[level].ways();
    }

    void access(std::uintptr_t address) noexcept {
        do_access(address);
    }

    unsigned instrumented_access(std::uintptr_t address) noexcept {
        return do_access(address);
    }

    const std::vector<region_type>& regions() const noexcept {
        return regions_;
    }
};
static_assert(instrumented_memory<simulated_memory<>>);

}

#endif /* CACHEHOUND_SIM_BACKENDS_SIMULATED_MEMORY_HPP */
