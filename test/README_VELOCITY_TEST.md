# VelocityCalculator 测试说明

## 概述

本目录包含了VelocityCalculator（IMU线速度计算器）的所有测试和示例程序。

## 文件结构

### Velocity测试相关文件
- **`test_velocity_calculator.cpp`** - 功能测试程序
- **`velocity_example.cpp`** - 基本使用示例
- **`integration_example.cpp`** - 主程序集成示例
- **`build_velocity_test.sh`** - 编译和运行测试的脚本
- **`build_velocity_integration.sh`** - 编译和运行集成示例的脚本

### 原有GRPC测试文件（已重命名）
- **`test_grpc_client_original.cpp`** - 原GRPC客户端测试
- **`run_grpc_test.sh`** - 原GRPC测试运行脚本
- **`build_grpc_test.sh`** - 原GRPC测试编译脚本

## 使用方法

### 1. 运行Velocity功能测试
```bash
./build_velocity_test.sh
```
这个脚本会：
- 编译velocity_calculator.cpp
- 编译并运行test_velocity_calculator.cpp
- 编译并运行velocity_example.cpp

### 2. 运行Velocity集成示例
```bash
./build_velocity_integration.sh
```
这个脚本会：
- 编译velocity_calculator.cpp
- 编译并运行integration_example.cpp

### 3. 运行原有GRPC测试
```bash
./build_grpc_test.sh
./run_grpc_test.sh
```

## 测试内容

### Velocity功能测试
- 基本功能测试（静止状态检测）
- 加速度积分测试
- 重置功能测试
- 位移计算测试

### Velocity示例程序
- **velocity_example.cpp**: 演示基本的速度计算功能
- **integration_example.cpp**: 演示如何在主控制循环中集成速度计算器

## 编译输出

所有编译后的可执行文件都放在`build/`目录中：
- `build/test_velocity_calculator` - 功能测试程序
- `build/velocity_example` - 基本示例程序
- `build/velocity_integration` - 集成示例程序

## 注意事项

1. 确保在test目录中运行脚本
2. 脚本会自动创建build目录
3. 所有路径都是相对于test目录的
4. 测试程序会显示详细的输出信息 