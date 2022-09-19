cmake_minimum_required(VERSION 3.15)
include_guard(GLOBAL)
project(grpc_stream)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenCV REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

set(PROTO_FILES
    protos/stream.proto
)

add_library(proto ${PROTO_FILES})

target_link_libraries(proto
  PUBLIC
    protobuf::libprotobuf
    gRPC::grpc
    gRPC::grpc++
)

target_include_directories(proto 
  PUBLIC 
    ${CMAKE_CURRENT_BINARY_DIR}
)

get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET proto LANGUAGE cpp)
protobuf_generate(TARGET proto LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")

add_executable(grpc_server
  grpc_server.cpp
  ${PROTO_SRCS}
  ${GRPC_SRCS}
)

target_link_libraries(grpc_server
PUBLIC
  ${OpenCV_LIBRARIES}
  proto
  gRPC::grpc++_reflection
  protobuf::libprotobuf
)

target_include_directories(grpc_server
PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${OpenCV_INCLUDE_DIRS}
  ${Protobuf_INCLUDE_DIRS}
)

add_executable(grpc_client 
  grpc_client.cpp
  ${PROTO_SRCS}
  ${GRPC_SRCS}
)

target_link_libraries(grpc_client
PUBLIC
  ${OpenCV_LIBRARIES}
  proto
  gRPC::grpc++_reflection
)

target_include_directories(grpc_client
PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${OpenCV_INCLUDE_DIRS}
  ${Protobuf_INCLUDE_DIRS}
)