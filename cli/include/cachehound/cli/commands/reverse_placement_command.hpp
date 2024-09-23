#ifndef CACHEHOUND_CLI_COMMANDS_REVERSE_PLACEMENT_COMMAND_HPP
#define CACHEHOUND_CLI_COMMANDS_REVERSE_PLACEMENT_COMMAND_HPP

#include "./detail/command_base.hpp"
#include "./detail/eviction_strategy_command_base.hpp"

#include <argparse/argparse.hpp>
#include <cachehound/cachehound.hpp>

namespace cachehound::cli {

class reverse_placement_command
    : public command_base<reverse_placement_command>
    , public detail::eviction_strategy_command_base {

    std::size_t target_reversed_bits_ = 8 * sizeof(std::uintptr_t);
    std::size_t target_mappings_;

    int reverse_placement_policy(eviction_strategy auto&) const;

public:
    static constexpr const char* name = "placement";

    int main(instrumented_memory auto& memory) const;

    void setup_arguments(argparse::ArgumentParser& args) {
        detail::eviction_strategy_command_base::setup_arguments(args);

        args.add_argument("--mappings", "-M")
            .help("Number of address->set mappings to generate for reversal")
            .default_value(std::size_t{1000})
            .scan<'d', std::size_t>();
    }

    void parse_arguments(argparse::ArgumentParser& args) {
        detail::eviction_strategy_command_base::parse_arguments(args);

        target_mappings_ = args.get<std::size_t>("--mappings");
    }
};

}

#include "./impl/reverse_placement_command.hpp"

#endif /* CACHEHOUND_CLI_COMMANDS_REVERSE_PLACEMENT_COMMAND_HPP */
