#include "grpc_client.h"
#include "../include/velocity_calculator.h"
#include <iostream>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>

// 全局速度计算器实例
static VelocityCalculator velocity_calculator(10, 0.1f);
static float last_timestamp = 0.0f;

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

Observation ConvertRobotDataToObservation(const RobotData& robot_data, const std::vector<float>& action_data) {
    Observation obs;
    
    // 计算时间间隔（使用RobotData中的timestamp）
    float current_timestamp = static_cast<float>(robot_data.tick) / 1000.0f;  // 转换为秒
    float dt = current_timestamp - last_timestamp;
    last_timestamp = current_timestamp;
    
    // 确保时间间隔合理（避免第一次调用时dt过大或为负）
    if (dt <= 0.0f || dt > 0.1f) dt = 0.01f;  // 限制时间间隔在1-100ms之间
    
    // 使用VelocityCalculator计算精确的线速度
    VelocityCalculator::Velocity3D body_lin_vel = velocity_calculator.updateVelocity(robot_data.imu, dt);
    
    // 1. 身体线速度 (使用VelocityCalculator计算的精确值) - 3个值
    obs.data.push_back(body_lin_vel.vx);
    obs.data.push_back(body_lin_vel.vy);
    obs.data.push_back(body_lin_vel.vz);
    
    // 2. 身体角速度 (从IMU获取) - 3个值
    obs.data.push_back(robot_data.imu.angular_velocity_roll);
    obs.data.push_back(robot_data.imu.angular_velocity_pitch);
    obs.data.push_back(robot_data.imu.angular_velocity_yaw);
    
    // 3. 身体方向 (从IMU获取欧拉角) - 3个值
    obs.data.push_back(robot_data.imu.angle_roll);
    obs.data.push_back(robot_data.imu.angle_pitch);
    obs.data.push_back(0.0f); // 不使用 robot_data.imu.angle_yaw，因为这是全局坐标，此处在训练的时候恒为0 
    
    // 4. 命令值 (这里用零向量，实际应该从外部传入) - 3个值
    obs.data.push_back(0.0f);  // 线速度命令 x
    obs.data.push_back(0.0f);  // 线速度命令 y
    obs.data.push_back(0.0f);  // 角速度命令 z
    
    // 5. 关节位置相对于中性位置的偏差 (12个值)
    // 中性位置通常为站立姿态的关节角度
    // 此处参考训练代码中的neutral_joint_angles
    // # Init the robot in a standing position. Keep the order of the joints same as the joint_names 
    // # for reset basic pos or computing the reward easily.
    // "neutral_joint_angles" : {"FL_HipX_joint": 0.0, "FL_HipY_joint": -0.8, "FL_Knee_joint": 1.5,
    //                         "FR_HipX_joint": 0.0, "FR_HipY_joint": -0.8, "FR_Knee_joint": 1.5,
    //                         "HL_HipX_joint": 0.0, "HL_HipY_joint": -1.0, "HL_Knee_joint": 1.5,
    //                         "HR_HipX_joint": 0.0, "HR_HipY_joint": -1.0, "HR_Knee_joint": 1.5},
    const std::vector<float> neutral_joint_values = {
        0.0f, -0.8f, 1.5f,  // FL: hip, thigh, calf
        0.0f, -0.8f, 1.5f,  // FR: hip, thigh, calf
        0.0f, -1.0f, 1.5f,  // HL: hip, thigh, calf
        0.0f, -1.0f, 1.5f   // HR: hip, thigh, calf
    };
    
    for (int i = 0; i < 12; ++i) {
        obs.data.push_back(robot_data.joint_data.joint_data[i].position - neutral_joint_values[i]);
    }
    
    // 6. 关节速度 (12个值)
    for (int i = 0; i < 12; ++i) {
        obs.data.push_back(robot_data.joint_data.joint_data[i].velocity);
    }
    
    // 7. 动作 (从上一时刻的动作获取) - 12个值
    for (int i = 0; i < 12; ++i) {
        obs.data.push_back(action_data[i]);
    }
    
    // 8. 身体高度 16 个浮点数，计算周边16个点位，从激光雷达获取。当前训练用的16个0值
    for (int i = 0; i < 16; ++i) {
        obs.data.push_back(0.0f);
    }
    
    // 9. 额外的值（可能是achieved_goal或desired_goal，或者其他任务相关的值）
    obs.data.push_back(0.0f);  // 添加第65个值
    
    return obs;
}

RobotCmd CreateRobotCmd(const RobotAction& action) {
    RobotCmd cmd;
    
    // 初始化所有关节命令
    for (int i = 0; i < 12; ++i) {
        cmd.joint_cmd[i].position = 0.0f;
        cmd.joint_cmd[i].velocity = 0.0f;
        cmd.joint_cmd[i].torque = 0.0f;
        cmd.joint_cmd[i].kp = 20.0f;  // 默认比例增益
        cmd.joint_cmd[i].kd = 0.5f;   // 默认微分增益
    }
    
    // 如果动作数据足够，则设置关节位置
    if (action.data.size() >= 12) {
        for (int i = 0; i < 12; ++i) {
            cmd.joint_cmd[i].position = action.data[i];
        }
    }
    
    return cmd;
}

RobotAction ConvertResponseToAction(const inference::InferenceResponse& response) {
    RobotAction action;
    
    if (response.success()) {
        // 将响应中的动作数据复制到RobotAction结构
        for (int i = 0; i < response.action_size(); ++i) {
            action.data.push_back(response.action(i));
        }
    } else {
        std::cerr << "Inference failed: " << response.error_message() << std::endl;
    }
    
    return action;
} 