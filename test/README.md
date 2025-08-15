# 测试模块

这个目录包含了用于验证GRPC推理服务器通信流程的测试程序和工具。

## 文件结构

```
test/
├── README.md              # 本文件
├── README_TEST.md         # 详细测试说明文档
├── test_grpc_client.cpp   # 测试程序源代码
├── build_test.sh          # 构建脚本
├── run_test.sh           # 运行脚本
└── CMakeLists.txt        # CMake配置文件
```

## 快速开始

### 1. 构建测试程序

```bash
# 在test目录下
./build_test.sh
```

### 2. 运行测试

```bash
# 使用默认服务器地址 (localhost:50051)
./run_test.sh

# 或指定自定义服务器地址
./run_test.sh 192.168.1.100:50051
```

## 测试功能

- ✅ 构造模拟的RobotData数据
- ✅ 将RobotData转换为Observation格式
- ✅ 通过GRPC与推理服务器通信
- ✅ 接收推理结果并转换为RobotAction
- ✅ 将RobotAction转换为RobotCmd
- ✅ 验证数据格式和通信流程

## 详细文档

请查看 [README_TEST.md](README_TEST.md) 获取详细的测试说明、数据格式说明和故障排除指南。

## 注意事项

1. 测试程序使用模拟数据，不会控制真实机器人
2. 确保推理服务器支持45维的观察输入和12维的动作输出
3. 测试程序会验证数据一致性，确保转换过程正确
4. 如果服务器未运行，程序会优雅地退出并显示错误信息 