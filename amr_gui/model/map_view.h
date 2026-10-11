#pragma once
#ifndef MAP_VIEW_H
#define MAP_VIEW_H

#include <QQuickPaintedItem>
#include <QPainter>
#include <QImage>
#include <QMutex>
#include <QPointF>
#include <QVector>
#include <cmath>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>

#include "../common/ros_manager.h"
class MapView : public QQuickPaintedItem
{
    Q_OBJECT
        Q_PROPERTY(qreal zoom READ zoom WRITE setZoom NOTIFY zoomChanged)
        Q_PROPERTY(QPointF pan READ pan WRITE setPan NOTIFY panChanged)
        Q_PROPERTY(qreal robotX READ robotX NOTIFY robotPoseChanged)
        Q_PROPERTY(qreal robotY READ robotY NOTIFY robotPoseChanged)
        Q_PROPERTY(qreal robotYaw READ robotYaw NOTIFY robotPoseChanged)
public:
    explicit MapView(QQuickItem *parent = nullptr);

    void paint(QPainter *painter) override;

    // Getters
    qreal zoom() const;
    QPointF pan() const;
    qreal robotX() const;
    qreal robotY() const;
    qreal robotYaw() const;

    // Setters
    void setPan(const QPointF &p);
    void setZoom(qreal z);

    // QINVOKABLE: helpers exposed to QML
    Q_INVOKABLE void addPan(qreal dx, qreal dy);
    Q_INVOKABLE void zoomAt(qreal factor, const QPointF &center);
    Q_INVOKABLE void resetView();

public slots:
    void updateOccupancyGrid(const nav_msgs::msg::OccupancyGrid &mapMsg);
    void updateLaserScan(const sensor_msgs::msg::LaserScan &scanMsg);
    void updateRobotPose(double x, double y, double yaw);

signals:
    void zoomChanged();
    void panChanged();
    void robotPoseChanged();

private:
    QPointF worldToMapPixel(double wx, double wy) const;

    QMutex m_mutex;
    QImage m_mapImage;
    double m_resolution{0.05};
    double m_originX{0.0};
    double m_originY{0.0};
    int m_mapWidth{0};
    int m_mapHeight{0};

    // Sensor and pose state
    QPointF m_robotPose{0.0, 0.0}; // In world frame (meters)
    double m_robotYaw{0.0};        // In radians
    QVector<QPointF> m_laserPoints; // In world frame (meters)

    qreal m_zoom{1.0};
    QPointF m_pan{0.0, 0.0};
};

#endif // MAP_VIEW_H
