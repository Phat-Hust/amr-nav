#include <memory>
#include <cstdlib>
#include <string>

#include "rclcpp/rclcpp.hpp"

#include "pi_amr_controller/app_controller.hpp"

int main(int argc, char ** argv)
{
  // Khởi động ROS 2
  rclcpp::init(argc, argv);

  const char * config_directory = std::getenv("CONFIG_FOLDER");

  // Tạo đường dẫn tới config.yaml
  const std::string config_file =
    std::string(config_directory ? config_directory : "/var/amr/config") +
    "/pi_amr_controller.yaml";

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
