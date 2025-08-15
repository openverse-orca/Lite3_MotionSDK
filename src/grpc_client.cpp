#include "grpc_client.h"
#include <iostream>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>

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

Observation ConvertRobotDataToObservation(const RobotData& robot_data) {
    Observation obs;
    
    // 添加IMU数据 (9个值)
    obs.data.push_back(robot_data.imu.angle_roll);
    obs.data.push_back(robot_data.imu.angle_pitch);
    obs.data.push_back(robot_data.imu.angle_yaw);
    obs.data.push_back(robot_data.imu.angular_velocity_roll);
    obs.data.push_back(robot_data.imu.angular_velocity_pitch);
    obs.data.push_back(robot_data.imu.angular_velocity_yaw);
    obs.data.push_back(robot_data.imu.acc_x);
    obs.data.push_back(robot_data.imu.acc_y);
    obs.data.push_back(robot_data.imu.acc_z);
    
    // 添加关节位置数据 (12个值)
    for (int i = 0; i < 12; ++i) {
        obs.data.push_back(robot_data.joint_data.joint_data[i].position);
    }
    
    // 添加关节速度数据 (12个值)
    for (int i = 0; i < 12; ++i) {
        obs.data.push_back(robot_data.joint_data.joint_data[i].velocity);
    }
    
    // 添加接触力数据 (12个值)
    for (int i = 0; i < 12; ++i) {
        obs.data.push_back(static_cast<float>(robot_data.contact_force.leg_force[i]));
    }
    
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