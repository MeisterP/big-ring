#include "profileitem.h"

#include <QtCore/QPair>
#include <QtCore/QtDebug>
#include <QtGui/QPainter>

namespace
{
QPair<float,float> findMinimumAndMaximumAltiude(const QList<ProfileEntry>& profileEntries)
{
    float minimumAltitude = std::numeric_limits<float>::max();
    float maximumAltitude = std::numeric_limits<float>::min();
    foreach (const ProfileEntry& entry, profileEntries) {
        minimumAltitude = qMin(minimumAltitude, entry.altitude());
        maximumAltitude = qMax(maximumAltitude, entry.altitude());
    }
    return qMakePair(minimumAltitude, maximumAltitude);
}
}
ProfileItem::ProfileItem(Simulation& simulation, QObject *parent) :
    QObject(parent), _simulation(simulation)
{
    setOpacity(0.65);
}

QRectF ProfileItem::boundingRect() const
{
    return QRectF();
}

void ProfileItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QPen pen(QColor(Qt::green));
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(2);
    painter->setOpacity(0.5);
    painter->setPen(pen);
    painter->setBrush(Qt::lightGray);
    painter->drawRoundedRect(0, 0, _size.width(), _size.height(), 5, 5);
    if (_rlv.isValid()) {
        if (!_profile.isEmpty()) {
            painter->setRenderHint(QPainter::Antialiasing);
            painter->fillPath(_profile, QBrush(Qt::white));

            float distanceRatio = _simulation.cyclist().distance() / _rlv.totalDistance();
            QPen pen(QColor(Qt::red));
            pen.setStyle(Qt::SolidLine);
            pen.setWidth(5);
            painter->setPen(pen);
            painter->drawLine(distanceRatio * _size.width() + 10, 5, distanceRatio * _size.width() + 10, _size.height());
        }
    }
}

void ProfileItem::setSize(const QSize &size)
{
    qDebug() << "size =" << size.width() << size.height();
    _size = size;
    if (_rlv.isValid()) {
        _profile = drawProfile();
    }
    update();
}

void ProfileItem::setRlv(const RealLifeVideo &rlv)
{
    _rlv = rlv;
    _profile = drawProfile();
    update();
}

QPainterPath ProfileItem::drawProfile()
{
    qDebug() << "creating profile path";
    int pathHeight = _size.height() - 10;
    int xMargin = 10;

    auto profileEntries = _rlv.profile().entries();
    auto minAndMaxAltitude = findMinimumAndMaximumAltiude(profileEntries);

    float minimumAltitude = minAndMaxAltitude.first;
    float maximumAltitude = minAndMaxAltitude.second;

    float altitudeDiff = maximumAltitude - minimumAltitude;

    QPainterPath path;
    path.moveTo(xMargin, _size.height());
    foreach(const ProfileEntry& entry, profileEntries) {
        qreal x = distanceToX(entry.totalDistance(), xMargin);
        qreal y = altitudeToY(entry.altitude() - minimumAltitude, altitudeDiff, pathHeight);

        path.lineTo(x, y);
    }
    path.lineTo(_size.width() - xMargin, _size.height());
    path.lineTo(xMargin, _size.height());

    return path;
}

qreal ProfileItem::distanceToX(float distance, int xMargin) const
{
    int pathWidth = _size.width() - xMargin * 2;
    return (distance / _rlv.totalDistance()) * pathWidth + xMargin;
}

qreal ProfileItem::altitudeToY(float altitudeAboveMinimum, float altitudeDiff, int pathHeight) const
{
    return _size.height() - (((altitudeAboveMinimum) / altitudeDiff) * pathHeight);
}
