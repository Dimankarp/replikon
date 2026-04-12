cmake_minimum_required(VERSION 3.27)

project(Compiler C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/depends/sqlite3
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE DEPENDENCIES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/depends/**/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/depends/**/*.cpp")

add_executable(replikon
    ${SOURCES}
    ${DEPENDENCIES}
)

set_target_properties(replikon PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_compile_options(replikon PRIVATE -Wall -Wextra -Wpedantic)

# Sqlite dependencies
find_package(Threads REQUIRED)
target_link_libraries(replikon PRIVATE Threads::Threads ${CMAKE_DL_LIBS})
