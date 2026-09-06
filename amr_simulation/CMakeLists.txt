cmake_minimum_required(VERSION 3.8)
project(amr_simulation)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(MSVC)
  add_compile_definitions(_USE_MATH_DEFINES)
endif()

# Core ROS 2 packages
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(xacro REQUIRED)

# Modern Gazebo (Ignition Fortress for ROS 2 Humble)
find_package(ros_gz_sim REQUIRED)
find_package(ros_gz_bridge REQUIRED)
# If compiling C++ plugins against Ignition directly:
# find_package(ignition-gazebo6 REQUIRED)

include_directories(
  include
)

install(CODE "
  execute_process(
    COMMAND bash -c \"xacro mecanum_drive.urdf.xacro > amr.urdf && ign sdf -p amr.urdf > amr.sdf\"
    WORKING_DIRECTORY \${CMAKE_CURRENT_SOURCE_DIR}/urdf/robots
  )"
)

# Install assets (including config directory used by your controllers)
install(
  DIRECTORY launch worlds rviz urdf meshes models config
  DESTINATION share/${PROJECT_NAME}/
)



ament_export_include_directories(include)
ament_export_dependencies(
  rclcpp
  geometry_msgs
  nav_msgs
  sensor_msgs
  tf2
  tf2_ros
  ros_gz_sim
  ros_gz_bridge
)

ament_package()