#ifndef CACHEHOUND_CONCEPTS_TIMED_MEMORY_HPP
#define CACHEHOUND_CONCEPTS_TIMED_MEMORY_HPP

#include <cstdint>
#include <concepts>

#include "./memory.hpp"

namespace cachehound {

template<typename M>
concept timed_memory = memory<M>
    and requires(M& memory, std::uintptr_t address) {
    { memory.timed_access(address) } -> std::same_as<std::uint64_t>;
};

}

#endif /* CACHEHOUND_CONCEPTS_TIMED_MEMORY_HPP */
