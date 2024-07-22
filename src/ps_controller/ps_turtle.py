from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='ps_controller',
            executable='joy_control',
            name='joy_control'
        ),
        Node(
            package='joy',
            executable='joy_node',
            name='joy_node'
        ),
        Node(
            package='turtlesim',
            executable='turtlesim_node',
            name='turtlesim_node'
        )
    ])

