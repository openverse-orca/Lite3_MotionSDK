/// @file debug_imu_processor.cpp
/// @author Assistant
/// @brief 调试IMU处理器的各个组件
/// @version 0.1
/// @date 2024-01-01
/// @copyright Copyright (c) 2024

#include "../include/imu_processor.h"
#include <iostream>
#include <iomanip>

void printAcceleration(const std::string& label, float ax, float ay, float az) {
    std::cout << std::fixed << std::setprecision(3);
    std::cout << label << ": [" << std::setw(8) << ax << ", " 
              << std::setw(8) << ay << ", " << std::setw(8) << az << "] m/s²" << std::endl;
}

int main() {
    std::cout << "IMU处理器调试程序" << std::endl;
    std::cout << "==================" << std::endl;
    
    ImuProcessor processor(5, 0.1f);
    
    // 测试1：只测试坐标轴矫正
    std::cout << "\n测试1：只测试坐标轴矫正" << std::endl;
    processor.enableGravityCompensation(false);
    processor.enableAxisCorrection(true);
    
    ImuData imu_data;
    imu_data.angle_roll = 0.0f;
    imu_data.angle_pitch = 0.0f;
    imu_data.acc_x = -2.0f;
    imu_data.acc_y = 1.0f;
    imu_data.acc_z = 9.81f;
    
    auto processed = processor.processAcceleration(imu_data);
    printAcceleration("原始加速度", imu_data.acc_x, imu_data.acc_y, imu_data.acc_z);
    printAcceleration("处理后加速度", processed.ax, processed.ay, processed.az);
    
    // 测试2：只测试重力补偿
    std::cout << "\n测试2：只测试重力补偿" << std::endl;
    processor.enableGravityCompensation(true);
    processor.enableAxisCorrection(false);
    
    imu_data.acc_x = 0.0f;
    imu_data.acc_y = 0.0f;
    imu_data.acc_z = 9.81f;
    
    processed = processor.processAcceleration(imu_data);
    printAcceleration("原始加速度", imu_data.acc_x, imu_data.acc_y, imu_data.acc_z);
    printAcceleration("处理后加速度", processed.ax, processed.ay, processed.az);
    
    // 测试3：测试完整流程
    std::cout << "\n测试3：测试完整流程" << std::endl;
    processor.enableGravityCompensation(true);
    processor.enableAxisCorrection(true);
    
    imu_data.acc_x = -2.0f;
    imu_data.acc_y = 0.0f;
    imu_data.acc_z = 9.81f;
    
    processed = processor.processAcceleration(imu_data);
    printAcceleration("原始加速度", imu_data.acc_x, imu_data.acc_y, imu_data.acc_z);
    printAcceleration("处理后加速度", processed.ax, processed.ay, processed.az);
    
    return 0;
} 