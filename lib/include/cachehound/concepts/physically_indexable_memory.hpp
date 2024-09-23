#ifndef CACHEHOUND_CONCEPTS_PHYSICALLY_INDEXABLE_MEMORY_HPP
#define CACHEHOUND_CONCEPTS_PHYSICALLY_INDEXABLE_MEMORY_HPP

#include "./memory.hpp"
#include  "./extended_memory_region_range.hpp"

namespace cachehound {

template<typename M>
concept physically_indexable_memory = memory<M> and requires(const M const_memory){
    { const_memory.regions() } noexcept -> extended_memory_region_range;
};

}

#endif /* CACHEHOUND_CONCEPTS_PHYSICALLY_INDEXABLE_MEMORY_HPP */
