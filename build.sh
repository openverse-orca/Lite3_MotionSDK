#!/bin/bash

# Lite3 Motion SDK 构建脚本

echo "=== Building Lite3 Motion SDK ==="

# 检查是否存在build目录，如果不存在则创建
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

# 进入build目录
cd build

# 配置CMake
echo "Configuring CMake..."
cmake ..

# 检查CMake配置是否成功
if [ $? -ne 0 ]; then
    echo "✗ CMake configuration failed!"
    exit 1
fi

# 编译
echo "Building..."
make -j$(nproc)

# 检查编译是否成功
if [ $? -ne 0 ]; then
    echo "✗ Build failed!"
    exit 1
fi

echo "✓ Build completed successfully!"
echo ""
echo "Generated executables:"
echo "  - build/Lite_motion (main program)"
echo "  - build/test_grpc_client (test program)"
echo ""
echo "To run the main program:"
echo "  cd build && export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:\$LD_LIBRARY_PATH && ./Lite_motion"
echo ""
echo "To run the test program:"
echo "  ./test/run_grpc_test.sh"
echo "  ./test/quick_test.sh"
echo ""
echo "To clean build:"
echo "  cd build && make clean" 