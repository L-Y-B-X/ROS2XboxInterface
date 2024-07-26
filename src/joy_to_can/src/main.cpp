#include <rclcpp/rclcpp.hpp>
#include "joy_to_can/joy_to_can_node.hpp"

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<JoyToCANNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}

