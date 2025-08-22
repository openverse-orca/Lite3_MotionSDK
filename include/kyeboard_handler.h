#ifndef KEYBOARD_HANDLER_H
#define KEYBOARD_HANDLER_H
#include <SDL2/SDL.h>
#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>

class KeyboardHandler {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    std::unordered_map<SDL_Keycode, bool> keyStates;
    bool running;

    // 将 SDL 键码映射到可读名称
    std::unordered_map<SDL_Keycode, std::string> keyNameMap = {
        {SDLK_a, "a"}, {SDLK_b, "b"}, {SDLK_c, "c"}, {SDLK_d, "d"}, {SDLK_e, "e"},
        {SDLK_f, "f"}, {SDLK_g, "g"}, {SDLK_h, "h"}, {SDLK_i, "i"}, {SDLK_j, "j"},
        {SDLK_k, "k"}, {SDLK_l, "l"}, {SDLK_m, "m"}, {SDLK_n, "n"}, {SDLK_o, "o"},
        {SDLK_p, "p"}, {SDLK_q, "q"}, {SDLK_r, "r"}, {SDLK_s, "s"}, {SDLK_t, "t"},
        {SDLK_u, "u"}, {SDLK_v, "v"}, {SDLK_w, "w"}, {SDLK_x, "x"}, {SDLK_y, "y"},
        {SDLK_z, "z"},
        {SDLK_0, "0"}, {SDLK_1, "1"}, {SDLK_2, "2"}, {SDLK_3, "3"}, {SDLK_4, "4"},
        {SDLK_5, "5"}, {SDLK_6, "6"}, {SDLK_7, "7"}, {SDLK_8, "8"}, {SDLK_9, "9"},
        {SDLK_UP, "up"}, {SDLK_DOWN, "down"}, {SDLK_LEFT, "left"}, {SDLK_RIGHT, "right"},
        {SDLK_SPACE, "space"}, {SDLK_RETURN, "enter"}, {SDLK_ESCAPE, "escape"},
        {SDLK_LSHIFT, "left_shift"}, {SDLK_RSHIFT, "right_shift"},
        {SDLK_LCTRL, "left_ctrl"}, {SDLK_RCTRL, "right_ctrl"},
        {SDLK_LALT, "left_alt"}, {SDLK_RALT, "right_alt"},
        {SDLK_TAB, "tab"}, {SDLK_BACKSPACE, "backspace"},
        {SDLK_F1, "f1"}, {SDLK_F2, "f2"}, {SDLK_F3, "f3"}, {SDLK_F4, "f4"},
        {SDLK_F5, "f5"}, {SDLK_F6, "f6"}, {SDLK_F7, "f7"}, {SDLK_F8, "f8"},
        {SDLK_F9, "f9"}, {SDLK_F10, "f10"}, {SDLK_F11, "f11"}, {SDLK_F12, "f12"}
    };

public:
    KeyboardHandler() : window(nullptr), renderer(nullptr), running(false) {
        // 初始化 SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
            return;
        }
        
        // 创建一个小的可见窗口
        window = SDL_CreateWindow(
            "Keyboard Handler - Focus this window to capture keys", 
            SDL_WINDOWPOS_CENTERED, 
            SDL_WINDOWPOS_CENTERED, 
            300, 100, 
            SDL_WINDOW_SHOWN
        );
        
        if (window == nullptr) {
            std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            return;
        }
        
        // 创建渲染器
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (renderer == nullptr) {
            std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            return;
        }
        
        running = true;
        std::cout << "Keyboard handler initialized with visible SDL window" << std::endl;
        std::cout << "Please focus the SDL window to capture keyboard input" << std::endl;
        std::cout << "Press ESC in the SDL window to exit" << std::endl;
    }
    
    ~KeyboardHandler() {
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        SDL_Quit();
        std::cout << "Keyboard handler shutdown" << std::endl;
    }
    
    void update() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_KEYDOWN:
                    keyStates[event.key.keysym.sym] = true;
                    // 输出按键信息到控制台
                    if (keyNameMap.find(event.key.keysym.sym) != keyNameMap.end()) {
                        // std::cout << "Key pressed: " << keyNameMap[event.key.keysym.sym];
                        
                        // 显示修饰键状态
                        std::string modifiers;
                        if (SDL_GetModState() & KMOD_SHIFT) modifiers += "SHIFT+";
                        if (SDL_GetModState() & KMOD_CTRL) modifiers += "CTRL+";
                        if (SDL_GetModState() & KMOD_ALT) modifiers += "ALT+";
                        
                        if (!modifiers.empty()) {
                            modifiers.pop_back(); // 移除最后一个+
                            // std::cout << " (with modifiers: " << modifiers << ")";
                        }
                        // std::cout << std::endl;
                    }
                    break;
                case SDL_KEYUP:
                    keyStates[event.key.keysym.sym] = false;
                    break;
            }
        }
        
        // 更新窗口显示
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);
        
        // 显示提示信息
        // 这里可以添加文本渲染代码，但为了简化，我们只绘制一个简单的窗口
        
        SDL_RenderPresent(renderer);
    }
    
    bool IsKeyPressed(SDL_Keycode key) {
        return keyStates[key];
    }
    
    bool IsKeyPressed(const std::string& keyName) {
        // 查找键名对应的键码
        for (const auto& pair : keyNameMap) {
            if (pair.second == keyName) {
                return keyStates[pair.first];
            }
        }
        return false;
    }
    
    // 检查通用修饰键状态（不区分左右）
    bool IsShiftPressed() {
        return (SDL_GetModState() & KMOD_SHIFT);
    }
    
    bool IsCtrlPressed() {
        return (SDL_GetModState() & KMOD_CTRL);
    }
    
    bool IsAltPressed() {
        return (SDL_GetModState() & KMOD_ALT);
    }
    
    bool IsRunning() {
        return running;
    }
    
    // 获取所有当前按下的键
    std::vector<std::string> GetPressedKeys() {
        std::vector<std::string> pressedKeys;
        for (const auto& pair : keyStates) {
            if (pair.second && keyNameMap.find(pair.first) != keyNameMap.end()) {
                pressedKeys.push_back(keyNameMap[pair.first]);
            }
        }
        return pressedKeys;
    }
};

#endif // KEYBOARD_HANDLER_H