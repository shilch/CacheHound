#ifndef CACHEHOUND_ADAPTERS_DETAIL_BYPASS_ADAPTER_BASE_HPP
#define CACHEHOUND_ADAPTERS_DETAIL_BYPASS_ADAPTER_BASE_HPP

#include "../../concepts/memory.hpp"
#include "../../concepts/stats_memory.hpp"

namespace cachehound::detail {

template<memory Memory, typename = void>
struct bypass_adapter_base {
};

template<memory Memory>
struct bypass_adapter_base<Memory, std::enable_if_t<stats_memory<Memory>, void>> {
    using stats_type  = typename Memory::stats_type;
};

}

#endif
