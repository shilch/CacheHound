#ifndef CACHEHOUND_CLI_UTIL_A64FX_PLACEMENT_HPP
#define CACHEHOUND_CLI_UTIL_A64FX_PLACEMENT_HPP

#include "cachehound/policies/placement/generic_placement_policy.hpp"
#include <cachehound/cachehound.hpp>

namespace cachehound::cli {

inline generic_placement_policy make_a64fx_placement_policy() {
    // From the documentation:
    // index <10:0> = ((PA<36:34> xor PA<32:30> xor PA<31:29> xor PA<27:25> xor PA<23:21>) << 8) xor PA<18:8>
    return generic_placement_policy(1 << 10, [](std::uintptr_t a) -> std::size_t {
        return ((((a >> 34) & 0b111) ^ ((a >> 30) & 0b111) ^ ((a >> 29) & 0b111) ^ ((a >> 25) & 0b111) ^ ((a >> 21) & 0b111)) << 8) ^ ((a >> 8) & 0b11111111111);
    });
}

}

#endif /* CACHEHOUND_CLI_UTIL_A64FX_PLACEMENT_HPP */
