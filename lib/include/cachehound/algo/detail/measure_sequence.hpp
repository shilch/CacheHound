#ifndef CACHEHOUND_ALGO_DETAIL_MEASURE_SEQUENCE_HPP
#define CACHEHOUND_ALGO_DETAIL_MEASURE_SEQUENCE_HPP

namespace cachehound::detail {

template<bool Safe>
std::optional<std::size_t> measure_sequence(
    instrumented_memory auto& memory,
    sequence auto&& sequence,
    address_range auto const& addresses
) {
    std::vector<bool> seen;
    std::size_t hit_counter = 0;

    for(auto addr : std::forward<decltype(sequence)>(sequence)) {
        if (addr >= seen.size()) {
            seen.resize(1 + addr);
        }

        auto actual_addr = *(std::ranges::begin(addresses) + (addr));

        if(Safe || seen[addr]) {
            if(!seen[addr]){
                unsigned level = memory.instrumented_access(actual_addr);
                if(level == 0) {
                    return std::nullopt;
                }
                seen[addr] = true;
            } else {
                hit_counter += memory.instrumented_access(actual_addr) == 0;
            }
        } else {
            memory.access(actual_addr);
            seen[addr] = true;
        }
    }

    return hit_counter;
}



}

#endif