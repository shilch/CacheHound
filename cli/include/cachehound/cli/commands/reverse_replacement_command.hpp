#ifndef CACHEHOUND_CLI_COMMANDS_REVERSE_REPLACEMENT_COMMAND_HPP
#define CACHEHOUND_CLI_COMMANDS_REVERSE_REPLACEMENT_COMMAND_HPP

#include "./detail/command_base.hpp"

#include <argparse/argparse.hpp>
#include <cachehound/concepts/instrumented_memory.hpp>

namespace cachehound::cli {

class reverse_replacement_command : public command_base<reverse_replacement_command> {
    int index_;
    std::size_t pollution_;
    std::size_t repetitions_;
    std::size_t length_;
    std::size_t minimum_rounds_;

public:
    static constexpr const char* name = "replacement";

    void setup_arguments(argparse::ArgumentParser& args) {
        args.add_description("Reversal of the replacement policy");

        args.add_argument("-i", "--index")
            .help("Select the set index for which to reverse the replacement policy")
            .default_value(0)
            .scan<'d', std::size_t>();
        args.add_argument("--pollution", "-P")
            .help("Number of cache pollution accesses before each sequence measurement")
            .default_value(std::size_t{10'000})
            .scan<'d', std::size_t>();
        args.add_argument("--repetitions", "-R")
            .help("Number of repetitions per sequence measurement")
            .default_value(std::size_t{10})
            .scan<'d', std::size_t>();
        args.add_argument("--length", "-l")
            .help("Sequence length")
            .default_value(std::size_t{30})
            .scan<'d', std::size_t>();
        args.add_argument("--rounds", "-r")
            .help("Minimum number of elimination rounds")
            .default_value(std::size_t{10})
            .scan<'d', std::size_t>();
    }

    void parse_arguments(argparse::ArgumentParser& args) {
        index_ = args.get<std::size_t>("-i");
        pollution_ = args.get<std::size_t>("--pollution");
        repetitions_ = args.get<std::size_t>("--repetitions");
        if(repetitions_ == 0) {
            throw std::runtime_error("Number of --repetitions/-R must be greater than 0");
        }
        length_ = args.get<std::size_t>("--length");
        minimum_rounds_ = args.get<std::size_t>("--rounds");
    }

    int main(instrumented_memory auto& memory) const;
};

}

#include "./impl/reverse_replacement_command.hpp"

#endif /* CACHEHOUND_CLI_COMMANDS_REVERSE_REPLACEMENT_COMMAND_HPP */
