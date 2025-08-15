#!/bin/bash

# 快速GRPC测试脚本 - 用于快速验证服务器连接

echo "=== Quick GRPC Test ==="

# 获取服务器地址参数
SERVER_ADDRESS=${1:-"localhost:50151"}

echo "Testing connection to: $SERVER_ADDRESS"
echo ""

# 设置库路径
export LD_LIBRARY_PATH="/lib/x86_64-linux-gnu:/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH"

# 检查可执行文件
if [ ! -f "build/test_grpc_client" ]; then
    echo "✗ Test executable not found!"
    echo "Please build first: ./test/build_grpc_test.sh"
    exit 1
fi

# 快速连接测试
echo "Running quick test..."
timeout 30s build/test_grpc_client "$SERVER_ADDRESS" > /tmp/grpc_test_output.txt 2>&1
EXIT_CODE=$?

if [ $EXIT_CODE -eq 0 ]; then
    echo "✓ Quick test passed!"
    echo ""
    echo "Key results:"
    grep -E "(✓|Observation data size|Response action size|Test Completed Successfully)" /tmp/grpc_test_output.txt | head -5
else
    echo "✗ Quick test failed (exit code: $EXIT_CODE)"
    echo ""
    echo "Last few lines of output:"
    tail -10 /tmp/grpc_test_output.txt
fi

# 清理临时文件
rm -f /tmp/grpc_test_output.txt

exit $EXIT_CODE 