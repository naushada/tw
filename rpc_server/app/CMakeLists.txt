cmake_minimum_required(VERSION 3.16.3)
project(app)
find_package(PkgConfig REQUIRED)


# Find the Threads library, which provides pthread
find_package(Threads REQUIRED)

# Find GoogleTest
find_package(GTest REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -Wall -Wextra")

include_directories(inc)
include_directories(middleware/inc)
include_directories(libevent_adapter/inc)
include_directories(nghttp2_adapter/inc)
include_directories(idl)

add_subdirectory(libevent_adapter)
add_subdirectory(nghttp2_adapter)
add_subdirectory(middleware)
add_subdirectory(idl)

file(GLOB SOURCES "src/*.cpp")

add_executable(app ${SOURCES})

target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/idl)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/idl)

target_link_libraries(app PUBLIC gnmi app_intf)
target_include_directories(app PUBLIC idl)

install(TARGETS app
        CONFIGURATIONS Release
        RUNTIME DESTINATION /usr/local/bin
)
