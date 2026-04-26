cmake_minimum_required(VERSION 3.27)

project(Compiler C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/depends/sqlite3
)


file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE APP CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/app/*.cpp")
file(GLOB_RECURSE DEPENDENCIES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/depends/**/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/depends/**/*.cpp")


# Sqlite dependencies
find_package(Threads REQUIRED)

include(cmake/gtest.cmake)
include(cmake/libsodium.cmake)


add_library(replikon_lib
    ${SOURCES}
    ${DEPENDENCIES}
)
target_compile_options(replikon_lib PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(replikon_lib PRIVATE Threads::Threads ${CMAKE_DL_LIBS} sodium)

add_executable(replikon ${APP})
target_link_libraries(replikon PRIVATE replikon_lib)

enable_testing()
add_executable(test_log_crdt tests/test_log_crdt.cpp)
target_link_libraries(test_log_crdt PRIVATE replikon_lib GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_log_crdt)

set_target_properties(replikon test_log_crdt PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
