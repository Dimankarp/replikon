cmake_minimum_required(VERSION 3.27)

project(Compiler CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE DEPENDENCIES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/depends/*.cpp")

add_executable(replikon
    ${SOURCES}
    ${DEPENDENCIES}
)

set_target_properties(replikon PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
target_compile_options(replikon PRIVATE -Wall -Wextra -Wpedantic -Wno-sign-compare)


