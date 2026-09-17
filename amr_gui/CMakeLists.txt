cmake_minimum_required(VERSION 3.16)
project(amr_gui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(amr_common REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Quick Core Gui)

qt_add_resources(RESOURCES_RCC qml.qrc)

add_executable(amr_gui
    main.cpp
    common/ros_manager.cpp
    common/ros_manager.h
    common/defines.h
    controller/pid_controller.cpp
    controller/pid_controller.h
    model/wheel_data_model.cpp
    model/wheel_data_model.h
    ${RESOURCES_RCC}
)

target_include_directories(amr_gui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(amr_gui rclcpp amr_common)
target_link_libraries(amr_gui Qt6::Core Qt6::Gui Qt6::Quick)

install(TARGETS amr_gui DESTINATION lib/${PROJECT_NAME})
ament_package()
