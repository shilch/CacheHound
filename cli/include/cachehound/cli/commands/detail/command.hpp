#ifndef CACHEHOUND_CLI_COMMANDS_DETAIL_COMMAND_HPP
#define CACHEHOUND_CLI_COMMANDS_DETAIL_COMMAND_HPP

#include <argparse/argparse.hpp>
#include <concepts>
#include <utility>

namespace cachehound::cli {

template<typename C, typename... RunArgs>
concept command = requires(C command, const C& const_command, RunArgs&&... run_args) {
    command.setup();
    { command.get_argparse() } -> std::same_as<argparse::ArgumentParser&>;
    command.parse();
    { const_command.run(std::forward<RunArgs>(run_args)...) } -> std::same_as<int>;
};

}

#endif /* CACHEHOUND_CLI_COMMANDS_DETAIL_COMMAND_HPP */
