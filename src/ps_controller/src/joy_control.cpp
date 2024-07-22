#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <sensor_msgs/msg/joy.hpp>
#include <std_srvs/srv/empty.hpp>  // 清除

class TeleopTurtle : public rclcpp::Node
{
public:
    TeleopTurtle();

private:
    void joy_callback(const sensor_msgs::msg::Joy::SharedPtr joy);
    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr sub;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr pub;
    rclcpp::Client<std_srvs::srv::Empty>::SharedPtr clear_client;  // 清除服务的客户端
    int axis_linear, axis_angular, clear_button;  // 添加用于清除的按键索引
};

TeleopTurtle::TeleopTurtle() : Node("teleop_turtle")
{
    this->declare_parameter<int>("axis_linear", 1);
    this->declare_parameter<int>("axis_angular", 4);
    this->declare_parameter<int>("clear_button", 0);  // 声明清除按钮的参数
    this->get_parameter("axis_linear", axis_linear);
    this->get_parameter("axis_angular", axis_angular);
    this->get_parameter("clear_button", clear_button);  // 获取清除按钮的索引

    pub = this->create_publisher<geometry_msgs::msg::Twist>("/turtle1/cmd_vel", 10);
    sub = this->create_subscription<sensor_msgs::msg::Joy>("joy", 10, std::bind(&TeleopTurtle::joy_callback, this, std::placeholders::_1));
    clear_client = this->create_client<std_srvs::srv::Empty>("/clear");  // 初始化清除服务的客户端
}

void TeleopTurtle::joy_callback(const sensor_msgs::msg::Joy::SharedPtr joy)
{
    auto vel = geometry_msgs::msg::Twist();
    vel.linear.x = joy->axes[axis_linear];
    vel.angular.z = joy->axes[axis_angular];
    pub->publish(vel);

    // 检查是否按下了清除按钮
    if (joy->buttons[clear_button] == 1) {
        auto request = std::make_shared<std_srvs::srv::Empty::Request>();
        clear_client->async_send_request(request,
            [this](rclcpp::Client<std_srvs::srv::Empty>::SharedFuture future) {
                if(future.get()) {
                    RCLCPP_INFO(this->get_logger(), "Turtlesim轨迹已清除");
                }
            });
    }
}

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<TeleopTurtle>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}