cmake_minimum_required(VERSION 3.14)
project(maximum_induced_subtree LANGUAGES CXX)

add_library(hrp_lib INTERFACE)

if (MSVC)
	add_compile_options(/W4 /WX)
else()
	add_compile_options(-Wall -Wextra -Wshadow -Wpedantic -Wconversion -Werror)
endif()

target_include_directories(hrp_lib INTERFACE include)

target_compile_features(hrp_lib INTERFACE cxx_std_20)

include(FetchContent)

FetchContent_Declare(
	doctest
	GIT_REPOSITORY "https://github.com/doctest/doctest"
	GIT_TAG "2.4.7"
)

FetchContent_MakeAvailable(doctest)

add_executable(tests test.cpp)

target_link_libraries(tests PUBLIC hrp_lib doctest)

target_compile_features(tests PRIVATE cxx_std_20)

