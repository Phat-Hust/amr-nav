#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "ament_index_cpp/get_package_share_directory.hpp"

#include "pi_amr_controller/app_controller.hpp"

int main(int argc, char ** argv)
{
  // Khởi động ROS 2
  rclcpp::init(argc, argv);

  // Tìm thư mục share của package pi_amr_controller
  const std::string package_share =
    ament_index_cpp::get_package_share_directory(
      "pi_amr_controller"
    );

  // Tạo đường dẫn tới config.yaml
  const std::string config_file =
    package_share + "/config/config.yaml";

  // Tạo NodeOptions
  rclcpp::NodeOptions options;

  // Tự động truyền config.yaml cho ROS 2
  options.arguments({
    "--ros-args",
    "--params-file",
    config_file
  });

  // Tạo node AppController với parameter từ YAML
  auto node =
    std::make_shared<AppController>(options);

  // Giữ node chạy
  rclcpp::spin(node);

  // Tắt ROS 2
  rclcpp::shutdown();

  return 0;
}