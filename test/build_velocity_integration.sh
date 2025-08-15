#!/bin/bash

echo "=== 编译VelocityCalculator集成示例 ==="

# 创建build目录
mkdir -p build

# 编译速度计算器库
echo "编译velocity_calculator.cpp..."
g++ -c -std=c++11 -I../include ../src/velocity_calculator.cpp -o build/velocity_calculator.o

if [ $? -ne 0 ]; then
    echo "编译velocity_calculator.cpp失败"
    exit 1
fi

# 编译集成示例程序
echo "编译integration_example.cpp..."
g++ -std=c++11 -I../include integration_example.cpp build/velocity_calculator.o -o build/velocity_integration

if [ $? -ne 0 ]; then
    echo "编译集成示例程序失败"
    exit 1
fi

echo "编译完成！"

# 运行集成示例
echo ""
echo "=== 运行集成示例程序 ==="
echo "按Ctrl+C停止示例程序"
./build/velocity_integration 