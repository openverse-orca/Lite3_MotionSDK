#ifndef GRPC_CLIENT_H
#define GRPC_CLIENT_H

#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "robot_types.h"
#include "inference.grpc.pb.h"

// 前向声明
namespace inference {
    class InferenceService;
}

/// @brief gRPC客户端类，用于与推理服务器通信
class GrpcClient {
public:
    /// @brief 构造函数
    /// @param server_address 服务器地址，格式为 "ip:port"
    explicit GrpcClient(const std::string& server_address);
    
    /// @brief 析构函数
    ~GrpcClient();
    
    /// @brief 连接到服务器
    /// @return 连接是否成功
    bool Connect();
    
    /// @brief 发送推理请求并获取响应
    /// @param observation 观察数据
    /// @param model_type 模型类型标识
    /// @param deterministic 是否确定性推理
    /// @return 推理响应
    inference::InferenceResponse Predict(const std::vector<float>& observation, 
                                       const std::string& model_type = "default",
                                       bool deterministic = true);
    
    /// @brief 检查连接状态
    /// @return 是否已连接
    bool IsConnected() const;

private:
    std::string server_address_;
    std::unique_ptr<inference::InferenceService::Stub> stub_;
    std::shared_ptr<grpc::Channel> channel_;
    bool connected_;
};

/// @brief 观察数据结构，用于存储机器人状态
struct Observation {
    std::vector<float> data;  // 观察数据向量
};

/// @brief 动作数据结构，用于存储控制指令
struct RobotAction {
    std::vector<float> data;  // 动作数据向量
};

/// @brief 将RobotData转换为Observation
/// @param robot_data 机器人数据
/// @param action_data 上一时刻的动作数据
/// @return 观察数据
Observation ConvertRobotDataToObservation(const RobotData& robot_data, const std::vector<float>& action_data, const RobotMoveCommand& robot_move_command);

/// @brief 将RobotAction转换为RobotCmd
/// @param action 动作数据
/// @return 机器人命令
RobotCmd CreateRobotCmd(const RobotAction& action);

/// @brief 将InferenceResponse转换为RobotAction
/// @param response 推理响应
/// @return 动作数据
RobotAction ConvertResponseToAction(const inference::InferenceResponse& response);

#endif // GRPC_CLIENT_H 