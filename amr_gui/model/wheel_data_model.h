// model/wheel_data_model.h
#pragma once
#include <QObject>
#include <QList>
#include <QPointF>
#include "../common/defines.h"

class WheelDataModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QList<QPointF> targetPoints READ targetPoints NOTIFY pointsChanged)
    Q_PROPERTY(QList<QPointF> currentPoints READ currentPoints NOTIFY pointsChanged)

public:
    explicit WheelDataModel(QObject *parent = nullptr);

    QList<QPointF> targetPoints() const { return m_targetPoints; }
    QList<QPointF> currentPoints() const { return m_currentPoints; }

    void addPoint(double time, double target, double current);

signals:
    void pointsChanged();

private:
    QList<QPointF> m_targetPoints;
    QList<QPointF> m_currentPoints;
};
