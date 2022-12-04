cmake_minimum_required(VERSION 3.14)
project(maximum_induced_subtree LANGUAGES CXX)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2
  GIT_TAG        v3.1.1
)

FetchContent_Declare(
  cppcoro
  GIT_REPOSITORY https://github.com/andreasbuhr/cppcoro
  GIT_TAG        10bbcdbf2be3ad3aa56febcf4c7662d771460a99
)

FetchContent_Declare(
  range-v3
  GIT_REPOSITORY https://github.com/ericniebler/range-v3
  GIT_TAG        0.12.0
)

FetchContent_Declare(
    lmrtfy
    GIT_REPOSITORY https://github.com/Ahajha/LMRTFY
    GIT_TAG        v0.2.1
)

set (BUILD_TESTING OFF)
FetchContent_MakeAvailable(Catch2 cppcoro range-v3 lmrtfy)

add_subdirectory(rust_src)

add_compile_options(-g1 -gdwarf-2)

set(CMAKE_CXX_EXTENSIONS OFF)

add_library(hrp_lib STATIC)

target_sources(hrp_lib PRIVATE
    source/border.cpp
    source/enumerate_subtrees.cpp
    source/permutation.cpp
)

target_include_directories(hrp_lib PUBLIC include)

target_compile_features(hrp_lib PUBLIC cxx_std_20)

target_link_libraries(hrp_lib PUBLIC cppcoro lmrtfy range-v3 permutation_impl)

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
    test/test_enumerate.cpp
    test/reference_enumerator.cpp
    test/test_permutation.cpp
)

add_executable(enumerate)
target_sources(enumerate PRIVATE source/induced_subtree_enumerate.cpp)
target_link_libraries(enumerate PRIVATE hrp_lib)

add_executable(max_subtree)
target_sources(max_subtree PRIVATE source/maximum_subtree.cpp)
target_link_libraries(max_subtree PRIVATE hrp_lib)

add_executable(tests ${TEST_SOURCE})

target_include_directories(tests PRIVATE test/include)

target_link_libraries(tests PUBLIC hrp_lib Catch2::Catch2WithMain range-v3::range-v3)

target_compile_features(tests PRIVATE cxx_std_20)

include(CTest)

add_test(NAME tests COMMAND tests)
