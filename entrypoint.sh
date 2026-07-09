#!/bin/bash
set -e

# Source the global ROS2 installation
source "/opt/ros/humble/setup.bash"

# Source your local workspace if it has been built
if [ -f "/ros2_ws/install/setup.bash" ]; then
  source "/ros2_ws/install/setup.bash"
fi

exec "$@"