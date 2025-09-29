#ifndef CACHEHOUND_CLI_COMMANDS_IMPL_REVERSE_HIST_COMMAND_HPP
#define CACHEHOUND_CLI_COMMANDS_IMPL_REVERSE_HIST_COMMAND_HPP

#include "../reverse_hist_command.hpp"

#include <cachehound/cachehound.hpp>
#include <map>
#include <random>
#include <spdlog/spdlog.h>

int cachehound::cli::reverse_hist_command::main(instrumented_memory auto &memory) const {
    spdlog::debug("Initializing random device");
    std::random_device rnd;
    auto seed = rnd();
    std::mt19937 mtwister {seed};

    uniform_address_distribution uad{memory};

    std::map<std::uintptr_t, std::size_t> hist;

    for(std::size_t access = 0; access < accesses_; access++) {
        auto address = uad(mtwister);

        auto level = memory.instrumented_access(address);
        ++hist[level];
    }

    spdlog::info("Histogram:");
    for(auto& entry : hist) {
        spdlog::info("Level {} - {}", entry.first, entry.second);
    }

    return 0;
}

#endif /* CACHEHOUND_CLI_COMMANDS_IMPL_REVERSE_HIST_COMMAND_HPP */
