#include "cachehound/cli/commands/root_command.hpp"

#include <iostream>
#include <cachehound/cachehound.hpp>

int main(int argc, char** argv) {
    cachehound::cli::root_command root_cmd;

    root_cmd.setup();
    auto& argparse = root_cmd.get_argparse();
    try {
        argparse.parse_args(argc, argv);
        root_cmd.parse();
    } catch(const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return root_cmd.run();
}
