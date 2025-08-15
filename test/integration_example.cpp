/// @file integration_example.cpp
/// @author Assistant
/// @brief 展示如何在主程序中集成VelocityCalculator
/// @version 0.1
/// @date 2024-01-01
/// @copyright Copyright (c) 2024

#include "../include/velocity_calculator.h"
#include "../include/robot_types.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>

// 模拟从IMU获取数据的函数
ImuData getImuData() {
    // 这里应该是您实际的IMU数据获取代码
    // 例如：从传感器读取数据，或者从网络接收数据
    
    ImuData imu_data;
    
    // 模拟数据 - 在实际应用中，这些数据来自真实的IMU
    static float time = 0.0f;
    time += 0.001f;  // 1ms时间步长
    
    // 模拟一个简单的运动模式
    if (time < 1.0f) {
        // 静止状态
        imu_data.angle_roll = 0.0f;
        imu_data.angle_pitch = 0.0f;
        imu_data.angle_yaw = 0.0f;
        imu_data.angular_velocity_roll = 0.0f;
        imu_data.angular_velocity_pitch = 0.0f;
        imu_data.angular_velocity_yaw = 0.0f;
        imu_data.acc_x = 0.0f;
        imu_data.acc_y = 0.0f;
        imu_data.acc_z = 9.81f;
    } else if (time < 2.0f) {
        // 向前加速
        imu_data.angle_roll = 0.0f;
        imu_data.angle_pitch = 0.0f;
        imu_data.angle_yaw = 0.0f;
        imu_data.angular_velocity_roll = 0.0f;
        imu_data.angular_velocity_pitch = 0.0f;
        imu_data.angular_velocity_yaw = 0.0f;
        imu_data.acc_x = 0.5f;  // 0.5 m/s² 向前加速度
        imu_data.acc_y = 0.0f;
        imu_data.acc_z = 9.81f;
    } else if (time < 3.0f) {
        // 匀速运动
        imu_data.angle_roll = 0.0f;
        imu_data.angle_pitch = 0.0f;
        imu_data.angle_yaw = 0.0f;
        imu_data.angular_velocity_roll = 0.0f;
        imu_data.angular_velocity_pitch = 0.0f;
        imu_data.angular_velocity_yaw = 0.0f;
        imu_data.acc_x = 0.0f;
        imu_data.acc_y = 0.0f;
        imu_data.acc_z = 9.81f;
    } else {
        // 减速停止
        imu_data.angle_roll = 0.0f;
        imu_data.angle_pitch = 0.0f;
        imu_data.angle_yaw = 0.0f;
        imu_data.angular_velocity_roll = 0.0f;
        imu_data.angular_velocity_pitch = 0.0f;
        imu_data.angular_velocity_yaw = 0.0f;
        imu_data.acc_x = -0.5f;  // -0.5 m/s² 向后加速度
        imu_data.acc_y = 0.0f;
        imu_data.acc_z = 9.81f;
    }
    
    imu_data.timestamp = static_cast<int32_t>(time * 1000);  // 转换为毫秒
    return imu_data;
}

// 主控制循环函数
void mainControlLoop() {
    std::cout << "=== 主控制循环集成示例 ===" << std::endl;
    
    // 创建速度计算器
    VelocityCalculator velocity_calc(10, 0.1f);
    
    // 控制循环参数
    const float control_frequency = 1000.0f;  // 1kHz
    const float dt = 1.0f / control_frequency;  // 1ms
    
    std::cout << "控制频率: " << control_frequency << " Hz" << std::endl;
    std::cout << "时间间隔: " << dt * 1000 << " ms" << std::endl;
    std::cout << std::endl;
    
    // 主控制循环
    auto start_time = std::chrono::steady_clock::now();
    int loop_count = 0;
    
    while (loop_count < 4000) {  // 运行4秒
        auto loop_start = std::chrono::steady_clock::now();
        
        // 1. 获取传感器数据
        ImuData imu_data = getImuData();
        
        // 2. 更新速度计算器
        VelocityCalculator::Velocity3D velocity = velocity_calc.updateVelocity(imu_data, dt);
        
        // 3. 获取位移信息
        VelocityCalculator::Displacement3D displacement = velocity_calc.getDisplacement();
        
        // 4. 基于速度信息进行控制决策
        // 这里可以添加您的控制逻辑
        if (std::abs(velocity.vx) > 0.1f) {
            // 机器人正在移动
            // 可以在这里添加运动控制逻辑
        }
        
        // 5. 每100ms打印一次状态信息
        if (loop_count % 100 == 0) {
            float current_time = loop_count * dt;
            std::cout << "时间: " << current_time << " s" << std::endl;
            std::cout << "  线速度 (m/s): [" 
                      << velocity.vx << ", " 
                      << velocity.vy << ", " 
                      << velocity.vz << "]" << std::endl;
            std::cout << "  位移 (m): [" 
                      << displacement.x << ", " 
                      << displacement.y << ", " 
                      << displacement.z << "]" << std::endl;
            std::cout << "  速度幅值: " 
                      << std::sqrt(velocity.vx*velocity.vx + velocity.vy*velocity.vy + velocity.vz*velocity.vz) 
                      << " m/s" << std::endl;
            std::cout << std::endl;
        }
        
        // 6. 控制循环频率
        auto loop_end = std::chrono::steady_clock::now();
        auto loop_duration = std::chrono::duration_cast<std::chrono::microseconds>(loop_end - loop_start);
        auto target_duration = std::chrono::microseconds(static_cast<long>(dt * 1000000));
        
        if (loop_duration < target_duration) {
            std::this_thread::sleep_for(target_duration - loop_duration);
        }
        
        loop_count++;
    }
    
    auto end_time = std::chrono::steady_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "=== 控制循环完成 ===" << std::endl;
    std::cout << "总运行时间: " << total_duration.count() << " ms" << std::endl;
    std::cout << "平均循环时间: " << total_duration.count() / loop_count << " ms" << std::endl;
    std::cout << "总循环次数: " << loop_count << std::endl;
}

// 在您的main函数中调用这个函数
int main() {
    try {
        mainControlLoop();
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 