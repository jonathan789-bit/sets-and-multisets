cmake_minimum_required(VERSION 3.14)
project(CantorSetLab CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_COVERAGE "Build with gcov coverage flags" OFF)

# --- Set (1.4.1) ---
add_library(cantor_set CantorSet.cpp)
target_include_directories(cantor_set PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(cantor_set PRIVATE -Wall -Wextra)

add_executable(cantor_menu main.cpp)
target_link_libraries(cantor_menu PRIVATE cantor_set)

# --- Multiset (1.4.2) ---
add_library(cantor_multiset CantorMultiset.cpp)
target_include_directories(cantor_multiset PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(cantor_multiset PRIVATE -Wall -Wextra)

add_executable(multiset_menu main_multiset.cpp)
target_link_libraries(multiset_menu PRIVATE cantor_multiset)

# --- Tests ---
include(FetchContent)
FetchContent_Declare(googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(cantor_tests tests/test_cantor_set.cpp)
target_link_libraries(cantor_tests PRIVATE cantor_set GTest::gtest_main)

add_executable(multiset_tests tests/test_cantor_multiset.cpp)
target_link_libraries(multiset_tests PRIVATE cantor_multiset GTest::gtest_main)

if(ENABLE_COVERAGE)
  foreach(t cantor_set cantor_multiset)
    target_compile_options(${t} PRIVATE --coverage -O0)
    target_link_options(${t} PUBLIC --coverage)
  endforeach()
endif()

include(GoogleTest)
gtest_discover_tests(cantor_tests)
gtest_discover_tests(multiset_tests)