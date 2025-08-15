/// @file velocity_calculator.h
/// @author Assistant
/// @brief 从IMU加速度数据计算线速度的工具类
/// @version 0.1
/// @date 2024-01-01
/// @copyright Copyright (c) 2024

#ifndef VELOCITY_CALCULATOR_H_
#define VELOCITY_CALCULATOR_H_

#include "robot_types.h"
#include <deque>
#include <chrono>

/// @brief 从IMU加速度计算线速度的类
class VelocityCalculator {
public:
    /// @brief 构造函数
    /// @param window_size 滑动窗口大小，用于滤波
    /// @param gravity_threshold 重力阈值，用于零速度检测
    VelocityCalculator(size_t window_size = 10, float gravity_threshold = 0.1f);
    
    /// @brief 析构函数
    ~VelocityCalculator() = default;
    
    /// @brief 更新IMU数据并计算线速度
    /// @param imu_data IMU数据
    /// @param dt 时间间隔（秒）
    /// @return 计算得到的线速度 (vx, vy, vz)
    struct Velocity3D {
        float vx;  // X轴线速度 (m/s)
        float vy;  // Y轴线速度 (m/s)
        float vz;  // Z轴线速度 (m/s)
    };
    
    Velocity3D updateVelocity(const ImuData& imu_data, float dt);
    
    /// @brief 重置计算器状态
    void reset();
    
    /// @brief 设置重力阈值
    /// @param threshold 新的重力阈值
    void setGravityThreshold(float threshold);
    
    /// @brief 设置滑动窗口大小
    /// @param size 新的窗口大小
    void setWindowSize(size_t size);
    
    /// @brief 获取当前线速度
    /// @return 当前线速度
    Velocity3D getCurrentVelocity() const;
    
    /// @brief 获取累积位移
    /// @return 累积位移 (x, y, z)
    struct Displacement3D {
        float x;  // X轴位移 (m)
        float y;  // Y轴位移 (m)
        float z;  // Z轴位移 (m)
    };
    
    Displacement3D getDisplacement() const;

private:
    /// @brief 检测零速度状态
    /// @param acc_x X轴加速度
    /// @param acc_y Y轴加速度
    /// @param acc_z Z轴加速度
    /// @return 是否为零速度状态
    bool isZeroVelocity(float acc_x, float acc_y, float acc_z) const;
    
    /// @brief 应用低通滤波
    /// @param new_value 新值
    /// @param filtered_values 滤波值队列
    /// @return 滤波后的值
    float applyLowPassFilter(float new_value, std::deque<float>& filtered_values);
    
    /// @brief 移除重力影响（简单方法）
    /// @param acc_x X轴加速度
    /// @param acc_y Y轴加速度
    /// @param acc_z Z轴加速度
    /// @param roll 横滚角（弧度）
    /// @param pitch 俯仰角（弧度）
    /// @return 去除重力后的加速度
    struct Acceleration3D {
        float ax;
        float ay;
        float az;
    };
    
    Acceleration3D removeGravity(float acc_x, float acc_y, float acc_z, 
                                 float roll, float pitch) const;

private:
    size_t window_size_;           // 滑动窗口大小
    float gravity_threshold_;      // 重力阈值
    float gravity_;                // 重力加速度 (m/s^2)
    
    // 当前状态
    Velocity3D current_velocity_;
    Displacement3D displacement_;
    
    // 滤波队列
    std::deque<float> filtered_acc_x_;
    std::deque<float> filtered_acc_y_;
    std::deque<float> filtered_acc_z_;
    
    // 时间相关
    std::chrono::steady_clock::time_point last_update_time_;
    bool is_initialized_;
    
    // 零速度检测
    bool is_zero_velocity_state_;
    int zero_velocity_counter_;
    static const int ZERO_VELOCITY_THRESHOLD = 5;  // 连续零速度检测次数
};

#endif  // VELOCITY_CALCULATOR_H_ 