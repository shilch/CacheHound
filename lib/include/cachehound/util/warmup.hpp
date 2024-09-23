#ifndef CACHEHOUND_UTIL_WARMUP_HPP
#define CACHEHOUND_UTIL_WARMUP_HPP

#include <random>

#include "../concepts/memory.hpp"
#include "./uniform_address_distribution.hpp"

namespace cachehound {

void warmup(memory auto& memory, std::size_t accesses, std::uniform_random_bit_generator auto& gen) {
    uniform_address_distribution distribution(memory, 0);

    while(accesses-- > 0) {
        auto address = distribution(gen);
        memory.access(address);
    }
}

}

#endif /* CACHEHOUND_UTIL_WARMUP_HPP */
