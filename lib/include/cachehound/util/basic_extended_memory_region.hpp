#ifndef CACHEHOUND_UTIL_BASIC_EXTENDED_MEMORY_REGION_HPP
#define CACHEHOUND_UTIL_BASIC_EXTENDED_MEMORY_REGION_HPP

#include <cstdint>
#include <cstddef>

#include "../concepts/extended_memory_region.hpp"
#include "./basic_memory_region.hpp"

namespace cachehound {

class basic_extended_memory_region : public basic_memory_region {
    std::uintptr_t physical_base_;

public:
    basic_extended_memory_region(std::uintptr_t virtual_base, std::uintptr_t physical_base, std::size_t size)
        : basic_memory_region(virtual_base, size)
        , physical_base_(physical_base) {}

    constexpr std::uintptr_t physical_base() const noexcept {
        return physical_base_;
    }
};
static_assert(extended_memory_region<basic_extended_memory_region>);

}


#endif /* CACHEHOUND_UTIL_BASIC_EXTENDED_MEMORY_REGION_HPP */
