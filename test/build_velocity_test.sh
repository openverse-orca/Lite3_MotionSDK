#!/bin/bash

echo "=== 编译VelocityCalculator测试程序 ==="

# 创建build目录
mkdir -p build

# 编译速度计算器库
echo "编译velocity_calculator.cpp..."
g++ -c -std=c++11 -I../include ../src/velocity_calculator.cpp -o build/velocity_calculator.o

if [ $? -ne 0 ]; then
    echo "编译velocity_calculator.cpp失败"
    exit 1
fi

# 编译测试程序
echo "编译test_velocity_calculator.cpp..."
g++ -std=c++11 -I../include test_velocity_calculator.cpp build/velocity_calculator.o -o build/test_velocity_calculator

if [ $? -ne 0 ]; then
    echo "编译测试程序失败"
    exit 1
fi

# 编译示例程序
echo "编译velocity_example.cpp..."
g++ -std=c++11 -I../include velocity_example.cpp build/velocity_calculator.o -o build/velocity_example

if [ $? -ne 0 ]; then
    echo "编译示例程序失败"
    exit 1
fi

echo "编译完成！"

# 运行测试
echo ""
echo "=== 运行测试 ==="
./build/test_velocity_calculator

if [ $? -eq 0 ]; then
    echo ""
    echo "=== 运行示例程序 ==="
    echo "按Ctrl+C停止示例程序"
    ./build/velocity_example
else
    echo "测试失败，不运行示例程序"
    exit 1
fi 