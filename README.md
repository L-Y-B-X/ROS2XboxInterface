# 📦 ros2-mimotor-controller
> 🚀 在ros2中调用ps和xbox手柄信息进行小米电机的控制
![image](https://github.com/L-Y-B-X/ros2-mimotor-controller/blob/main/document/cybergear.png)
![image](https://github.com/L-Y-B-X/ros2-mimotor-controller/blob/main/document/psjoy.png)
## 🌟 Features

- 🎮 Integrate PS5 wireless and Xbox controllers for motor control
- 🔧 Control Xiaomi motor via CAN communication using CANable USB-CAN adapter
- 💻 Ideal for learning ROS2 communication and CAN communication
- 🌐 Open-source and community-friendly
- ✨ 使用 ROS2 接收 PS 和 Xbox 手柄输入
- 🔥 通过 CANable 与小米电机通信
- 💎 支持多种控制模式（速度模式、位置模式）

---

## 🛠️ Installation

### Prerequisites

Make sure you have the following installed:

- **Ubuntu 20.04**
- **ROS2 Foxy**: Follow the [official installation guide](https://docs.ros.org/en/foxy/Installation.html)
- **CANable firmware**: [Setup instructions](https://canable.io/getting-started.html)

1. **Clone the repository:**

    ```sh
    git clone https://github.com/yourusername/ROS2XboxInterface.git
    cd ROS2XboxInterface
    ```

2. **Install dependencies:**

    ```sh
    sudo apt update
    sudo apt install python3-colcon-common-extensions
    rosdep update
    rosdep install --from-paths src --ignore-src -r -y
    ```

3. **Build the package:**

    ```sh
    colcon build
    ```

4. **Source the setup script:**

    ```sh
    source install/setup.bash
    ```

---
## 🚀 Usage
### 特别注意，我的电机的canid设置为了127，允许的id最大值在代码中也就是0x7F，数据为写的0xFD，位置模式的速度限制写在L1按键上的，可以在代码中找到对应部分，没有写关于速度模式最大电流限制的can信号，也没有写初次获取canid的步骤。只适用于短暂验证电机使用
1. **Run the joy node:**

    ```sh
    ros2 run joy joy_node 
    ```

2. **Set the can sender**

    ```sh
    sudo ip link set can0 type can bitrate 1000000
    sudo ip link set can0 up
    ```
3. **Source the setup script:**

    ```sh
    source install/setup.bash
    ```
    
4. **Run the node we build:**

    ```sh
    ros2 run joy_to_can joy_to_can_node 
    ```

## 🧩 Structure
还没来得及写，过段时间补上

## 🤝 Contributing
Contributions are welcome!

## 📄 License 
This project is licensed under the MIT License - see the LICENSE file for details.

## 📧 Contact 
If you have any questions, feel free to reach out at **Yibo.Liu22@student.xjtlu.edu.cn**

