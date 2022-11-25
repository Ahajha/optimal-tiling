# Set release or debug in cargo based on CMake build type
# (Probably doesn't work well with multi-config generators)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CARGO_CMD cargo build)
    set(TARGET_DIR "debug")
else ()
    set(CARGO_CMD cargo build --release)
    set(TARGET_DIR "release")
endif()

if (WIN32)
    set(RUST_PART_LIB "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_DIR}/hrp.lib")
else()
    set(RUST_PART_LIB "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_DIR}/libhrp.a")
endif()

set (RUST_PART_CXX "${CMAKE_CURRENT_BINARY_DIR}/cxxbridge/hrp/src/lib.rs.cc")

add_custom_command(
    OUTPUT ${RUST_PART_LIB} ${RUST_PART_CXX}
    COMMAND ${CMAKE_COMMAND} -E env CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR} ${CARGO_CMD}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS src/lib.rs Cargo.toml build.rs
)

add_library(permutation_impl STATIC)
target_sources(permutation_impl PRIVATE ${RUST_PART_CXX})
target_include_directories(permutation_impl PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/cxxbridge)
target_link_libraries(permutation_impl INTERFACE ${RUST_PART_LIB})

if (WIN32)
    # Need to link against extra libraries for the rust stdlib
    target_link_libraries(permutation_impl INTERFACE ws2_32 userenv bcrypt)
endif()