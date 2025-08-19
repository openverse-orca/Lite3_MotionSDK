/// @file imu_processor_example.cpp
/// @author Assistant
/// @brief IMU处理器使用示例
/// @version 0.1
/// @date 2024-01-01
/// @copyright Copyright (c) 2024

#include "../include/imu_processor.h"
#include "../include/robot_types.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>

void printAcceleration(const std::string& label, float ax, float ay, float az) {
    std::cout << std::fixed << std::setprecision(3);
    std::cout << label << ": [" << std::setw(8) << ax << ", " 
              << std::setw(8) << ay << ", " << std::setw(8) << az << "] m/s²" << std::endl;
}

int main() {
    std::cout << "IMU处理器使用示例" << std::endl;
    std::cout << "==================" << std::endl;
    
    // 创建IMU处理器实例
    ImuProcessor processor(10, 0.1f);
    
    // 配置处理器参数
    processor.setGravityThreshold(0.2f);  // 设置重力阈值
    processor.setWindowSize(8);           // 设置滤波窗口大小
    processor.enableGravityCompensation(true);  // 启用重力补偿
    processor.enableAxisCorrection(true);       // 启用坐标轴矫正
    
    std::cout << "\n处理器配置完成，开始模拟IMU数据..." << std::endl;
    
    // 模拟机器人运动场景
    std::cout << "\n场景1：机器人静止（水平放置）" << std::endl;
    for (int i = 0; i < 5; ++i) {
        ImuData imu_data;
        imu_data.angle_roll = 0.0f;
        imu_data.angle_pitch = 0.0f;
        imu_data.acc_x = 0.1f;   // 轻微噪声
        imu_data.acc_y = -0.05f; // 轻微噪声
        imu_data.acc_z = 9.81f;  // 重力加速度
        
        auto processed = processor.processAcceleration(imu_data);
        
        std::cout << "步骤 " << (i+1) << ":" << std::endl;
        printAcceleration("  原始加速度", imu_data.acc_x, imu_data.acc_y, imu_data.acc_z);
        printAcceleration("  处理后加速度", processed.ax, processed.ay, processed.az);
        std::cout << std::endl;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "\n场景2：机器人前倾15度" << std::endl;
    for (int i = 0; i < 3; ++i) {
        ImuData imu_data;
        imu_data.angle_roll = 0.0f;
        imu_data.angle_pitch = 15.0f;  // 前倾15度
        imu_data.acc_x = 2.54f;        // 9.81 * sin(15°)
        imu_data.acc_y = 0.0f;
        imu_data.acc_z = 9.47f;        // 9.81 * cos(15°)
        
        auto processed = processor.processAcceleration(imu_data);
        
        std::cout << "步骤 " << (i+1) << ":" << std::endl;
        printAcceleration("  原始加速度", imu_data.acc_x, imu_data.acc_y, imu_data.acc_z);
        printAcceleration("  处理后加速度", processed.ax, processed.ay, processed.az);
        std::cout << std::endl;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "\n场景3：机器人向前加速" << std::endl;
    for (int i = 0; i < 5; ++i) {
        ImuData imu_data;
        imu_data.angle_roll = 0.0f;
        imu_data.angle_pitch = 0.0f;
        imu_data.acc_x = -1.0f - i * 0.5f;  // 向前加速（负值）
        imu_data.acc_y = 0.0f;
        imu_data.acc_z = 9.81f;
        
        auto processed = processor.processAcceleration(imu_data);
        
        std::cout << "步骤 " << (i+1) << ":" << std::endl;
        printAcceleration("  原始加速度", imu_data.acc_x, imu_data.acc_y, imu_data.acc_z);
        printAcceleration("  处理后加速度", processed.ax, processed.ay, processed.az);
        std::cout << std::endl;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "\n场景4：机器人左倾30度" << std::endl;
    for (int i = 0; i < 3; ++i) {
        ImuData imu_data;
        imu_data.angle_roll = 30.0f;   // 左倾30度
        imu_data.angle_pitch = 0.0f;
        imu_data.acc_x = 0.0f;
        imu_data.acc_y = -4.905f;      // -9.81 * sin(30°)
        imu_data.acc_z = 8.496f;       // 9.81 * cos(30°)
        
        auto processed = processor.processAcceleration(imu_data);
        
        std::cout << "步骤 " << (i+1) << ":" << std::endl;
        printAcceleration("  原始加速度", imu_data.acc_x, imu_data.acc_y, imu_data.acc_z);
        printAcceleration("  处理后加速度", processed.ax, processed.ay, processed.az);
        std::cout << std::endl;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "\n示例完成！" << std::endl;
    std::cout << "\n关键观察点：" << std::endl;
    std::cout << "1. 重力补偿：处理后加速度在静止状态下接近零" << std::endl;
    std::cout << "2. 坐标轴矫正：向前加速时X轴加速度变为正值" << std::endl;
    std::cout << "3. 滤波效果：数据更加平滑" << std::endl;
    std::cout << "4. 零速度检测：静止状态下加速度自动归零" << std::endl;
    
    return 0;
} 