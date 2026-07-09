# Step 1: Use the official ROS2 Humble desktop full image
FROM osrf/ros:humble-desktop-full

# Step 2: Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Step 3: Install system dependencies & ROS2 packages needed for glitch-amr
# (ros-humble-micro-ros-agent has been removed as it is not available in APT)
RUN apt-get update && apt-get install -y \
    python3-colcon-common-extensions \
    git \
    tmux \
    vim \
    ros-humble-cartographer \
    ros-humble-cartographer-ros \
    ros-humble-navigation2 \
    ros-humble-rplidar-ros \
    ros-humble-gazebo-ros-pkgs \
    ros-humble-ros-ign-gazebo \
    ros-humble-rclcpp \
    ros-humble-rclcpp-components \
    ros-humble-geometry-msgs \
    ros-humble-nav-msgs \
    ros-humble-sensor-msgs \
    ros-humble-tf2 \
    ros-humble-tf2-geometry-msgs \
    ros-humble-nav2-bringup \
    ros-humble-nav2-common \
    ros-humble-slam-toolbox \
    ros-humble-joint-state-publisher \
    ros-humble-robot-state-publisher \
    ros-humble-urdf \
    ros-humble-rviz2 \
    ros-humble-xacro \
    ros-humble-rqt-robot-monitor \
    ros-humble-controller-manager \
    ros-humble-tf2-ros \
    ros-humble-robot-localization \
    && rm -rf /var/lib/apt/lists/*

# Step 4: Set up the workspace, clone the micro-ROS Agent, and install its dependencies
WORKDIR /ros2_ws/src
RUN git clone -b humble https://github.com/micro-ROS/micro-ROS-Agent.git

# Step 5: Clone the rplidar_ros package
RUN git clone -b ros2 https://github.com/Slamtec/rplidar_ros.git

WORKDIR /ros2_ws
RUN apt-get update && rosdep update && \
    rosdep install --from-paths src --ignore-src -y && \
    rm -rf /var/lib/apt/lists/*

# Step 5: Set up bash environment and source ROS2 on startup
RUN echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc

# Step 6: Copy and set up the entrypoint script
WORKDIR /ros2_ws
COPY ./entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
CMD ["bash"]