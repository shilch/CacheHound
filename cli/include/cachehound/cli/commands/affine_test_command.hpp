#ifndef CACHEHOUND_CLI_COMMANDS_AFFINE_TEST_COMMAND_HPP
#define CACHEHOUND_CLI_COMMANDS_AFFINE_TEST_COMMAND_HPP

#include "./detail/command_base.hpp"

#include <cachehound/cachehound.hpp>

namespace cachehound::cli {

class affine_test_command : public command_base<affine_test_command> {
public:
    static constexpr const char* name = "affine-test";

public:
    int main() const;
};

}

#ifdef CACHEHOUND_CLI_HEADER_ONLY
#include "./impl/affine_test_command.ipp"
#endif

#endif /* CACHEHOUND_CLI_COMMANDS_AFFINE_TEST_COMMAND_HPP */
