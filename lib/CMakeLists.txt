add_library(cachehound INTERFACE)
target_compile_definitions(cachehound INTERFACE CACHEHOUND_HEADER_ONLY)
target_include_directories(cachehound INTERFACE include)
target_link_libraries(cachehound INTERFACE klib)

add_executable(cachehound_test test/src/replacement.cpp)
target_link_libraries(cachehound_test PRIVATE cachehound)

find_package(Catch2 CONFIG REQUIRED)
target_link_libraries(cachehound_test PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(cachehound_test PRIVATE nlohmann_json::nlohmann_json)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(cachehound_test PRIVATE fmt::fmt)
