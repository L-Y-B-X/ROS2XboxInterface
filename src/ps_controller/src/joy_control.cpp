#include <rclcpp/rclcpp.hpp>   // 包含ROS 2核心客户端库
#include <geometry_msgs/msg/twist.hpp>   // 包含用于发布速度消息的消息类型
#include <sensor_msgs/msg/joy.hpp> // 包含用于接收手柄输入的消息类型
#include <std_srvs/srv/empty.hpp>  // 包含空服务类型，用于清除turtlesim的轨迹

class TeleopTurtle : public rclcpp::Node
{
public:
    TeleopTurtle();// 构造函数声明

private:
    void joy_callback(const sensor_msgs::msg::Joy::SharedPtr joy);// 手柄消息的回调函数声明
    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr sub; // 手柄消息的订阅者
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr pub;// 速度消息的发布者
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
    
    // 创建速度消息的发布者，主题为 /turtle1/cmd_vel，队列大小为10
    pub = this->create_publisher<geometry_msgs::msg::Twist>("/turtle1/cmd_vel", 10);
    // 创建手柄消息的订阅者，主题为 joy，队列大小为10，绑定回调函数 joy_callback
    sub = this->create_subscription<sensor_msgs::msg::Joy>("joy", 10, std::bind(&TeleopTurtle::joy_callback, this, std::placeholders::_1));
    // 初始化清除服务的客户端，服务名称为 /clear
    clear_client = this->create_client<std_srvs::srv::Empty>("/clear");  
}

void TeleopTurtle::joy_callback(const sensor_msgs::msg::Joy::SharedPtr joy)
{
     // 创建一个Twist消息并设置线速度和角速度
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
    rclcpp::init(argc, argv);// 初始化ROS 2客户端库
    auto node = std::make_shared<TeleopTurtle>();// 创建TeleopTurtle节点
    rclcpp::spin(node);// 进入节点的事件循环
    rclcpp::shutdown();// 关闭ROS 2客户端库
    return 0;
}