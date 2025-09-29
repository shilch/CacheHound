#ifndef CACHEHOUND_CLI_COMMANDS_REVERSE_HIST_COMMAND_HPP
#define CACHEHOUND_CLI_COMMANDS_REVERSE_HIST_COMMAND_HPP

#include "./detail/command_base.hpp"

#include <argparse/argparse.hpp>
#include <cachehound/cachehound.hpp>

namespace cachehound::cli {

class reverse_hist_command : public command_base<reverse_hist_command> {

    std::size_t accesses_;

public:
    static constexpr const char* name = "hist";

    int main(instrumented_memory auto& memory) const;

    void setup_arguments(argparse::ArgumentParser& args) {
        args.add_argument("--accesses")
            .help("Number of accesses to perform")
            .default_value(std::size_t{1'000'000})
            .scan<'d', std::size_t>();
    }

    void parse_arguments(argparse::ArgumentParser& args) {
        accesses_ = args.get<std::size_t>("--accesses");
    }
};

}

#include "./impl/reverse_hist_command.hpp"

#endif /* CACHEHOUND_CLI_COMMANDS_REVERSE_HIST_COMMAND_HPP */