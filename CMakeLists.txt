cmake_minimum_required(VERSION 3.16)

project(performance_playground)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

include_directories(include)

add_executable(perf_playground
    src/main.cpp
    src/aos_vs_soa.cpp
    src/false_sharing.cpp
)