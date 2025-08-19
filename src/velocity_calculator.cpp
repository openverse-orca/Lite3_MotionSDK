/// @file velocity_calculator.cpp
/// @author Assistant
/// @brief 从IMU加速度数据计算线速度的工具类实现
/// @version 0.1
/// @date 2024-01-01
/// @copyright Copyright (c) 2024

#include "../include/velocity_calculator.h"
#include <cmath>
#include <algorithm>
#include <numeric>

const float DEGREE_TO_RADIAN = M_PI / 180.0f;

VelocityCalculator::VelocityCalculator(size_t window_size, float gravity_threshold)
    : window_size_(window_size)
    , gravity_threshold_(gravity_threshold)
    , gravity_(9.81f)
    , is_initialized_(false)
    , is_zero_velocity_state_(false)
    , zero_velocity_counter_(0) {
    
    // 初始化速度为零
    current_velocity_.vx = 0.0f;
    current_velocity_.vy = 0.0f;
    current_velocity_.vz = 0.0f;
    
    // 初始化位移为零
    displacement_.x = 0.0f;
    displacement_.y = 0.0f;
    displacement_.z = 0.0f;
    
    last_update_time_ = std::chrono::steady_clock::now();
}

VelocityCalculator::Velocity3D VelocityCalculator::updateVelocity(const ImuData& imu_data, float dt) {
    // 确保时间间隔为正数
    if (dt <= 0.0f) {
        return current_velocity_;
    }
    
    // 将角度从度转换为弧度
    float roll_rad = imu_data.angle_roll * DEGREE_TO_RADIAN;
    float pitch_rad = imu_data.angle_pitch * DEGREE_TO_RADIAN;
    
    // 移除重力影响
    Acceleration3D acc_no_gravity = removeGravity(imu_data.acc_x, imu_data.acc_y, imu_data.acc_z, 
                                                  roll_rad, pitch_rad);
    
    // 应用低通滤波
    float filtered_acc_x = applyLowPassFilter(acc_no_gravity.ax, filtered_acc_x_);
    float filtered_acc_y = applyLowPassFilter(acc_no_gravity.ay, filtered_acc_y_);
    float filtered_acc_z = applyLowPassFilter(acc_no_gravity.az, filtered_acc_z_);
    
    // 检测零速度状态
    bool is_zero_vel = isZeroVelocity(filtered_acc_x, filtered_acc_y, filtered_acc_z);
    
    if (is_zero_vel) {
        zero_velocity_counter_++;
        if (zero_velocity_counter_ >= ZERO_VELOCITY_THRESHOLD) {
            is_zero_velocity_state_ = true;
            // 在零速度状态下，逐渐将速度归零
            current_velocity_.vx *= 0.95f;
            current_velocity_.vy *= 0.95f;
            current_velocity_.vz *= 0.95f;
        }
    } else {
        zero_velocity_counter_ = 0;
        is_zero_velocity_state_ = false;
        
        // 使用梯形积分计算速度
        current_velocity_.vx += filtered_acc_x * dt;
        current_velocity_.vy += filtered_acc_y * dt;
        current_velocity_.vz += filtered_acc_z * dt;
    }
    
    // 计算位移（使用梯形积分）
    displacement_.x += current_velocity_.vx * dt;
    displacement_.y += current_velocity_.vy * dt;
    displacement_.z += current_velocity_.vz * dt;
    
    // 更新最后更新时间
    last_update_time_ = std::chrono::steady_clock::now();
    is_initialized_ = true;
    
    return current_velocity_;
}

void VelocityCalculator::reset() {
    current_velocity_.vx = 0.0f;
    current_velocity_.vy = 0.0f;
    current_velocity_.vz = 0.0f;
    
    displacement_.x = 0.0f;
    displacement_.y = 0.0f;
    displacement_.z = 0.0f;
    
    filtered_acc_x_.clear();
    filtered_acc_y_.clear();
    filtered_acc_z_.clear();
    
    is_initialized_ = false;
    is_zero_velocity_state_ = false;
    zero_velocity_counter_ = 0;
    
    last_update_time_ = std::chrono::steady_clock::now();
}

void VelocityCalculator::setGravityThreshold(float threshold) {
    gravity_threshold_ = threshold;
}

void VelocityCalculator::setWindowSize(size_t size) {
    window_size_ = size;
    
    // 调整滤波队列大小
    while (filtered_acc_x_.size() > window_size_) {
        filtered_acc_x_.pop_front();
    }
    while (filtered_acc_y_.size() > window_size_) {
        filtered_acc_y_.pop_front();
    }
    while (filtered_acc_z_.size() > window_size_) {
        filtered_acc_z_.pop_front();
    }
}

VelocityCalculator::Velocity3D VelocityCalculator::getCurrentVelocity() const {
    return current_velocity_;
}

VelocityCalculator::Displacement3D VelocityCalculator::getDisplacement() const {
    return displacement_;
}

bool VelocityCalculator::isZeroVelocity(float acc_x, float acc_y, float acc_z) const {
    // 计算加速度的幅值
    float acc_magnitude = std::sqrt(acc_x * acc_x + acc_y * acc_y + acc_z * acc_z);
    
    // 如果加速度幅值小于阈值，认为是零速度状态
    return acc_magnitude < gravity_threshold_;
}

float VelocityCalculator::applyLowPassFilter(float new_value, std::deque<float>& filtered_values) {
    // 添加新值到队列
    filtered_values.push_back(new_value);
    
    // 保持队列大小
    if (filtered_values.size() > window_size_) {
        filtered_values.pop_front();
    }
    
    // 计算平均值作为滤波结果
    if (filtered_values.empty()) {
        return new_value;
    }
    
    float sum = std::accumulate(filtered_values.begin(), filtered_values.end(), 0.0f);
    return sum / filtered_values.size();
}

VelocityCalculator::Acceleration3D VelocityCalculator::removeGravity(float acc_x, float acc_y, float acc_z, 
                                                                      float roll, float pitch) const {
    // 重力补偿方法
    // 根据横滚角和俯仰角计算重力在各轴的分量
    
    // 重力在X轴的分量（俯仰角影响）
    float gravity_x = gravity_ * std::sin(pitch);
    
    // 重力在Y轴的分量（横滚角影响）
    float gravity_y = -gravity_ * std::sin(roll) * std::cos(pitch);
    
    // 重力在Z轴的分量（主要分量）
    float gravity_z = gravity_ * std::cos(roll) * std::cos(pitch);
    
    Acceleration3D result;
    result.ax = acc_x - gravity_x;
    result.ay = acc_y - gravity_y;
    result.az = acc_z - gravity_z;
    
    return result;
} 