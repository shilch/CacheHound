#ifndef CACHEHOUND_CLI_COMMANDS_ROOT_COMMAND_HPP
#define CACHEHOUND_CLI_COMMANDS_ROOT_COMMAND_HPP

#include "./detail/command.hpp"
#include "./detail/multicommand_base.hpp"

#include "./affine_test_command.hpp"
#include "./cycle_test_command.hpp"
#include "./reverse_command.hpp"

#include <argparse/argparse.hpp>
#include <spdlog/spdlog.h>

namespace cachehound::cli {

class root_command : public multicommand_base<root_command, cycle_test_command, affine_test_command, reverse_command> {
public:
    static constexpr const char* name    = "cachehound";
    static constexpr const char* version = "unknown";

private:
    int verbosity_ = 0;

public:
    void setup_arguments(argparse::ArgumentParser& args) {
        multicommand_base::setup_arguments(args);

        args.add_description("Automated reverse-engineering of CPU cache policies");

        args.add_argument("-V", "--verbose")
            .help("Each additional -V increases the verbosity")
            .action([&](const auto &) { ++verbosity_; })
            .append()
            .default_value(false)
            .implicit_value(true)
            .nargs(0);
    }

    void parse_arguments(argparse::ArgumentParser& args) {
        multicommand_base::parse_arguments(args);

        switch(verbosity_) {
            case 0:
                spdlog::set_level(spdlog::level::info);
                break;
            case 1:
                spdlog::set_level(spdlog::level::debug);
                break;
            default:
                spdlog::set_level(spdlog::level::trace);
                break;
        }
    }
};
static_assert(command<root_command>);

}

#endif /* CACHEHOUND_CLI_COMMANDS_ROOT_COMMAND_HPP */
