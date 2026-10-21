cmake_minimum_required(VERSION 3.8)
project(scan_safety)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(diagnostic_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)

add_executable(scan_safety_node src/scan_safety_node.cpp)
target_compile_features(scan_safety_node PUBLIC cxx_std_17)
target_include_directories(scan_safety_node PRIVATE include)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(scan_safety_node PRIVATE -Wall -Wextra -Wpedantic)
endif()
ament_target_dependencies(scan_safety_node
  rclcpp sensor_msgs diagnostic_msgs geometry_msgs tf2 tf2_ros tf2_geometry_msgs)

install(TARGETS scan_safety_node DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY launch config DESTINATION share/${PROJECT_NAME})
install(FILES config/config.yaml DESTINATION /var/amr/config RENAME scan_safety.yaml)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  find_package(ament_cmake_pytest REQUIRED)
  ament_add_gtest(test_rectangle test/test_rectangle.cpp)
  target_compile_features(test_rectangle PUBLIC cxx_std_17)
  target_include_directories(test_rectangle PRIVATE include)
  ament_add_pytest_test(test_scan_safety test/test_scan_safety.py
    TIMEOUT 60
    ENV
      "SCAN_SAFETY_EXECUTABLE=$<TARGET_FILE:scan_safety_node>"
      "SCAN_SAFETY_CONFIG=${CMAKE_CURRENT_SOURCE_DIR}/config/config.yaml"
      "PYTHONDONTWRITEBYTECODE=1"
      "ROS_DOMAIN_ID=163"
      "ROS_LOG_DIR=${CMAKE_CURRENT_BINARY_DIR}/ros_logs")
endif()

ament_package()
