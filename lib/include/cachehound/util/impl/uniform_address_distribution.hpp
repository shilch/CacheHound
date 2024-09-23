#ifndef CACHEHOUND_UTIL_IMPL_UNIFORM_ADDRESS_DISTRIBUTION_HPP
#define CACHEHOUND_UTIL_IMPL_UNIFORM_ADDRESS_DISTRIBUTION_HPP

#include "../uniform_address_distribution.hpp"
#include <cstdint>

cachehound::uniform_address_distribution::distributions cachehound::uniform_address_distribution::derive_distributions(const memory auto& memory, std::uint8_t alignment)
{
    std::vector<std::size_t> sizes;
    std::vector<std::uniform_int_distribution<std::uintptr_t>> addresses;
    for (auto& region : memory.regions()) {
        sizes.emplace_back(region.size());
        addresses.emplace_back(region.base() >> alignment, ((region.base() + region.size()) >> alignment) - 1);
    }

    std::discrete_distribution<std::uintptr_t> regions(sizes.begin(), sizes.end());
    return cachehound::uniform_address_distribution::distributions {
        .regions = std::move(regions),
        .addresses = std::move(addresses)
    };
}

cachehound::uniform_address_distribution::uniform_address_distribution(const memory auto& memory, std::uint8_t alignment, std::uintptr_t offset)
    : alignment_(alignment)
    , distributions_(derive_distributions(memory, alignment))
    , offset_(offset)
{
    assert(offset < (1 << alignment_));
}

cachehound::uniform_address_distribution::uniform_address_distribution(const memory auto& memory)
    : uniform_address_distribution(memory, memory.offset_bits())
{
}

std::uintptr_t cachehound::uniform_address_distribution::operator()(std::uniform_random_bit_generator auto& g) noexcept(noexcept(distributions_.regions(g)) && noexcept(distributions_.addresses.front()(g)))
{
    auto region = distributions_.regions(g);
    return (distributions_.addresses[region](g) << alignment_) + offset_;
}

#endif /* CACHEHOUND_UTIL_IMPL_UNIFORM_ADDRESS_DISTRIBUTION_HPP */
