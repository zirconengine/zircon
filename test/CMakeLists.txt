cmake_minimum_required(VERSION 3.10)

set(UNITY_DIR ${CMAKE_SOURCE_DIR}/thirdparty/unity)

add_library(unity STATIC ${UNITY_DIR}/unity.c)
target_include_directories(unity PUBLIC ${UNITY_DIR})

# Single test executable
add_executable(zi_tests
    test_entry_point.c
    test_math.c
    test_core.c
)
target_link_libraries(zi_tests unity zi-runtime)
target_include_directories(zi_tests PRIVATE ${CMAKE_SOURCE_DIR}/runtime)

# Register tests with CTest
enable_testing()
add_test(NAME zi_tests COMMAND zi_tests)
