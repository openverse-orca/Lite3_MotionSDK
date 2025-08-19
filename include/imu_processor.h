/// @file imu_processor.h
/// @author Assistant
/// @brief IMU数据处理器，用于重力补偿和坐标轴矫正
/// @version 0.1
/// @date 2024-01-01
/// @copyright Copyright (c) 2024

#ifndef IMU_PROCESSOR_H_
#define IMU_PROCESSOR_H_

#include "robot_types.h"
#include <deque>
#include <chrono>

/// @brief IMU数据处理器类
class ImuProcessor {
public:
    /// @brief 构造函数
    /// @param window_size 滑动窗口大小，用于滤波
    /// @param gravity_threshold 重力阈值，用于零速度检测
    ImuProcessor(size_t window_size = 10, float gravity_threshold = 0.1f);
    
    /// @brief 析构函数
    ~ImuProcessor() = default;
    
    /// @brief 处理IMU数据，进行重力补偿和坐标轴矫正
    /// @param imu_data 原始IMU数据
    /// @return 处理后的加速度数据
    struct ProcessedAcceleration {
        float ax;  // X轴加速度 (m/s^2)
        float ay;  // Y轴加速度 (m/s^2)
        float az;  // Z轴加速度 (m/s^2)
    };
    
    ProcessedAcceleration processAcceleration(const ImuData& imu_data);
    
    /// @brief 重置处理器状态
    void reset();
    
    /// @brief 设置重力阈值
    /// @param threshold 新的重力阈值
    void setGravityThreshold(float threshold);
    
    /// @brief 设置滑动窗口大小
    /// @param size 新的窗口大小
    void setWindowSize(size_t size);
    
    /// @brief 设置重力加速度值
    /// @param gravity 重力加速度值 (m/s^2)
    void setGravity(float gravity);
    
    /// @brief 启用/禁用重力补偿
    /// @param enable 是否启用重力补偿
    void enableGravityCompensation(bool enable);
    
    /// @brief 启用/禁用坐标轴矫正
    /// @param enable 是否启用坐标轴矫正
    void enableAxisCorrection(bool enable);
    
    /// @brief 获取处理后的加速度
    /// @return 处理后的加速度
    ProcessedAcceleration getProcessedAcceleration() const;
    
    /// @brief 获取原始加速度
    /// @return 原始加速度
    ProcessedAcceleration getRawAcceleration() const;

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
    
    /// @brief 重力补偿
    /// @param acc_x X轴加速度
    /// @param acc_y Y轴加速度
    /// @param acc_z Z轴加速度
    /// @param roll 横滚角（弧度）
    /// @param pitch 俯仰角（弧度）
    /// @return 去除重力后的加速度
    ProcessedAcceleration compensateGravity(float acc_x, float acc_y, float acc_z, 
                                           float roll, float pitch) const;
    
    /// @brief 坐标轴矫正
    /// @param acc_x X轴加速度
    /// @param acc_y Y轴加速度
    /// @param acc_z Z轴加速度
    /// @return 矫正后的加速度
    ProcessedAcceleration correctAxes(float acc_x, float acc_y, float acc_z) const;
    
    /// @brief 计算重力在各轴的分量
    /// @param roll 横滚角（弧度）
    /// @param pitch 俯仰角（弧度）
    /// @return 重力分量
    struct GravityComponents {
        float gx;  // X轴重力分量
        float gy;  // Y轴重力分量
        float gz;  // Z轴重力分量
    };
    
    GravityComponents calculateGravityComponents(float roll, float pitch) const;

private:
    size_t window_size_;           // 滑动窗口大小
    float gravity_threshold_;      // 重力阈值
    float gravity_;                // 重力加速度 (m/s^2)
    
    // 功能开关
    bool enable_gravity_compensation_;  // 是否启用重力补偿
    bool enable_axis_correction_;       // 是否启用坐标轴矫正
    
    // 当前状态
    ProcessedAcceleration processed_acc_;
    ProcessedAcceleration raw_acc_;
    
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
    
    // 坐标轴矫正参数
    static constexpr float AXIS_CORRECTION_X = -1.0f;  // X轴方向矫正
    static constexpr float AXIS_CORRECTION_Y = -1.0f;   // Y轴方向矫正
    static constexpr float AXIS_CORRECTION_Z = -1.0f;  // Z轴方向矫正（翻转Z轴）
};

#endif  // IMU_PROCESSOR_H_ 