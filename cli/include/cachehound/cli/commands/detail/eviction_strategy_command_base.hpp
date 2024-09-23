#ifndef CACHEHOUND_CLI_COMMANDS_DETAIL_EVICTION_STRATEGY_COMMAND_BASE_HPP
#define CACHEHOUND_CLI_COMMANDS_DETAIL_EVICTION_STRATEGY_COMMAND_BASE_HPP

#include <argparse/argparse.hpp>
#include <cassert>
#include <cachehound/cachehound.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace cachehound::cli::detail {

class eviction_strategy_command_base {
private:
    enum class strategy {
        automatic = 0,
        eviction_set = 1,
        cisw = 2
    };

    strategy strategy_;

    std::size_t is_eviction_set_pollution_;

    static constexpr std::size_t local_pollution_fail_threshold = 5;
    static constexpr std::size_t global_pollution_fail_threshold = 3;

    std::size_t is_eviction_set_repetitions_;

    int provide_eviction_set_strategy(instrumented_memory auto&, auto&& func) const;
    int provide_cisw_eviction_strategy(armv8_memory auto& memory, auto&& func) const;

    int provide_is_eviction_set(instrumented_memory auto& memory, std::uniform_random_bit_generator auto& urbg, auto&& func) const;

protected:
    int provide_eviction_strategy(memory auto& memory, auto&& func) const {
        if(strategy_ == strategy::automatic) {
            if constexpr(armv8_memory<decltype(memory)>) {
                return provide_cisw_eviction_strategy(memory, std::forward<decltype(func)>(func));
            } else if constexpr(instrumented_memory<decltype(memory)>) {
                return provide_eviction_set_strategy(memory, std::forward<decltype(func)>(func));
            } else {
                spdlog::error("Failed to find eviction strategy for memory type");
                return 1;
            }
        } else if(strategy_ == strategy::eviction_set) {
            if constexpr(instrumented_memory<decltype(memory)>) {
                return provide_eviction_set_strategy(memory, std::forward<decltype(func)>(func));
            } else {
                spdlog::error("Eviction strategy 'eviction-set' only supported on cachehound::instrumented_memory");
                return 1;
            }
        }  else if(strategy_ == strategy::cisw) {
            if constexpr(armv8_memory<decltype(memory)>) {
                return provide_cisw_eviction_strategy(memory, std::forward<decltype(func)>(func));
            } else {
                spdlog::error("Eviction strategy 'cisw' only supported on cachehound::armv8_memory");
                return 1;
            }
        } else {
            assert(!"Unreachable");
            return 1;
        }
    }

public:
    void setup_arguments(argparse::ArgumentParser& args) {
        args.add_argument("--eviction-strategy", "-S")
            .help("The eviction strategy to use (auto/eviction-set/cisw)")
            .choices("auto", "eviction-set", "cisw")
            .default_value("auto");
        args.add_argument("--pollution", "-P")
            .help("Number of cache pollution accesses (only applicable to eviction-set strategy)")
            .default_value(std::size_t{10'000})
            .scan<'d', std::size_t>();
        args.add_argument("--repetitions", "-R")
            .help("Number of repetitions to use for is_eviction_set check (only applicable to eviction-set strategy)")
            .default_value(std::size_t{1})
            .scan<'d', std::size_t>();
    }

    void parse_arguments(argparse::ArgumentParser& args) {
        auto strategy_str = args.get<std::string>("--eviction-strategy");
        if(strategy_str == "auto") {
            strategy_ = strategy::automatic;
        } else if(strategy_str == "eviction-set") {
            strategy_ = strategy::eviction_set;
        } else if(strategy_str == "cisw") {
            strategy_ = strategy::cisw;
        } else {
            assert(!"Unreachable");
        }

        is_eviction_set_pollution_ = args.get<std::size_t>("--pollution");

        is_eviction_set_repetitions_ = args.get<std::size_t>("--repetitions");
        if(is_eviction_set_repetitions_ == 0) {
            throw std::runtime_error("Number of --repetitions/-R must be greater than 0");
        }
    }
};

}

#include "./impl/eviction_strategy_command_base.hpp"

#endif /* CACHEHOUND_CLI_COMMANDS_DETAIL_EVICTION_STRATEGY_COMMAND_BASE_HPP */
