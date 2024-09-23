#ifndef CACHEHOUND_CLI_COMMANDS_IMPL_AFFINE_TEST_COMMAND_IPP
#define CACHEHOUND_CLI_COMMANDS_IMPL_AFFINE_TEST_COMMAND_IPP

#include "../affine_test_command.hpp"

#include <cachehound/cachehound.hpp>
#include "../../util/a64fx_placement.hpp"
#include "../../placement/affine_placement_reverser.hpp"

#include <random>
#include <spdlog/spdlog.h>

int cachehound::cli::affine_test_command::main() const
{
    std::random_device rand;
    std::uniform_int_distribution<std::uintptr_t> uid;

    auto given_placement_policy = make_a64fx_placement_policy();

    affine_placement_reverser reverser{8, 11};

    while(reverser.reversed_bits() < 40) {
        auto addr = uid(rand);
        auto set = given_placement_policy(addr);
        reverser.feed_mapping(addr & ~((std::uintptr_t{1} << 8) - 1), set);
    }

    auto reversed_placement_policy = reverser.reverse();

    spdlog::info("Found index function: {}", reversed_placement_policy.to_string());

    return 0;
}

#endif /* CACHEHOUND_CLI_COMMANDS_IMPL_AFFINE_TEST_COMMAND_IPP */
