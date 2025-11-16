cmake_minimum_required(VERSION 3.16.3)
project(app)
find_package(PkgConfig REQUIRED)

# Use pkg-config to find libevent and set variables
pkg_check_modules(LIBEVENT REQUIRED libevent)

# Find the Threads library, which provides pthread
find_package(Threads REQUIRED)

# Find GoogleTest
find_package(GTest REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -Wall -Wextra")

include_directories(inc)
include_directories(../nghttp2_adapter/inc)
include_directories(../libevent_adapter/inc)

#all files from src directory, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "src/*.cpp")

add_library(app_intf ${SOURCES})
target_link_libraries(app_intf PUBLIC gnmi http2_proto pthread ssl crypto ${LIBEVENT_LIBRARIES})
target_include_directories(app_intf PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

install(TARGETS app_intf
        CONFIGURATIONS Release
        RUNTIME DESTINATION /usr/local/lib
)

