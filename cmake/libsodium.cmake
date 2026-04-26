include(ExternalProject)
set(LIBSODIUM_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libsodium_proj)
ExternalProject_Add(
    libsodium_ext
    URL https://github.com/jedisct1/libsodium/releases/download/1.0.19-RELEASE/libsodium-1.0.19.tar.gz
    PREFIX ${LIBSODIUM_PREFIX}
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --disable-shared --enable-static
    BUILD_COMMAND make
    INSTALL_COMMAND make install
)

add_library(sodium STATIC IMPORTED)
set_target_properties(sodium PROPERTIES
    IMPORTED_LOCATION ${LIBSODIUM_PREFIX}/lib/libsodium.a
    INTERFACE_INCLUDE_DIRECTORIES ${LIBSODIUM_PREFIX}/include
)
add_dependencies(sodium libsodium_ext)