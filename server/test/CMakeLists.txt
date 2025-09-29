cmake_minimum_required(VERSION 3.16.3)
project(remote_app)

set(CMAKE_CXX_STANDARD 17) # Google Test requires C++11 or higher
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use FetchContent to download and configure Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1 # Specify a stable version tag
)
# Ensure targets are populated
FetchContent_MakeAvailable(googletest)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBEVENT REQUIRED libevent)

# Find the Threads library, which encapsulates the correct flags for pthreads
find_package(Threads REQUIRED)

# Find GoogleTest
find_package(GTest REQUIRED)

include_directories(../inc)

#Adding Complier flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -g -Wall -Wextra")

#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "*.cpp")

enable_testing()

add_executable(remote_app_test ${SOURCES})
target_link_libraries(remote_app_test
    PRIVATE
        GTest::Main
        ${LIBEVENT_LIBRARIES}
        Threads::Threads
)
#target_link_libraries(remote_app_test gtest_main pthread ssl crypto event gtest)

#target_link_libraries(remote_app_test pthread)
#target_link_libraries(remote_app_test ssl crypto)
#target_link_libraries(remote_app_test event)
#target_link_libraries(remote_app_test gtest)

add_test(remote_app_gtests remote_app_test)
gtest_discover_tests(remote_app_test)
