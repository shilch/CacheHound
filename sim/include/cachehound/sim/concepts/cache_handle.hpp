#ifndef CACHEHOUND_SIM_CONCEPTS_CACHE_HANDLE_HPP
#define CACHEHOUND_SIM_CONCEPTS_CACHE_HANDLE_HPP

#include <concepts>

namespace cachehound::sim {

template<typename H>
concept cache_handle = requires(const H& const_handle) {
    { const_handle } -> std::same_as<bool>;
    { const_handle.tag() } -> std::same_as<std::uintptr_t>;
};

}

#endif /* CACHEHOUND_SIM_CONCEPTS_CACHE_HANDLE_HPP */
