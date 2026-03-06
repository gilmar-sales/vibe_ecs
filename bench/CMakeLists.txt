file(GLOB_RECURSE BENCH_FILES "bench_*.cpp")

add_executable(vibe_ecs_benchmarks ${BENCH_FILES})

target_link_libraries(vibe_ecs_benchmarks vibe_ecs benchmark::benchmark)
