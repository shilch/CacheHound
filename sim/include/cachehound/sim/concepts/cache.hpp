#ifndef CACHEHOUND_SIM_CONCEPTS_CACHE_HPP
#define CACHEHOUND_SIM_CONCEPTS_CACHE_HPP

#include <concepts>
#include <optional>

#include "./cache_handle.hpp"

namespace cachehound::sim {

template<typename Cache>
concept cache = requires(Cache c, const Cache& const_c, std::uintptr_t aligned_address, const void* handle) {
    { const_c.lookup(aligned_address) } noexcept -> std::convertible_to<const void*>;
    { c.hit(handle) } noexcept -> std::same_as<void>;
    { c.invalidate(handle) } noexcept -> std::same_as<void>;
    { c.fill(aligned_address) } noexcept -> std::same_as<std::optional<std::uintptr_t>>;
};

}

#endif /* CACHEHOUND_SIM_CONCEPTS_CACHE_HPP */
