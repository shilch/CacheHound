#ifndef CACHEHOUND_CLI_COMMANDS_DETAIL_MULTICOMMAND_BASE_HPP
#define CACHEHOUND_CLI_COMMANDS_DETAIL_MULTICOMMAND_BASE_HPP

#include "./command_base.hpp"

#include <argparse/argparse.hpp>
#include <cassert>
#include <exception>
#include <tuple>

namespace cachehound::cli {

template<typename Self, typename... Subcommands>
class multicommand_base : public command_base<Self> {
    static_assert(sizeof...(Subcommands) > 0, "At least one subcommand must be specified");

    std::tuple<Subcommands...> subcommands_;
    std::size_t selected_subcommand_ = 0;

    template<typename Lambda>
    void for_each_subcommand(Lambda&& lambda) {
        std::apply([lambda = std::forward<Lambda>(lambda)](auto&... subcommand) mutable {
            (std::forward<Lambda>(lambda)(subcommand),...);
        }, subcommands_);
    }

    template<typename Lambda>
    void for_each_subcommand(Lambda&& lambda) const {
        std::apply([lambda = std::forward<Lambda>(lambda)](const auto&... subcommand) mutable {
            (std::forward<Lambda>(lambda)(subcommand),...);
        }, subcommands_);
    }

    std::string argparse_help_;

protected:
    template<typename... Args>

    int run_fallback(Args&&... args) const {
        std::cerr << argparse_help_;
        return 1;
    }

    template<typename Subcommand, typename... Args>
    int run_subcommand(Subcommand& subcommand, Args&&... args) const {
        return subcommand.run(std::forward<Args>(args)...);
    }

    bool uses_subcommand() const {
        return selected_subcommand_ != 0;
    }

public:
    void setup_arguments(argparse::ArgumentParser& args) {
        for_each_subcommand([&]<typename Subcommand>(Subcommand& subcommand) mutable {
            subcommand.setup();
            args.add_subparser(subcommand.get_argparse());
        });
    }

    void parse_arguments(argparse::ArgumentParser& args) {
        argparse_help_ = args.help().str();

        std::size_t current_subcommand = 0;
        for_each_subcommand([&]<typename Subcommand>(Subcommand& subcommand) {
            if(selected_subcommand_ != 0) /* We already found the selected command */
                return;

            current_subcommand++;
            if(args.is_subcommand_used(Subcommand::name)) {
                subcommand.parse();
                selected_subcommand_ = current_subcommand;
            }
        });
    }

    template<typename... Args>
    int main(Args&&... args) const {
        if(selected_subcommand_ == 0) {
            return static_cast<const Self*>(this)->run_fallback(std::forward<Args>(args)...);
        } else {
            std::size_t current_subcommand = 0;
            int exit_code;

            for_each_subcommand([this, &current_subcommand, &exit_code, ...args = std::forward<Args>(args)]<typename Subcommand>(Subcommand& subcommand) mutable {
                current_subcommand++;
                if(current_subcommand == selected_subcommand_) {
                    exit_code = static_cast<const Self*>(this)->run_subcommand(subcommand, std::forward<Args>(args)...);
                }
            });

            return exit_code;
        }
    }
};

}

#endif /* CACHEHOUND_CLI_COMMANDS_DETAIL_MULTICOMMAND_BASE_HPP */
