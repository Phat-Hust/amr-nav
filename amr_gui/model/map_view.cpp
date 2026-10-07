#include "map_view.h"

MapView::MapView(QQuickItem *parent) : QQuickPaintedItem(parent)  {
    setAntialiasing(true);
    setAcceptedMouseButtons(Qt::AllButtons);
}

// Getters
qreal MapView::zoom() const {
    return m_zoom;
}

QPointF MapView::pan() const {
    return m_pan;
}

qreal MapView::robotX() const {
    return m_robotPose.x();
}

qreal MapView::robotY() const {
    return m_robotPose.y();
}

qreal MapView::robotYaw() const {
    return m_robotYaw;
}

// Setters
void MapView::setZoom(qreal z) {
    z = qBound(0.1, z, 20.0);
    if (!qFuzzyCompare(m_zoom, z)) {
        m_zoom = z;
        emit zoomChanged();
        update();
    }
}

void MapView::setPan(const QPointF &p) {
    if (m_pan != p) {
        m_pan = p;
        emit panChanged();
        update();
    }
}

void MapView::addPan(qreal dx, qreal dy) {
    m_pan += QPointF(dx, dy);
    emit panChanged();
    update();
}

void MapView::zoomAt(qreal factor, const QPointF &center) {
    qreal oldZoom = m_zoom;
    qreal newZoom = qBound(0.1, m_zoom * factor, 20.0);

    // Zoom centered at the cursor position
    m_pan = center - (center - m_pan) * (newZoom / oldZoom);
    m_zoom = newZoom;

    emit zoomChanged();
    emit panChanged();
    update();
}

void MapView::resetView() {
    m_zoom = 1.0;
    m_pan = QPointF(0.0, 0.0);
    emit zoomChanged();
    emit panChanged();
    update();
}

void MapView::updateOccupancyGrid(const nav_msgs::msg::OccupancyGrid &mapMsg) {
    QMutexLocker locker(&m_mutex);

    m_resolution = mapMsg.info.resolution;
    m_originX    = mapMsg.info.origin.position.x;
    m_originY    = mapMsg.info.origin.position.y;
    m_mapWidth   = mapMsg.info.width;
    m_mapHeight  = mapMsg.info.height;

    if (m_mapWidth <= 0 || m_mapHeight <= 0) {
        qDebug() << "Failed to load occupancy grid map due to unconsist map (width or height is zero)";
        return;
    }

    QImage img(m_mapWidth, m_mapHeight, QImage::Format_RGBA8888);

    for (int y = 0; y < m_mapHeight; ++y) {
        // ROS grid has (0,0) at bottom-left; QImage has (0,0) at top-left
        uchar *scanline = img.scanLine(m_mapHeight - 1 - y);

        for (int x = 0; x < m_mapWidth; ++x) {
            int8_t cell = mapMsg.data[y * m_mapWidth + x];
            uchar r, g, b, a;

            if (cell == -1) {        // Unknown space -> Dark gray
                r = 45;  g = 45;  b = 45;  a = 255;
            } else if (cell == 0) {  // Free navigable space -> Light white/gray
                r = 230; g = 230; b = 230; a = 255;
            } else {                 // Occupied / Wall (1 to 100) -> Solid Dark
                uchar val = static_cast<uchar>(230 - (cell * 200 / 100));
                r = val; g = val; b = val; a = 255;
            }

            scanline[x * 4 + 0] = r;
            scanline[x * 4 + 1] = g;
            scanline[x * 4 + 2] = b;
            scanline[x * 4 + 3] = a;
        }
    }

    m_mapImage = img;
    update();
}

void MapView::updateLaserScan(const sensor_msgs::msg::LaserScan &scanMsg) {
    QVector<QPointF> points;
    points.reserve(scanMsg.ranges.size());

    double angle = scanMsg.angle_min;
    double rx = m_robotPose.x();
    double ry = m_robotPose.y();
    double yaw = m_robotYaw;

    for (size_t i = 0; i < scanMsg.ranges.size(); ++i, angle += scanMsg.angle_increment) {
        double r = scanMsg.ranges[i];
        if (r < scanMsg.range_min || r > scanMsg.range_max || std::isnan(r) || std::isinf(r)) {
            continue;
        }

        // 1. Transform from Laser frame to Robot Base frame
        double lx = r * std::cos(angle);
        double ly = r * std::sin(angle);

        // 2. Transform from Robot Base frame to Global Map frame
        double gx = rx + (lx * std::cos(yaw) - ly * std::sin(yaw));
        double gy = ry + (lx * std::sin(yaw) + ly * std::cos(yaw));

        points.append(QPointF(gx, gy));
    }

    {
        QMutexLocker locker(&m_mutex);
        m_laserPoints = std::move(points);
    }
    update();
}

void MapView::updateRobotPose(double x, double y, double yaw) {
    m_robotPose = QPointF(x, y);
    m_robotYaw = yaw;
    emit robotPoseChanged();
    update();
}

QPointF MapView::worldToMapPixel(double wx, double wy) const {
    if (m_resolution <= 0.0) return QPointF(0, 0);

    // Pixel X
    double px = (wx - m_originX) / m_resolution;
    // Pixel Y inverted because (0,0) in QImage is top-left
    double py = m_mapHeight - ((wy - m_originY) / m_resolution);

    return QPointF(px, py);
}

void MapView::paint(QPainter *painter) {
    QMutexLocker locker(&m_mutex);

    painter->save();
    painter->translate(m_pan);
    painter->scale(m_zoom, m_zoom);

    // Reder occupancy grid map
    if (!m_mapImage.isNull()) {
        painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
        painter->drawImage(0, 0, m_mapImage);
    }

    // Render laser scan points
    if (!m_laserPoints.isEmpty()) {
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(255, 30, 30, 220)); // Bright Red

        qreal pointRadius = 2.0 / m_zoom; // Constant screen-pixel size
        for (const QPointF &worldPt : m_laserPoints) {
            QPointF mapPt = worldToMapPixel(worldPt.x(), worldPt.y());
            painter->drawEllipse(mapPt, pointRadius, pointRadius);
        }
    }

    if (m_mapWidth > 0 && m_mapHeight > 0) {
        QPointF robotMapPt = worldToMapPixel(m_robotPose.x(), m_robotPose.y());

        painter->save();
        painter->translate(robotMapPt);
        // Note: invert rotation angle because the Y-axis is inverted
        painter->rotate(-m_robotYaw * (180.0 / M_PI));

        qreal radius = 8.0 / m_zoom;

        // Draw footprint body
        painter->setPen(QPen(QColor(0, 229, 255), 2.0 / m_zoom));
        painter->setBrush(QColor(0, 229, 255, 100));
        painter->drawEllipse(QPointF(0, 0), radius, radius);

        // Draw heading arrow pointing along +X
        painter->setPen(QPen(QColor(255, 235, 59), 2.5 / m_zoom));
        painter->drawLine(QPointF(0, 0), QPointF(radius * 1.6, 0));

        painter->restore();
    }

    painter->restore();
}


