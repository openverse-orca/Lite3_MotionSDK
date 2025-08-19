/// @file imu_processor.cpp
/// @author Assistant
/// @brief IMU数据处理器实现，用于重力补偿和坐标轴矫正
/// @version 0.1
/// @date 2024-01-01
/// @copyright Copyright (c) 2024

#include "../include/imu_processor.h"
#include <cmath>
#include <algorithm>
#include <numeric>

const float DEGREE_TO_RADIAN = M_PI / 180.0f;

ImuProcessor::ImuProcessor(size_t window_size, float gravity_threshold)
    : window_size_(window_size)
    , gravity_threshold_(gravity_threshold)
    , gravity_(9.81f)
    , enable_gravity_compensation_(true)
    , enable_axis_correction_(true)
    , is_initialized_(false)
    , is_zero_velocity_state_(false)
    , zero_velocity_counter_(0) {
    
    // 初始化加速度为零
    processed_acc_.ax = 0.0f;
    processed_acc_.ay = 0.0f;
    processed_acc_.az = 0.0f;
    
    raw_acc_.ax = 0.0f;
    raw_acc_.ay = 0.0f;
    raw_acc_.az = 0.0f;
    
    last_update_time_ = std::chrono::steady_clock::now();
}

ImuProcessor::ProcessedAcceleration ImuProcessor::processAcceleration(const ImuData& imu_data) {
    // 保存原始加速度数据
    raw_acc_.ax = imu_data.acc_x;
    raw_acc_.ay = imu_data.acc_y;
    raw_acc_.az = imu_data.acc_z;
    
    // 将角度从度转换为弧度
    float roll_rad = imu_data.angle_roll * DEGREE_TO_RADIAN;
    float pitch_rad = imu_data.angle_pitch * DEGREE_TO_RADIAN;
    
    // 第一步：重力补偿（基于原始数据）
    ProcessedAcceleration compensated_acc;
    if (enable_gravity_compensation_) {
        compensated_acc = compensateGravity(imu_data.acc_x, imu_data.acc_y, imu_data.acc_z, 
                                           roll_rad, pitch_rad);
    } else {
        compensated_acc.ax = imu_data.acc_x;
        compensated_acc.ay = imu_data.acc_y;
        compensated_acc.az = imu_data.acc_z;
    }
    
    // 第二步：坐标轴矫正
    ProcessedAcceleration corrected_acc;
    if (enable_axis_correction_) {
        corrected_acc = correctAxes(compensated_acc.ax, compensated_acc.ay, compensated_acc.az);
    } else {
        corrected_acc = compensated_acc;
    }
    
    // 第三步：低通滤波
    float filtered_acc_x = applyLowPassFilter(corrected_acc.ax, filtered_acc_x_);
    float filtered_acc_y = applyLowPassFilter(corrected_acc.ay, filtered_acc_y_);
    float filtered_acc_z = applyLowPassFilter(corrected_acc.az, filtered_acc_z_);
    
    // 检测零速度状态
    bool is_zero_vel = isZeroVelocity(filtered_acc_x, filtered_acc_y, filtered_acc_z);
    
    if (is_zero_vel) {
        zero_velocity_counter_++;
        if (zero_velocity_counter_ >= ZERO_VELOCITY_THRESHOLD) {
            is_zero_velocity_state_ = true;
            // 在零速度状态下，将加速度归零
            processed_acc_.ax = 0.0f;
            processed_acc_.ay = 0.0f;
            processed_acc_.az = 0.0f;
        }
    } else {
        zero_velocity_counter_ = 0;
        is_zero_velocity_state_ = false;
        
        // 更新处理后的加速度
        processed_acc_.ax = filtered_acc_x;
        processed_acc_.ay = filtered_acc_y;
        processed_acc_.az = filtered_acc_z;
    }
    
    // 更新最后更新时间
    last_update_time_ = std::chrono::steady_clock::now();
    is_initialized_ = true;
    
    return processed_acc_;
}

void ImuProcessor::reset() {
    processed_acc_.ax = 0.0f;
    processed_acc_.ay = 0.0f;
    processed_acc_.az = 0.0f;
    
    raw_acc_.ax = 0.0f;
    raw_acc_.ay = 0.0f;
    raw_acc_.az = 0.0f;
    
    filtered_acc_x_.clear();
    filtered_acc_y_.clear();
    filtered_acc_z_.clear();
    
    is_initialized_ = false;
    is_zero_velocity_state_ = false;
    zero_velocity_counter_ = 0;
    
    last_update_time_ = std::chrono::steady_clock::now();
}

void ImuProcessor::setGravityThreshold(float threshold) {
    gravity_threshold_ = threshold;
}

void ImuProcessor::setWindowSize(size_t size) {
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

void ImuProcessor::setGravity(float gravity) {
    gravity_ = gravity;
}

void ImuProcessor::enableGravityCompensation(bool enable) {
    enable_gravity_compensation_ = enable;
}

void ImuProcessor::enableAxisCorrection(bool enable) {
    enable_axis_correction_ = enable;
}

ImuProcessor::ProcessedAcceleration ImuProcessor::getProcessedAcceleration() const {
    return processed_acc_;
}

ImuProcessor::ProcessedAcceleration ImuProcessor::getRawAcceleration() const {
    return raw_acc_;
}

bool ImuProcessor::isZeroVelocity(float acc_x, float acc_y, float acc_z) const {
    // 计算加速度的幅值
    float acc_magnitude = std::sqrt(acc_x * acc_x + acc_y * acc_y + acc_z * acc_z);
    
    // 如果加速度幅值小于阈值，认为是零速度状态
    return acc_magnitude < gravity_threshold_;
}

float ImuProcessor::applyLowPassFilter(float new_value, std::deque<float>& filtered_values) {
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

ImuProcessor::ProcessedAcceleration ImuProcessor::compensateGravity(float acc_x, float acc_y, float acc_z, 
                                                                   float roll, float pitch) const {
    // 计算重力在各轴的分量
    GravityComponents gravity_comp = calculateGravityComponents(roll, pitch);
    
    ProcessedAcceleration result;
    result.ax = acc_x - gravity_comp.gx;
    result.ay = acc_y - gravity_comp.gy;
    result.az = acc_z - gravity_comp.gz;
    
    return result;
}

ImuProcessor::ProcessedAcceleration ImuProcessor::correctAxes(float acc_x, float acc_y, float acc_z) const {
    ProcessedAcceleration result;
    
    if (enable_axis_correction_) {
        // 应用坐标轴矫正
        result.ax = acc_x * AXIS_CORRECTION_X;
        result.ay = acc_y * AXIS_CORRECTION_Y;
        result.az = acc_z * AXIS_CORRECTION_Z;
    } else {
        // 不进行坐标轴矫正
        result.ax = acc_x;
        result.ay = acc_y;
        result.az = acc_z;
    }
    
    return result;
}

ImuProcessor::GravityComponents ImuProcessor::calculateGravityComponents(float roll, float pitch) const {
    GravityComponents result;
    
    // 重力补偿算法
    // 根据横滚角和俯仰角计算重力在各轴的分量
    
    // 重力在X轴的分量（俯仰角影响）
    result.gx = gravity_ * std::sin(pitch);
    
    // 重力在Y轴的分量（横滚角影响）
    result.gy = -gravity_ * std::sin(roll) * std::cos(pitch);
    
    // 重力在Z轴的分量（主要分量）
    result.gz = gravity_ * std::cos(roll) * std::cos(pitch);
    
    return result;
} 