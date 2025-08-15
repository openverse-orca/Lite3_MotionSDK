#include <iostream>
#include <fstream> 
#include "robot_types.h"
#include "grpc_client.h"
#include "motion_spline.h"

const double kRadian2Degree = 180 / 3.1415926;

/// @brief Function to print all robot data.
/// @param robot_data Pointer to the RobotData structure containing the data to print.
void PrintRobotData(const RobotData* robot_data, std::ofstream& file) {
    if (!robot_data) {
        std::cerr << "Invalid robot data pointer!" << std::endl;
        return;
    }

    if (!file.is_open()) {
        std::cerr << "File stream is not open!" << std::endl;
        return;
    }

    // Print tick (timestamp)
    file << "Tick: " << robot_data->tick << std::endl;

    // Print IMU data
    file << "IMU Data:" << std::endl;
    file << "  Roll Angle: " << robot_data->imu.angle_roll << " degrees" << std::endl;
    file << "  Pitch Angle: " << robot_data->imu.angle_pitch << " degrees" << std::endl;
    file << "  Yaw Angle: " << robot_data->imu.angle_yaw << " degrees" << std::endl;
    file << "  Roll Angular Velocity: " << robot_data->imu.angular_velocity_roll << " deg/s" << std::endl;
    file << "  Pitch Angular Velocity: " << robot_data->imu.angular_velocity_pitch << " deg/s" << std::endl;
    file << "  Yaw Angular Velocity: " << robot_data->imu.angular_velocity_yaw << " deg/s" << std::endl;
    file << "  Acceleration X: " << robot_data->imu.acc_x << " m/s^2" << std::endl;
    file << "  Acceleration Y: " << robot_data->imu.acc_y << " m/s^2" << std::endl;
    file << "  Acceleration Z: " << robot_data->imu.acc_z << " m/s^2" << std::endl;

    // Print joint data
    file << "Joint Data:" << std::endl;
    const char* leg_names[] = {"Front Left Leg", "Front Right Leg", "Hind Left Leg", "Hind Right Leg"};
    const JointData* legs[] = {robot_data->joint_data.fl_leg, robot_data->joint_data.fr_leg,
                               robot_data->joint_data.hl_leg, robot_data->joint_data.hr_leg};

    for (int leg = 0; leg < 4; ++leg) {
        file << "  " << leg_names[leg] << ":" << std::endl;
        for (int joint = 0; joint < 3; ++joint) {
            const JointData& joint_data = legs[leg][joint];
            file << "    Joint " << joint + 1 << ":" << std::endl;
            file << "      Position: " << joint_data.position * kRadian2Degree << " degrees" << std::endl;
            file << "      Velocity: " << joint_data.velocity * kRadian2Degree << " degrees/s" << std::endl;
            file << "      Torque: " << joint_data.torque << " Nm" << std::endl;
            file << "      Temperature: " << joint_data.temperature << " °C" << std::endl;
        }
    }

    // Print contact force data
    file << "Contact Force Data:" << std::endl;
    for (int leg = 0; leg < 4; ++leg) {
        file << "  " << leg_names[leg] << " Forces (x, y, z): ";
        file << robot_data->contact_force.leg_force[leg * 3 + 0] << ", "
                  << robot_data->contact_force.leg_force[leg * 3 + 1] << ", "
                  << robot_data->contact_force.leg_force[leg * 3 + 2] << std::endl;
    }
}

/// @brief Creates a RobotCmd structure from a set of leg positions.
/// @param fl_leg_positions The positions of the front left leg.
/// @param fr_leg_positions The positions of the front right leg.
/// @param hl_leg_positions The positions of the hind left leg.
/// @param hr_leg_positions The positions of the hind right leg.
/// @return A RobotCmd structure populated with the given leg positions.
RobotCmd CreateRobotCmdFromNumber(double fl_leg_positions[3], double fr_leg_positions[3], double hl_leg_positions[3], double hr_leg_positions[3], double kp, double kd) {
    RobotCmd robot_cmd;
    memset(&robot_cmd, 0, sizeof(robot_cmd));

    for (int i = 0; i < 3; i++) {
        robot_cmd.fl_leg[i].position = fl_leg_positions[i];
        robot_cmd.fl_leg[i].velocity = 0;
        robot_cmd.fl_leg[i].torque = 0;
        robot_cmd.fl_leg[i].kp = kp;
        robot_cmd.fl_leg[i].kd = kd;
    }

    for (int i = 0; i < 3; i++) {
        robot_cmd.fr_leg[i].position = fr_leg_positions[i];
        robot_cmd.fr_leg[i].velocity = 0;
        robot_cmd.fr_leg[i].torque = 0;
        robot_cmd.fr_leg[i].kp = kp;
        robot_cmd.fr_leg[i].kd = kd;
    }

    for (int i = 0; i < 3; i++) {
        robot_cmd.hl_leg[i].position = hl_leg_positions[i];
        robot_cmd.hl_leg[i].velocity = 0;
        robot_cmd.hl_leg[i].torque = 0;
        robot_cmd.hl_leg[i].kp = kp;
        robot_cmd.hl_leg[i].kd = kd;
    }

    for (int i = 0; i < 3; i++) {
        robot_cmd.hr_leg[i].position = hr_leg_positions[i];
        robot_cmd.hr_leg[i].velocity = 0;
        robot_cmd.hr_leg[i].torque = 0;
        robot_cmd.hr_leg[i].kp = kp;
        robot_cmd.hr_leg[i].kd = kd;
    }

    return robot_cmd;
}

/// @brief Prints the RobotCmd structure into a file.
/// @param robot_cmd The RobotCmd structure to print.
/// @param file The output file stream to write the data.
void PrintRobotCmd(const RobotCmd& robot_cmd, std::ofstream& file) {
    if (!file.is_open()) {
        std::cerr << "File stream is not open!" << std::endl;
        return;
    }

    const char* leg_names[] = {"Front Left Leg", "Front Right Leg", "Hind Left Leg", "Hind Right Leg"};
    const JointCmd* legs[] = {robot_cmd.fl_leg, robot_cmd.fr_leg, robot_cmd.hl_leg, robot_cmd.hr_leg};

    file << "Robot Command:" << std::endl;

    for (int leg = 0; leg < 4; ++leg) {
        file << "  " << leg_names[leg] << ":" << std::endl;
        for (int joint = 0; joint < 3; ++joint) {
            const JointCmd& joint_data = legs[leg][joint];
            file << "    Joint " << joint + 1 << ":" << std::endl;
            file << "      Position: " << joint_data.position * kRadian2Degree << " degrees" << std::endl;
            file << "      Velocity: " << joint_data.velocity * kRadian2Degree << " degrees/s" << std::endl;
            file << "      Torque: " << joint_data.torque << " Nm" << std::endl;
            file << "      Kp: " << joint_data.kp << std::endl;
            file << "      Kd: " << joint_data.kd << std::endl;
        }
    }
}

/// @brief Saves robot data to a CSV file.
/// @param robot_data Pointer to the RobotData structure containing the data to save.
/// @param file The output file stream to write the data.
void SaveRobotDataToCSV(const RobotData* robot_data, std::ofstream& file) {
    if (!robot_data || !file.is_open()) {
        std::cerr << "Invalid robot data pointer or file not open!" << std::endl;
        return;
    }

    // Write tick
    file << robot_data->tick << ",";

    // Write IMU data
    file << robot_data->imu.angle_roll << ","
         << robot_data->imu.angle_pitch << ","
         << robot_data->imu.angle_yaw << ","
         << robot_data->imu.angular_velocity_roll << ","
         << robot_data->imu.angular_velocity_pitch << ","
         << robot_data->imu.angular_velocity_yaw << ","
         << robot_data->imu.acc_x << ","
         << robot_data->imu.acc_y << ","
         << robot_data->imu.acc_z << ",";

    // Write joint data for each leg
    const JointData* legs[] = {robot_data->joint_data.fl_leg, 
                              robot_data->joint_data.fr_leg,
                              robot_data->joint_data.hl_leg, 
                              robot_data->joint_data.hr_leg};

    for (int leg = 0; leg < 4; ++leg) {
        for (int joint = 0; joint < 3; ++joint) {
            const JointData& joint_data = legs[leg][joint];
            file << joint_data.position * kRadian2Degree << ","
                 << joint_data.velocity * kRadian2Degree << ","
                 << joint_data.torque << ","
                 << joint_data.temperature << ",";
        }
    }

    // Write contact force data
    for (int leg = 0; leg < 4; ++leg) {
        file << robot_data->contact_force.leg_force[leg * 3 + 0] << ","
             << robot_data->contact_force.leg_force[leg * 3 + 1] << ","
             << robot_data->contact_force.leg_force[leg * 3 + 2] << ",";
    }

    file << std::endl;
}

/// @brief Writes CSV header to the file.
/// @param file The output file stream to write the header.
void WriteCSVHeader(std::ofstream& file) {
    if (!file.is_open()) {
        std::cerr << "File stream is not open!" << std::endl;
        return;
    }

    // Write header for tick
    file << "tick,";

    // Write header for IMU data
    file << "imu_roll,imu_pitch,imu_yaw,"
         << "imu_roll_vel,imu_pitch_vel,imu_yaw_vel,"
         << "imu_acc_x,imu_acc_y,imu_acc_z,";

    // Write header for joint data
    const char* leg_names[] = {"fl", "fr", "hl", "hr"};
    for (const char* leg : leg_names) {
        for (int joint = 1; joint <= 3; ++joint) {
            file << leg << "_j" << joint << "_pos,"
                 << leg << "_j" << joint << "_vel,"
                 << leg << "_j" << joint << "_torque,"
                 << leg << "_j" << joint << "_temp,";
        }
    }

    // Write header for contact forces
    for (const char* leg : leg_names) {
        file << leg << "_force_x," << leg << "_force_y," << leg << "_force_z,";
    }

    file << std::endl;
}