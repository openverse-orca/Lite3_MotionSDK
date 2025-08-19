/// @file test_imu_processor.cpp
/// @author Assistant
/// @brief 测试IMU处理器的重力补偿和坐标轴矫正功能
/// @version 0.1
/// @date 2024-01-01
/// @copyright Copyright (c) 2024

#include "../include/imu_processor.h"
#include <iostream>
#include <iomanip>
#include <cmath>

void printAcceleration(const std::string& label, float ax, float ay, float az) {
    std::cout << std::fixed << std::setprecision(3);
    std::cout << label << ": [" << std::setw(8) << ax << ", " 
              << std::setw(8) << ay << ", " << std::setw(8) << az << "] m/s²" << std::endl;
}

void testGravityCompensation() {
    std::cout << "\n=== 测试重力补偿功能 ===" << std::endl;
    
    ImuProcessor processor(5, 0.1f);
    
    // 测试1：水平放置（roll=0, pitch=0）
    std::cout << "\n测试1：水平放置 (roll=0°, pitch=0°)" << std::endl;
    ImuData imu_data;
    imu_data.angle_roll = 0.0f;
    imu_data.angle_pitch = 0.0f;
    imu_data.acc_x = 0.0f;
    imu_data.acc_y = 0.0f;
    imu_data.acc_z = 9.81f;  // 原始Z轴为+G
    
    auto processed = processor.processAcceleration(imu_data);
    printAcceleration("原始加速度", imu_data.acc_x, imu_data.acc_y, imu_data.acc_z);
    printAcceleration("处理后加速度", processed.ax, processed.ay, processed.az);
    
    // 测试2：前倾30度（pitch=30°）
    std::cout << "\n测试2：前倾30度 (roll=0°, pitch=30°)" << std::endl;
    imu_data.angle_roll = 0.0f;
    imu_data.angle_pitch = 30.0f;
    imu_data.acc_x = 4.905f;  // 9.81 * sin(30°)
    imu_data.acc_y = 0.0f;
    imu_data.acc_z = 8.496f;  // 9.81 * cos(30°)
    
    processed = processor.processAcceleration(imu_data);
    printAcceleration("原始加速度", imu_data.acc_x, imu_data.acc_y, imu_data.acc_z);
    printAcceleration("处理后加速度", processed.ax, processed.ay, processed.az);
    
    // 测试3：左倾45度（roll=45°）
    std::cout << "\n测试3：左倾45度 (roll=45°, pitch=0°)" << std::endl;
    imu_data.angle_roll = 45.0f;
    imu_data.angle_pitch = 0.0f;
    imu_data.acc_x = 0.0f;
    imu_data.acc_y = -6.936f;  // -9.81 * sin(45°)
    imu_data.acc_z = 6.936f;   // 9.81 * cos(45°)
    
    processed = processor.processAcceleration(imu_data);
    printAcceleration("原始加速度", imu_data.acc_x, imu_data.acc_y, imu_data.acc_z);
    printAcceleration("处理后加速度", processed.ax, processed.ay, processed.az);
}

void testAxisCorrection() {
    std::cout << "\n=== 测试坐标轴矫正功能 ===" << std::endl;
    
    ImuProcessor processor(5, 0.1f);
    
    // 禁用重力补偿，只测试坐标轴矫正
    processor.enableGravityCompensation(false);
    
    // 测试向前推动（X轴负值）
    std::cout << "\n测试向前推动 (X轴负值，禁用重力补偿)" << std::endl;
    ImuData imu_data;
    imu_data.angle_roll = 0.0f;
    imu_data.angle_pitch = 0.0f;
    imu_data.acc_x = -2.0f;  // 向前推动，原始为负值
    imu_data.acc_y = 0.0f;
    imu_data.acc_z = 9.81f;
    
    auto processed = processor.processAcceleration(imu_data);
    printAcceleration("原始加速度", imu_data.acc_x, imu_data.acc_y, imu_data.acc_z);
    printAcceleration("处理后加速度", processed.ax, processed.ay, processed.az);
    
    // 测试Z轴翻转
    std::cout << "\n测试Z轴翻转 (禁用重力补偿)" << std::endl;
    imu_data.acc_x = 0.0f;
    imu_data.acc_y = 0.0f;
    imu_data.acc_z = 9.81f;  // 原始Z轴为+G
    
    processed = processor.processAcceleration(imu_data);
    printAcceleration("原始加速度", imu_data.acc_x, imu_data.acc_y, imu_data.acc_z);
    printAcceleration("处理后加速度", processed.ax, processed.ay, processed.az);
    
    // 重新启用重力补偿
    processor.enableGravityCompensation(true);
    
    // 测试完整流程：重力补偿 + 坐标轴矫正
    std::cout << "\n测试完整流程：重力补偿 + 坐标轴矫正" << std::endl;
    imu_data.angle_roll = 0.0f;
    imu_data.angle_pitch = 0.0f;
    imu_data.acc_x = -2.0f;  // 向前推动，原始为负值
    imu_data.acc_y = 0.0f;
    imu_data.acc_z = 9.81f;
    
    processed = processor.processAcceleration(imu_data);
    printAcceleration("原始加速度", imu_data.acc_x, imu_data.acc_y, imu_data.acc_z);
    printAcceleration("处理后加速度", processed.ax, processed.ay, processed.az);
}

void testFiltering() {
    std::cout << "\n=== 测试滤波功能 ===" << std::endl;
    
    ImuProcessor processor(3, 0.1f);  // 使用较小的窗口进行测试
    
    ImuData imu_data;
    imu_data.angle_roll = 0.0f;
    imu_data.angle_pitch = 0.0f;
    
    std::cout << "\n输入带噪声的X轴加速度序列：" << std::endl;
    float noise_values[] = {1.0f, 1.2f, 0.8f, 1.1f, 0.9f, 1.0f};
    
    for (int i = 0; i < 6; ++i) {
        imu_data.acc_x = noise_values[i];
        imu_data.acc_y = 0.0f;
        imu_data.acc_z = 9.81f;
        
        auto processed = processor.processAcceleration(imu_data);
        std::cout << "步骤 " << (i+1) << ": 原始=" << noise_values[i] 
                  << ", 滤波后=" << processed.ax << std::endl;
    }
}

void testZeroVelocityDetection() {
    std::cout << "\n=== 测试零速度检测功能 ===" << std::endl;
    
    ImuProcessor processor(5, 0.5f);  // 设置较小的阈值
    
    ImuData imu_data;
    imu_data.angle_roll = 0.0f;
    imu_data.angle_pitch = 0.0f;
    
    std::cout << "\n模拟静止状态：" << std::endl;
    for (int i = 0; i < 10; ++i) {
        imu_data.acc_x = 0.1f;  // 很小的加速度
        imu_data.acc_y = 0.1f;
        imu_data.acc_z = 9.81f;
        
        auto processed = processor.processAcceleration(imu_data);
        std::cout << "步骤 " << (i+1) << ": [" << processed.ax << ", " 
                  << processed.ay << ", " << processed.az << "]" << std::endl;
    }
}

int main() {
    std::cout << "IMU处理器测试程序" << std::endl;
    std::cout << "==================" << std::endl;
    
    testGravityCompensation();
    testAxisCorrection();
    testFiltering();
    testZeroVelocityDetection();
    
    std::cout << "\n测试完成！" << std::endl;
    return 0;
} 