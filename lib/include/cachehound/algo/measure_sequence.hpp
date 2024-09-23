#ifndef CACHEHOUND_ALGO_MEASURE_SEQUENCE_HPP
#define CACHEHOUND_ALGO_MEASURE_SEQUENCE_HPP

#include <unordered_set>

#include "../concepts/address_range.hpp"
#include "../concepts/instrumented_memory.hpp"
#include "../concepts/resettable_memory.hpp"
#include "../concepts/sequence.hpp"
#include "../concepts/x86_memory.hpp"

#include "./detail/measure_sequence.hpp"

namespace cachehound {

std::optional<std::size_t> safe_measure_sequence(
    instrumented_memory auto& memory,
    sequence auto&& sequence,
    address_range auto const& addresses
) {
    return detail::measure_sequence<true>(
        memory, sequence, addresses
    );
}

std::size_t unsafe_measure_sequence(
    instrumented_memory auto& memory,
    sequence auto&& sequence,
    address_range auto const& addresses
) {
    return *detail::measure_sequence<false>(
        memory, sequence, addresses
    );
}

}

#endif /* CACHEHOUND_ALGO_MEASURE_SEQUENCE_HPP */
