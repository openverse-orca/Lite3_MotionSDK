#include "keyboard_controller.h"
#include <iostream>
#include <chrono>
#include <thread>

void OnSpaceKeyPressed() {
    std::cout << "Space key pressed!" << std::endl;
}

void OnQuitKeyPressed() {
    std::cout << "Quit key pressed. Exiting..." << std::endl;
    exit(0);
}

void OnHelpKeyPressed() {
    std::cout << "\n=== Help ===" << std::endl;
    std::cout << "SPACE - Test space key" << std::endl;
    std::cout << "'q'   - Quit" << std::endl;
    std::cout << "'h'   - Show help" << std::endl;
    std::cout << "===========\n" << std::endl;
}

int main() {
    std::cout << "Keyboard Controller Test" << std::endl;
    std::cout << "Press keys to test. Press 'q' to quit." << std::endl;
    
    KeyboardController keyboard_controller;
    
    if (!keyboard_controller.Initialize()) {
        std::cerr << "Failed to initialize keyboard controller" << std::endl;
        return -1;
    }
    
    // Register callbacks
    keyboard_controller.RegisterKeyCallback(' ', OnSpaceKeyPressed);
    keyboard_controller.RegisterKeyCallback('q', OnQuitKeyPressed);
    keyboard_controller.RegisterKeyCallback('h', OnHelpKeyPressed);
    
    // Main loop
    while (true) {
        keyboard_controller.ProcessKeyInput();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    return 0;
} 