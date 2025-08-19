/// @file main.cpp
/// @author your name (you@domain.com)
/// @brief 
/// @version 0.1
/// @date 2022-09-13 
/// @copyright Copyright (c) 2022


#include "udpsocket.hpp"
#include "udpserver.hpp"
#include "sender.h"
#include "dr_timer.h"
#include "receiver.h"
#include "motion_spline.h"
#include "utils.h"
#include "grpc_client.h"
#include "data_logger.h"
#include "keyboard_controller.h"
#include <memory>
#include <iostream>
#include <time.h>
#include <string.h>
#include <fstream>
#include <cmath>

#define M_PI 3.14159265358979323846 

using namespace std;

  bool is_message_updated_ = false; ///< Flag to check if message has been updated
  bool debug_zero_actions_ = true; ///< Flag to enable zero actions debugging mode
  int key_space_cooldown_timer = 0;

  /**
   * @brief Callback function to set message update flag
   * 
   * @param code The code indicating the type of message received
   */
  void OnMessageUpdate(uint32_t code){
    if(code == 0x0906){
      is_message_updated_ = true;
    }
  }

  
  /**
   * @brief Update robot move command based on keyboard input
   */
  void UpdateRobotMoveCommand(KeyboardController* keyboard_controller, RobotMoveCommand& robot_move_command) {
    // Reset all speeds
    robot_move_command.forward_speed = 0.0f;
    robot_move_command.left_speed = 0.0f;
    robot_move_command.turn_speed = 0.0f;
    
    // Check for continuous key presses
    if (keyboard_controller->IsKeyPressed('w')) {
      robot_move_command.forward_speed = 1.0f;
    }
    if (keyboard_controller->IsKeyPressed('s')) {
      robot_move_command.forward_speed = -0.5f;
    }
    if (keyboard_controller->IsKeyPressed('q')) {
      robot_move_command.left_speed = 0.2f;
    }
    if (keyboard_controller->IsKeyPressed('e')) {
      robot_move_command.left_speed = -0.2f;
    }

    // if (keyboard_controller->IsKeyPressed("shift"))
    
    // Handle turning (Q for left turn, E for right turn)
    if (keyboard_controller->IsKeyPressed('a')) {
      robot_move_command.turn_speed = M_PI / 4.0f;  // Left turn
    }
    if (keyboard_controller->IsKeyPressed('d')) {
      robot_move_command.turn_speed = -M_PI / 4.0f;   // Right turn
    }

    if (keyboard_controller->IsKeyPressed(' ')) {
      debug_zero_actions_ = !debug_zero_actions_;
      std::cout << "Zero actions debug mode: " << (debug_zero_actions_ ? "ENABLED" : "DISABLED") << std::endl;
    }
  }


int main(int argc, char* argv[]){

  DRTimer set_timer;
  double now_time,start_time;
  RobotCmd robot_joint_cmd;
  RobotCmd robot_joint_cmd_nn;
  memset(&robot_joint_cmd, 0, sizeof(robot_joint_cmd));
  memset(&robot_joint_cmd_nn, 0, sizeof(robot_joint_cmd_nn));

  Sender* send_cmd          = new Sender("192.168.2.1",43893);              ///< Create send thread
  // Sender* send_cmd          = new Sender("192.168.1.120",43893);              ///< Create send thread
  Receiver* robot_data_recv = new Receiver();                                 ///< Create a receive resolution
  robot_data_recv->RegisterCallBack(OnMessageUpdate);
  MotionSpline motion_spline;                                            ///< Demos for testing can be deleted by yourself
  RobotData *robot_data = &robot_data_recv->GetState();

  // Initialize gRPC client
  std::string server_address = "localhost:50151";  // 默认服务器地址，可以通过命令行参数修改
  if (argc > 1) {
    server_address = argv[1];
  }
  
  std::unique_ptr<GrpcClient> client = std::make_unique<GrpcClient>(server_address);
  
  // Connect to gRPC server
  if (!client->Connect()) {
    std::cerr << "Failed to connect to gRPC server. Exiting..." << std::endl;
    return -1;
  }
  
  // Initialize data logger
  std::unique_ptr<DataLogger> data_logger = std::make_unique<DataLogger>("robot_data");
  if (!data_logger->Initialize()) {
    std::cerr << "Failed to initialize data logger. Exiting..." << std::endl;
    return -1;
  }
  
  // Initialize keyboard controller
  std::unique_ptr<KeyboardController> keyboard_controller = std::make_unique<KeyboardController>();
  if (!keyboard_controller->Initialize()) {
    std::cerr << "Failed to initialize keyboard controller. Exiting..." << std::endl;
    return -1;
  }
  

  robot_data_recv->StartWork();
  set_timer.TimeInit(5);                                                      ///< Timer initialization, input: cycle; Unit: ms
  send_cmd->RobotStateInit();                                                 ///< Return all joints to zero and gain control

  start_time = set_timer.GetCurrentTime();                                    ///< Obtain time for algorithm usage
  motion_spline.GetInitData(robot_data->joint_data,0.000);                ///< Obtain all joint states once before each stage (action)
  
  double fl_leg_positions[3];  
  double fr_leg_positions[3];
  double hl_leg_positions[3];
  double hr_leg_positions[3];


  int time_step = 5;

  int time_tick = 0;
  while(1){
    // Process keyboard input
    keyboard_controller->ProcessKeyInput();
    
    // Update robot move command based on continuous key presses
    RobotMoveCommand robot_move_command = {0.0f, 0.0f, 0.0f};
    UpdateRobotMoveCommand(keyboard_controller.get(), robot_move_command);
    
    // Print current move command status (optional, for debugging)
    if (robot_move_command.forward_speed > 0 || robot_move_command.left_speed > 0 || 
        robot_move_command.turn_speed != 0) {
      std::cout << "Move Command - F:" << robot_move_command.forward_speed 
                << " L:" << robot_move_command.left_speed 
                << " T:" << robot_move_command.turn_speed << std::endl;
    }
    
    if (set_timer.TimerInterrupt() == true){                                  ///< Time interrupt flag
      continue;
    }
    now_time = set_timer.GetIntervalTime(start_time);                         ///< Get the current time
    time_tick++;
    // stand up first
    if(time_tick < 5000 / time_step){
      // 
      cout << "try to pre stand" << endl;
      fl_leg_positions[0] = 0 * kDegree2Radian;  
      fl_leg_positions[1] = -70 * kDegree2Radian;
      fl_leg_positions[2] = 150 * kDegree2Radian;
      fr_leg_positions[0] = 0 * kDegree2Radian;  
      fr_leg_positions[1] = -70 * kDegree2Radian;
      fr_leg_positions[2] = 150 * kDegree2Radian;
      hl_leg_positions[0] = 0 * kDegree2Radian;  
      hl_leg_positions[1] = -70 * kDegree2Radian;
      hl_leg_positions[2] = 150 * kDegree2Radian;
      hr_leg_positions[0] = 0 * kDegree2Radian;  
      hr_leg_positions[1] = -70 * kDegree2Radian;
      hr_leg_positions[2] = 150 * kDegree2Radian;
      robot_joint_cmd = CreateRobotCmdFromNumber(fl_leg_positions, fr_leg_positions, hl_leg_positions, hr_leg_positions, 45, 0.7);

      motion_spline.Motion(robot_joint_cmd,now_time,*robot_data, 45, 0.7, 1.0);    
    } 
    if(time_tick == 5000 / time_step){
      motion_spline.GetInitData(robot_data->joint_data,now_time);         ///< Obtain all joint states once before each stage (action)
    }
    // if(time_tick >= 5000){
    if(time_tick >= 5000 / time_step && time_tick < 10000 / time_step){
      // 
      cout << "try to stand" << endl;
      fl_leg_positions[0] = 0 * kDegree2Radian;  
      fl_leg_positions[1] = -57 * kDegree2Radian;
      fl_leg_positions[2] = 103 * kDegree2Radian;
      fr_leg_positions[0] = 0 * kDegree2Radian;  
      fr_leg_positions[1] = -57 * kDegree2Radian;
      fr_leg_positions[2] = 103 * kDegree2Radian;
      hl_leg_positions[0] = 0 * kDegree2Radian;  
      hl_leg_positions[1] = -57 * kDegree2Radian;
      hl_leg_positions[2] = 103 * kDegree2Radian;
      hr_leg_positions[0] = 0 * kDegree2Radian;  
      hr_leg_positions[1] = -57 * kDegree2Radian;
      hr_leg_positions[2] = 103 * kDegree2Radian;
      robot_joint_cmd = CreateRobotCmdFromNumber(fl_leg_positions, fr_leg_positions, hl_leg_positions, hr_leg_positions, 45, 0.7);

      motion_spline.Motion(robot_joint_cmd,now_time,*robot_data, 45, 0.7, 1.5); 
    }
    // compute action from neural network every 0.02s (50Hz)   4 * 0.005
    if (time_tick % (20 / time_step) == 0 && time_tick >= 10000 / time_step) {

      static vector<float> last_action;
      if (last_action.empty()) {
        last_action = vector<float>(12, 0.0f);
      }
      // Convert RobotData to Observation
      Observation observation = ConvertRobotDataToObservation(*robot_data, last_action, robot_move_command);

      // Save observation data to file
      data_logger->SaveObservation(time_tick, observation);
      
      // Apply scaling and noise to match training conditions
      Observation processed_observation = ApplyObservationScalingAndNoise(observation);

      // Send the observation and receive the action
      inference::InferenceResponse response = client->Predict(processed_observation.data, "flat_terrain", true); // stand_still, flat_terrain
      
      // Extract action data from response (original model output, not scaled)
      last_action.clear();
      for (int i = 0; i < response.action_size(); ++i) {
          last_action.push_back(response.action(i));
      }

      // Save raw action data to file
      data_logger->SaveRawAction(time_tick, last_action);

      // Convert the response to RobotAction (with action scaling applied for robot control)
      RobotAction action = ConvertResponseToAction(response);

      // Set Zero actions for debugging (only when debug mode is enabled)
      if (debug_zero_actions_) {
        for (int i = 0; i < 12; ++i) {
          action.data[i] = 0.0f;
        }
        std::cout << "Applied zero actions (debug mode active)" << std::endl;
      }

      // Convert the action back to RobotCmd
      robot_joint_cmd_nn = CreateRobotCmd(action);

      // Save processed action data to file
      data_logger->SaveAction(time_tick, action);

      fl_leg_positions[0] = robot_joint_cmd_nn.fl_leg[0].position;
      fl_leg_positions[1] = robot_joint_cmd_nn.fl_leg[1].position;
      fl_leg_positions[2] = robot_joint_cmd_nn.fl_leg[2].position;  
      fr_leg_positions[0] = robot_joint_cmd_nn.fr_leg[0].position;  
      fr_leg_positions[1] = robot_joint_cmd_nn.fr_leg[1].position;
      fr_leg_positions[2] = robot_joint_cmd_nn.fr_leg[2].position;
      hl_leg_positions[0] = robot_joint_cmd_nn.hl_leg[0].position;
      hl_leg_positions[1] = robot_joint_cmd_nn.hl_leg[1].position;
      hl_leg_positions[2] = robot_joint_cmd_nn.hl_leg[2].position;
      hr_leg_positions[0] = robot_joint_cmd_nn.hr_leg[0].position;
      hr_leg_positions[1] = robot_joint_cmd_nn.hr_leg[1].position;
      hr_leg_positions[2] = robot_joint_cmd_nn.hr_leg[2].position;
    }
    // // do spline interpolation
    if (time_tick >= 10000 / time_step) {
      robot_joint_cmd = CreateRobotCmdFromNumber(fl_leg_positions, fr_leg_positions, hl_leg_positions, hr_leg_positions, 25, 1.0);
    }
    if(is_message_updated_){ 
      // if (time_tick < 10000){
      //   send_cmd->SendCmd(robot_joint_cmd);
      // }
      send_cmd->SendCmd(robot_joint_cmd);  
    } 

    // // print robot data
    // // Open the file in append mode
    // std::ofstream file("robot_data.csv", std::ios::app);

    // // print robot data
    // // Open the file in append mode
    // if (!file.is_open()) {
    //     std::cerr << "Failed to open file: robot_data.csv" << std::endl;
    //     return 1; // Exit the program if the file cannot be opened
    // }
    // // // Print robot data to the file
    // // file << time_tick << endl;
    // // // PrintRobotCmd(robot_joint_cmd, file);
    // // PrintRobotData(robot_data, file);
    // if (time_tick == 1) {
    //   WriteCSVHeader(file);
    // }
    // SaveRobotDataToCSV(robot_data, file);
    // // Close the file
    // file.close();
 
  }
  
  // Close data logger before exiting
  if (data_logger) {
    data_logger->Close();
  }
  
  return 0;
} 
