#ifndef CACHEHOUND_CONCEPTS_RESETTABLE_MEMORY_HPP
#define CACHEHOUND_CONCEPTS_RESETTABLE_MEMORY_HPP

#include <concepts>

#include "./memory.hpp"

namespace cachehound {

template<typename M>
concept resettable_memory = memory<M> and requires(M& memory) {
    { memory.reset() } -> std::same_as<void>;
};

}

#endif /* CACHEHOUND_CONCEPTS_RESETTABLE_MEMORY_HPP */
