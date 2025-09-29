cmake_minimum_required(VERSION 3.16.3)
project(remote_app)
find_package(PkgConfig REQUIRED)

# Use pkg-config to find libevent and set variables
pkg_check_modules(LIBEVENT REQUIRED libevent)

# Find the Threads library, which provides pthread
find_package(Threads REQUIRED)

# Find GoogleTest
find_package(GTest REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -Wall -Wextra")

include_directories(inc)
add_subdirectory(test)

#all files from src directory, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "src/*.cpp")

add_executable(remote_app ${SOURCES})

target_link_libraries(remote_app pthread ssl crypto ${LIBEVENT_LIBRARIES})

install(TARGETS remote_app
        CONFIGURATIONS Release
        RUNTIME DESTINATION /usr/local/bin
)
