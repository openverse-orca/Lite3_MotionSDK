# IMU线速度计算器使用说明

## 概述

`VelocityCalculator` 是一个用于从IMU加速度数据实时计算线速度的工具类。它通过积分IMU的加速度数据来获得线速度，并包含多种滤波和补偿技术来提高精度。

## 主要特性

- **重力补偿**: 根据IMU的姿态角度自动移除重力影响
- **低通滤波**: 使用滑动窗口平均滤波减少噪声
- **零速度检测**: 自动检测静止状态并防止速度漂移
- **实时计算**: 支持高频实时数据流处理
- **位移计算**: 同时计算累积位移

## 使用方法

### 1. 基本使用

```cpp
#include "velocity_calculator.h"
#include "robot_types.h"

// 创建速度计算器实例
// 参数：滑动窗口大小=10，重力阈值=0.1 m/s²
VelocityCalculator velocity_calc(10, 0.1f);

// 在实时循环中使用
while (true) {
    // 获取IMU数据
    ImuData imu_data = getImuData();  // 从您的IMU获取数据
    
    // 计算时间间隔（秒）
    float dt = 0.001f;  // 假设1kHz更新频率
    
    // 更新速度计算器
    VelocityCalculator::Velocity3D velocity = velocity_calc.updateVelocity(imu_data, dt);
    
    // 获取当前线速度
    std::cout << "X轴线速度: " << velocity.vx << " m/s" << std::endl;
    std::cout << "Y轴线速度: " << velocity.vy << " m/s" << std::endl;
    std::cout << "Z轴线速度: " << velocity.vz << " m/s" << std::endl;
    
    // 获取累积位移
    VelocityCalculator::Displacement3D displacement = velocity_calc.getDisplacement();
    std::cout << "X轴位移: " << displacement.x << " m" << std::endl;
    std::cout << "Y轴位移: " << displacement.y << " m" << std::endl;
    std::cout << "Z轴位移: " << displacement.z << " m" << std::endl;
}
```

### 2. 参数调优

```cpp
// 创建自定义参数的速度计算器
VelocityCalculator velocity_calc(
    15,    // 滑动窗口大小（更大的窗口 = 更平滑但响应更慢）
    0.05f  // 重力阈值（更小的阈值 = 更敏感的零速度检测）
);

// 运行时调整参数
velocity_calc.setWindowSize(20);           // 调整滤波窗口大小
velocity_calc.setGravityThreshold(0.08f);  // 调整重力阈值
```

### 3. 重置状态

```cpp
// 重置计算器状态（速度、位移、滤波队列等）
velocity_calc.reset();
```

## 技术原理

### 1. 重力补偿

IMU测量的加速度包含重力分量，需要根据IMU的姿态角度进行补偿：

```cpp
// 重力在各轴的分量
gravity_x = g * sin(pitch)
gravity_y = -g * sin(roll) * cos(pitch)
gravity_z = -g * cos(roll) * cos(pitch)

// 去除重力后的加速度
acc_no_gravity = acc_measured - gravity
```

### 2. 速度积分

使用梯形积分方法计算速度：

```cpp
velocity_new = velocity_old + acceleration * dt
```

### 3. 零速度检测

当加速度幅值小于阈值时，认为机器人处于静止状态：

```cpp
acc_magnitude = sqrt(acc_x² + acc_y² + acc_z²)
is_zero_velocity = (acc_magnitude < threshold)
```

### 4. 低通滤波

使用滑动窗口平均滤波减少噪声：

```cpp
filtered_value = average(window_values)
```

## 注意事项

### 1. 精度限制

- **积分漂移**: 长时间积分会导致速度漂移，建议定期重置或使用其他传感器辅助
- **噪声影响**: IMU噪声会累积，滤波参数需要根据实际环境调整
- **重力补偿精度**: 重力补偿依赖于IMU姿态精度

### 2. 使用建议

- **更新频率**: 建议使用1kHz或更高的更新频率
- **参数调优**: 根据实际应用场景调整滤波窗口和重力阈值
- **定期重置**: 长时间运行时建议定期重置状态
- **多传感器融合**: 考虑结合其他传感器（如编码器、GPS等）提高精度

### 3. 适用场景

- 机器人运动控制
- 运动状态监测
- 简单的定位应用
- 实时速度反馈

## 示例程序

运行示例程序查看效果：

```bash
cd examples
g++ -o velocity_example velocity_example.cpp ../src/velocity_calculator.cpp -I../include
./velocity_example
```

示例程序会模拟以下运动场景：
1. 静止状态（0-0.2s）
2. 向前加速（0.2-0.4s）
3. 匀速运动（0.4-0.6s）
4. 减速停止（0.6-0.8s）
5. 静止状态（0.8-1.0s）

## 编译集成

将以下文件添加到您的项目中：

- `include/velocity_calculator.h` - 头文件
- `src/velocity_calculator.cpp` - 实现文件

在CMakeLists.txt中添加：

```cmake
add_library(velocity_calculator src/velocity_calculator.cpp)
target_include_directories(velocity_calculator PUBLIC include)
```

## 故障排除

### 常见问题

1. **速度漂移严重**
   - 降低重力阈值
   - 增加滤波窗口大小
   - 检查IMU校准

2. **响应太慢**
   - 减小滤波窗口大小
   - 提高更新频率

3. **零速度检测不准确**
   - 调整重力阈值
   - 检查IMU噪声水平

### 调试技巧

- 打印原始IMU数据和计算后的速度进行对比
- 监控零速度检测状态
- 观察滤波效果 