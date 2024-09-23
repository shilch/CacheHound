#ifndef CACHEHOUND_CLI_COMMANDS_IMPL_CYCLE_TEST_COMMAND_IPP
#define CACHEHOUND_CLI_COMMANDS_IMPL_CYCLE_TEST_COMMAND_IPP

#include "../cycle_test_command.hpp"

#include <cachehound/cachehound.hpp>

#include <random>
#include <spdlog/spdlog.h>

int cachehound::cli::cycle_test_command::main() const
{
    std::random_device rand;

    for (std::size_t associativity = 2; associativity <= 16; associativity *= 2) {
        spdlog::info("Associativity = {}", associativity);

        spdlog::debug("Testing for initially cyclic eviction");
        {
            auto policies = get_replacement_policies(associativity);
            for (auto& policy_name : policies) {
                auto policy = make_replacement_policy(policy_name, associativity);
                assert(policy.has_value());
                bool initially_cyclic = check_cyclic_eviction(policy.value());
                spdlog::info("Is {} initially cyclically evicting: {}", policy_name, initially_cyclic);
            }
        }

        spdlog::debug("Testing for cyclic eviction");
        {
            auto policies = get_replacement_policies(associativity);
            for (auto& policy_name : policies) {
                auto policy = make_replacement_policy(policy_name, associativity);

                bool cyclic = true;
                for (std::size_t attempt = 0; cyclic && attempt < 1000; attempt++) {
                    auto seq = generate_sequence(30, 30, rand);
                    simulate_sequence(seq, policy.value());
                    cyclic = cyclic && check_cyclic_eviction(policy.value());
                }

                spdlog::info("Is {} generally cyclically evicting: {}", policy_name, cyclic);
            }
        }
    }

    return 0;
}

#endif /* CACHEHOUND_CLI_COMMANDS_IMPL_CYCLE_TEST_COMMAND_IPP */
