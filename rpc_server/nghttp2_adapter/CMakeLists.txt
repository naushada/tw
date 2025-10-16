cmake_minimum_required(VERSION 3.16.3)
project(remote_app)
find_package(PkgConfig REQUIRED)

# Use pkg-config to find libevent and set variables
pkg_check_modules(LIBEVENT REQUIRED libevent)

# Find the Threads library, which provides pthread
find_package(Threads REQUIRED)

# Find GoogleTest
find_package(GTest REQUIRED)

# Find a library named 'foo' in specific custom paths
find_library(NGHTTP2_LIBRARIES NAMES nghttp2 PATHS /usr/local/lib)

# Find a header file 'bar.h' in specific custom paths
find_path(NGHTTP2_INCLUDE_DIR NAMES nghttp2.h PATHS /usr/local/include/nghttp2)



#find_package(nghttp2 REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -Wall -Wextra")

include_directories(inc)
include_directories(../libevent_adapter/inc)
#add_subdirectory(test)

#all files from src directory, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "src/*.cpp")

add_executable(remote_app ${SOURCES})

# Use the found paths
if(NGHTTP2_LIBRARIES AND NGHTTP2_INCLUDE_DIR)
  target_link_libraries(remote_app ${NGHTTP2_LIBRARIES})
  target_include_directories(remote_app PUBLIC ${NGHTTP2_INCLUDE_DIR})
endif()

target_link_libraries(remote_app pthread ssl crypto ${LIBEVENT_LIBRARIES} ${NGHTTP2_LIBRARIES})

install(TARGETS remote_app
        CONFIGURATIONS Release
        RUNTIME DESTINATION /usr/local/bin
)
