#!/bin/bash

# GRPC推理服务器测试程序运行脚本

echo "=== Running GRPC Inference Server Test ==="

# 检查可执行文件是否存在
if [ ! -f "build/test_grpc_client" ]; then
    echo "✗ Test executable not found!"
    echo "Please build the test program first using:"
    echo "  ./build_test.sh"
    exit 1
fi

# 获取服务器地址参数
SERVER_ADDRESS=${1:-"localhost:50151"}

echo "Using server address: $SERVER_ADDRESS"
echo ""

# 设置库路径以避免conda环境冲突
export LD_LIBRARY_PATH="/lib/x86_64-linux-gnu:/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH"

# 运行测试程序
echo "Starting test..."
build/test_grpc_client "$SERVER_ADDRESS"

# 检查运行结果
if [ $? -eq 0 ]; then
    echo ""
    echo "✓ Test completed successfully!"
else
    echo ""
    echo "✗ Test failed!"
    echo ""
    echo "Troubleshooting tips:"
    echo "1. Make sure the GRPC inference server is running on $SERVER_ADDRESS"
    echo "2. Check if the server supports the expected input/output format"
    echo "3. Verify network connectivity"
    exit 1
fi 