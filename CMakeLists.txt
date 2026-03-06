cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(VibeECS VERSION 0.0.1 LANGUAGES CXX)

if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    set(VIBE_ECS_BUILD_BENCHMARKS ON)
    set(VIBE_ECS_BUILD_TESTS ON)
endif ()

include(FetchContent)

if (VIBE_ECS_BUILD_BENCHMARKS)
    option(BENCHMARK_ENABLE_TESTING "Disable tests" OFF)
    FetchContent_Declare(
        google-benchmark
        GIT_REPOSITORY "https://github.com/google/benchmark.git"
        GIT_TAG "v1.9.5"
    )
    FetchContent_MakeAvailable(google-benchmark)

    add_subdirectory(bench)
endif()

if (VIBE_ECS_BUILD_TESTS)
    FetchContent_Declare(
        google-test
        GIT_REPOSITORY "https://github.com/google/googletest.git"
        GIT_TAG "v1.17.0"
    )
    FetchContent_MakeAvailable(google-test)

    enable_testing()
    add_subdirectory(tests)
endif()

file(GLOB_RECURSE SRC_FILES "src/*.cpp")

add_library(vibe_ecs ${SRC_FILES})
target_include_directories(vibe_ecs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
