#ifndef CACHEHOUND_CONCEPTS_SWITCH_CHANNEL_MEMORY_HPP
#define CACHEHOUND_CONCEPTS_SWITCH_CHANNEL_MEMORY_HPP

#include <concepts>

#include "./memory.hpp"

namespace cachehound {

template<typename M>
concept switch_channel_memory = memory<M> and requires(M& memory) {
    { memory.switch_channel() } -> std::same_as<void>;
};

}

#endif /* CACHEHOUND_CONCEPTS_SWITCH_CHANNEL_MEMORY_HPP */
