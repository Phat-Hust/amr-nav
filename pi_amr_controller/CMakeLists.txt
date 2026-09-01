cmake_minimum_required(VERSION 3.8)        # Yêu cầu phiên bản CMake tối thiểu là 3.8
project(pi_amr_controller)                 # Tên ROS 2 package/project

# Nếu compiler là GCC hoặc Clang thì bật các cảnh báo khi compile
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)   # Bật warning để dễ phát hiện lỗi code
endif()


# =========================
# ROS 2 DEPENDENCIES
# =========================

find_package(ament_cmake REQUIRED)          # Hệ thống build chuẩn của ROS 2
find_package(rclcpp REQUIRED)               # Dùng ROS 2 C++: Node, publisher, subscriber, timer...
find_package(geometry_msgs REQUIRED)        # Dùng geometry_msgs::msg::Twist cho /cmd_vel
find_package(nav_msgs REQUIRED)             # Dùng nav_msgs::msg::Odometry cho /odom
find_package(ament_index_cpp REQUIRED)
find_package(tf2 REQUIRED)                  # Dùng tf2::Quaternion để tạo orientation
find_package(tf2_ros REQUIRED)              # Dùng TransformBroadcaster để publish TF


# =========================
# CREATE EXECUTABLE
# =========================

add_executable(app_controller_node               # Tạo executable tên "app_controller"
  src/app_controller.cpp                    # File chứa logic AppController
)


# =========================
# HEADER DIRECTORY
# =========================

target_include_directories(app_controller_node PUBLIC

  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  # Khi đang build, compiler tìm header trong pi_amr_controller/include/

  $<INSTALL_INTERFACE:include>
  # Sau khi package được install, header sẽ được tìm trong thư mục include
)


# =========================
# LINK ROS 2 DEPENDENCIES
# =========================

ament_target_dependencies(app_controller_node
  rclcpp                                     # Cho rclcpp::Node, timer, publisher...
  geometry_msgs                              # Cho geometry_msgs::msg::Twist
  nav_msgs                                   # Cho nav_msgs::msg::Odometry
  tf2                                        # Cho tf2::Quaternion
  tf2_ros                                    # Cho tf2_ros::TransformBroadcaster
  ament_index_cpp
)


# =========================
# INSTALL EXECUTABLE
# =========================

install(
  TARGETS app_controller_node                     # Install executable app_controller
  DESTINATION lib/${PROJECT_NAME}
  # ROS 2 sẽ đặt executable tại: install/pi_amr_controller/lib/pi_amr_controller/app_controller
  #
  # Nhờ đó mới chạy được: ros2 run pi_amr_controller app_controller_node
)

# INSTALL CONFIG
# =========================

install(
  DIRECTORY config
  DESTINATION share/${PROJECT_NAME}
)

# =========================
# TEST / LINT
# =========================

if(BUILD_TESTING)

  find_package(ament_lint_auto REQUIRED)      # ROS 2 lint/check code

  set(ament_cmake_copyright_FOUND TRUE)
  # Tạm bỏ qua kiểm tra copyright

  set(ament_cmake_cpplint_FOUND TRUE)
  # Tạm bỏ qua cpplint

  ament_lint_auto_find_test_dependencies()
  # Tự tìm các dependency phục vụ kiểm tra code

endif()

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)
# =========================
# END ROS 2 PACKAGE
# =========================

ament_package()
# Dòng bắt buộc cho package ament_cmake
# Thường đặt ở cuối CMakeLists.txt