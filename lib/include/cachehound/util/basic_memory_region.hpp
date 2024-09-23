#ifndef CACHEHOUND_UTIL_BASIC_MEMORY_REGION_HPP
#define CACHEHOUND_UTIL_BASIC_MEMORY_REGION_HPP

#include <cstdint>
#include <cstddef>

#include "../concepts/memory_region.hpp"

namespace cachehound {

class basic_memory_region {
    std::uintptr_t base_;
    std::size_t size_;

public:
    basic_memory_region(std::uintptr_t base, std::size_t size)
        : base_(base), size_(size) {}

    constexpr std::uintptr_t base() const noexcept {
        return base_;
    }

    constexpr std::size_t size() const noexcept {
        return size_;
    }
};
static_assert(memory_region<basic_memory_region>);

}

#endif /* CACHEHOUND_UTIL_BASIC_MEMORY_REGION_HPP */
