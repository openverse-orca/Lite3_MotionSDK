/// @file test_velocity_calculator.cpp
/// @author Assistant
/// @brief 测试VelocityCalculator的功能
/// @version 0.1
/// @date 2024-01-01
/// @copyright Copyright (c) 2024

#include "../include/velocity_calculator.h"
#include "../include/robot_types.h"
#include <iostream>
#include <cassert>
#include <cmath>

// 简单的测试框架
#define TEST_ASSERT(condition, message) \
    if (!(condition)) { \
        std::cerr << "测试失败: " << message << std::endl; \
        return false; \
    }

bool testBasicFunctionality() {
    std::cout << "测试基本功能..." << std::endl;
    
    VelocityCalculator calc(5, 0.1f);
    const float dt = 0.001f;
    
    // 测试静止状态
    ImuData imu_data;
    imu_data.angle_roll = 0.0f;
    imu_data.angle_pitch = 0.0f;
    imu_data.angle_yaw = 0.0f;
    imu_data.angular_velocity_roll = 0.0f;
    imu_data.angular_velocity_pitch = 0.0f;
    imu_data.angular_velocity_yaw = 0.0f;
    imu_data.acc_x = 0.0f;
    imu_data.acc_y = 0.0f;
    imu_data.acc_z = 9.81f;
    
    // 运行一段时间
    for (int i = 0; i < 100; ++i) {
        calc.updateVelocity(imu_data, dt);
    }
    
    VelocityCalculator::Velocity3D velocity = calc.getCurrentVelocity();
    
    // 静止状态下速度应该接近零
    TEST_ASSERT(std::abs(velocity.vx) < 0.1f, "静止状态下X轴速度应该接近零");
    TEST_ASSERT(std::abs(velocity.vy) < 0.1f, "静止状态下Y轴速度应该接近零");
    TEST_ASSERT(std::abs(velocity.vz) < 0.1f, "静止状态下Z轴速度应该接近零");
    
    std::cout << "基本功能测试通过" << std::endl;
    return true;
}

bool testAccelerationIntegration() {
    std::cout << "测试加速度积分..." << std::endl;
    
    VelocityCalculator calc(5, 0.1f);
    const float dt = 0.001f;
    const float acceleration = 1.0f;  // 1 m/s²
    const float test_time = 0.1f;     // 0.1秒
    const int steps = static_cast<int>(test_time / dt);
    
    ImuData imu_data;
    imu_data.angle_roll = 0.0f;
    imu_data.angle_pitch = 0.0f;
    imu_data.angle_yaw = 0.0f;
    imu_data.angular_velocity_roll = 0.0f;
    imu_data.angular_velocity_pitch = 0.0f;
    imu_data.angular_velocity_yaw = 0.0f;
    imu_data.acc_x = acceleration;
    imu_data.acc_y = 0.0f;
    imu_data.acc_z = 9.81f;
    
    // 应用恒定加速度
    for (int i = 0; i < steps; ++i) {
        calc.updateVelocity(imu_data, dt);
    }
    
    VelocityCalculator::Velocity3D velocity = calc.getCurrentVelocity();
    float expected_velocity = acceleration * test_time;
    
    // 计算的速度应该接近理论值
    TEST_ASSERT(std::abs(velocity.vx - expected_velocity) < 0.1f, 
                "积分速度应该接近理论值");
    
    std::cout << "加速度积分测试通过" << std::endl;
    return true;
}

bool testResetFunction() {
    std::cout << "测试重置功能..." << std::endl;
    
    VelocityCalculator calc(5, 0.1f);
    const float dt = 0.001f;
    
    ImuData imu_data;
    imu_data.angle_roll = 0.0f;
    imu_data.angle_pitch = 0.0f;
    imu_data.angle_yaw = 0.0f;
    imu_data.angular_velocity_roll = 0.0f;
    imu_data.angular_velocity_pitch = 0.0f;
    imu_data.angular_velocity_yaw = 0.0f;
    imu_data.acc_x = 1.0f;
    imu_data.acc_y = 0.0f;
    imu_data.acc_z = 9.81f;
    
    // 运行一段时间产生速度
    for (int i = 0; i < 50; ++i) {
        calc.updateVelocity(imu_data, dt);
    }
    
    VelocityCalculator::Velocity3D velocity_before = calc.getCurrentVelocity();
    TEST_ASSERT(std::abs(velocity_before.vx) > 0.01f, "应该有非零速度");
    
    // 重置
    calc.reset();
    
    VelocityCalculator::Velocity3D velocity_after = calc.getCurrentVelocity();
    TEST_ASSERT(std::abs(velocity_after.vx) < 0.001f, "重置后速度应该为零");
    TEST_ASSERT(std::abs(velocity_after.vy) < 0.001f, "重置后速度应该为零");
    TEST_ASSERT(std::abs(velocity_after.vz) < 0.001f, "重置后速度应该为零");
    
    std::cout << "重置功能测试通过" << std::endl;
    return true;
}

bool testDisplacementCalculation() {
    std::cout << "测试位移计算..." << std::endl;
    
    VelocityCalculator calc(5, 0.1f);
    const float dt = 0.001f;
    const float acceleration = 1.0f;
    const float test_time = 0.1f;
    const int steps = static_cast<int>(test_time / dt);
    
    ImuData imu_data;
    imu_data.angle_roll = 0.0f;
    imu_data.angle_pitch = 0.0f;
    imu_data.angle_yaw = 0.0f;
    imu_data.angular_velocity_roll = 0.0f;
    imu_data.angular_velocity_pitch = 0.0f;
    imu_data.angular_velocity_yaw = 0.0f;
    imu_data.acc_x = acceleration;
    imu_data.acc_y = 0.0f;
    imu_data.acc_z = 9.81f;
    
    // 应用恒定加速度
    for (int i = 0; i < steps; ++i) {
        calc.updateVelocity(imu_data, dt);
    }
    
    VelocityCalculator::Displacement3D displacement = calc.getDisplacement();
    float expected_displacement = 0.5f * acceleration * test_time * test_time;
    
    // 计算的位移应该接近理论值
    TEST_ASSERT(std::abs(displacement.x - expected_displacement) < 0.01f, 
                "位移应该接近理论值");
    
    std::cout << "位移计算测试通过" << std::endl;
    return true;
}

int main() {
    std::cout << "=== VelocityCalculator 测试开始 ===" << std::endl;
    
    bool all_tests_passed = true;
    
    all_tests_passed &= testBasicFunctionality();
    all_tests_passed &= testAccelerationIntegration();
    all_tests_passed &= testResetFunction();
    all_tests_passed &= testDisplacementCalculation();
    
    if (all_tests_passed) {
        std::cout << "=== 所有测试通过 ===" << std::endl;
        return 0;
    } else {
        std::cout << "=== 部分测试失败 ===" << std::endl;
        return 1;
    }
} 