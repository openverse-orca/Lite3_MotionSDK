#!/bin/bash

# GRPC推理服务器测试程序运行脚本

echo "=== Running GRPC Inference Server Test ==="

# 检查可执行文件是否存在
if [ ! -f "build/test_grpc_client" ]; then
    echo "✗ Test executable not found!"
    echo "Please build the test program first using:"
    echo "  ./test/build_grpc_test.sh"
    echo "or manually:"
    echo "  mkdir -p build && cd build && cmake .. && make test_grpc_client"
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
echo "Command: build/test_grpc_client \"$SERVER_ADDRESS\""
echo ""

# 检查服务器连接性
echo "Checking server connectivity..."
if ! nc -z $(echo $SERVER_ADDRESS | cut -d: -f1) $(echo $SERVER_ADDRESS | cut -d: -f2) 2>/dev/null; then
    echo "⚠️  Warning: Cannot connect to server $SERVER_ADDRESS"
    echo "   Make sure the GRPC inference server is running"
    echo ""
fi

build/test_grpc_client "$SERVER_ADDRESS"

# 检查运行结果
EXIT_CODE=$?
echo ""

if [ $EXIT_CODE -eq 0 ]; then
    echo "✓ Test completed successfully!"
    echo ""
    echo "Summary:"
    echo "  - Successfully connected to GRPC server at $SERVER_ADDRESS"
    echo "  - Observation data (65 values) generated correctly"
    echo "  - Neural network inference completed"
    echo "  - Action data (12 values) received and converted"
    echo "  - RobotCmd format validated"
else
    echo "✗ Test failed with exit code: $EXIT_CODE"
    echo ""
    echo "Troubleshooting tips:"
    echo "1. Make sure the GRPC inference server is running on $SERVER_ADDRESS"
    echo "2. Check if the server supports the expected input/output format"
    echo "3. Verify network connectivity"
    echo "4. Check server logs for detailed error messages"
    echo "5. Ensure the model expects 65 observation values and 1 desired_goal/achieved_goal each"
    echo ""
    echo "Common issues:"
    echo "  - Server not running: Start your inference server"
    echo "  - Wrong port: Check if server is on port 50151"
    echo "  - Model mismatch: Ensure model expects correct input dimensions"
    exit 1
fi 