#ifndef CACHEHOUND_UTIL_SEQUENCE_HPP
#define CACHEHOUND_UTIL_SEQUENCE_HPP

#include "../algo/simulate_sequence.hpp"
#include "./replacement_policy.hpp"

namespace cachehound {

std::vector<std::size_t> generate_sequence(
    std::size_t n,
    std::size_t max,
    std::uniform_random_bit_generator auto& urbg
) {
    assert(max > 0);
    std::vector<std::size_t> out;

    std::uniform_int_distribution<std::size_t> dist{0, max - 1};
    while(n-- > 0) {
        out.emplace_back(dist(urbg));
    }
    return out;
}

std::size_t simulate_sequence(
    sequence auto&& sequence,
    std::string_view policy_name,
    std::size_t associativity
) {
    auto policy = make_replacement_policy(policy_name, associativity);
    if(!policy.has_value()) {
        throw std::invalid_argument{std::string{policy_name}};
    }
    return simulate_sequence(
        std::forward<decltype(sequence)>(sequence),
        policy.value()
    );
}

template<typename ForwardIt>
ForwardIt filter_replacement_policies(
    ForwardIt first, ForwardIt last,
    sequence auto&& sequence,
    std::size_t measured_hit_counter,
    std::size_t associativity
) {
    ForwardIt out = first;

    while(first != last) {
        std::string policy_name = *first;
        auto hit_counter = simulate_sequence(std::forward<decltype(sequence)>(sequence), policy_name, associativity);
        if(hit_counter == measured_hit_counter) {
            std::swap(*out, *first);
            ++out;
        }
        ++first;
    }

    return out;
}

}

#endif /* CACHEHOUND_UTIL_SEQUENCE_HPP */
