# IMU处理器使用指南

## 概述

IMU处理器（`ImuProcessor`）是一个专门用于处理IMU数据的工具类，主要解决以下问题：

1. **重力补偿**：根据机器人的姿态角度（roll、pitch）自动去除重力加速度的影响
2. **坐标轴矫正**：修正IMU坐标轴方向，使其符合标准坐标系
3. **低通滤波**：减少噪声影响，提高数据质量
4. **零速度检测**：在静止状态下自动将加速度归零

## 问题背景

根据你的观察，IMU数据存在以下特点：
- 没有重力补偿
- Z轴加速度为+G而不是-G
- 向前推动机器人时，X轴加速度为负值

## 解决方案

### 1. 重力补偿算法

重力补偿基于以下公式：

```
gx = g * sin(pitch)
gy = -g * sin(roll) * cos(pitch)
gz = g * cos(roll) * cos(pitch)
```

其中：
- `g` = 9.81 m/s²（重力加速度）
- `roll` = 横滚角（弧度）
- `pitch` = 俯仰角（弧度）

处理后的加速度 = 原始加速度 - 重力分量

### 2. 坐标轴矫正

根据你的观察，应用以下矫正：

```cpp
// 坐标轴矫正参数
static constexpr float AXIS_CORRECTION_X = -1.0f;  // X轴方向矫正
static constexpr float AXIS_CORRECTION_Y = 1.0f;   // Y轴方向矫正
static constexpr float AXIS_CORRECTION_Z = -1.0f;  // Z轴方向矫正（翻转Z轴）
```

### 3. 处理流程

1. **坐标轴矫正**：首先修正坐标轴方向
2. **重力补偿**：根据姿态角度去除重力影响
3. **低通滤波**：使用滑动窗口平均滤波
4. **零速度检测**：检测静止状态并归零

## 使用方法

### 基本使用

```cpp
#include "imu_processor.h"

// 创建IMU处理器实例
ImuProcessor processor(10, 0.1f);  // 窗口大小=10, 重力阈值=0.1

// 处理IMU数据
ImuData imu_data;
// ... 设置IMU数据 ...
auto processed_acc = processor.processAcceleration(imu_data);

// 使用处理后的加速度
float ax = processed_acc.ax;
float ay = processed_acc.ay;
float az = processed_acc.az;
```

### 配置参数

```cpp
// 设置重力阈值（用于零速度检测）
processor.setGravityThreshold(0.2f);

// 设置滤波窗口大小
processor.setWindowSize(15);

// 设置重力加速度值
processor.setGravity(9.81f);

// 启用/禁用重力补偿
processor.enableGravityCompensation(true);

// 启用/禁用坐标轴矫正
processor.enableAxisCorrection(true);
```

### 在grpc_client.cpp中的集成

已经修改了`ConvertRobotDataToObservation`函数：

```cpp
// 使用IMU处理器处理加速度数据，进行重力补偿和坐标轴矫正
ImuProcessor::ProcessedAcceleration processed_acc = imu_processor.processAcceleration(robot_data.imu);

// 1. 身体线加速度 - 3个值（使用处理后的加速度）
obs.data.push_back(processed_acc.ax);
obs.data.push_back(processed_acc.ay);
obs.data.push_back(processed_acc.az);
```

## 测试

运行测试程序验证功能：

```bash
cd build
make test_imu_processor
./test_imu_processor
```

测试包括：
- 重力补偿测试（不同姿态角度）
- 坐标轴矫正测试（X轴负值、Z轴翻转）
- 滤波功能测试
- 零速度检测测试

## 参数调优

### 重力阈值（gravity_threshold）
- **默认值**：0.1 m/s²
- **调优建议**：
  - 如果机器人经常误判为静止：增大阈值
  - 如果机器人运动时被误判为静止：减小阈值

### 滤波窗口大小（window_size）
- **默认值**：10
- **调优建议**：
  - 需要更平滑的数据：增大窗口
  - 需要更快的响应：减小窗口

### 重力加速度值（gravity）
- **默认值**：9.81 m/s²
- **调优建议**：
  - 根据实际地理位置调整
  - 可以通过校准获得更精确的值

## 注意事项

1. **角度单位**：IMU角度数据为度，内部自动转换为弧度
2. **时间同步**：处理器内部维护时间状态，无需外部时间同步
3. **初始化**：首次调用时会自动初始化
4. **重置**：调用`reset()`可以重置所有状态

## 故障排除

### 问题1：处理后加速度仍然很大
- 检查姿态角度是否正确
- 确认重力补偿是否启用
- 验证重力加速度值设置

### 问题2：滤波效果不明显
- 增大滤波窗口大小
- 检查输入数据是否有异常值

### 问题3：零速度检测不准确
- 调整重力阈值
- 检查加速度数据的噪声水平

## 扩展功能

可以根据需要添加以下功能：
- 卡尔曼滤波
- 自适应重力补偿
- 温度补偿
- 动态阈值调整 