#ifndef CACHEHOUND_CLI_COMMANDS_IMPL_REVERSE_REPLACEMENT_COMMAND_HPP
#define CACHEHOUND_CLI_COMMANDS_IMPL_REVERSE_REPLACEMENT_COMMAND_HPP

#include "../reverse_replacement_command.hpp"

#include <cachehound/cachehound.hpp>
#include <spdlog/spdlog.h>
#include <ranges>

auto majority_element(auto&& range) -> std::ranges::range_value_t<decltype(range)> {
    auto it = range.begin();
    assert(it != range.end());

    std::ranges::range_value_t<decltype(range)> maj = *it;
    std::size_t count = 1;

    while(it != range.end()) {
        if(*it == maj) {
            ++count;
        } else if(count == 1) {
            maj = *it;
        } else {
            --count;
        }

        ++it;
    }
    return maj;
}

template<cachehound::instrumented_memory Memory>
int cachehound::cli::reverse_replacement_command::main(Memory& memory) const {
    spdlog::debug("Initializing random device");
    std::random_device rnd;
    auto seed = rnd();
    std::mt19937 mtwister {seed};

    uniform_address_distribution uad{memory};
    modular_placement_policy placement{memory.offset_bits(), std::size_t{1} << memory.index_bits(0)};

    std::vector<std::uintptr_t> addresses;
    std::unordered_set<std::uintptr_t> blacklist;
    while(addresses.size() < 50) {
        auto address = uad(rnd);
        auto set = placement(address);
        if(set != index_) continue;
        if(blacklist.contains(address)) continue;
        addresses.emplace_back(address);
        blacklist.emplace(address);
    }

    const std::size_t ways = memory.ways(0);
    std::size_t rounds = 0;

    auto candidates = get_replacement_policies(ways);
    std::size_t local_pollution_fail_counter = 0;

    while(candidates.size() > 1 || rounds < minimum_rounds_) {
        auto seq = generate_sequence(length_, std::min(2 * ways, addresses.size()), mtwister);
        spdlog::debug("Sequence: {}", fmt::join(seq, ", "));

        std::vector<int> measurements{};
        while(measurements.size() < repetitions_) {
            if constexpr(resettable_memory<decltype(memory)>) {
                memory.reset();
            } else if constexpr(x86_memory<decltype(memory)>) {
                memory.wbinvd();
            }

            // Cache pollution
            for(std::size_t i = 0; i < pollution_;) {
                auto address = uad(rnd);
                if(blacklist.contains(address)) continue;
                memory.access(address);
                i++;
            }

            auto measured_hit_counter = safe_measure_sequence(memory, seq, addresses);
            if(!measured_hit_counter.has_value()) {
                spdlog::trace("Repeating measure_sequence due to insufficient cache pollution ({} attempts thus far)", local_pollution_fail_counter++);
                continue;
            }
            local_pollution_fail_counter = 0;
            measurements.emplace_back(*measured_hit_counter);
        }
        spdlog::debug("Measured hit counters: {}", fmt::join(measurements, ", "));
        
        auto majority_hit_counter = majority_element(measurements);
        spdlog::debug("Majority element: {}", majority_hit_counter);

        auto it = filter_replacement_policies(candidates.begin(), candidates.end(), seq, majority_hit_counter, ways);
        spdlog::info("Eliminated {} replacement policy candidates", std::distance(it, candidates.end()));
        candidates.erase(it, candidates.end());
        rounds++;
    }

    if(candidates.empty()) {
        spdlog::error("Failed to find replacement policy, might be non-standard");
    } else {
        spdlog::info("Found replacement policy: {}", candidates.front());
    }

    return 0;
}

#endif /* CACHEHOUND_CLI_COMMANDS_IMPL_REVERSE_REPLACEMENT_COMMAND_HPP */
