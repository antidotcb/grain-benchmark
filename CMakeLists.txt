cmake_minimum_required(VERSION 3.0)
project(test1)

set(BENCHMARK_ENABLE_TESTING OFF)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(benchmark)

add_executable(test1 main.cpp)

target_link_libraries(test1 benchmark::benchmark benchmark_main)
