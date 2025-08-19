/// @file y_axis_verification.cpp
/// @author Assistant
/// @brief 验证Y轴重力补偿和坐标轴矫正
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
    std::cout << "Y轴重力补偿和坐标轴矫正验证" << std::endl;
    std::cout << "============================" << std::endl;
    
    ImuProcessor processor(1, 0.1f);  // 最小窗口，避免滤波影响
    
    // 测试Y轴重力补偿
    std::cout << "\n测试Y轴重力补偿（左倾30度）" << std::endl;
    processor.enableGravityCompensation(true);
    processor.enableAxisCorrection(false);
    
    ImuData imu_data;
    imu_data.angle_roll = 30.0f;   // 左倾30度
    imu_data.angle_pitch = 0.0f;
    imu_data.acc_x = 0.0f;
    imu_data.acc_y = -4.905f;      // -9.81 * sin(30°)
    imu_data.acc_z = 8.496f;       // 9.81 * cos(30°)
    
    auto processed = processor.processAcceleration(imu_data);
    printAcceleration("原始加速度", imu_data.acc_x, imu_data.acc_y, imu_data.acc_z);
    printAcceleration("重力补偿后", processed.ax, processed.ay, processed.az);
    
    // 测试Y轴坐标轴矫正
    std::cout << "\n测试Y轴坐标轴矫正" << std::endl;
    processor.enableGravityCompensation(false);
    processor.enableAxisCorrection(true);
    
    imu_data.angle_roll = 0.0f;
    imu_data.angle_pitch = 0.0f;
    imu_data.acc_x = 0.0f;
    imu_data.acc_y = -2.0f;        // Y轴负值
    imu_data.acc_z = 9.81f;
    
    processed = processor.processAcceleration(imu_data);
    printAcceleration("原始加速度", imu_data.acc_x, imu_data.acc_y, imu_data.acc_z);
    printAcceleration("坐标轴矫正后", processed.ax, processed.ay, processed.az);
    
    // 测试Y轴完整流程
    std::cout << "\n测试Y轴完整流程（重力补偿 + 坐标轴矫正）" << std::endl;
    processor.enableGravityCompensation(true);
    processor.enableAxisCorrection(true);
    
    imu_data.angle_roll = 30.0f;   // 左倾30度
    imu_data.angle_pitch = 0.0f;
    imu_data.acc_x = 0.0f;
    imu_data.acc_y = -4.905f;      // -9.81 * sin(30°)
    imu_data.acc_z = 8.496f;       // 9.81 * cos(30°)
    
    processed = processor.processAcceleration(imu_data);
    printAcceleration("原始加速度", imu_data.acc_x, imu_data.acc_y, imu_data.acc_z);
    printAcceleration("完整处理后", processed.ax, processed.ay, processed.az);
    
    return 0;
} 