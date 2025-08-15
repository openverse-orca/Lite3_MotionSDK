#!/bin/bash

# GRPC推理服务器测试程序构建脚本

echo "=== Building GRPC Inference Server Test Program ==="

# 检查是否存在build目录，如果不存在则创建
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

# 进入build目录
cd build

# 配置CMake (从test目录回到项目根目录)
echo "Configuring CMake..."
cmake ../..

# 检查CMake配置是否成功
if [ $? -ne 0 ]; then
    echo "✗ CMake configuration failed!"
    exit 1
fi

# 编译
echo "Building..."
make -j4

# 检查编译是否成功
if [ $? -ne 0 ]; then
    echo "✗ Build failed!"
    exit 1
fi

echo "✓ Build completed successfully!"
echo ""
echo "Generated executables:"
echo "  - Lite_motion (main program)"
echo "  - test_grpc_client (test program)"
echo ""
echo "To run the test program:"
echo "  ./test_grpc_client"
echo "  ./test_grpc_client [server_address:port]"
echo ""
echo "Example:"
echo "  ./test_grpc_client localhost:50051" 