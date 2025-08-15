# Observation 数据结构说明

## 概述

Observation 数据是机器人的状态观察数据，总共包含65个维度，用于神经网络的输入。这些数据按照特定的顺序组织，每个维度都有明确的物理意义。

## 数据结构

### 1. 身体线速度 (Body Linear Velocity) - 索引 0-2
- **obs_0**: vx - 身体在x方向的线速度 (m/s)
- **obs_1**: vy - 身体在y方向的线速度 (m/s)  
- **obs_2**: vz - 身体在z方向的线速度 (m/s)

**说明**: 这些值通过VelocityCalculator计算得出，比直接从IMU获取更精确。

### 2. 身体角速度 (Body Angular Velocity) - 索引 3-5
- **obs_3**: roll_vel - 身体绕x轴的角速度 (rad/s)
- **obs_4**: pitch_vel - 身体绕y轴的角速度 (rad/s)
- **obs_5**: yaw_vel - 身体绕z轴的角速度 (rad/s)

**说明**: 直接从IMU传感器获取。

### 3. 身体方向 (Body Orientation) - 索引 6-8
- **obs_6**: roll - 身体绕x轴的欧拉角 (rad)
- **obs_7**: pitch - 身体绕y轴的欧拉角 (rad)
- **obs_8**: yaw - 身体绕z轴的欧拉角 (rad) - 通常为0

**说明**: 从IMU获取欧拉角，yaw角在训练时通常设为0（全局坐标系）。

### 4. 命令值 (Command Values) - 索引 9-12
- **obs_9**: cmd_vx - 期望的x方向线速度 (m/s)
- **obs_10**: cmd_vy - 期望的y方向线速度 (m/s)
- **obs_11**: cmd_vz - 期望的z方向线速度 (m/s)
- **obs_12**: cmd_yaw - 期望的z轴角速度 (rad/s)

**说明**: 当前实现中这些值都设为0，实际应该从外部传入运动命令。

### 5. 关节位置偏移 (Joint Position Offsets) - 索引 13-24
- **obs_13-15**: FL (Front Left) 腿的3个关节位置偏移
  - obs_13: FL_HipX_joint 位置偏移 (rad)
  - obs_14: FL_HipY_joint 位置偏移 (rad)
  - obs_15: FL_Knee_joint 位置偏移 (rad)
- **obs_16-18**: FR (Front Right) 腿的3个关节位置偏移
- **obs_19-21**: HL (Hind Left) 腿的3个关节位置偏移
- **obs_22-24**: HR (Hind Right) 腿的3个关节位置偏移

**说明**: 相对于中性位置（站立姿态）的偏移量。中性位置定义为：
```cpp
const std::vector<float> neutral_joint_values = {
    0.0f, -0.8f, 1.5f,  // FL: hip, thigh, calf
    0.0f, -0.8f, 1.5f,  // FR: hip, thigh, calf
    0.0f, -1.0f, 1.5f,  // HL: hip, thigh, calf
    0.0f, -1.0f, 1.5f   // HR: hip, thigh, calf
};
```

### 6. 关节速度 (Joint Velocities) - 索引 25-36
- **obs_25-27**: FL 腿的3个关节速度 (rad/s)
- **obs_28-30**: FR 腿的3个关节速度 (rad/s)
- **obs_31-33**: HL 腿的3个关节速度 (rad/s)
- **obs_34-36**: HR 腿的3个关节速度 (rad/s)

**说明**: 直接从关节编码器获取的角速度。

### 7. 上一时刻动作 (Previous Actions) - 索引 37-48
- **obs_37-39**: FL 腿的3个关节上一时刻动作
- **obs_40-42**: FR 腿的3个关节上一时刻动作
- **obs_43-45**: HL 腿的3个关节上一时刻动作
- **obs_46-48**: HR 腿的3个关节上一时刻动作

**说明**: 神经网络上一时刻输出的动作值，用于提供动作连续性信息。

### 8. 高度测量 (Height Measurements) - 索引 49-64
- **obs_49-64**: 16个高度测量点 (m)

**说明**: 从激光雷达获取的地面高度信息，当前实现中全部设为0。

## 数据生成过程

1. **时间计算**: 使用RobotData中的tick计算时间间隔
2. **速度计算**: 通过VelocityCalculator计算精确的线速度
3. **数据组装**: 按照上述顺序将各个数据源的数据组装成65维向量
4. **数据预处理**: 关节位置计算相对于中性位置的偏移

## 数据可视化

在分析脚本中，observation数据按照以下分类进行可视化：

1. **Body Linear Velocity** (蓝色): 身体线速度
2. **Body Angular Velocity** (红色): 身体角速度  
3. **Body Orientation** (绿色): 身体方向
4. **Command Values** (橙色): 命令值
5. **Joint Position Offsets** (紫色): 关节位置偏移
6. **Joint Velocities** (棕色): 关节速度
7. **Previous Actions** (粉色): 上一时刻动作
8. **Height Measurements** (灰色): 高度测量

每个分类在同一张子图中用不同颜色的线段显示，便于分析同类数据的变化趋势。

## 使用建议

1. **数据完整性**: 确保所有65个维度都有有效值
2. **数据范围**: 注意各维度的数值范围，异常值可能表示传感器故障
3. **时间同步**: 确保时间戳的准确性，这对速度计算很重要
4. **数据预处理**: 在训练前可能需要额外的缩放和噪声处理 