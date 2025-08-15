/// @file test_grpc_client.cpp
/// @brief 测试程序，用于验证GRPC推理服务器的通信流程
/// @version 0.1
/// @date 2024-01-01

#include <iostream>
#include <memory>
#include <vector>
#include <cstring>
#include <cmath>
#include <algorithm>
#include "robot_types.h"
#include "grpc_client.h"
#include "utils.h"

using namespace std;

/// @brief 创建模拟的RobotData用于测试
/// @return 包含测试数据的RobotData结构
RobotData CreateMockRobotData() {
    RobotData robot_data;
    memset(&robot_data, 0, sizeof(robot_data));
    
    // 设置时间戳
    robot_data.tick = 1000;
    
    // 设置IMU数据 - 模拟机器人站立状态
    robot_data.imu.angle_roll = 0.0f;      // 横滚角 0度
    robot_data.imu.angle_pitch = 0.0f;     // 俯仰角 0度
    robot_data.imu.angle_yaw = 0.0f;       // 偏航角 0度
    robot_data.imu.angular_velocity_roll = 0.0f;   // 横滚角速度
    robot_data.imu.angular_velocity_pitch = 0.0f;  // 俯仰角速度
    robot_data.imu.angular_velocity_yaw = 0.0f;    // 偏航角速度
    robot_data.imu.acc_x = 0.0f;           // X轴加速度
    robot_data.imu.acc_y = 0.0f;           // Y轴加速度
    robot_data.imu.acc_z = 9.81f;          // Z轴加速度 (重力)
    
    // 设置关节数据 - 模拟站立姿态
    // 前左腿 (FL)
    robot_data.joint_data.fl_leg[0].position = 0.0f;        // 髋关节
    robot_data.joint_data.fl_leg[1].position = -0.8f;     // 大腿关节 (-57度)
    robot_data.joint_data.fl_leg[2].position = 1.5f;      // 小腿关节 (103度)
    robot_data.joint_data.fl_leg[0].velocity = 0.0f;
    robot_data.joint_data.fl_leg[1].velocity = 0.0f;
    robot_data.joint_data.fl_leg[2].velocity = 0.0f;
    robot_data.joint_data.fl_leg[0].torque = 0.0f;
    robot_data.joint_data.fl_leg[1].torque = 0.0f;
    robot_data.joint_data.fl_leg[2].torque = 0.0f;
    robot_data.joint_data.fl_leg[0].temperature = 25.0f;
    robot_data.joint_data.fl_leg[1].temperature = 25.0f;
    robot_data.joint_data.fl_leg[2].temperature = 25.0f;
    
    // 前右腿 (FR) - 与前左腿对称
    robot_data.joint_data.fr_leg[0].position = 0.0f;
    robot_data.joint_data.fr_leg[1].position = -0.8f;
    robot_data.joint_data.fr_leg[2].position = 1.5f;
    robot_data.joint_data.fr_leg[0].velocity = 0.0f;
    robot_data.joint_data.fr_leg[1].velocity = 0.0f;
    robot_data.joint_data.fr_leg[2].velocity = 0.0f;
    robot_data.joint_data.fr_leg[0].torque = 0.0f;
    robot_data.joint_data.fr_leg[1].torque = 0.0f;
    robot_data.joint_data.fr_leg[2].torque = 0.0f;
    robot_data.joint_data.fr_leg[0].temperature = 25.0f;
    robot_data.joint_data.fr_leg[1].temperature = 25.0f;
    robot_data.joint_data.fr_leg[2].temperature = 25.0f;
    
    // 后左腿 (HL) - 与前左腿相同
    robot_data.joint_data.hl_leg[0].position = 0.0f;
    robot_data.joint_data.hl_leg[1].position = -1.0f;
    robot_data.joint_data.hl_leg[2].position = 1.5f;
    robot_data.joint_data.hl_leg[0].velocity = 0.0f;
    robot_data.joint_data.hl_leg[1].velocity = 0.0f;
    robot_data.joint_data.hl_leg[2].velocity = 0.0f;
    robot_data.joint_data.hl_leg[0].torque = 0.0f;
    robot_data.joint_data.hl_leg[1].torque = 0.0f;
    robot_data.joint_data.hl_leg[2].torque = 0.0f;
    robot_data.joint_data.hl_leg[0].temperature = 25.0f;
    robot_data.joint_data.hl_leg[1].temperature = 25.0f;
    robot_data.joint_data.hl_leg[2].temperature = 25.0f;
    
    // 后右腿 (HR) - 与前右腿相同
    robot_data.joint_data.hr_leg[0].position = 0.0f;
    robot_data.joint_data.hr_leg[1].position = -1.0f;
    robot_data.joint_data.hr_leg[2].position = 1.5f;
    robot_data.joint_data.hr_leg[0].velocity = 0.0f;
    robot_data.joint_data.hr_leg[1].velocity = 0.0f;
    robot_data.joint_data.hr_leg[2].velocity = 0.0f;
    robot_data.joint_data.hr_leg[0].torque = 0.0f;
    robot_data.joint_data.hr_leg[1].torque = 0.0f;
    robot_data.joint_data.hr_leg[2].torque = 0.0f;
    robot_data.joint_data.hr_leg[0].temperature = 25.0f;
    robot_data.joint_data.hr_leg[1].temperature = 25.0f;
    robot_data.joint_data.hr_leg[2].temperature = 25.0f;
    
    // 设置接触力数据 - 模拟四足着地
    for (int i = 0; i < 12; ++i) {
        robot_data.contact_force.leg_force[i] = 0.0;  // 初始化为0
    }
    // 设置Z轴接触力 (模拟重力分布)
    robot_data.contact_force.fl_leg[2] = 24.5;  // 前左腿Z轴力
    robot_data.contact_force.fr_leg[2] = 24.5;  // 前右腿Z轴力
    robot_data.contact_force.hl_leg[2] = 24.5;  // 后左腿Z轴力
    robot_data.contact_force.hr_leg[2] = 24.5;  // 后右腿Z轴力
    
    return robot_data;
}

/// @brief 创建模拟的action数据用于测试
/// @return 包含测试数据的action向量
std::vector<float> CreateMockActionData() {
    std::vector<float> action_data(12, 0.0f);
    
    // 设置一些模拟的动作数据（关节位置命令）
    // 这些值应该与neutral_joint_values对应
    action_data[0] = 0.0f;   // FL_HipX_joint
    action_data[1] = 0.0f;  // FL_HipY_joint
    action_data[2] = 0.0f;   // FL_Knee_joint
    
    action_data[3] = 0.0f;   // FR_HipX_joint
    action_data[4] = 0.0f;  // FR_HipY_joint
    action_data[5] = 0.0f;   // FR_Knee_joint
    
    action_data[6] = 0.0f;   // HL_HipX_joint
    action_data[7] = 0.0f;  // HL_HipY_joint
    action_data[8] = 0.0f;   // HL_Knee_joint
    
    action_data[9] = 0.0f;   // HR_HipX_joint
    action_data[10] = 0.0f; // HR_HipY_joint
    action_data[11] = 0.0f;  // HR_Knee_joint
    
    return action_data;
}

/// @brief 打印Observation数据
/// @param obs 观察数据
void PrintObservation(const Observation& obs) {
    cout << "=== Observation Data ===" << endl;
    cout << "Data size: " << obs.data.size() << endl;
    
    if (obs.data.size() >= 65) {
        cout << "Body Linear Velocity (3 values):" << endl;
        cout << "  Vx: " << obs.data[0] << " m/s" << endl;
        cout << "  Vy: " << obs.data[1] << " m/s" << endl;
        cout << "  Vz: " << obs.data[2] << " m/s" << endl;
        
        cout << "Body Angular Velocity (3 values):" << endl;
        cout << "  Roll Vel: " << obs.data[3] << " deg/s" << endl;
        cout << "  Pitch Vel: " << obs.data[4] << " deg/s" << endl;
        cout << "  Yaw Vel: " << obs.data[5] << " deg/s" << endl;
        
        cout << "Body Orientation (3 values):" << endl;
        cout << "  Roll: " << obs.data[6] << " deg" << endl;
        cout << "  Pitch: " << obs.data[7] << " deg" << endl;
        cout << "  Yaw: " << obs.data[8] << " deg" << endl;
        
        cout << "Command Values (4 values):" << endl;
        cout << "  Cmd Vx: " << obs.data[9] << " m/s" << endl;
        cout << "  Cmd Vy: " << obs.data[10] << " m/s" << endl;
        cout << "  Cmd Vz: " << obs.data[11] << " m/s" << endl;
        cout << "  Cmd Yaw: " << obs.data[12] << " rad/s" << endl;
        
        cout << "Joint Position Deviations (12 values):" << endl;
        for (int i = 0; i < 12; ++i) {
            cout << "  J" << i << ": " << obs.data[13 + i] << " rad" << endl;
        }
        
        cout << "Joint Velocities (12 values):" << endl;
        for (int i = 0; i < 12; ++i) {
            cout << "  J" << i << ": " << obs.data[25 + i] << " rad/s" << endl;
        }
        
        cout << "Previous Actions (12 values):" << endl;
        for (int i = 0; i < 12; ++i) {
            cout << "  A" << i << ": " << obs.data[37 + i] << " rad" << endl;
        }
        
        cout << "Height Map (16 values):" << endl;
        for (int i = 0; i < 16; ++i) {
            cout << "  H" << i << ": " << obs.data[49 + i] << " m" << endl;
        }
    } else {
        cout << "Warning: Observation data size is " << obs.data.size() << " (expected 65)" << endl;
    }
    cout << endl;
}

/// @brief 打印RobotAction数据
/// @param action 动作数据
void PrintRobotAction(const RobotAction& action) {
    cout << "=== Robot Action Data ===" << endl;
    cout << "Action size: " << action.data.size() << endl;
    
    if (action.data.size() >= 12) {
        cout << "Joint Commands (12 values):" << endl;
        const char* leg_names[] = {"FL", "FR", "HL", "HR"};
        const char* joint_names[] = {"Hip", "Thigh", "Calf"};
        
        for (int leg = 0; leg < 4; ++leg) {
            cout << "  " << leg_names[leg] << " Leg:" << endl;
            for (int joint = 0; joint < 3; ++joint) {
                int idx = leg * 3 + joint;
                cout << "    " << joint_names[joint] << ": " 
                     << action.data[idx] << " rad (" 
                     << action.data[idx] * 180.0 / M_PI << " deg)" << endl;
            }
        }
    }
    cout << endl;
}

/// @brief 打印RobotCmd数据
/// @param cmd 机器人命令
void PrintRobotCmd(const RobotCmd& cmd) {
    cout << "=== Robot Command Data ===" << endl;
    
    const char* leg_names[] = {"FL", "FR", "HL", "HR"};
    const char* joint_names[] = {"Hip", "Thigh", "Calf"};
    const JointCmd* legs[] = {cmd.fl_leg, cmd.fr_leg, cmd.hl_leg, cmd.hr_leg};
    
    for (int leg = 0; leg < 4; ++leg) {
        cout << "  " << leg_names[leg] << " Leg:" << endl;
        for (int joint = 0; joint < 3; ++joint) {
            const JointCmd& joint_cmd = legs[leg][joint];
            cout << "    " << joint_names[joint] << ":" << endl;
            cout << "      Position: " << joint_cmd.position << " rad (" 
                 << joint_cmd.position * 180.0 / M_PI << " deg)" << endl;
            cout << "      Velocity: " << joint_cmd.velocity << " rad/s" << endl;
            cout << "      Torque: " << joint_cmd.torque << " Nm" << endl;
            cout << "      Kp: " << joint_cmd.kp << endl;
            cout << "      Kd: " << joint_cmd.kd << endl;
        }
    }
    cout << endl;
}

/// @brief 验证数据格式
/// @param robot_data 原始机器人数据
/// @param obs 观察数据
/// @param action 动作数据
/// @param cmd 机器人命令
void ValidateDataFormat(const RobotData& robot_data, 
                       const Observation& obs, 
                       const RobotAction& action, 
                       const RobotCmd& cmd) {
    cout << "=== Data Format Validation ===" << endl;
    
    // 验证Observation数据大小
    if (obs.data.size() == 65) {
        cout << "✓ Observation data size is correct (65 values)" << endl;
    } else {
        cout << "✗ Observation data size is incorrect: " << obs.data.size() << " (expected 65)" << endl;
    }
    
    // 验证Action数据大小
    if (action.data.size() >= 12) {
        cout << "✓ Action data size is sufficient (≥12 values)" << endl;
    } else {
        cout << "✗ Action data size is insufficient: " << action.data.size() << " (expected ≥12)" << endl;
    }
    
    // 验证IMU数据一致性
    bool imu_consistent = true;
    if (obs.data.size() >= 9) {
        // 检查身体方向数据（欧拉角）
        imu_consistent = (abs(obs.data[6] - robot_data.imu.angle_roll) < 1e-6) &&
                        (abs(obs.data[7] - robot_data.imu.angle_pitch) < 1e-6) &&
                        (abs(obs.data[8] - 0.0f) < 1e-6); // Yaw在训练时恒为0
    }
    
    if (imu_consistent) {
        cout << "✓ IMU data consistency verified" << endl;
    } else {
        cout << "✗ IMU data consistency check failed" << endl;
    }
    
    // 验证关节数据一致性
    bool joint_consistent = true;
    if (obs.data.size() >= 37) {
        // 检查关节位置偏差数据（相对于中性位置的偏差）
        const std::vector<float> neutral_joint_values = {
            0.0f, -0.8f, 1.5f,  // FL: hip, thigh, calf
            0.0f, -0.8f, 1.5f,  // FR: hip, thigh, calf
            0.0f, -1.0f, 1.5f,  // HL: hip, thigh, calf
            0.0f, -1.0f, 1.5f   // HR: hip, thigh, calf
        };
        
        for (int i = 0; i < 12; ++i) {
            float expected_deviation = robot_data.joint_data.joint_data[i].position - neutral_joint_values[i];
            if (abs(obs.data[13 + i] - expected_deviation) > 1e-6) {
                joint_consistent = false;
                break;
            }
        }
    }
    
    if (joint_consistent) {
        cout << "✓ Joint position data consistency verified" << endl;
    } else {
        cout << "✗ Joint position data consistency check failed" << endl;
    }
    
    // 验证RobotCmd的合理性
    bool cmd_reasonable = true;
    const JointCmd* legs[] = {cmd.fl_leg, cmd.fr_leg, cmd.hl_leg, cmd.hr_leg};
    
    for (int leg = 0; leg < 4; ++leg) {
        for (int joint = 0; joint < 3; ++joint) {
            const JointCmd& joint_cmd = legs[leg][joint];
            // 检查位置是否在合理范围内 (-π 到 π)
            if (joint_cmd.position < -M_PI || joint_cmd.position > M_PI) {
                cmd_reasonable = false;
                cout << "✗ Joint position out of range: " << joint_cmd.position << endl;
            }
            // 检查增益是否为正数
            if (joint_cmd.kp <= 0 || joint_cmd.kd < 0) {
                cmd_reasonable = false;
                cout << "✗ Invalid gains: Kp=" << joint_cmd.kp << ", Kd=" << joint_cmd.kd << endl;
            }
        }
    }
    
    if (cmd_reasonable) {
        cout << "✓ RobotCmd data is reasonable" << endl;
    } else {
        cout << "✗ RobotCmd data validation failed" << endl;
    }
    
    cout << endl;
}

int main(int argc, char* argv[]) {
    cout << "=== GRPC Inference Server Test Program ===" << endl;
    cout << "This program tests the complete pipeline:" << endl;
    cout << "1. Create mock RobotData" << endl;
    cout << "2. Create mock action data" << endl;
    cout << "3. Convert to Observation" << endl;
    cout << "4. Send to GRPC server" << endl;
    cout << "5. Receive inference result" << endl;
    cout << "6. Convert to RobotCmd" << endl;
    cout << "7. Validate data format" << endl;
    cout << endl;
    
    // 获取服务器地址
    std::string server_address = "localhost:50151";
    if (argc > 1) {
        server_address = argv[1];
    }
    cout << "Using server address: " << server_address << endl;
    cout << endl;
    
    try {
        // 步骤1: 创建模拟的RobotData
        cout << "Step 1: Creating mock RobotData..." << endl;
        RobotData robot_data = CreateMockRobotData();
        cout << "✓ Mock RobotData created successfully" << endl;
        cout << "  - Tick: " << robot_data.tick << endl;
        cout << "  - IMU roll: " << robot_data.imu.angle_roll << " deg" << endl;
        cout << "  - IMU pitch: " << robot_data.imu.angle_pitch << " deg" << endl;
        cout << "  - IMU yaw: " << robot_data.imu.angle_yaw << " deg" << endl;
        cout << endl;
        
        // 步骤2: 创建模拟的action数据
        cout << "Step 2: Creating mock action data..." << endl;
        std::vector<float> action_data = CreateMockActionData();
        cout << "✓ Mock action data created successfully" << endl;
        cout << "  - Action data size: " << action_data.size() << endl;
        cout << endl;
        
        // 步骤3: 转换为Observation
        cout << "Step 3: Converting RobotData to Observation..." << endl;
        Observation observation = ConvertRobotDataToObservation(robot_data, action_data);
        cout << "✓ Observation created successfully" << endl;
        cout << "Note: Observation data will be processed with new noise for each iteration" << endl;
        cout << endl;
        
        // 步骤4: 创建GRPC客户端并连接
        cout << "Step 4: Creating GRPC client and connecting..." << endl;
        std::unique_ptr<GrpcClient> client = std::make_unique<GrpcClient>(server_address);
        
        if (!client->Connect()) {
            cout << "✗ Failed to connect to GRPC server" << endl;
            cout << "Please make sure the inference server is running on " << server_address << endl;
            return -1;
        }
        cout << "✓ Successfully connected to GRPC server" << endl;
        cout << endl;
        
        // 步骤5: 循环发送100次推理请求并收集统计数据
        cout << "Step 5: Sending 100 inference requests with different noise each time..." << endl;
        const int num_iterations = 100;
        std::vector<std::vector<float>> all_actions;  // 存储所有返回的action数据
        
        for (int i = 0; i < num_iterations; ++i) {
            if (i % 10 == 0) {
                cout << "  Progress: " << i << "/" << num_iterations << " requests sent" << endl;
            }
            
            // 每次迭代都生成新的噪声
            Observation processed_observation = ApplyObservationScalingAndNoise(observation);
            
            inference::InferenceResponse response = client->Predict(processed_observation.data, "stand_still", true);
            
            if (!response.success()) {
                cout << "✗ Inference request " << i << " failed: " << response.error_message() << endl;
                continue;
            }
            
            RobotAction action = ConvertResponseToAction(response);
            all_actions.push_back(action.data);
        }
        
        cout << "✓ Successfully collected " << all_actions.size() << " action responses" << endl;
        cout << endl;
        
        // 步骤6: 计算统计信息
        cout << "Step 6: Calculating statistics..." << endl;
        if (all_actions.empty()) {
            cout << "✗ No successful responses to analyze" << endl;
            return -1;
        }
        
        const int num_joints = 12;  // 12个关节
        const int num_samples = all_actions.size();
        
        cout << "=== Action Statistics (based on " << num_samples << " samples) ===" << endl;
        
        // 关节名称
        const char* joint_names[] = {
            "FL_HipX", "FL_HipY", "FL_Knee",
            "FR_HipX", "FR_HipY", "FR_Knee", 
            "HL_HipX", "HL_HipY", "HL_Knee",
            "HR_HipX", "HR_HipY", "HR_Knee"
        };
        
        for (int joint = 0; joint < num_joints; ++joint) {
            std::vector<float> joint_values;
            joint_values.reserve(num_samples);
            
            // 收集该关节的所有值
            for (const auto& action : all_actions) {
                if (joint < action.size()) {
                    joint_values.push_back(action[joint]);
                }
            }
            
            if (joint_values.empty()) {
                cout << joint_names[joint] << ": No data available" << endl;
                continue;
            }
            
            // 排序用于计算中位数
            std::sort(joint_values.begin(), joint_values.end());
            
            // 计算统计值
            float sum = 0.0f;
            float min_val = joint_values[0];
            float max_val = joint_values.back();
            
            for (float val : joint_values) {
                sum += val;
            }
            float mean = sum / joint_values.size();
            
            // 计算标准差
            float variance = 0.0f;
            for (float val : joint_values) {
                variance += (val - mean) * (val - mean);
            }
            float std_dev = sqrt(variance / joint_values.size());
            
            // 计算中位数
            float median;
            if (joint_values.size() % 2 == 0) {
                median = (joint_values[joint_values.size()/2 - 1] + joint_values[joint_values.size()/2]) / 2.0f;
            } else {
                median = joint_values[joint_values.size()/2];
            }
            
            cout << joint_names[joint] << ":" << endl;
            cout << "  Mean: " << mean << " rad" << endl;
            cout << "  Std Dev: " << std_dev << " rad" << endl;
            cout << "  Median: " << median << " rad" << endl;
            cout << "  Min: " << min_val << " rad" << endl;
            cout << "  Max: " << max_val << " rad" << endl;
            cout << "  Range: " << (max_val - min_val) << " rad" << endl;
            cout << endl;
        }
        
        cout << "=== Test Completed Successfully ===" << endl;
        cout << "Statistics analysis completed for " << num_samples << " inference requests." << endl;
        
    } catch (const std::exception& e) {
        cout << "✗ Exception occurred: " << e.what() << endl;
        return -1;
    }
    
    return 0;
} 