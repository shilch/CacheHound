#ifndef CACHEHOUND_CONCEPTS_STATS_MEMORY_HPP
#define CACHEHOUND_CONCEPTS_STATS_MEMORY_HPP

#include <concepts>
#include <type_traits>

#include "./memory.hpp"

namespace cachehound {

template<typename M>
concept stats_memory = memory<M> and requires(const M const_memory) {
    { const_memory.stats() } noexcept -> std::same_as<typename std::remove_cvref_t<M>::stats_type>;
};

}

#endif /* CACHEHOUND_CONCEPTS_STATS_MEMORY_HPP */
