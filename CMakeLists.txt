cmake_minimum_required(VERSION 3.22)

project(cachehound)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++20 -O2")
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(kernel)
add_subdirectory(sim)
add_subdirectory(lib)
add_subdirectory(cli)
