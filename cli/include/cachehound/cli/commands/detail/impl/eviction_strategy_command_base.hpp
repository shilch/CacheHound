#ifndef CACHEHOUND_CLI_COMMANDS_DETAIL_IMPL_EVICTION_STRATEGY_COMMAND_BASE_HPP
#define CACHEHOUND_CLI_COMMANDS_DETAIL_IMPL_EVICTION_STRATEGY_COMMAND_BASE_HPP

#include "../eviction_strategy_command_base.hpp"

#include "cachehound/algo/is_eviction_set.hpp"
#include "cachehound/algo/locate_eviction_set.hpp"
#include "cachehound/concepts/instrumented_memory.hpp"
#include "cachehound/util/blacklisted_address_distribution.hpp"
#include "cachehound/util/uniform_address_distribution.hpp"

#include <cachehound/cachehound.hpp>
#include <spdlog/spdlog.h>
#include <utility>

int cachehound::cli::detail::eviction_strategy_command_base::provide_eviction_set_strategy(instrumented_memory auto& memory, auto&& func) const {
    spdlog::info("Using eviction strategy based on eviction sets");

    spdlog::debug("Initializing random device");
    std::random_device rnd;
    auto seed = rnd();
    std::mt19937 mtwister {seed};

    spdlog::trace("Warming up memory");
    warmup(memory, 1'000'000, mtwister);
    spdlog::debug("Warmed up memory");

    return provide_is_eviction_set(memory, mtwister, [&](is_eviction_set auto&& is_eviction_set){
        set_cycling_address_distribution distribution{memory};

        std::vector<std::vector<std::uintptr_t>> evsets;
        std::unordered_set<std::uintptr_t> seen_addresses;

        std::size_t switch_channel_counter = 0;

        std::size_t sets = (1 << memory.index_bits(0))
                , ways = memory.ways(0);

        while(evsets.size() < sets) {
            if(++switch_channel_counter == 10) {
                if constexpr(switch_channel_memory<decltype(memory)>) {
                    spdlog::debug("Switching channel");
                    memory.switch_channel();
                    spdlog::info("Switched channels");
                }
                switch_channel_counter = 0;
            }

            std::uintptr_t target;

            target = distribution(mtwister);
            spdlog::trace("target = {:#x}", target);

            if(seen_addresses.contains(target)) {
                spdlog::trace("target already seen");
                continue;
            }

            auto it = locate_eviction_set(
                std::forward<decltype(is_eviction_set)>(is_eviction_set),
                memory, target, evsets
            );
            if(it != evsets.end()) {
                spdlog::trace("Eviction set for target already exists");
                seen_addresses.emplace(target);
                it->emplace_back(target);
                continue;
            }

            spdlog::debug("Finding eviction set for target address {:#x}", target);
            auto evset = find_eviction_set(std::forward<decltype(is_eviction_set)>(is_eviction_set), memory, target, mtwister);
            spdlog::info("Found eviction set of size {} for target address {:#x}", evset.size(), target);

            spdlog::debug("Reducing eviction set to size {}", ways);
            bool reduced = reduce_eviction_set(std::forward<decltype(is_eviction_set)>(is_eviction_set), memory, target, evset, ways);
            if (!reduced) {
                spdlog::error("Eviction set reduction failed");
                continue;
            }

            spdlog::info("Eviction set for target address {:#x} set minimized to size {}: {:#x}", target, evset.size(), fmt::join(evset, ", "));
            evset.emplace_back(target);
            for(auto addr : evset) seen_addresses.emplace(addr);
            evsets.emplace_back(std::move(evset));

            spdlog::info("Found {} distinct eviction sets thus far", evsets.size());
        }

        spdlog::debug("Reordering stored eviction sets");

        std::sort(evsets.begin(), evsets.end(), [&](auto& lhs, auto& rhs) {
            auto lhs_set = (lhs.front() >> memory.offset_bits()) % sets;
            auto rhs_set = (rhs.front() >> memory.offset_bits()) % sets;
            return lhs_set < rhs_set;
        });

        spdlog::info("Reordered eviction sets");

        eviction_set_strategy eviction_strategy{memory, std::move(evsets), std::forward<decltype(is_eviction_set)>(is_eviction_set)};
        // TODO: Drop used addresses from memory
        return std::forward<decltype(func)>(func)(eviction_strategy);
    });
}

int cachehound::cli::detail::eviction_strategy_command_base::provide_cisw_eviction_strategy(cachehound::armv8_memory auto& memory, auto&& func) const {
    spdlog::info("Using CISW-based eviction strategy");
    cisw_eviction_strategy eviction_strategy{memory};
    return std::forward<decltype(func)>(func)(eviction_strategy);
}


int cachehound::cli::detail::eviction_strategy_command_base::provide_is_eviction_set(
    instrumented_memory auto& memory,
    std::uniform_random_bit_generator auto& urbg,
    auto&& func
) const {
    uniform_address_distribution uniform_distribution{memory};
    blacklisted_address_distribution<uniform_address_distribution&> dist{uniform_distribution};

    std::size_t global_pollution_fail_counter = 0;
    auto is_eviction_set_single = [this, &global_pollution_fail_counter, &dist, &urbg](instrumented_memory auto& memory, std::uintptr_t target, address_range auto&& addresses){
        std::unordered_set<std::uintptr_t> blacklist(addresses.begin(), addresses.end());
        blacklist.emplace(target);

        std::optional<bool> result;
        std::size_t local_pollution_fail_counter = 0;
        do {
            if constexpr(resettable_memory<decltype(memory)>) {
                memory.reset();
            } else if constexpr(x86_memory<decltype(memory)>) {
                memory.wbinvd();
            }

            // Cache pollution
            for(std::size_t i = 0; i < is_eviction_set_pollution_;) {
                auto address = dist(urbg);
                if(blacklist.contains(address)) continue;
                memory.access(address);
                i++;
            }

            result = safe_is_eviction_set(memory, target, addresses);
            if(!result.has_value()) {
                local_pollution_fail_counter++;
                spdlog::trace("Repeating is_eviction_set due to insufficient cache pollution ({} attempts thus far)", local_pollution_fail_counter);

                if(local_pollution_fail_counter == local_pollution_fail_threshold) {
                    global_pollution_fail_counter++;
                    spdlog::trace("Encountered {} cases thus far with >={} repetitions due to insufficient cache pollution", global_pollution_fail_counter, local_pollution_fail_threshold);

                    if(global_pollution_fail_counter == global_pollution_fail_threshold) {
                        spdlog::warn("Cache pollution failed for at least {} times in {} distinct is_eviction_set calls", local_pollution_fail_threshold, global_pollution_fail_counter);
                        spdlog::warn("Consider increasing the number of cache pollution accesses");
                    }
                }
            }
        } while(!result.has_value());
        return result.value();
    };

    auto is_eviction_set = [this, &is_eviction_set_single](auto&&... args){
        bool result = true;
        for(std::size_t i = 0; i < is_eviction_set_repetitions_ && result; i++) {
            result = result && is_eviction_set_single(std::forward<decltype(args)>(args)...);
        }
        return result;
    };

    return std::forward<decltype(func)>(func)(is_eviction_set);
}

#endif /* CACHEHOUND_CLI_COMMANDS_DETAIL_IMPL_EVICTION_STRATEGY_COMMAND_BASE_HPP */
