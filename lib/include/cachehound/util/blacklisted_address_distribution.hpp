#ifndef CACHEHOUND_UTIL_BLACKLISTED_ADDRESS_DISTRIBUTION_HPP
#define CACHEHOUND_UTIL_BLACKLISTED_ADDRESS_DISTRIBUTION_HPP

#include <cstdint>
#include <unordered_set>

#include "../concepts/address_distribution.hpp"
#include "../concepts/address_range.hpp"

namespace cachehound {

template<address_distribution BaseDistribution>
class blacklisted_address_distribution {
    BaseDistribution distribution_;
    std::unordered_set<std::uintptr_t> blacklist_;

public:
    blacklisted_address_distribution(auto&&... args)
        : distribution_(std::forward<decltype(args)>(args)...) {}

    void blacklist(std::uintptr_t address) {
        blacklist_.emplace(address);
    }

    void blacklist(address_range auto&& addresses) {
        for(std::uintptr_t address : std::forward<decltype(addresses)>(addresses)) {
            blacklist(address);
        }
    }

    std::uintptr_t operator()(auto&& urbg) {
        std::uintptr_t address;
        do {
            address = distribution_(std::forward<decltype(urbg)>(urbg));
        } while(blacklist_.contains(address));
        return address;
    }

    void reset() {
        distribution_.reset();
    }
};

}

#endif /* CACHEHOUND_UTIL_BLACKLISTED_ADDRESS_DISTRIBUTION_HPP */
