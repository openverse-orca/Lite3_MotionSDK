# 数据记录功能修改总结

## 修改概述

根据用户需求，将原来在控制台中打印的 observation、raw action 和 action 数据改为保存到 CSV 文件中，并提供了按分类的数据可视化功能。

## 主要修改内容

### 1. 新增文件

#### 头文件
- `include/data_logger.h`: 数据记录器类的头文件

#### 实现文件  
- `src/data_logger.cpp`: 数据记录器类的实现文件

#### 分析脚本
- `scripts/analyze_data.py`: 数据分析脚本，支持按分类可视化

#### 测试文件
- `test/test_data_logger.py`: 数据记录功能测试脚本

#### 文档
- `docs/data_logging_usage.md`: 数据记录功能使用说明
- `docs/observation_data_structure.md`: Observation数据结构详细说明
- `docs/modification_summary.md`: 本修改总结文档

### 2. 修改的文件

#### main.cpp
- 添加了 `#include "data_logger.h"`
- 在 gRPC 客户端初始化后添加了数据记录器初始化
- 将原来的打印语句替换为数据保存调用：
  - `cout << "row_observation: ..."` → `data_logger->SaveObservation(time_tick, observation)`
  - `cout << "row_action: ..."` → `data_logger->SaveRawAction(time_tick, last_action)`
  - `cout << "action: ..."` → `data_logger->SaveAction(time_tick, action)`
- 在程序结束时添加了数据记录器关闭代码

#### CMakeLists.txt
- 添加了 `src/data_logger.cpp` 到源文件列表

## 功能特点

### 1. 数据记录功能
- **自动文件命名**: 数据文件自动添加时间戳，避免覆盖
- **分离存储**: 观察数据、原始动作数据和处理后动作数据分别存储
- **CSV格式**: 标准CSV格式，便于在Excel、Python等工具中分析
- **数据完整性**: 程序正常退出时自动关闭文件，确保数据完整性

### 2. 数据分析功能
- **按分类可视化**: 将65维observation数据按8个分类进行可视化
- **多颜色显示**: 同一分类的数据用相同颜色显示，便于分析
- **统计信息**: 提供数据的基本统计信息
- **图片保存**: 自动保存分析图表为PNG文件

### 3. Observation数据分类
根据 `ConvertRobotDataToObservation` 函数的实现，将65维数据分为8类：

1. **Body Linear Velocity** (索引0-2): 身体线速度 (vx, vy, vz)
2. **Body Angular Velocity** (索引3-5): 身体角速度 (roll_vel, pitch_vel, yaw_vel)
3. **Body Orientation** (索引6-8): 身体方向 (roll, pitch, yaw)
4. **Command Values** (索引9-12): 命令值 (cmd_vx, cmd_vy, cmd_vz, cmd_yaw)
5. **Joint Position Offsets** (索引13-24): 关节位置偏移 (12个关节)
6. **Joint Velocities** (索引25-36): 关节速度 (12个关节)
7. **Previous Actions** (索引37-48): 上一时刻动作 (12个关节)
8. **Height Measurements** (索引49-64): 高度测量 (16个点)

## 生成的文件

运行程序后会在当前目录生成以下文件：
- `robot_data_YYYY-MM-DD_HH-MM-SS_observation.csv`: 观察数据文件
- `robot_data_YYYY-MM-DD_HH-MM-SS_raw_action.csv`: 原始动作数据文件
- `robot_data_YYYY-MM-DD_HH-MM-SS_action.csv`: 处理后动作数据文件

## 使用方法

### 1. 编译程序
```bash
./build.sh
```

### 2. 运行程序
```bash
./Lite_motion [gRPC服务器地址]
```

### 3. 分析数据
```bash
cd scripts
python3 analyze_data.py [基础文件名]
```

## 依赖库

数据分析脚本需要以下Python库：
```bash
pip install pandas matplotlib numpy
```

## 测试验证

- 编译测试: ✅ 通过
- 功能测试: ✅ 通过  
- 数据分析测试: ✅ 通过
- 图表生成测试: ✅ 通过

## 优势

1. **数据持久化**: 数据保存到文件，便于后续分析
2. **结构化存储**: 按数据类型分别存储，便于管理
3. **可视化分析**: 提供按分类的数据可视化，便于理解数据变化
4. **易于扩展**: 模块化设计，便于添加新的数据记录功能
5. **文档完善**: 提供了详细的使用说明和数据结构文档

## 注意事项

1. **文件大小**: 长时间运行可能生成较大文件，注意磁盘空间
2. **数据频率**: 数据记录频率为50Hz（每20个tick记录一次）
3. **时间戳**: 使用程序内部的tick计数，不是真实时间
4. **数据范围**: 程序需要运行到神经网络推理阶段（tick >= 10000）才开始记录数据 