#ifndef CACHEHOUND_CLI_COMMANDS_IMPL_REVERSE_PLACEMENT_COMMAND_HPP
#define CACHEHOUND_CLI_COMMANDS_IMPL_REVERSE_PLACEMENT_COMMAND_HPP

#include "../reverse_placement_command.hpp"

#include <cachehound/cachehound.hpp>
#include <random>
#include <ranges>
#include <spdlog/spdlog.h>

#include "../../placement/affine_placement_reverser.hpp"

int cachehound::cli::reverse_placement_command::reverse_placement_policy(eviction_strategy auto& strategy) const {
    spdlog::debug("Initializing random device");
    std::random_device rnd;
    auto seed = rnd();
    std::mt19937 mtwister {seed};
    std::unordered_map<std::uintptr_t, std::size_t> address_set;

    uniform_address_distribution uad{strategy.memory()};

    spdlog::debug("Assuming the hash function can be modelled as an affine transformation");

    affine_placement_reverser reverser{strategy.memory().offset_bits(), strategy.memory().index_bits(0)};

    while(reverser.reversed_bits() == 0 || (reverser.reversed_bits() < target_reversed_bits_ && address_set.size() < target_mappings_)) {
        std::uintptr_t target = uad(mtwister);
        auto set = locate_set(strategy, target);

        if(!set.has_value()) {
            spdlog::error("Failed to map {:#x} to set", target);
        } else if(!address_set.contains(target)) {
            address_set[target] = *set;
            spdlog::trace("Found new mapping: {:#x} -> {} (Mapping {}/{})", target, *set, address_set.size(), target_mappings_);

            auto fed = reverser.feed_mapping(target, *set);
            if(fed) {
                spdlog::debug("Uncovered {} bits thus far", reverser.reversed_bits());
            }
        }
    }

    spdlog::info("Uncovered {} bits for reversal of index function", reverser.reversed_bits());
    spdlog::trace("Recovering affine transformation");
    auto placement_policy = reverser.reverse();
    spdlog::info("Transformation matrix: {}", placement_policy.matrix_string());
    spdlog::info("Translation vector: {}", placement_policy.vector_string());
    spdlog::info("Index function: {}", placement_policy.to_string());

    if(address_set.size() < target_mappings_) {
        spdlog::debug("Collecting more mappings");
        do {
            std::uintptr_t target = uad(mtwister);
            auto set = locate_set(strategy, target);
            if(!set.has_value()) {
                spdlog::error("Failed to map {:#x} to set", target);
            } else {
                address_set[target] = *set;
            }
        } while(address_set.size() < target_mappings_);
    }

    spdlog::debug("Verifying index function");
    std::size_t matches = 0;
    for(auto [address, set] : address_set) {
        matches += placement_policy(address) == set;
    }
    spdlog::info("Placement policy confidence: {}/{}", matches, address_set.size());

    if(matches <= (target_mappings_ / 5 * 4)) {
        // Confidence <=80%
        spdlog::warn("The confidence for this placement policy is low");
        spdlog::warn("Consider increasing the memory size and/or adjusting the indexing method (virtual/physical)");
    }

    return 0;
}

int cachehound::cli::reverse_placement_command::main(instrumented_memory auto &memory) const {
    return detail::eviction_strategy_command_base::provide_eviction_strategy(memory, [this](auto& strategy){
        return reverse_placement_policy(strategy);
    });
}

#endif /* CACHEHOUND_CLI_COMMANDS_IMPL_REVERSE_PLACEMENT_COMMAND_HPP */
