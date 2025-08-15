cmake_minimum_required(VERSION 3.15)

project(Lite_motion)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(BUILD_PLATFORM "x86" CACHE STRING "select build cpu type")
if (BUILD_PLATFORM STREQUAL arm)
  message("this is arm platform")
  SET(CMAKE_C_COMPILER "aarch64-linux-gnu-gcc")
  SET(CMAKE_CXX_COMPILER "aarch64-linux-gnu-g++") 
else()
  message("this is x86 platform")
endif()

# 设置 OpenSSL 路径
# set(OPENSSL_ROOT_DIR "$ENV{HOME}/.orca/3rdParty/packages/OpenSSL-1.1.1t-rev1-linux/OpenSSL")
# set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")
# set(OPENSSL_CRYPTO_LIBRARY "${OPENSSL_ROOT_DIR}/lib/libcrypto.a")
# set(OPENSSL_SSL_LIBRARY "${OPENSSL_ROOT_DIR}/lib/libssl.a")

# 查找 OpenSSL 库
find_package(OpenSSL REQUIRED)
message(STATUS "Using OpenSSL version: ${OPENSSL_VERSION}")

# vcpkg
# set(Protobuf_DIR "$ENV{HOME}/.orca/3rdParty/packages/grpc-1.51.1-rev2-linux/grpc/lib/cmake/protobuf")
# set(gRPC_DIR "$ENV{HOME}/.orca/3rdParty/packages/grpc-1.51.1-rev2-linux/grpc/lib/cmake/grpc")
# set(absl_DIR "$ENV{HOME}/.orca/3rdParty/packages/grpc-1.51.1-rev2-linux/grpc/lib/cmake/absl")

# Protobuf
# set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf REQUIRED)
message(STATUS "Using protobuf ${protobuf_VERSION}")

# Protobuf-compiler
set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)

# gRPC
find_package(gRPC REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")
set(_GRPC_GRPCPP gRPC::grpc++)
set(_GRPC_GRPC gRPC::grpc)
set(_SSL_CRYPTO OpenSSL::Crypto)
set(_SSL_SSL OpenSSL::SSL)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)

# Proto file
get_filename_component(hw_proto "include/proto/inference.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

# Generated sources
set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/inference.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/inference.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/inference.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/inference.grpc.pb.h")

# Generate protobuf and gRPC files
add_custom_command(
    OUTPUT ${hw_proto_srcs} ${hw_proto_hdrs} ${hw_grpc_srcs} ${hw_grpc_hdrs}
    COMMAND ${_PROTOBUF_PROTOC}
        --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
        --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
        --plugin=protoc-gen-grpc=${_GRPC_CPP_PLUGIN_EXECUTABLE}
        -I${hw_proto_path}
        ${hw_proto}
    DEPENDS ${hw_proto}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set(CMAKE_BUILD_TYPE Release)
set(SRC_DIR_LIST "." src)
add_definitions(-w) # warning ignore

foreach(VAR ${SRC_DIR_LIST})
  set(TEMP)
  aux_source_directory(./src/ TEMP)
  set(SRC_LIST ${RCS_SRC_LIST} ${TEMP})
endforeach(VAR)

# Add data_logger.cpp to the source list
list(APPEND SRC_LIST "src/data_logger.cpp")

# 在 include_directories 中添加 OpenSSL 头文件路径
include_directories(
    ./include/
    ./include/common/
    ./lib/eigen3
    ./build
    # ${OPENSSL_INCLUDE_DIR}  # 添加 OpenSSL 头文件路径
)

link_directories(
  ./lib/
)

add_executable(${PROJECT_NAME} 
  "main.cpp" 
  ${SRC_LIST} 
  ${hw_proto_srcs}
  ${hw_grpc_srcs}
)

# 添加测试程序
add_executable(test_grpc_client
  "test/test_grpc_client_original.cpp"
  ${SRC_LIST}
  ${hw_proto_srcs}
  ${hw_grpc_srcs}
)

# 链接动态库target_link_libraries(myprogram /path/to/lib/libfoo.so)

# 外部用cmake . -DBUILD_PLATFORM=arm进行值传入，便可以执行不同的逻辑
if (BUILD_PLATFORM STREQUAL arm)
  target_link_libraries(${PROJECT_NAME} libdeeprobotics_legged_sdk_aarch64.so)
  target_link_libraries(test_grpc_client libdeeprobotics_legged_sdk_aarch64.so)
else()
  target_link_libraries(${PROJECT_NAME} libdeeprobotics_legged_sdk_x86_64.so)
  target_link_libraries(test_grpc_client libdeeprobotics_legged_sdk_x86_64.so)
endif()

target_link_libraries(${PROJECT_NAME} -lpthread -lm -lrt -ldl -lstdc++ -lssl -lcrypto)
target_link_libraries(test_grpc_client -lpthread -lm -lrt -ldl -lstdc++ -lssl -lcrypto)

target_link_libraries(${PROJECT_NAME}
    ${_REFLECTION}
    ${_SSL_CRYPTO}
    ${_SSL_SSL}
    ${_GRPC_GRPCPP}
    ${_GRPC_GRPC}
    ${_PROTOBUF_LIBPROTOBUF}
)

target_link_libraries(test_grpc_client
    ${_REFLECTION}
    ${_SSL_CRYPTO}
    ${_SSL_SSL}
    ${_GRPC_GRPCPP}
    ${_GRPC_GRPC}
    ${_PROTOBUF_LIBPROTOBUF}
)


