#ifndef CACHEHOUND_CLI_COMMANDS_REVERSE_INFO_COMMAND_HPP
#define CACHEHOUND_CLI_COMMANDS_REVERSE_INFO_COMMAND_HPP

#include "./detail/command.hpp"
#include "./detail/command_base.hpp"

#include <cachehound/cachehound.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace cachehound::cli {

class reverse_info_command : public command_base<reverse_info_command> {
public:
    static constexpr const char* name = "info";

    int main(instrumented_memory auto& memory) const {
        spdlog::info("Memory size:     {}", memory_size(memory));
        spdlog::info("Cache line size: {}", 1 << memory.offset_bits());
        spdlog::info("Levels:          {}", memory.levels());

        std::vector<std::string> features;
        if constexpr(switch_channel_memory<decltype(memory)>) {
            features.emplace_back("switch-channel");
        }
        if constexpr(x86_memory<decltype(memory)>) {
            features.emplace_back("clflush");
            features.emplace_back("wbinvd");
        }
        if constexpr(armv8_memory<decltype(memory)>) {
            features.emplace_back("cisw");
        }
        spdlog::info("Features:        {}", fmt::join(features, ", "));

        for(unsigned level = 0; level < memory.levels(); ++level) {
            spdlog::info("");
            spdlog::info("Level {}:", level + 1);
            spdlog::info("  Associativity: {}", memory.ways(level));
            spdlog::info("  Sets:          {}", 1 << memory.index_bits(level));
        }

        return 0;
    }
};

}

#endif /* CACHEHOUND_CLI_COMMANDS_REVERSE_INFO_COMMAND_HPP */
