cmake_minimum_required(VERSION 3.8)
project(glitch_nodes)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(irobot_create_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)


set(dependencies
  rclcpp
  rclcpp_components
  geometry_msgs
  nav_msgs
  sensor_msgs
  irobot_create_msgs
  tf2
  tf2_geometry_msgs
)

# Motion control node
add_library(motion_control_lib SHARED)
target_sources(
  motion_control_lib
  PRIVATE
    src/motion_control_node.cpp
)
target_include_directories(motion_control_lib PUBLIC include)
ament_target_dependencies(motion_control_lib ${dependencies})

rclcpp_components_register_node(motion_control_lib
  PLUGIN "glitch_nodes::MotionControlNode"
  EXECUTABLE motion_control)

set(libraries_names motion_control_lib)

install(TARGETS ${libraries_names}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

# Install headers
# install(DIRECTORY include/
#   DESTINATION include
# )


#### Install

ament_export_include_directories(include)
ament_export_libraries(${libraries_names})
ament_export_dependencies(${dependencies})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
