#include "keyboard_controller.h"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    std::cout << "Continuous Keyboard Controller Test" << std::endl;
    std::cout << "Press WASD keys to test continuous detection." << std::endl;
    std::cout << "Press 'q' to quit." << std::endl;
    
    KeyboardController keyboard_controller;
    
    if (!keyboard_controller.Initialize()) {
        std::cerr << "Failed to initialize keyboard controller" << std::endl;
        return -1;
    }
    
    // Main loop
    while (true) {
        keyboard_controller.ProcessKeyInput();
        
        // Check for continuous key presses
        std::vector<char> pressed_keys = keyboard_controller.GetPressedKeys();
        
        if (!pressed_keys.empty()) {
            std::cout << "Pressed keys: ";
            for (char key : pressed_keys) {
                std::cout << "'" << key << "' ";
            }
            std::cout << std::endl;
        }
        
        // Check for quit
        if (keyboard_controller.IsKeyPressed('q')) {
            std::cout << "Quit key pressed. Exiting..." << std::endl;
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    return 0;
} 