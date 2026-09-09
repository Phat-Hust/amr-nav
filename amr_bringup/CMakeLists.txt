cmake_minimum_required(VERSION 3.8)
project(amr_bringup)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(MSVC)
  add_compile_definitions(_USE_MATH_DEFINES)
endif()

# Core dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)

# Include directories (if you have headers under include/)
include_directories(include)

# Optional executable (keep only if you have C++ nodes in amr_bringup/src)
# add_executable(amr_bringup src/amr_bringup.cpp)
# target_compile_features(amr_bringup PUBLIC cxx_std_17)
# ament_target_dependencies(amr_bringup rclcpp)
# install(TARGETS amr_bringup DESTINATION lib/${PROJECT_NAME})

# Install runtime assets matching your bringup/simulation structure
install(
  DIRECTORY launch config params rviz maps
  DESTINATION share/${PROJECT_NAME}/
)

# Export include directories (if applicable)
ament_export_include_directories(include)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()