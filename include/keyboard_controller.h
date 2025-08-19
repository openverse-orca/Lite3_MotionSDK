#ifndef KEYBOARD_CONTROLLER_H
#define KEYBOARD_CONTROLLER_H

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <functional>
#include <vector> // Added for std::vector
#include "robot_types.h"

/**
 * @brief 键盘控制器类，提供非阻塞键盘输入功能，支持持续按键检测
 */
class KeyboardController {
public:
    /**
     * @brief 按键回调函数类型
     */
    using KeyCallback = std::function<void()>;
    
    /**
     * @brief 构造函数
     */
    KeyboardController();
    
    /**
     * @brief 析构函数
     */
    ~KeyboardController();
    
    /**
     * @brief 初始化键盘控制器
     * @return 是否初始化成功
     */
    bool Initialize();
    
    /**
     * @brief 注册按键回调函数
     * @param key 按键字符
     * @param callback 回调函数
     */
    void RegisterKeyCallback(char key, KeyCallback callback);
    
    /**
     * @brief 检查是否有按键输入
     * @return 是否有按键输入
     */
    bool HasKeyInput();
    
    /**
     * @brief 获取按键输入
     * @return 按键字符，如果没有输入返回0
     */
    char GetKeyInput();
    
    /**
     * @brief 处理按键输入
     */
    void ProcessKeyInput();
    
    /**
     * @brief 检查按键是否持续按下
     * @param key 按键字符
     * @return 是否持续按下
     */
    bool IsKeyPressed(char key);
    
    /**
     * @brief 检查按键是否刚被释放
     * @param key 按键字符
     * @return 是否刚被释放
     */
    bool IsKeyReleased(char key);
    
    /**
     * @brief 获取当前按下的所有按键
     * @return 当前按下的按键集合
     */
    std::vector<char> GetPressedKeys();
    
    /**
     * @brief 恢复终端设置
     */
    void RestoreTerminal();

private:
    struct termios old_termios_;  ///< 原始终端设置
    struct termios new_termios_;  ///< 新的终端设置
    std::map<char, KeyCallback> key_callbacks_;  ///< 按键回调函数映射
    std::map<char, bool> key_states_;  ///< 按键状态映射 (true=按下, false=释放)
    std::map<char, bool> key_just_released_;  ///< 按键刚释放状态映射
    bool initialized_;  ///< 是否已初始化
};

#endif // KEYBOARD_CONTROLLER_H 