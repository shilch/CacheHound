#ifndef CACHEHOUND_UTIL_IMPL_UNIFORM_ADDRESS_DISTRIBUTION_IPP
#define CACHEHOUND_UTIL_IMPL_UNIFORM_ADDRESS_DISTRIBUTION_IPP

#include "../uniform_address_distribution.hpp"

void cachehound::uniform_address_distribution::reset() {
    for(auto& dist : distributions_.addresses) {
        dist.reset();
    }
    distributions_.regions.reset();
}

#endif /* CACHEHOUND_UTIL_IMPL_UNIFORM_ADDRESS_DISTRIBUTION_IPP */
