# GRPC推理服务器测试程序

这个测试程序用于验证GRPC推理服务器的完整通信流程，包括数据格式转换、服务器通信和推理结果处理。

## 功能特性

- ✅ 构造模拟的RobotData数据
- ✅ 将RobotData转换为Observation格式
- ✅ 通过GRPC与推理服务器通信
- ✅ 接收推理结果并转换为RobotAction
- ✅ 将RobotAction转换为RobotCmd
- ✅ 验证数据格式和通信流程

## 编译

## 方法1: 使用构建脚本（推荐）

```bash
# 在test目录下运行
./build_test.sh
```

## 方法2: 手动编译

```bash
# 在项目根目录下
mkdir build
cd build
cmake ..
make -j4
```

编译后会生成两个可执行文件：
- `Lite_motion` - 主程序（控制真机）
- `test_grpc_client` - 测试程序

## 使用方法

### 1. 启动推理服务器

首先确保您的GRPC推理服务器正在运行。默认服务器地址为 `localhost:50051`。

### 2. 运行测试程序

#### 方法1: 使用运行脚本（推荐）

```bash
# 在test目录下运行
./run_test.sh

# 或指定自定义服务器地址
./run_test.sh 192.168.1.100:50051
```

#### 方法2: 直接运行

```bash
# 在build目录下运行
cd build
./test_grpc_client

# 或指定自定义服务器地址
./test_grpc_client 192.168.1.100:50051
```

### 3. 查看测试结果

程序会输出详细的测试信息，包括：

```
=== GRPC Inference Server Test Program ===
This program tests the complete pipeline:
1. Create mock RobotData
2. Convert to Observation
3. Send to GRPC server
4. Receive inference result
5. Convert to RobotCmd
6. Validate data format

Using server address: localhost:50051

Step 1: Creating mock RobotData...
✓ Mock RobotData created successfully
  - Tick: 1000
  - IMU roll: 0 deg
  - IMU pitch: 0 deg
  - IMU yaw: 0 deg

Step 2: Converting RobotData to Observation...
✓ Observation created successfully
=== Observation Data ===
Data size: 45
IMU Data (9 values):
  Roll: 0 deg
  Pitch: 0 deg
  Yaw: 0 deg
  ...

Step 3: Creating GRPC client and connecting...
✓ Successfully connected to GRPC server

Step 4: Sending inference request...
✓ Inference request successful
  - Response action size: 12

Step 5: Converting response to RobotAction...
✓ RobotAction created successfully
=== Robot Action Data ===
Action size: 12
Joint Commands (12 values):
  FL Leg:
    Hip: 0 rad (0 deg)
    Thigh: -0.995 rad (-57 deg)
    Calf: 1.798 rad (103 deg)
  ...

Step 6: Converting RobotAction to RobotCmd...
✓ RobotCmd created successfully
=== Robot Command Data ===
  FL Leg:
    Hip:
      Position: 0 rad (0 deg)
      Velocity: 0 rad/s
      Torque: 0 Nm
      Kp: 20
      Kd: 0.5
    ...

Step 7: Validating data format...
=== Data Format Validation ===
✓ Observation data size is correct (65 values)
✓ Action data size is sufficient (≥12 values)
✓ IMU data consistency verified
✓ Joint position data consistency verified
✓ RobotCmd data is reasonable

=== Test Completed Successfully ===
All steps passed! The GRPC inference pipeline is working correctly.
```

## 数据格式说明

### RobotData结构
- **IMU数据**: 9个值（角度、角速度、加速度）
- **关节数据**: 12个关节的位置、速度、扭矩、温度
- **接触力数据**: 12个接触力值

### Observation格式
- **总大小**: 65个float值
- **身体线速度**: 索引0-2（3个值，使用VelocityCalculator计算）
- **身体角速度**: 索引3-5（3个值，从IMU获取）
- **身体方向**: 索引6-8（3个值，欧拉角，Yaw恒为0）
- **命令值**: 索引9-12（4个值，当前为0）
- **关节位置偏差**: 索引13-24（12个值，相对于中性位置的偏差）
- **关节速度**: 索引25-36（12个值）
- **上一时刻动作**: 索引37-48（12个值）
- **身体高度**: 索引49-64（16个值，当前为0）


### RobotAction格式
- **总大小**: 12个float值（每个关节的目标位置）
- **顺序**: FL_Hip, FL_Thigh, FL_Calf, FR_Hip, FR_Thigh, FR_Calf, HL_Hip, HL_Thigh, HL_Calf, HR_Hip, HR_Thigh, HR_Calf

### RobotCmd结构
- **每个关节**: 位置、速度、扭矩、Kp、Kd
- **默认增益**: Kp=20, Kd=0.5

## 故障排除

### 连接失败
```
✗ Failed to connect to GRPC server
Please make sure the inference server is running on localhost:50051
```
**解决方案**: 确保推理服务器正在运行，并检查服务器地址和端口。

### 推理失败
```
✗ Inference request failed: [错误信息]
```
**解决方案**: 检查服务器日志，确认模型加载正常，输入数据格式正确。

### 数据格式错误
```
✗ Observation data size is incorrect: X (expected 65)
```
**解决方案**: 检查`ConvertRobotDataToObservation`函数的实现。

## 注意事项

1. 测试程序使用模拟数据，不会控制真实机器人
2. 确保推理服务器支持65维的观察输入和12维的动作输出
3. 测试程序会验证数据一致性，确保转换过程正确
4. 如果服务器未运行，程序会优雅地退出并显示错误信息

## 扩展功能

您可以根据需要修改测试程序：

- 修改`CreateMockRobotData()`函数来测试不同的机器人状态
- 调整`ValidateDataFormat()`函数来添加更多验证规则
- 添加性能测试来测量通信延迟
- 实现批量测试来验证多个请求的处理 