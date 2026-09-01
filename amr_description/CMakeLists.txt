################################################################################
# Set minimum required version of cmake, project name and compile options
################################################################################
cmake_minimum_required(VERSION 3.5)
project(tvc_description)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(MSVC)
  add_compile_definitions(_USE_MATH_DEFINES)
endif()

################################################################################
# Find ament packages and libraries for ament and system dependencies
################################################################################
find_package(ament_cmake REQUIRED)

################################################################################
# Build
################################################################################

################################################################################
# Install
################################################################################

install(DIRECTORY launch urdf meshes
  DESTINATION share/${PROJECT_NAME}/
)

configure_file(urdf/amr_1lidar.urdf ${CMAKE_INSTALL_PREFIX}/../../rootfs/var/tvcamr/config/amr_1lidar.urdf COPYONLY)
configure_file(urdf/amr_2lidar.urdf ${CMAKE_INSTALL_PREFIX}/../../rootfs/var/tvcamr/config/amr_2lidar.urdf COPYONLY)

################################################################################
# Macro for ament package
################################################################################
ament_package()



