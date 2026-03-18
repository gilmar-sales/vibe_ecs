include(GoogleTest)

file(GLOB_RECURSE TEST_FILES "*_test.cpp")

add_executable(vibe_ecs_tests ${TEST_FILES})

target_include_directories(vibe_ecs_tests PRIVATE ${GTEST_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(vibe_ecs_tests vibe_ecs gtest gtest_main)

gtest_discover_tests(vibe_ecs_tests)
