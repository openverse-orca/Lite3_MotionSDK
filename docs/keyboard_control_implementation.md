# 键盘控制实现文档

## 概述

本项目实现了支持持续按键检测的键盘控制器，可以用于机器人移动控制。

## 功能特性

### 1. 持续按键检测
- 支持检测按键的持续按下状态
- 提供按键状态查询接口
- 支持多按键同时检测

### 2. 机器人移动控制
- **W键**: 前进 (速度: 0.5)
- **S键**: 后退 (速度: 0.5)  
- **A键**: 左移 (速度: 0.5)
- **D键**: 右移 (速度: 0.5)
- **Q键**: 左转 (角度: -π/4)
- **E键**: 右转 (角度: π/4)
- **空格键**: 切换调试模式

## 类接口

### KeyboardController 类

#### 主要方法
```cpp
// 初始化键盘控制器
bool Initialize();

// 注册按键回调函数
void RegisterKeyCallback(char key, KeyCallback callback);

// 处理按键输入
void ProcessKeyInput();

// 检查按键是否持续按下
bool IsKeyPressed(char key);

// 检查按键是否刚被释放
bool IsKeyReleased(char key);

// 获取当前按下的所有按键
std::vector<char> GetPressedKeys();
```

## 使用示例

### 基本使用
```cpp
#include "keyboard_controller.h"

int main() {
    KeyboardController keyboard_controller;
    
    if (!keyboard_controller.Initialize()) {
        return -1;
    }
    
    while (true) {
        keyboard_controller.ProcessKeyInput();
        
        // 检查持续按键
        if (keyboard_controller.IsKeyPressed('w')) {
            // 处理前进
        }
        
        if (keyboard_controller.IsKeyPressed('s')) {
            // 处理后退
        }
    }
}
```

### 机器人移动控制
```cpp
// 在主循环中调用
UpdateRobotMoveCommand(keyboard_controller.get());

// robot_move_command 结构体包含：
struct RobotMoveCommand {
    float forward_speed;   // 前进速度
    float backward_speed;  // 后退速度
    float left_speed;      // 左移速度
    float right_speed;     // 右移速度
    float turn_speed;      // 转向速度
};
```

## 实现细节

### 1. 终端设置
- 使用非规范模式 (non-canonical mode)
- 禁用回显 (echo)
- 非阻塞输入

### 2. 按键状态跟踪
- 使用 `std::map<char, bool>` 跟踪按键状态
- 支持按键按下和释放检测
- 提供按键状态查询接口

### 3. 持续按键处理
- 在主循环中持续调用 `ProcessKeyInput()`
- 使用 `IsKeyPressed()` 检查按键状态
- 支持多按键组合

## 编译和运行

### 编译
```bash
./build.sh
```

### 运行主程序
```bash
cd build && export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH && ./Lite_motion
```

### 测试键盘控制器
```bash
cd build && ./test_keyboard_controller
```

## 注意事项

1. **终端兼容性**: 此实现基于Linux终端，在其他系统上可能需要修改
2. **按键冲突**: 某些按键组合可能产生冲突，需要根据具体需求调整
3. **性能考虑**: 持续按键检测会增加CPU使用率，建议适当调整检测频率
4. **权限要求**: 需要终端访问权限，在某些环境下可能需要特殊配置

## 扩展功能

### 可能的改进
1. 添加按键映射配置
2. 支持组合键检测
3. 添加按键事件历史记录
4. 实现按键宏功能
5. 添加配置文件支持 