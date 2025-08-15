# Test CMakeLists.txt
# 用于构建测试程序

# 添加测试程序
add_executable(test_grpc_client
  "test_grpc_client.cpp"
  ${SRC_LIST}
  ${hw_proto_srcs}
  ${hw_grpc_srcs}
)

# 链接动态库
if (BUILD_PLATFORM STREQUAL arm)
  target_link_libraries(test_grpc_client libdeeprobotics_legged_sdk_aarch64.so)
else()
  target_link_libraries(test_grpc_client libdeeprobotics_legged_sdk_x86_64.so)
endif()

target_link_libraries(test_grpc_client -lpthread -lm -lrt -ldl -lstdc++ -lssl -lcrypto)

target_link_libraries(test_grpc_client
    ${_REFLECTION}
    ${_SSL_CRYPTO}
    ${_SSL_SSL}
    ${_GRPC_GRPCPP}
    ${_GRPC_GRPC}
    ${_PROTOBUF_LIBPROTOBUF}
) 