# GRPC测试脚本使用说明

本目录包含了用于测试GRPC推理服务器的脚本和程序。

## 文件说明

- `build_grpc_test.sh` - 构建测试程序的脚本
- `run_grpc_test.sh` - 完整的GRPC测试脚本（详细输出）
- `quick_test.sh` - 快速测试脚本（简洁输出）
- `test_grpc_client_original.cpp` - 测试程序源代码

## 使用方法

### 1. 构建测试程序

```bash
# 方法1：使用构建脚本
./test/build_grpc_test.sh

# 方法2：手动构建
mkdir -p build && cd build && cmake .. && make test_grpc_client
```

### 2. 运行测试

#### 完整测试（推荐）
```bash
# 使用默认服务器地址 (localhost:50151)
./test/run_grpc_test.sh

# 指定服务器地址
./test/run_grpc_test.sh localhost:50151
```

#### 快速测试
```bash
# 使用默认服务器地址
./test/quick_test.sh

# 指定服务器地址
./test/quick_test.sh localhost:50151
```

### 3. 直接运行测试程序

```bash
# 使用默认服务器地址
./build/test_grpc_client

# 指定服务器地址
./build/test_grpc_client localhost:50151
```

## 测试内容

测试程序会验证以下功能：

1. ✅ **Mock RobotData创建** - 创建模拟的机器人数据
2. ✅ **Mock Action Data创建** - 创建模拟的动作数据
3. ✅ **Observation转换** - 将RobotData转换为65维observation
4. ✅ **GRPC连接** - 连接到推理服务器
5. ✅ **推理请求** - 发送observation并接收action
6. ✅ **数据转换** - 将action转换为RobotCmd
7. ✅ **数据验证** - 验证数据格式和合理性

## 预期输出

### 成功情况
```
✓ Test completed successfully!

Summary:
  - Successfully connected to GRPC server at localhost:50151
  - Observation data (65 values) generated correctly
  - Neural network inference completed
  - Action data (12 values) received and converted
  - RobotCmd format validated
```

### 失败情况
如果测试失败，脚本会提供详细的故障排除建议。

## 故障排除

### 常见问题

1. **服务器未运行**
   ```
   ✗ Failed to connect to GRPC server
   ```
   解决：启动你的GRPC推理服务器

2. **端口错误**
   ```
   ✗ Failed to connect to GRPC server
   ```
   解决：检查服务器端口，默认是50151

3. **模型维度不匹配**
   ```
   ✗ Inference request failed: Got invalid dimensions
   ```
   解决：确保模型期望65个observation值和1个desired_goal/achieved_goal

4. **库路径问题**
   ```
   ./test_grpc_client: version `GLIBCXX_3.4.32' not found
   ```
   解决：脚本已自动设置正确的库路径

### 调试技巧

1. **查看详细输出**
   ```bash
   ./test/run_grpc_test.sh
   ```

2. **检查服务器状态**
   ```bash
   netstat -tlnp | grep 50151
   ```

3. **测试网络连接**
   ```bash
   nc -zv localhost 50151
   ```

## 数据格式

### Observation数据 (65个值)
- 身体线速度 (3个值)
- 身体角速度 (3个值)
- 身体方向 (3个值)
- 命令值 (3个值)
- 关节位置偏差 (12个值)
- 关节速度 (12个值)
- 上一时刻动作 (12个值)
- 高度图 (16个值)
- 额外值 (1个值)

### Action数据 (12个值)
- 12个关节的位置命令

### GRPC请求格式
- `observation`: 65个float值
- `desired_goal`: 1个float值 (0.0)
- `achieved_goal`: 1个float值 (0.0)

## 环境要求

- Linux系统
- CMake 3.10+
- gRPC库
- OpenSSL
- Protobuf

脚本会自动处理库路径冲突问题。 