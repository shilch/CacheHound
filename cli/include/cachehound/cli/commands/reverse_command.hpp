#ifndef CACHEHOUND_CLI_COMMANDS_REVERSE_COMMAND_HPP
#define CACHEHOUND_CLI_COMMANDS_REVERSE_COMMAND_HPP

#include "./detail/multicommand_base.hpp"
#include "./detail/command.hpp"

#include "./reverse_info_command.hpp"
#include "./reverse_placement_command.hpp"
#include "./reverse_replacement_command.hpp"
#if __has_include("./reverse_playground_command.hpp")
#include "./reverse_playground_command.hpp"
#endif

#include <argparse/argparse.hpp>

namespace cachehound::cli {

class reverse_command : public multicommand_base<
    reverse_command,
    reverse_placement_command,
    reverse_replacement_command,
#if __has_include("./reverse_playground_command.hpp")
    reverse_playground_command,
#endif
    reverse_info_command> {

public:
    static constexpr const char* name = "reverse";

private:
    unsigned level_;
    std::size_t memory_size_;
    bool simulate_;
    unsigned cpu_;
    std::vector<std::uint64_t> pmu_events_;
    std::function<kernel_memory::pmu_handler_type> pmu_handler_;
    kernel_memory::isolation_level isolation_;
    bool physical_;

    template<std::size_t MaxDepth = 4>
    int provide_memory(auto&& func, unsigned levels_remaining) const;
    int adapt_indexing(memory auto&, auto&& func) const;

public:
    void setup_arguments(argparse::ArgumentParser& args);
    void parse_arguments(argparse::ArgumentParser& args);
    int run_subcommand(auto& subcommand) const;
};
static_assert(command<reverse_command>);

}

#include "./impl/reverse_command.hpp"

#endif /* CACHEHOUND_CLI_COMMANDS_REVERSE_COMMAND_HPP */
