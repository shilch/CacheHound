#ifndef CACHEHOUND_CONCEPTS_X86_MEMORY_HPP
#define CACHEHOUND_CONCEPTS_X86_MEMORY_HPP

#include <cstdint>
#include <concepts>

#include "./memory.hpp"

namespace cachehound {

template<typename M>
concept x86_memory = memory<M> and requires(M& memory, std::uintptr_t address) {
    { memory.clflush(address) } -> std::same_as<void>;
    { memory.wbinvd() } -> std::same_as<void>;
};

}

#endif /* CACHEHOUND_CONCEPTS_X86_MEMORY_HPP */
