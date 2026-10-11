#include "map_view.h"

MapView::MapView(QQuickItem *parent) : QQuickPaintedItem(parent)  {
    setAntialiasing(true);
    setAcceptedMouseButtons(Qt::AllButtons);
    connect(&RosManager::instance(), &RosManager::occupancyGridReceived,
            this, &MapView::updateOccupancyGrid);

    connect(&RosManager::instance(), &RosManager::laserScanReceived,
            this, &MapView::updateLaserScan);

    connect(&RosManager::instance(), &RosManager::handleRobotPose,
            this, &MapView::updateRobotPose);

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

    if (m_resolution > 0.0 && m_mapWidth > 0 && m_mapHeight > 0) {
        // 1. Calculate map pixel location for world origin (0.0, 0.0)
        QPointF worldOriginPixel = worldToMapPixel(0.0, 0.0);

        // 2. Pan to center world origin (0,0) in the middle of the QML MapView viewport
        qreal centerX = width() / 2.0;
        qreal centerY = height() / 2.0;

        m_pan = QPointF(centerX - worldOriginPixel.x() * m_zoom,
                        centerY - worldOriginPixel.y() * m_zoom);
    } else {
        // Fallback if no map is loaded yet
        m_pan = QPointF(0.0, 0.0);
    }

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
        qDebug() << "[MapView] Invalid map dimensions:" << m_mapWidth << "x" << m_mapHeight;
        return;
    }

    QImage img(m_mapWidth, m_mapHeight, QImage::Format_RGBA8888);

    for (int y = 0; y < m_mapHeight; ++y) {
        // ROS grid origin is bottom-left; QImage is top-left
        uchar *scanline = img.scanLine(m_mapHeight - 1 - y);

        for (int x = 0; x < m_mapWidth; ++x) {
            int8_t cell = mapMsg.data[y * m_mapWidth + x];
            uchar r, g, b, a;

            if (cell == -1) {        // Unknown space -> Dark Gray
                r = 45;  g = 45;  b = 45;  a = 255;
            } else if (cell == 0) {  // Free space -> Light Gray
                r = 255; g = 255; b = 255; a = 255;
            } else {                 // Occupied wall -> Black / Dark
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

    // Center view if initializing for the first time or pan is default
    if (m_pan.x() == 0.0 && m_pan.y() == 0.0) {
        m_pan = QPointF(width() / 2.0 - (m_mapWidth / 2.0), height() / 2.0 - (m_mapHeight / 2.0));
    }

    qDebug() << "[MapView] Occupancy grid image updated (" << m_mapWidth << "x" << m_mapHeight << ")";
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

    // Fill dark background behind viewport
    painter->fillRect(boundingRect(), QColor("#ffffff"));

    if (m_mapImage.isNull()) {
        // Draw centered hint text when no map is loaded
        painter->setPen(QColor(120, 120, 120));
        painter->drawText(boundingRect(), Qt::AlignCenter, "No Map Loaded\nSelect a .yaml or start SLAM");
        return;
    }

    painter->save();

    // Apply pan and zoom transforms
    painter->translate(m_pan);
    painter->scale(m_zoom, m_zoom);

    // 1. Render occupancy grid image (Pixel coordinates [0, 0] to [m_mapWidth, m_mapHeight])
    painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter->drawImage(0, 0, m_mapImage);

    // 2. Render laser scan points
    if (!m_laserPoints.isEmpty()) {
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(255, 30, 30, 220)); // Bright Red

        qreal pointRadius = 2.0 / m_zoom; // Fixed screen-pixel size
        for (const QPointF &worldPt : m_laserPoints) {
            QPointF mapPt = worldToMapPixel(worldPt.x(), worldPt.y());
            painter->drawEllipse(mapPt, pointRadius, pointRadius);
        }
    }

    // 3. Render Robot Footprint & Heading
    if (m_mapWidth > 0 && m_mapHeight > 0) {
        QPointF robotMapPt = worldToMapPixel(m_robotPose.x(), m_robotPose.y());

        painter->save();
        painter->translate(robotMapPt);
        painter->rotate(-m_robotYaw * (180.0 / M_PI)); // Invert Y-axis rotation

        qreal radius = 8.0 / m_zoom;

        // Footprint body
        painter->setPen(QPen(QColor(0, 229, 255), 2.0 / m_zoom));
        painter->setBrush(QColor(0, 229, 255, 100));
        painter->drawEllipse(QPointF(0, 0), radius, radius);

        // Direction arrow
        painter->setPen(QPen(QColor(255, 235, 59), 2.5 / m_zoom));
        painter->drawLine(QPointF(0, 0), QPointF(radius * 1.6, 0));

        painter->restore();
    }

    painter->restore();
}

