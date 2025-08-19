#include "keyboard_controller.h"
#include <iostream>
#include <sys/ioctl.h>
#include <termios.h>

KeyboardController::KeyboardController() : initialized_(false) {
}

KeyboardController::~KeyboardController() {
    RestoreTerminal();
}

bool KeyboardController::Initialize() {
    // 获取当前终端设置
    if (tcgetattr(STDIN_FILENO, &old_termios_) != 0) {
        std::cerr << "Failed to get terminal attributes" << std::endl;
        return false;
    }
    
    // 复制设置
    new_termios_ = old_termios_;
    
    // 设置非规范模式（非阻塞）
    new_termios_.c_lflag &= ~(ICANON | ECHO);
    new_termios_.c_cc[VMIN] = 0;   // 最小输入字符数
    new_termios_.c_cc[VTIME] = 0;  // 超时时间
    
    // 应用新设置
    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_termios_) != 0) {
        std::cerr << "Failed to set terminal attributes" << std::endl;
        return false;
    }
    
    initialized_ = true;
    std::cout << "Keyboard controller initialized for continuous key detection." << std::endl;
    return true;
}

void KeyboardController::RegisterKeyCallback(char key, KeyCallback callback) {
    key_callbacks_[key] = callback;
}

bool KeyboardController::HasKeyInput() {
    if (!initialized_) return false;
    
    fd_set readfds;
    struct timeval timeout;
    
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    
    int result = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);
    return result > 0;
}

char KeyboardController::GetKeyInput() {
    if (!HasKeyInput()) {
        return 0;
    }
    
    char ch;
    if (read(STDIN_FILENO, &ch, 1) == 1) {
        return ch;
    }
    
    return 0;
}

void KeyboardController::ProcessKeyInput() {
    // 重置刚释放状态
    for (auto& pair : key_just_released_) {
        pair.second = false;
    }
    
    // 处理所有可用的输入
    while (HasKeyInput()) {
        char key = GetKeyInput();
        if (key != 0) {
            // 更新按键状态
            bool was_pressed = key_states_[key];
            key_states_[key] = true;
            
            // 如果是新按下的按键，触发回调
            if (!was_pressed) {
                auto it = key_callbacks_.find(key);
                if (it != key_callbacks_.end()) {
                    it->second();
                }
            }
        }
    }
    
    // 使用超时机制检测按键释放
    // 由于主循环是5ms，200ms = 40次循环
    static std::map<char, int> key_press_time;
    static int frame_counter = 0;
    frame_counter++;
    
    // 更新按键按下时间
    for (auto& pair : key_states_) {
        if (pair.second) {
            if (key_press_time.find(pair.first) == key_press_time.end()) {
                key_press_time[pair.first] = frame_counter;
            }
        }
    }
    
    // 检查按键释放 - 200ms超时
    const int RELEASE_TIMEOUT = 40; // 200ms = 40 * 5ms
    std::vector<char> keys_to_release;
    
    for (const auto& pair : key_states_) {
        if (pair.second) {
            auto it = key_press_time.find(pair.first);
            if (it != key_press_time.end()) {
                // 如果按键按下时间超过200ms没有新的输入事件，认为它被释放了
                if (frame_counter - it->second > RELEASE_TIMEOUT) {
                    keys_to_release.push_back(pair.first);
                }
            }
        }
    }
    
    // 释放超时的按键
    for (char key : keys_to_release) {
        key_states_[key] = false;
        key_just_released_[key] = true;
        key_press_time.erase(key);
    }
}

bool KeyboardController::IsKeyPressed(char key) {
    return key_states_[key];
}

bool KeyboardController::IsKeyReleased(char key) {
    return key_just_released_[key];
}

std::vector<char> KeyboardController::GetPressedKeys() {
    std::vector<char> pressed_keys;
    for (const auto& pair : key_states_) {
        if (pair.second) {
            pressed_keys.push_back(pair.first);
        }
    }
    return pressed_keys;
}

void KeyboardController::RestoreTerminal() {
    if (initialized_) {
        tcsetattr(STDIN_FILENO, TCSANOW, &old_termios_);
        initialized_ = false;
    }
} 