#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include "common/ros_manager.h"
#include "controller/pid_controller.h"
#include "model/wheel_data_model.h"
#include "model/map_view.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    // Register type
    qRegisterMetaType<amr_common::msg::WheelTelemetry>("amr_common::msg::WheelTelemetry");
    qmlRegisterUncreatableType<WheelDataModel>("WheelDataModel", 1, 0, "WheelDataModel",
            "WheelDataModel cannot be instantiated directly in QML");
    qmlRegisterType<MapView>("AmrGui", 1, 0, "MapView");

    RosManager::instance().init(argc, argv);
    PidController pidController;
    MapView mapView;

    // Define signals and slots
    QObject::connect(&RosManager::instance(), &RosManager::handleWheelTelemetry,
                     &pidController, &PidController::updateWheelTelemetry);
    QObject::connect(&RosManager::instance(), &RosManager::handleOccupancyGridMap,
                     &mapView, &MapView::updateOccupancyGrid);
    QObject::connect(&RosManager::instance(), &RosManager::handleLaserScan,
                     &mapView, &MapView::updateLaserScan);
    QObject::connect(&RosManager::instance(), &RosManager::handleRobotPose,
                     &mapView, &MapView::updateRobotPose);

    QTimer rosTimer;
    QObject::connect(&rosTimer, &QTimer::timeout, []() {
        RosManager::instance().spinSome();
    });
    rosTimer.start(20); // 50 Hz


    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("PidController", &pidController);
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    int execReturn = app.exec();
    RosManager::instance().shutdown();
    return execReturn;
}
