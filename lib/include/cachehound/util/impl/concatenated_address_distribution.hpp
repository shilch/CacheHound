#ifndef CACHEHOUND_UTIL_IMPL_CONCATENATED_ADDRESS_DISTRIBUTION_HPP
#define CACHEHOUND_UTIL_IMPL_CONCATENATED_ADDRESS_DISTRIBUTION_HPP

#include "../concatenated_address_distribution.hpp"

#include "cachehound/concepts/address_distribution.hpp"

#include <utility>

template<cachehound::address_distribution D1, cachehound::address_distribution D2>
cachehound::concatenated_address_distribution<D1, D2>::concatenated_address_distribution(D1&& d1, std::size_t draws, D2&& d2)
    : draws_(draws)
    , current_draws_(0)
    , d1_(std::forward<D1>(d1))
    , d2_(std::forward<D2>(d2)) {}

template<cachehound::address_distribution D1, cachehound::address_distribution D2>
template<std::uniform_random_bit_generator URBG>
std::uintptr_t cachehound::concatenated_address_distribution<D1, D2>::operator()(URBG& urbg) {
    if(current_draws_ == draws_) return d2_(urbg);
    ++current_draws_;
    return d1_(urbg);
}

template<cachehound::address_distribution D1, cachehound::address_distribution D2>
void cachehound::concatenated_address_distribution<D1, D2>::reset() {
    if(current_draws_ == draws_) {
        d2_.reset();
    }
    current_draws_ = 0;
    d1_.reset();
}

#endif /* CACHEHOUND_UTIL_IMPL_CONCATENATED_ADDRESS_DISTRIBUTION_HPP */
