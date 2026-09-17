#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include "common/ros_manager.h"
#include "controller/pid_controller.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    RosManager::instance().init(argc, argv);
    PidController pidController;

    QTimer rosTimer;
    QObject::connect(&rosTimer, &QTimer::timeout, []() {
        RosManager::instance().spinSome();
    });
    rosTimer.start(10); // 100 Hz

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("PidController", &pidController);
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    int execReturn = app.exec();
    RosManager::instance().shutdown();
    return execReturn;
}
