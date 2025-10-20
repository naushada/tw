cmake_minimum_required(VERSION 3.16.3)
project(app)
find_package(PkgConfig REQUIRED)

# Use pkg-config to find libevent and set variables
#pkg_check_modules(LIBEVENT REQUIRED libevent)

#pkg_check_modules(NGHTTP2 REQUIRED nghttp2)
# Find the Threads library, which provides pthread
find_package(Threads REQUIRED)

# Find GoogleTest
find_package(GTest REQUIRED)
find_path(NGHTTP2_INCLUDE_DIR nghttp2/nghttp2.h)

# find_library will search standard library locations for the specified library name
# The name to search for is "nghttp2", without the "lib" prefix or extension
find_library(NGHTTP2_LIBRARY nghttp2)

#find_package(nghttp2 REQUIRED)
# Check that the library and headers were found
if(NOT NGHTTP2_LIBRARY OR NOT NGHTTP2_INCLUDE_DIR)
    message(FATAL_ERROR "Could not find nghttp2 library or headers. Please check installation.")
endif()

add_subdirectory(app)

