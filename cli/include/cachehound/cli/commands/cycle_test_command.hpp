#ifndef CACHEHOUND_CLI_COMMANDS_CYCLE_TEST_COMMAND_HPP
#define CACHEHOUND_CLI_COMMANDS_CYCLE_TEST_COMMAND_HPP

#include "./detail/command_base.hpp"

#include <cachehound/cachehound.hpp>

namespace cachehound::cli {

class cycle_test_command : public command_base<cycle_test_command> {
public:
    static constexpr const char* name = "cycle-test";

public:
    int main() const;
};

}

#ifdef CACHEHOUND_CLI_HEADER_ONLY
#include "./impl/cycle_test_command.ipp"
#endif

#endif /* CACHEHOUND_CLI_COMMANDS_CYCLE_TEST_COMMAND_HPP */
