cmake_minimum_required(VERSION 3.10)

set(UNITY_DIR ${CMAKE_SOURCE_DIR}/thirdparty/unity)

add_library(unity STATIC ${UNITY_DIR}/unity.c)
target_include_directories(unity PUBLIC ${UNITY_DIR})

# Test executable for math functions
add_executable(test_math test_math.c)
target_link_libraries(test_math unity)
target_include_directories(test_math PRIVATE ${CMAKE_SOURCE_DIR}/runtime)

# Test executable for core functions (allocator, hashmap)
add_executable(test_core test_core.c)
target_link_libraries(test_core unity zi-runtime)
target_include_directories(test_core PRIVATE ${CMAKE_SOURCE_DIR}/runtime)

# Register tests with CTest
enable_testing()
add_test(NAME test_math COMMAND test_math)
add_test(NAME test_core COMMAND test_core)
