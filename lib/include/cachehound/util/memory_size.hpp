#ifndef CACHEHOUND_UTIL_MEMORY_SIZE_HPP
#define CACHEHOUND_UTIL_MEMORY_SIZE_HPP

#include <cstddef>

#include "../concepts/memory.hpp"

namespace cachehound {

template<memory M>
std::size_t memory_size(const M& m) {
    std::size_t total_size = 0;
    for(auto& region : m.regions()) {
        total_size += region.size();
    }
    return total_size;
}

}

#endif /* CACHEHOUND_UTIL_MEMORY_SIZE_HPP */
