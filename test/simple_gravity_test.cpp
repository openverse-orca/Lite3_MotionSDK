/// @file simple_gravity_test.cpp
/// @author Assistant
/// @brief 简单的重力补偿测试
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

int main() {
    std::cout << "简单重力补偿测试" << std::endl;
    std::cout << "==================" << std::endl;
    
    ImuProcessor processor(1, 0.1f);  // 最小窗口，避免滤波影响
    
    // 禁用坐标轴矫正，只测试重力补偿
    processor.enableGravityCompensation(true);
    processor.enableAxisCorrection(false);
    
    ImuData imu_data;
    imu_data.angle_roll = 0.0f;
    imu_data.angle_pitch = 0.0f;
    imu_data.acc_x = 0.0f;
    imu_data.acc_y = 0.0f;
    imu_data.acc_z = 9.81f;
    
    std::cout << "\n测试水平放置 (roll=0°, pitch=0°)" << std::endl;
    printAcceleration("原始加速度", imu_data.acc_x, imu_data.acc_y, imu_data.acc_z);
    
    auto processed = processor.processAcceleration(imu_data);
    printAcceleration("处理后加速度", processed.ax, processed.ay, processed.az);
    
    // 手动计算重力分量
    float g = 9.81f;
    float roll_rad = 0.0f * M_PI / 180.0f;
    float pitch_rad = 0.0f * M_PI / 180.0f;
    
    float gx = g * std::sin(pitch_rad);
    float gy = -g * std::sin(roll_rad) * std::cos(pitch_rad);
    float gz = g * std::cos(roll_rad) * std::cos(pitch_rad);
    
    std::cout << "\n手动计算重力分量:" << std::endl;
    std::cout << "gx = " << g << " * sin(" << 0.0f << "°) = " << gx << std::endl;
    std::cout << "gy = -" << g << " * sin(" << 0.0f << "°) * cos(" << 0.0f << "°) = " << gy << std::endl;
    std::cout << "gz = " << g << " * cos(" << 0.0f << "°) * cos(" << 0.0f << "°) = " << gz << std::endl;
    
    float expected_ax = imu_data.acc_x - gx;
    float expected_ay = imu_data.acc_y - gy;
    float expected_az = imu_data.acc_z - gz;
    
    std::cout << "\n期望结果:" << std::endl;
    printAcceleration("期望加速度", expected_ax, expected_ay, expected_az);
    
    return 0;
} 