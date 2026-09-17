// model/wheel_data_model.cpp
#include "wheel_data_model.h"

WheelDataModel::WheelDataModel(QObject *parent) : QObject(parent) {}

void WheelDataModel::addPoint(double time, double target, double current) {
    m_targetPoints.append(QPointF(time, target));
    m_currentPoints.append(QPointF(time, current));

    if (m_targetPoints.size() > amr::MAX_BUFFER_POINTS) {
        m_targetPoints.removeFirst();
        m_currentPoints.removeFirst();
    }
    emit pointsChanged();
}
