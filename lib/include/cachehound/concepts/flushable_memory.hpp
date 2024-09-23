#ifndef CACHEHOUND_CONCEPTS_FLUSHABLE_MEMORY_HPP
#define CACHEHOUND_CONCEPTS_FLUSHABLE_MEMORY_HPP

#include <concepts>

#include "./memory.hpp"

namespace cachehound {

template<typename M>
concept flushable_memory = memory<M> and requires(M& memory) {
    { memory.flush() } noexcept -> std::same_as<void>;
};

}

#endif /* CACHEHOUND_CONCEPTS_FLUSHABLE_MEMORY_HPP */
