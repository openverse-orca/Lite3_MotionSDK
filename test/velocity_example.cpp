/// @file velocity_example.cpp
/// @author Assistant
/// @brief 演示如何使用VelocityCalculator从IMU数据计算线速度
/// @version 0.1
/// @date 2024-01-01
/// @copyright Copyright (c) 2024

#include "../include/velocity_calculator.h"
#include "../include/robot_types.h"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    std::cout << "=== IMU线速度计算示例 ===" << std::endl;
    
    // 创建速度计算器实例
    // 参数：滑动窗口大小=10，重力阈值=0.1 m/s²
    VelocityCalculator velocity_calc(10, 0.1f);
    
    // 模拟IMU数据更新频率（1kHz）
    const float dt = 0.001f;  // 1ms
    
    std::cout << "开始模拟IMU数据流..." << std::endl;
    std::cout << "时间间隔: " << dt * 1000 << " ms" << std::endl;
    std::cout << std::endl;
    
    // 模拟一些IMU数据
    for (int i = 0; i < 1000; ++i) {  // 模拟1秒的数据
        // 创建模拟IMU数据
        ImuData imu_data;
        imu_data.timestamp = i;
        
        // 模拟机器人静止状态（只有重力）
        if (i < 200) {
            // 静止状态
            imu_data.angle_roll = 0.0f;
            imu_data.angle_pitch = 0.0f;
            imu_data.angle_yaw = 0.0f;
            imu_data.angular_velocity_roll = 0.0f;
            imu_data.angular_velocity_pitch = 0.0f;
            imu_data.angular_velocity_yaw = 0.0f;
            imu_data.acc_x = 0.0f;
            imu_data.acc_y = 0.0f;
            imu_data.acc_z = 9.81f;  // 重力加速度
        }
        // 模拟向前加速
        else if (i < 400) {
            imu_data.angle_roll = 0.0f;
            imu_data.angle_pitch = 0.0f;
            imu_data.angle_yaw = 0.0f;
            imu_data.angular_velocity_roll = 0.0f;
            imu_data.angular_velocity_pitch = 0.0f;
            imu_data.angular_velocity_yaw = 0.0f;
            imu_data.acc_x = 1.0f;  // 1 m/s² 向前加速度
            imu_data.acc_y = 0.0f;
            imu_data.acc_z = 9.81f;
        }
        // 模拟匀速运动
        else if (i < 600) {
            imu_data.angle_roll = 0.0f;
            imu_data.angle_pitch = 0.0f;
            imu_data.angle_yaw = 0.0f;
            imu_data.angular_velocity_roll = 0.0f;
            imu_data.angular_velocity_pitch = 0.0f;
            imu_data.angular_velocity_yaw = 0.0f;
            imu_data.acc_x = 0.0f;  // 匀速运动，加速度为0
            imu_data.acc_y = 0.0f;
            imu_data.acc_z = 9.81f;
        }
        // 模拟减速
        else if (i < 800) {
            imu_data.angle_roll = 0.0f;
            imu_data.angle_pitch = 0.0f;
            imu_data.angle_yaw = 0.0f;
            imu_data.angular_velocity_roll = 0.0f;
            imu_data.angular_velocity_pitch = 0.0f;
            imu_data.angular_velocity_yaw = 0.0f;
            imu_data.acc_x = -1.0f;  // -1 m/s² 向后加速度（减速）
            imu_data.acc_y = 0.0f;
            imu_data.acc_z = 9.81f;
        }
        // 模拟静止
        else {
            imu_data.angle_roll = 0.0f;
            imu_data.angle_pitch = 0.0f;
            imu_data.angle_yaw = 0.0f;
            imu_data.angular_velocity_roll = 0.0f;
            imu_data.angular_velocity_pitch = 0.0f;
            imu_data.angular_velocity_yaw = 0.0f;
            imu_data.acc_x = 0.0f;
            imu_data.acc_y = 0.0f;
            imu_data.acc_z = 9.81f;
        }
        
        // 更新速度计算器
        VelocityCalculator::Velocity3D velocity = velocity_calc.updateVelocity(imu_data, dt);
        
        // 每100ms打印一次结果
        if (i % 100 == 0) {
            VelocityCalculator::Displacement3D displacement = velocity_calc.getDisplacement();
            
            std::cout << "时间: " << (i * dt) << " s" << std::endl;
            std::cout << "  IMU加速度 (m/s²): [" 
                      << imu_data.acc_x << ", " 
                      << imu_data.acc_y << ", " 
                      << imu_data.acc_z << "]" << std::endl;
            std::cout << "  计算线速度 (m/s): [" 
                      << velocity.vx << ", " 
                      << velocity.vy << ", " 
                      << velocity.vz << "]" << std::endl;
            std::cout << "  累积位移 (m): [" 
                      << displacement.x << ", " 
                      << displacement.y << ", " 
                      << displacement.z << "]" << std::endl;
            std::cout << std::endl;
        }
        
        // 模拟实时数据流
        std::this_thread::sleep_for(std::chrono::microseconds(1000));  // 1ms
    }
    
    std::cout << "=== 示例完成 ===" << std::endl;
    
    return 0;
} 