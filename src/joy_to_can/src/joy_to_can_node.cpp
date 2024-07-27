#include "joy_to_can/joy_to_can_node.hpp"
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <cstring>

JoyToCANNode::JoyToCANNode() : Node("joy_to_can_node") {
    setup_can_socket();
    joy_subscription_ = this->create_subscription<sensor_msgs::msg::Joy>(
        "joy", 10, std::bind(&JoyToCANNode::joy_callback, this, std::placeholders::_1));
}

void CAN_SendFrame(int sock, can_frame_t* frame) {
    uint32_t combined_value = 0;
    uint32_t id_value;
    uint32_t data_value;
    uint32_t mode_value;
    struct can_frame TxMessage;
    TxMessage.can_id = 0; // 初始化

    // 获取 can_frame_t 结构体中的值
    id_value = frame->id;
    data_value = frame->data;
    mode_value = frame->mode;

    // 将 id, data, mode 组合成一个 29 位的值
    combined_value |= (mode_value << 24);
    combined_value |= (data_value << 8);
    combined_value |= id_value;

    // 设置 TxMessage 的字段
    TxMessage.can_id = combined_value | CAN_EFF_FLAG;
    TxMessage.can_dlc = 8;
    memcpy(TxMessage.data, frame->tx_data, sizeof(frame->tx_data));

    // 发送消息
    if (write(sock, &TxMessage, sizeof(TxMessage)) != sizeof(TxMessage)) {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Failed to write CAN frame");
    }
}

void Motor_Enable(int sock) {
    can_frame_t Motor_Enable_Frame;
    Motor_Enable_Frame.mode = 3;
    Motor_Enable_Frame.data = 0xFD;
    Motor_Enable_Frame.id = 0x7F;
    memset(Motor_Enable_Frame.tx_data, 0, sizeof(Motor_Enable_Frame.tx_data));

    CAN_SendFrame(sock, &Motor_Enable_Frame);
}

void Motor_Stop(int sock) {
    can_frame_t Motor_Stop_Frame;
    Motor_Stop_Frame.mode = 4;
    Motor_Stop_Frame.data = 0xFD;
    Motor_Stop_Frame.id = 0x7F;
    memset(Motor_Stop_Frame.tx_data, 0, sizeof(Motor_Stop_Frame.tx_data));

    CAN_SendFrame(sock, &Motor_Stop_Frame);
}

void JoyToCANNode::setup_can_socket() {
    if ((sock_ = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        RCLCPP_ERROR(this->get_logger(), "Error while opening socket");
        throw std::runtime_error("Error while opening socket");
    }

    struct ifreq ifr;
    strcpy(ifr.ifr_name, "can0");
    ioctl(sock_, SIOCGIFINDEX, &ifr);

    struct sockaddr_can addr;
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(sock_, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        RCLCPP_ERROR(this->get_logger(), "Error in socket bind");
        throw std::runtime_error("Error in socket bind");
    }
}

void JoyToCANNode::setMotorPosition(uint8_t motor_id, float position) {
  (void)motor_id;
    can_frame_t frame;
    frame.id = 0x7F; // 使用 motor_id 参数
    frame.mode = 18; // Some mode value
    frame.data = 0xFD; // Some data value
    memset(frame.tx_data, 0, sizeof(frame.tx_data));

    // Convert float position to CAN data (assuming little-endian system)
    uint32_t pos = *reinterpret_cast<uint32_t*>(&position);
    frame.tx_data[0] = 0x16; // Index for position
    frame.tx_data[1] = 0x70; // Sub-index
    frame.tx_data[4] = pos & 0xFF;
    frame.tx_data[5] = (pos >> 8) & 0xFF;
    frame.tx_data[6] = (pos >> 16) & 0xFF;
    frame.tx_data[7] = (pos >> 24) & 0xFF;

    CAN_SendFrame(sock_, &frame);
}

void JoyToCANNode::setMotorParameter(uint8_t motor_id, uint16_t index, float value) {
  (void)motor_id;
    can_frame_t frame;
    frame.id = 0x7F;  // Motor ID
    frame.mode = 18;  // Mode value for setting parameter
    frame.data = 0xFD; // Additional identifier if needed

    // Clear data array
    memset(frame.tx_data, 0, sizeof(frame.tx_data));

    // Convert float value to CAN data (assuming little-endian system)
    uint32_t pos = *reinterpret_cast<uint32_t*>(&value);
    frame.tx_data[0] = index & 0xFF;        // Low byte of index
    frame.tx_data[1] = (index >> 8) & 0xFF; // High byte of index
    frame.tx_data[4] = pos & 0xFF;
    frame.tx_data[5] = (pos >> 8) & 0xFF;
    frame.tx_data[6] = (pos >> 16) & 0xFF;
    frame.tx_data[7] = (pos >> 24) & 0xFF;

    CAN_SendFrame(sock_, &frame);
}

void JoyToCANNode::setMotorMode(uint8_t motor_id, uint16_t index, uint8_t mode) {
  (void)motor_id;
    can_frame_t frame;
    frame.id = 0x7F;
    frame.mode = 18; // Use the parameter setting mode
    frame.data = 0xFD;

    memset(frame.tx_data, 0, sizeof(frame.tx_data));
    frame.tx_data[0] = index & 0xFF;
    frame.tx_data[1] = (index >> 8) & 0xFF;
    frame.tx_data[4] = mode; // Only needs one byte for the mode

    CAN_SendFrame(sock_, &frame);
}

void JoyToCANNode::joy_callback(const sensor_msgs::msg::Joy::SharedPtr msg) {
    RCLCPP_INFO(this->get_logger(), "Button0: %d, Button1: %d, Button2: %d, Button3: %d, Axis1: %f", 
                msg->buttons[0], msg->buttons[1], msg->buttons[2], msg->buttons[3], msg->axes[1]);
    RCLCPP_INFO(this->get_logger(), "Current mode: %d, Motor enabled: %d", current_mode, motor_enabled);

    if (!msg->buttons.empty() && msg->buttons[0] == 1) {
        Motor_Enable(sock_);
        motor_enabled = true; // 启用电机
    }

    if (!msg->buttons.empty() && msg->buttons[1] == 1) {
        Motor_Stop(sock_);
        motor_enabled = false; // 停止电机
    }

    if (!msg->buttons.empty() && msg->buttons[2] == 1) {
        setMotorMode(1, 0x7005, 1); // 设置为position模式
        current_mode = 1;
    }

    if (!msg->buttons.empty() && msg->buttons[3] == 1) {
        setMotorMode(1, 0x7005, 2); // 设置为speed模式
        current_mode = 2;
    }
        if (!msg->buttons.empty() && msg->buttons[4] == 1) {
        setMotorParameter(1, 0X7017, 2.0f); //set position mode speed
    }
    if (motor_enabled && msg->axes.size() > 1) {
        if (current_mode == 2) { // 速度模式
            float speed = msg->axes[1] * V_MAX; // 调整速度
            setMotorParameter(1, 0X700A, speed);
        }

        if (current_mode == 1) { // 位置模式
            float position = msg->axes[1] * P_MAX; // 调整位置
            setMotorParameter(1, 0x7016, position);
        }
    }
}





