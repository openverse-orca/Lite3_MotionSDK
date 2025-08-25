#include "../include/grpc_client.h"
#include "../include/imu_processor.h"
#include "../include/square_wave.h"
#include <iostream>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <cstdlib>
#include <ctime>
#include <cmath>

// 全局IMU处理器实例
static ImuProcessor imu_processor(10, 0.1f);
static float last_timestamp = 0.0f;

// 全局方波生成器实例
static SquareWaveGenerator square_wave_generator;

// 初始化随机数种子
static bool random_initialized = false;

GrpcClient::GrpcClient(const std::string& server_address) 
    : server_address_(server_address), connected_(false) {
}

GrpcClient::~GrpcClient() {
    // gRPC会自动清理资源
}

bool GrpcClient::Connect() {
    try {
        // 创建不安全的通道（用于测试，生产环境应使用SSL）
        channel_ = grpc::CreateChannel(server_address_, grpc::InsecureChannelCredentials());
        
        // 创建stub
        stub_ = inference::InferenceService::NewStub(channel_);
        
        // 测试连接
        grpc::ClientContext context;
        context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));
        
        inference::InferenceRequest request;
        inference::InferenceResponse response;
        
        // 发送一个简单的测试请求
        request.add_observation(0.0f);
        request.set_model_type("test");
        request.set_deterministic(true);
        
        grpc::Status status = stub_->Predict(&context, request, &response);
        
        if (status.ok()) {
            connected_ = true;
            std::cout << "Successfully connected to gRPC server at " << server_address_ << std::endl;
            return true;
        } else {
            std::cerr << "Failed to connect to gRPC server: " << status.error_message() << std::endl;
            connected_ = false;
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception during gRPC connection: " << e.what() << std::endl;
        connected_ = false;
        return false;
    }
}

inference::InferenceResponse GrpcClient::Predict(const std::vector<float>& observation, 
                                               const std::string& model_type,
                                               bool deterministic) {
    inference::InferenceResponse response;
    
    if (!connected_) {
        response.set_success(false);
        response.set_error_message("Not connected to server");
        return response;
    }
    
    try {
        grpc::ClientContext context;
        context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(10));
        
        inference::InferenceRequest request;
        
        // 设置观察数据
        for (float obs : observation) {
            request.add_observation(obs);
        }
        
        // 设置desired_goal (1个值，通常为0.0表示任务未完成)
        request.add_desired_goal(0.0f);
        
        // 设置achieved_goal (1个值，通常为0.0表示任务未完成)
        request.add_achieved_goal(0.0f);
        
        // 设置其他参数
        request.set_model_type(model_type);
        request.set_deterministic(deterministic);
        
        // 发送请求
        grpc::Status status = stub_->Predict(&context, request, &response);
        
        if (!status.ok()) {
            response.set_success(false);
            response.set_error_message(status.error_message());
        }
        
    } catch (const std::exception& e) {
        response.set_success(false);
        response.set_error_message(std::string("Exception: ") + e.what());
    }
    
    return response;
}

bool GrpcClient::IsConnected() const {
    return connected_;
}

struct Acceleration {
    float ax;
    float ay;
    float az;
};

// 重力补偿函数（直接修改ImuData结构中的加速度值）
void gravity_compensation(const ImuData& imu, float g, Acceleration& acc) 
{
    // 角度转弧度（姿态角单位为度）
    const float roll_rad = imu.angle_roll * M_PI / 180.0f;
    const float pitch_rad = imu.angle_pitch * M_PI / 180.0f;

    // 计算当前姿态下重力在三轴的分量
    const float g_x = -g * std::sin(pitch_rad);
    const float g_y = g * std::sin(roll_rad) * std::cos(pitch_rad);
    const float g_z = g * std::cos(roll_rad) * std::cos(pitch_rad);

    // 重力补偿：从原始加速度中减去重力分量
    acc.ax = imu.acc_x - g_x;
    acc.ay = imu.acc_y - g_y;
    acc.az = imu.acc_z - g_z;
}

Observation ConvertRobotDataToObservation(const RobotData& robot_data, const std::vector<float>& action_data, const RobotMoveCommand& robot_move_command) {
    Observation obs;

    Acceleration acc;
    gravity_compensation(robot_data.imu, 9.80665f, acc);

    // 1. 身体线加速度 - 3个值（使用处理后的加速度）
    obs.data.push_back(acc.ax);
    obs.data.push_back(acc.ay);
    obs.data.push_back(acc.az);
    
    // 2. 身体角速度 (从IMU获取) - 3个值，将度数转换为弧度
    obs.data.push_back(robot_data.imu.angular_velocity_roll * M_PI / 180.0f);
    obs.data.push_back(robot_data.imu.angular_velocity_pitch * M_PI / 180.0f);
    obs.data.push_back(robot_data.imu.angular_velocity_yaw * M_PI / 180.0f);
    
    // 3. 身体方向 (从IMU获取欧拉角) - 3个值，将度数转换为弧度
    obs.data.push_back(robot_data.imu.angle_roll * M_PI / 180.0f);
    obs.data.push_back(robot_data.imu.angle_pitch * M_PI / 180.0f);
    // obs.data.push_back(robot_data.imu.angle_yaw * M_PI / 180.0f);
    obs.data.push_back(0.0f); // 不使用 robot_data.imu.angle_yaw，因为这是全局坐标，此处在训练的时候恒为0 
    
    // 4. 命令值 (这里用零向量，实际应该从外部传入) - 4个值
    obs.data.push_back(robot_move_command.forward_speed);  // 线速度命令 x
    obs.data.push_back(robot_move_command.left_speed);  // 线速度命令 y
    obs.data.push_back(0.0f);  // 线速度命令 z
    obs.data.push_back(robot_move_command.turn_speed);  // 角速度命令 z
    
    // 5. 方波信号 - 1个值
    // 设置方波生成器的时间步长，200Hz，0.005s
    square_wave_generator.set_dt(0.005f);
    // 设置方波参数，参考 Lite3_confg.py
    square_wave_generator.set_foot_square_wave(0.5f, 0.8f, 0.2f);  // p5=0.5, phase_freq=1.0, eps=0.1 
    // 计算方波信号，使用前向速度作为输入
    float square_wave = square_wave_generator.compute_square_wave(robot_move_command.forward_speed);
    obs.data.push_back(square_wave);
    
    // 6. 关节位置相对于中性位置的偏差 (12个值)
    // 中性位置通常为站立姿态的关节角度
    // 此处参考训练代码中的neutral_joint_angles
    // # Init the robot in a standing position. Keep the order of the joints same as the joint_names 
    // # for reset basic pos or computing the reward easily.
    // "neutral_joint_angles" : {"FL_HipX_joint": 0.0, "FL_HipY_joint": -0.8, "FL_Knee_joint": 1.5,
    //                         "FR_HipX_joint": 0.0, "FR_HipY_joint": -0.8, "FR_Knee_joint": 1.5,
    //                         "HL_HipX_joint": 0.0, "HL_HipY_joint": -1.0, "HL_Knee_joint": 1.5,
    //                         "HR_HipX_joint": 0.0, "HR_HipY_joint": -1.0, "HR_Knee_joint": 1.5},
    const std::vector<float> neutral_joint_values = {
        0.0f, -1.0f, 1.8f,  // FL: hip, thigh, calf
        0.0f, -1.0f, 1.8f,  // FR: hip, thigh, calf
        0.0f, -1.0f, 1.8f,  // HL: hip, thigh, calf
        0.0f, -1.0f, 1.8f   // HR: hip, thigh, calf
    };
    
    for (int i = 0; i < 12; ++i) {
        obs.data.push_back(robot_data.joint_data.joint_data[i].position - neutral_joint_values[i]);
    }
    
    // 7. 关节速度 (12个值)
    for (int i = 0; i < 12; ++i) {
        obs.data.push_back(robot_data.joint_data.joint_data[i].velocity);
    }
    
    // 8. 动作 (从上一时刻的动作获取) - 12个值
    for (int i = 0; i < 12; ++i) {
        obs.data.push_back(action_data[i]);
    }
    
    // 9. 身体高度 16 个浮点数，计算周边16个点位，从激光雷达获取。当前训练用的16个0值
    for (int i = 0; i < 16; ++i) {
        obs.data.push_back(0.0f);
    }
    
    return obs;
}

// 配置参数，对应Python代码中的LeggedObsConfig
struct LeggedObsConfig {
    struct Scale {
        float lin_vel = 2.0f;
        float ang_vel = 0.25f;
        float qpos = 1.0f;
        float qvel = 0.05f;
        float height = 5.0f;
    } scale;
    
    struct Noise {
        float noise_level = 1.0f;
        float qpos = 0.01f;
        float qvel = 1.5f;
        float lin_vel = 0.1f;
        float ang_vel = 0.2f;
        float orientation = 0.05f;
        float height = 0.1f;
    } noise;
};

// 全局配置实例
static LeggedObsConfig g_legged_obs_config;

/// @brief 生成[-1, 1]范围内的均匀分布随机数
float generateUniformNoise() {
    if (!random_initialized) {
        srand(static_cast<unsigned int>(time(nullptr)));
        random_initialized = true;
    }
    return (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
}

/// @brief 获取缩放向量，对应Python代码中的_get_obs_scale_vec
std::vector<float> getObsScaleVec() {
    std::vector<float> scale_vec;
    
    // 身体线速度缩放 (3个值)
    for (int i = 0; i < 3; ++i) {
        scale_vec.push_back(g_legged_obs_config.scale.lin_vel);
    }
    
    // 身体角速度缩放 (3个值)
    for (int i = 0; i < 3; ++i) {
        scale_vec.push_back(g_legged_obs_config.scale.ang_vel);
    }
    
    // 身体方向缩放 (3个值，无缩放)
    for (int i = 0; i < 3; ++i) {
        scale_vec.push_back(1.0f);
    }
    
    // 命令值缩放 (4个值)
    scale_vec.push_back(g_legged_obs_config.scale.lin_vel);  // Cmd Vx
    scale_vec.push_back(g_legged_obs_config.scale.lin_vel);  // Cmd Vy
    scale_vec.push_back(g_legged_obs_config.scale.lin_vel);  // Cmd Vz
    scale_vec.push_back(g_legged_obs_config.scale.ang_vel);  // Cmd Yaw

    // 方波, 无缩放（1个值）
    scale_vec.push_back(1.0f);
    
    // 关节位置偏差缩放 (12个值)
    for (int i = 0; i < 12; ++i) {
        scale_vec.push_back(g_legged_obs_config.scale.qpos);
    }
    
    // 关节速度缩放 (12个值)
    for (int i = 0; i < 12; ++i) {
        scale_vec.push_back(g_legged_obs_config.scale.qvel);
    }
    
    // 动作缩放 (12个值，无缩放)
    for (int i = 0; i < 12; ++i) {
        scale_vec.push_back(1.0f);
    }
    
    // 身体高度缩放 (16个值)
    for (int i = 0; i < 16; ++i) {
        scale_vec.push_back(g_legged_obs_config.scale.height);
    }
    
    return scale_vec;
}

/// @brief 获取噪声向量，对应Python代码中的_get_noise_scale_vec
std::vector<float> getNoiseScaleVec() {
    std::vector<float> noise_vec;
    
    // 身体线速度噪声 (3个值)
    for (int i = 0; i < 3; ++i) {
        noise_vec.push_back(g_legged_obs_config.noise.noise_level * 
                           g_legged_obs_config.noise.lin_vel * 
                           g_legged_obs_config.scale.lin_vel);
    }
    
    // 身体角速度噪声 (3个值)
    for (int i = 0; i < 3; ++i) {
        noise_vec.push_back(g_legged_obs_config.noise.noise_level * 
                           g_legged_obs_config.noise.ang_vel * 
                           g_legged_obs_config.scale.ang_vel);
    }
    
    // 身体方向噪声 (3个值)
    for (int i = 0; i < 3; ++i) {
        noise_vec.push_back(g_legged_obs_config.noise.noise_level * 
                           g_legged_obs_config.noise.orientation);
    }
    
    // 命令值噪声 (4个值，无噪声)
    for (int i = 0; i < 4; ++i) {
        noise_vec.push_back(0.0f);
    }
    
    // 方波噪声 (1个值，无噪声)
    noise_vec.push_back(0.0f);
    
    // 关节位置偏差噪声 (12个值)
    for (int i = 0; i < 12; ++i) {
        noise_vec.push_back(g_legged_obs_config.noise.noise_level * 
                           g_legged_obs_config.noise.qpos * 
                           g_legged_obs_config.scale.qpos);
    }
    
    // 关节速度噪声 (12个值)
    for (int i = 0; i < 12; ++i) {
        noise_vec.push_back(g_legged_obs_config.noise.noise_level * 
                           g_legged_obs_config.noise.qvel * 
                           g_legged_obs_config.scale.qvel);
    }
    
    // 动作噪声 (12个值，无噪声)
    for (int i = 0; i < 12; ++i) {
        noise_vec.push_back(0.0f);
    }
    
    // 身体高度噪声 (16个值)
    for (int i = 0; i < 16; ++i) {
        noise_vec.push_back(g_legged_obs_config.noise.noise_level * 
                           g_legged_obs_config.noise.height * 
                           g_legged_obs_config.scale.height);
    }
    
    return noise_vec;
}

Observation ApplyObservationScalingAndNoise(const Observation& obs) {
    Observation processed_obs;
    
    if (obs.data.size() != 66) {
        std::cerr << "Warning: Observation data size is " << obs.data.size() 
                  << " (expected 65), skipping scaling and noise" << std::endl;
        return obs;
    }
    
    // 获取缩放和噪声向量
    std::vector<float> scale_vec = getObsScaleVec();
    std::vector<float> noise_vec = getNoiseScaleVec();
    
    // 应用缩放和噪声
    for (size_t i = 0; i < obs.data.size(); ++i) {
        float scaled_value = obs.data[i] * scale_vec[i];
        float noise = noise_vec[i] * generateUniformNoise();
        processed_obs.data.push_back(scaled_value + noise);
    }
    
    return processed_obs;
}

RobotCmd CreateRobotCmd(const RobotAction& action) {
    RobotCmd cmd;
    
    // 初始化所有关节命令
    for (int i = 0; i < 12; ++i) {
        cmd.joint_cmd[i].position = 0.0f;
        cmd.joint_cmd[i].velocity = 0.0f;
        cmd.joint_cmd[i].torque = 0.0f;
        cmd.joint_cmd[i].kp = 0.0f;  // 默认比例增益
        cmd.joint_cmd[i].kd = 0.0f;   // 默认微分增益
    }

    const std::vector<float> neutral_joint_values = {
        0.0f, -0.8f, 1.5f,  // FL: hip, thigh, calf
        0.0f, -0.8f, 1.5f,  // FR: hip, thigh, calf
        0.0f, -1.0f, 1.5f,  // HL: hip, thigh, calf
        0.0f, -1.0f, 1.5f   // HR: hip, thigh, calf
    };


    // 如果动作数据足够，则设置关节位置
    if (action.data.size() >= 12) {
        for (int i = 0; i < 12; ++i) {
            cmd.joint_cmd[i].position = action.data[i] + neutral_joint_values[i];
        }
    }
    
    return cmd;
}

RobotAction ConvertResponseToAction(const inference::InferenceResponse& response) {
    RobotAction action;
    
    // 定义动作缩放因子，对应12个关节
    // 顺序：FL_HipX, FL_HipY, FL_Knee, FR_HipX, FR_HipY, FR_Knee, HL_HipX, HL_HipY, HL_Knee, HR_HipX, HR_HipY, HR_Knee
    const std::vector<float> action_scale = {
        0.2f,    // FL_HipX_joint: range="-0.523 0.523", neutral=0.0
        1.0f,    // FL_HipY_joint: range="-2.67 0.314", neutral=-1.0
        0.8f,    // FL_Knee_joint: range="0.524 2.792", neutral=1.8
        
        0.2f,    // FR_HipX_joint: range="-0.523 0.523", neutral=0.0
        1.0f,    // FR_HipY_joint: range="-2.67 0.314", neutral=-1.0
        0.8f,    // FR_Knee_joint: range="0.524 2.792", neutral=1.8
        
        0.2f,    // HL_HipX_joint: range="-0.523 0.523", neutral=0.0
        1.0f,    // HL_HipY_joint: range="-2.67 0.314", neutral=-1.0
        0.8f,    // HL_Knee_joint: range="0.524 2.792", neutral=1.8
        
        0.2f,    // HR_HipX_joint: range="-0.523 0.523", neutral=0.0
        1.0f,    // HR_HipY_joint: range="-2.67 0.314", neutral=-1.0
        0.8f,    // HR_Knee_joint: range="0.524 2.792", neutral=1.8
    };
    
    if (response.success()) {
        // 将响应中的动作数据复制到RobotAction结构，并应用缩放
        for (int i = 0; i < response.action_size(); ++i) {
            float scaled_action = response.action(i);
            
            // 应用缩放因子（如果索引在范围内）
            if (i < action_scale.size()) {
                scaled_action *= action_scale[i];
            }
            
            action.data.push_back(scaled_action);
        }
    } else {
        std::cerr << "Inference failed: " << response.error_message() << std::endl;
    }
    
    return action;
} 