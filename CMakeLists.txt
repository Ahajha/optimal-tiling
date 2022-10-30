cmake_minimum_required(VERSION 3.14)
project(maximum_induced_subtree LANGUAGES CXX)

add_library(hrp_lib STATIC)

target_sources(hrp_lib PRIVATE source/border.cpp)

target_include_directories(hrp_lib PUBLIC include)

target_compile_features(hrp_lib PUBLIC cxx_std_20)

include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2
  GIT_TAG        v3.1.1
)

FetchContent_Declare(
  range-v3
  GIT_REPOSITORY https://github.com/ericniebler/range-v3
  GIT_TAG        0.12.0
)

FetchContent_MakeAvailable(Catch2 range-v3)

if (MSVC)
	add_compile_options(/W4 /WX)
else()
	add_compile_options(-Wall -Wextra -Wshadow -Wpedantic -Wconversion -Werror)
endif()

set(TEST_SOURCE
    test/test_graph.cpp
    test/test_subtree.cpp
    test/test_ordered_index_set.cpp
    test/test_border.cpp
)

add_executable(enumerate)
target_sources(enumerate PRIVATE source/induced_subtree_enumerate.cpp)
target_link_libraries(enumerate PRIVATE hrp_lib)

add_executable(tests ${TEST_SOURCE})

target_link_libraries(tests PUBLIC hrp_lib Catch2::Catch2WithMain range-v3::range-v3)

target_compile_features(tests PRIVATE cxx_std_20)

include(CTest)

add_test(NAME tests COMMAND tests)
