cmake_minimum_required(VERSION 3.14)
project(CantorSetLab CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_COVERAGE "Build with gcov coverage flags" OFF)

add_library(cantor_set CantorSet.cpp)
target_include_directories(cantor_set PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(cantor_set PRIVATE -Wall -Wextra)

add_executable(cantor_menu main.cpp)
target_link_libraries(cantor_menu PRIVATE cantor_set)

include(FetchContent)
FetchContent_Declare(googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(cantor_tests tests/test_cantor_set.cpp)
target_link_libraries(cantor_tests PRIVATE cantor_set GTest::gtest_main)

if(ENABLE_COVERAGE)
  target_compile_options(cantor_set PRIVATE --coverage -O0)
  target_link_options(cantor_set PUBLIC --coverage)
endif()

include(GoogleTest)
gtest_discover_tests(cantor_tests)