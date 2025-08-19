# IMU重力补偿和坐标轴矫正解决方案

## 问题描述

根据你的观察，IMU数据存在以下问题：
1. **没有重力补偿**：IMU输出的加速度包含重力分量
2. **Z轴方向错误**：Z轴加速度为+G而不是-G
3. **X轴方向错误**：向前推动机器人时，X轴加速度为负值

## 解决方案概述

我设计了一个完整的IMU处理器（`ImuProcessor`）来解决这些问题：

### 1. 重力补偿
- 根据机器人的roll和pitch角度计算重力在各轴的分量
- 从原始加速度中减去重力分量
- 使用标准重力补偿公式

### 2. 坐标轴矫正
- X轴：乘以-1（矫正方向）
- Y轴：保持不变
- Z轴：乘以-1（翻转Z轴方向）

### 3. 数据处理流程
1. 坐标轴矫正
2. 重力补偿
3. 低通滤波
4. 零速度检测

## 文件结构

```
├── include/
│   ├── imu_processor.h          # IMU处理器头文件
│   └── robot_types.h            # 机器人数据类型定义
├── src/
│   ├── imu_processor.cpp        # IMU处理器实现
│   └── grpc_client.cpp          # 修改后的gRPC客户端
├── test/
│   └── test_imu_processor.cpp   # 测试程序
├── examples/
│   └── imu_processor_example.cpp # 使用示例
└── docs/
    ├── IMU_Processor_Guide.md   # 详细使用指南
    └── IMU_Solution_Summary.md  # 本文件
```

## 核心算法

### 重力补偿公式
```cpp
gx = g * sin(pitch)
gy = -g * sin(roll) * cos(pitch)
gz = g * cos(roll) * cos(pitch)

processed_acc = raw_acc - gravity_components
```

### 坐标轴矫正
```cpp
static constexpr float AXIS_CORRECTION_X = -1.0f;
static constexpr float AXIS_CORRECTION_Y = 1.0f;
static constexpr float AXIS_CORRECTION_Z = -1.0f;

corrected_acc.x = raw_acc.x * AXIS_CORRECTION_X;
corrected_acc.y = raw_acc.y * AXIS_CORRECTION_Y;
corrected_acc.z = raw_acc.z * AXIS_CORRECTION_Z;
```

## 集成到现有代码

### 在grpc_client.cpp中的修改
```cpp
// 替换原来的VelocityCalculator
// 使用IMU处理器处理加速度数据，进行重力补偿和坐标轴矫正
ImuProcessor::ProcessedAcceleration processed_acc = imu_processor.processAcceleration(robot_data.imu);

// 1. 身体线加速度 - 3个值（使用处理后的加速度）
obs.data.push_back(processed_acc.ax);
obs.data.push_back(processed_acc.ay);
obs.data.push_back(processed_acc.az);
```

## 使用方法

### 基本使用
```cpp
#include "imu_processor.h"

// 创建处理器实例
ImuProcessor processor(10, 0.1f);

// 处理IMU数据
ImuData imu_data;
// ... 设置IMU数据 ...
auto processed_acc = processor.processAcceleration(imu_data);
```

### 配置参数
```cpp
processor.setGravityThreshold(0.2f);     // 重力阈值
processor.setWindowSize(8);              // 滤波窗口大小
processor.enableGravityCompensation(true); // 启用重力补偿
processor.enableAxisCorrection(true);      // 启用坐标轴矫正
```

## 测试和验证

### 运行测试
```bash
cd build
make test_imu_processor
./test_imu_processor
```

### 测试内容
1. **重力补偿测试**：验证不同姿态角度下的重力补偿效果
2. **坐标轴矫正测试**：验证X轴和Z轴的方向矫正
3. **滤波功能测试**：验证低通滤波效果
4. **零速度检测测试**：验证静止状态检测

## 预期效果

### 处理前（原始IMU数据）
- 静止时：`[0.1, -0.05, 9.81]` m/s²
- 向前推动：`[-2.0, 0.0, 9.81]` m/s²
- 前倾15°：`[2.54, 0.0, 9.47]` m/s²

### 处理后（经过IMU处理器）
- 静止时：`[0.0, 0.0, 0.0]` m/s²（接近零）
- 向前推动：`[2.0, 0.0, 0.0]` m/s²（正值）
- 前倾15°：`[0.0, 0.0, 0.0]` m/s²（重力补偿后）

## 参数调优建议

### 重力阈值（gravity_threshold）
- **默认值**：0.1 m/s²
- **调优**：根据机器人运动特性调整

### 滤波窗口大小（window_size）
- **默认值**：10
- **调优**：平衡平滑性和响应速度

### 重力加速度值（gravity）
- **默认值**：9.81 m/s²
- **调优**：根据地理位置调整

## 优势

1. **模块化设计**：独立的IMU处理器类，易于集成和维护
2. **可配置性**：支持启用/禁用各种功能
3. **实时处理**：高效的处理算法，适合实时应用
4. **鲁棒性**：包含滤波和零速度检测，提高数据质量
5. **可扩展性**：易于添加新功能（如卡尔曼滤波）

## 注意事项

1. **角度单位**：IMU角度数据为度，内部自动转换为弧度
2. **时间同步**：处理器内部维护时间状态
3. **初始化**：首次调用时自动初始化
4. **重置**：可调用`reset()`重置状态

## 后续改进

可以考虑添加以下功能：
- 卡尔曼滤波
- 自适应重力补偿
- 温度补偿
- 动态阈值调整
- 多传感器融合

## 总结

这个解决方案提供了一个完整的IMU数据处理框架，能够有效解决重力补偿和坐标轴矫正问题。通过模块化设计和可配置参数，可以适应不同的应用场景和需求。 