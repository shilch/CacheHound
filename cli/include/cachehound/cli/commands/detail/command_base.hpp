#ifndef CACHEHOUND_CLI_COMMANDS_DETAIL_COMMAND_BASE_HPP
#define CACHEHOUND_CLI_COMMANDS_DETAIL_COMMAND_BASE_HPP

#include <argparse/argparse.hpp>
#include <exception>

namespace cachehound::cli {

template<typename Self>
class command_base {
    template <typename, typename = void>
    struct has_version_member : std::false_type {};

    template <typename T>
    struct has_version_member<T, std::void_t<decltype(T::version)>> : std::is_same<char const*, decltype(T::version)>
    {};

    template<typename T>
    static constexpr bool has_version_member_t = has_version_member<T>::value;

    std::exception_ptr parse_exception_{};

    argparse::ArgumentParser argparse_;

public:
    command_base() requires has_version_member_t<Self>
        : argparse_(Self::name, Self::version, argparse::default_arguments::all) {}

    command_base() requires (not has_version_member_t<Self>)
        : argparse_(Self::name, "", argparse::default_arguments::help) {}

    void setup() {
        static_cast<Self*>(this)->setup_arguments(argparse_);
    }

    argparse::ArgumentParser& get_argparse() {
        return argparse_;
    }

    void parse() {
        try {
            static_cast<Self*>(this)->parse_arguments(argparse_);
        } catch(std::exception& ex) {
            parse_exception_ = std::current_exception();
        }
    }

    template<typename... Args>
    int run(Args&&... args) const {
        if(parse_exception_) {
            try {
                std::rethrow_exception(parse_exception_);
            } catch(std::exception& ex) {
                std::cerr << ex.what() << std::endl;
            }
            std::cerr << argparse_;
            return 1;
        } else {
            return static_cast<const Self*>(this)->main(std::forward<Args>(args)...);
        }
    }

    void setup_arguments(argparse::ArgumentParser&) {}
    void parse_arguments(argparse::ArgumentParser&) {}
    // int main(); To be implemented by child class
};

}

#endif /* CACHEHOUND_CLI_COMMANDS_DETAIL_COMMAND_BASE_HPP */
