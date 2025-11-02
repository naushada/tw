cmake_minimum_required(VERSION 3.10)
project(app LANGUAGES C CXX)

find_package(Protobuf REQUIRED)

message (STATUS " Binary DIR: ${CMAKE_CURRENT_BINARY_DIR}")
# Set where to find your .proto files
set(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proto")

message (STATUS "${PROTO_DIR}")

# List your proto files
set(PROTO_FILES
    ${PROTO_DIR}/gnmi/gnmi.proto
    ${PROTO_DIR}/gnmi_ext/gnmi_ext.proto
    ${PROTO_DIR}/collector/collector.proto
)

message(STATUS "Proto files: ${PROTO_FILES}")
message (STATUS "SRC_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

set(PROTO_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

message(STTAUS " SRCS: ${PROTO_SRCS}")
message(STATUS " HDRS: ${PROTO_HDRS}")
# this will generate libgnmi.a file 
add_library(gnmi STATIC ${PROTO_SRCS} ${PROTO_HDRS} ${PROTO_FILES})
target_link_libraries(gnmi ${PROTOBUF_LIBRARIES})
message (STATUS " " ${CMAKE_CURRENT_BINARY_DIR}) 
target_include_directories(gnmi PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

