#!/bin/bash

# 检查protoc是否安装
if ! command -v protoc &> /dev/null; then
    echo "Error: protoc is not installed. Please install protobuf compiler."
    exit 1
fi

# 检查grpc_cpp_plugin是否安装
if ! command -v grpc_cpp_plugin &> /dev/null; then
    echo "Error: grpc_cpp_plugin is not installed. Please install gRPC C++ plugin."
    exit 1
fi

# 创建build目录
mkdir -p ../build

# 生成protobuf和gRPC文件
echo "Generating protobuf and gRPC files..."
protoc --cpp_out=../build \
       --grpc_out=../build \
       --plugin=protoc-gen-grpc=$(which grpc_cpp_plugin) \
       -I../include/proto \
       ../include/proto/inference.proto

if [ $? -eq 0 ]; then
    echo "Successfully generated protobuf and gRPC files in ../build/ directory"
    echo "Generated files:"
    ls -la ../build/inference.*
else
    echo "Error: Failed to generate protobuf and gRPC files"
    exit 1
fi 