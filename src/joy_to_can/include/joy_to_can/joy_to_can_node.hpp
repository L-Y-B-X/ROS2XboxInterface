#ifndef JOY_TO_CAN_NODE_HPP
#define JOY_TO_CAN_NODE_HPP

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joy.hpp>
#include "can_structs_mi.hpp"

#define P_MAX 12.5f
#define V_MAX 30.0f

class JoyToCANNode : public rclcpp::Node {
public:
    JoyToCANNode();
    void setMotorPosition(uint8_t motor_id, float position);
    void setMotorMode(uint8_t motor_id, uint16_t index, uint8_t mode);
    void setMotorParameter(uint8_t motor_id, uint16_t index, float value);
    
private:
    int sock_;
    bool motor_enabled = false;
    int current_mode = 0;
    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joy_subscription_;

    void setup_can_socket();
    void joy_callback(const sensor_msgs::msg::Joy::SharedPtr msg);
};

void CAN_SendFrame(int sock, can_frame_t* frame);
void Motor_Enable(int sock);
void Motor_Stop(int sock);

#endif // JOY_TO_CAN_NODE_HPP

