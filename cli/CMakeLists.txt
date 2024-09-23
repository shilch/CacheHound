file(GLOB_RECURSE cli_SRC CONFIGURE_DEPENDS src "*.hpp" "*.cpp")
add_executable(cli ${cli_SRC})

target_compile_definitions(cli PRIVATE CACHEHOUND_CLI_HEADER_ONLY)
target_include_directories(cli PRIVATE include)
target_link_libraries(cli PRIVATE cachehound)
target_compile_options(cli PRIVATE -fsanitize=address)
target_link_options(cli PRIVATE -fsanitize=address)
target_link_libraries(cli PRIVATE cachehound_sim)

find_package(argparse CONFIG REQUIRED)
target_link_libraries(cli PRIVATE argparse::argparse)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(cli PRIVATE spdlog::spdlog)

find_package(Eigen3 CONFIG REQUIRED)
target_link_libraries(cli PRIVATE Eigen3::Eigen)

find_package(Boost REQUIRED)
target_link_libraries(cli PRIVATE Boost::boost)

find_package(PkgConfig REQUIRED)
pkg_check_modules(gmp REQUIRED IMPORTED_TARGET gmp)
target_link_libraries(cli PRIVATE PkgConfig::gmp)
