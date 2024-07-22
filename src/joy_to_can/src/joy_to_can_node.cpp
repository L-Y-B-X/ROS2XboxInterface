#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joy.hpp>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <cstring>
#include <chrono>

using namespace std::chrono_literals;

class JoyToCANNode : public rclcpp::Node {
public:
    // Constants for motor control ranges
    static constexpr float P_MIN = -12.5f;
    static constexpr float P_MAX = 12.5f;
    static constexpr float V_MIN = -30.0f;
    static constexpr float V_MAX = 30.0f;
    static constexpr float KP_MIN = 0.0f;
    static constexpr float KP_MAX = 500.0f;
    static constexpr float KD_MIN = 0.0f;
    static constexpr float KD_MAX = 5.0f;
    static constexpr float T_MIN = -12.0f;
    static constexpr float T_MAX = 12.0f;

    JoyToCANNode() : Node("joy_to_can_node") {
        joy_subscription_ = this->create_subscription<sensor_msgs::msg::Joy>(
            "joy", 10, std::bind(&JoyToCANNode::joy_callback, this, std::placeholders::_1));

        setup_can_socket();
    }

private:
    int sock_;
    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joy_subscription_;

    void setup_can_socket() {
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

    void joy_callback(const sensor_msgs::msg::Joy::SharedPtr msg) {
        if (!msg->buttons.empty() && msg->buttons[0] == 1) {
            enableMotor(1);
        }

        if (msg->axes.size() > 1) {
            float position = msg->axes[1] * P_MAX; // Scale joystick input to position range
            setMotorPosition(1, position); // Set motor position
        }
    }

    void enableMotor(uint8_t motor_id) {
        struct can_frame frame;
        frame.can_id = 0x0300FD7F| CAN_EFF_FLAG; // Custom CAN ID
        frame.can_dlc = 8;
        memset(frame.data, 0, sizeof(frame.data));
        frame.data[0] = motor_id;

        sendCanFrame(frame);
    }

    void setMotorPosition(uint8_t motor_id, float position) {
        struct can_frame frame;
        frame.can_id = 0x1200FD7F| CAN_EFF_FLAG; // Base ID for motor position
        frame.can_dlc = 8;
        memset(frame.data, 0, sizeof(frame.data));

        // Convert float position to CAN data (assuming little-endian system)
        uint32_t pos = *reinterpret_cast<uint32_t*>(&position);
        frame.data[0] = 0x16; // Index for position
        frame.data[1] = 0x70; // Sub-index
        frame.data[4] = pos & 0xFF;
        frame.data[5] = (pos >> 8) & 0xFF;
        frame.data[6] = (pos >> 16) & 0xFF;
        frame.data[7] = (pos >> 24) & 0xFF;

        sendCanFrame(frame);
    }

    void sendCanFrame(const struct can_frame &frame) {
        if (write(sock_, &frame, sizeof(frame)) != sizeof(frame)) {
            RCLCPP_ERROR(this->get_logger(), "Failed to write CAN frame");
        }
    }
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<JoyToCANNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}

