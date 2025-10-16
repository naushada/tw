cmake_minimum_required(VERSION 3.16.3)
project(rpc_app)
find_package(PkgConfig REQUIRED)

# Use pkg-config to find libevent and set variables
pkg_check_modules(LIBEVENT REQUIRED libevent)

# Find the Threads library, which provides pthread
find_package(Threads REQUIRED)

# Find GoogleTest
find_package(GTest REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -Wall -Wextra")

include_directories(inc)
include_directories(../middleware/inc)

#all files from src directory, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "src/*.cpp")

add_library(server_app ${SOURCES})
target_link_libraries(server_app pthread ssl crypto ${LIBEVENT_LIBRARIES})
target_include_directories(server_app PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

#add_subdirectory(test)
#target_include_directories(server_app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#set_target_properties(server_app PROPERTIES
#        IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/libserver_app.a"
#        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/include/libserver_app"
#    )

install(TARGETS server_app
        CONFIGURATIONS Release
        RUNTIME DESTINATION /usr/local/lib
)
