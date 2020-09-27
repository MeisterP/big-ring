/*
 * Copyright (c) 2015-2016 Ilja Booij (ibooij@gmail.com)
 * Copyright (c) 2016 Vlad Naoukin (vladn2000@hotmail.com)
 *
 * This file is part of Big Ring Indoor Video Cycling
 *
 * Big Ring Indoor Video Cycling is free software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Big Ring Indoor Video Cycling  is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Big Ring Indoor Video Cycling.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include "profilepainter.h"

#include <QtCore/QtDebug>
#include <QtGui/QPainter>
#include <QtGui/QPixmapCache>

#include "quantityprinter.h"
#include "model/reallifevideo.h"
#include "model/unitconverter.h"

#include <array>
#include <cmath>

namespace
{
const int MAXIMUM_HUE = 240; // dark blue;
const float MINIMUM_SLOPE = -12.0;
const float MAXIMUM_SLOPE = 18.0;
const float SLOPE_RANGE = MAXIMUM_SLOPE - MINIMUM_SLOPE;

const std::array<double,8> MARKER_DISTANCES = {{0.25, 0.5, 1, 2, 5, 10, 20, 50}};
const std::array<double,7> MARKER_ALTITUDES = {{10, 20, 50, 100, 200, 500, 1000}};
}

ProfilePainter::ProfilePainter(QObject *parent) :
    QObject(parent), _quantityPrinter(new QuantityPrinter(this)), _unitConverter(new UnitConverter(this))
{
    // empty
}

QPixmap ProfilePainter::paintProfile(const RealLifeVideo &rlv, const QRect &rect, float startDistance, float endDistance, bool withMarkers) const
{
    QPixmap profilePixmap;
    if (rlv.isValid()) {
        QRect profileRect = QRect(QPoint(0,0),rect.size());

        const QString pixmapName = QString("%1_%2_%3_%4x%5").arg(rlv.name()).arg(startDistance).arg(endDistance).arg(rect.size().width()).arg(rect.size().height());
        if (!QPixmapCache::find(pixmapName, &profilePixmap)) {
            qDebug() << "creating new profile pixmap for" << pixmapName;
            profilePixmap = drawProfilePixmap(profileRect, rlv, startDistance, endDistance, withMarkers);
            QPixmapCache::insert(pixmapName, profilePixmap);
        }
    }
    return profilePixmap;
}

QPixmap ProfilePainter::paintProfile(const RealLifeVideo &rlv, const QRect &rect, bool withMarkers) const
{
    return paintProfile(rlv, rect, 0, rlv.totalDistance(), withMarkers);
}

QPixmap ProfilePainter::paintProfileWithHighLight(const RealLifeVideo &rlv, qreal startDistance, qreal endDistance,
                                                  const QRect &rect, const QBrush highlightColor) const
{
    QPixmap profilePixmap = paintProfile(rlv, rect, true);
    if (profilePixmap.isNull()) {
        return profilePixmap;
    }

    int startX = distanceToX(rect, 0, rlv.totalDistance(), startDistance);
    int endX = distanceToX(rect, 0, rlv.totalDistance(), endDistance);

    QPixmap copy(profilePixmap);
    QPainter painter(&copy);

    QColor color(highlightColor.color());
    color.setAlphaF(0.5);
    painter.setBrush(color);
    painter.drawRect(startX, 0, endX - startX, rect.height());
    painter.end();

    return copy;
}

QPixmap ProfilePainter::drawProfilePixmap(QRect& rect, const RealLifeVideo& rlv, float startDistance, float endDistance, bool withMarkers ) const
{
    if (rect.isEmpty()) {
        return QPixmap();
    }

    QPixmap pixmap(rect.size());
    QPainter painter(&pixmap);

    float minimumAltitude = rlv.profile().minimumAltitudeForPart(startDistance, endDistance);
    float maximumAltitude = rlv.profile().maximumAltitudeForPart(startDistance, endDistance);

    float altitudeDiff = maximumAltitude - minimumAltitude;
    painter.setBrush(Qt::gray);
    painter.drawRect(rect);
    painter.setBrush(Qt::white);
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing);

    for(int x = 0; x < rect.width(); x += 1) {
        float distance = xToDistance(rect, startDistance, endDistance - startDistance, x);
        float altitude = rlv.profile().altitudeForDistance(distance);
        painter.setBrush(colorForSlope(rlv.profile().slopeForDistance(distance)));

        int y = altitudeToHeight(rect, altitude - minimumAltitude, altitudeDiff);
        QRect  box(x, rect.bottom() - y, 1, rect.bottom());
        painter.drawRect(box);
    }
    if (withMarkers) {
        drawDistanceMarkers(painter, rect, startDistance, endDistance - startDistance);
        drawAltitudeMarkers(painter, rect, rlv, startDistance, endDistance);
    }
    painter.end();
    return pixmap;
}

void ProfilePainter::drawDistanceMarkers(QPainter &painter, const QRect &rect, float startDistance, float totalDistance) const
{
    QPen pen(Qt::black, 1);
    painter.setPen(pen);
    const double distanceBetweenMarkers = determineDistanceMarkers(totalDistance);

    for (double distance = distanceBetweenMarkers; distance < totalDistance; distance += distanceBetweenMarkers) {
        int x = distanceToX(rect, startDistance, totalDistance, distance + startDistance);
        painter.drawLine(x, rect.height () - 20, x, rect.height());
        QString distanceMarker = QString("%1 %2").arg(_quantityPrinter->printDistance(distance))
                .arg(_quantityPrinter->unitForDistance(QuantityPrinter::Precision::NonPrecise,
                                                       QVariant::fromValue(distance)));
        painter.drawText(x, rect.height(), distanceMarker);
    }
}

double ProfilePainter::determineDistanceMarkers(float totalDistance) const
{
    std::vector<double> distances(MARKER_DISTANCES.size());
    std::transform(MARKER_DISTANCES.begin(), MARKER_DISTANCES.end(), distances.begin(), [this](double distance) {
        return _unitConverter->convertDistanceFromSystemUnit(distance);
    });
    const double optimalDistanceBetweenMarkers = totalDistance / 10;
    double distanceBetweenMarkers;
    auto distanceBetweenMarkersIt = std::lower_bound(distances.begin(), distances.end(), optimalDistanceBetweenMarkers);
    if (distanceBetweenMarkersIt == distances.end()) {
        distanceBetweenMarkers = distances.back();
    } else {
        distanceBetweenMarkers = *distanceBetweenMarkersIt;
    }
    return distanceBetweenMarkers;
}

void ProfilePainter::drawAltitudeMarkers(QPainter &painter, const QRect &rect, const RealLifeVideo &rlv, float startDistance, float endDistance) const
{
    const float minimumAltitude = rlv.profile().minimumAltitudeForPart(startDistance, endDistance);
    const float maximumAltitude = rlv.profile().maximumAltitudeForPart(startDistance, endDistance);
    const float altitudeRange = maximumAltitude - minimumAltitude;
    const double altitudeBetweenMarkers = determineAltitudeMarkers(altitudeRange);
    QPen pen(Qt::black, 1, Qt::DashLine);
    painter.setPen(pen);

    const float quotient = std::floor(rlv.profile().minimumAltitude() / altitudeBetweenMarkers);
    const float startAltitude = quotient * altitudeBetweenMarkers + altitudeBetweenMarkers;

    for (float altitude = startAltitude; altitude < maximumAltitude; altitude += altitudeBetweenMarkers) {
        int y = altitudeToHeight(rect, altitude - minimumAltitude, altitudeRange);
        painter.drawLine(0, rect.height() - y, rect.width(), rect.height() - y);
        QString text = QString("%1 %2").arg(_quantityPrinter->printAltitude(altitude))
                .arg(_quantityPrinter->unitForAltitude());
        painter.drawText(5, rect.height() - y - 5, text);
        QFontMetrics fontMetrics(painter.font());
        int width = fontMetrics.horizontalAdvance(text);
        painter.drawText(rect.width() - width - 10, rect.height() - y - 5, text);
    }
}

double ProfilePainter::determineAltitudeMarkers(const float altitudeRange) const
{
    std::vector<double> altitudes(MARKER_ALTITUDES.size());
    std::transform(MARKER_ALTITUDES.begin(), MARKER_ALTITUDES.end(), altitudes.begin(), [this](double altitude) {
        return _unitConverter->convertAltitudeFromSystemUnit(altitude);
    });
    const float optimalAltitudeMarkers = altitudeRange / 7;
    auto altitudeBetweenMarkersIt = std::lower_bound(altitudes.begin(), altitudes.end(), optimalAltitudeMarkers);
    if (altitudeBetweenMarkersIt == altitudes.end()) {
        return altitudes.back();
    } else {
        return *altitudeBetweenMarkersIt;
    }
}

qreal ProfilePainter::distanceToX(const QRect& rect, float startDistance, float totalDistance, float distance) const
{
    return ((distance - startDistance) / totalDistance) * rect.width() + rect.left();
}

float ProfilePainter::xToDistance(const QRect& rect, float startDistance, float totalDistance, int x) const
{
    float relative = x * 1.0 / rect.width();
    return totalDistance * relative + startDistance;
}

int ProfilePainter::altitudeToHeight(const QRect& rect, float altitudeAboveMinimum, float altitudeDiff) const
{
    return static_cast<int>(((altitudeAboveMinimum) / altitudeDiff) * rect.height() * .9);
}

/**
 * Determine the color for a slope. We'll use HSV color values from 240 (dark blue) for a steep downhill grade
 * through 0 to -60 (magenta) for a steep uphill grade. QColor::fromHsv does not deal with negative values, so we'll
 * add 360 for negative hues.
 *
 * For determining the hue, we'll take the slope, bounded by MINIMUM_SLOPE and MAXIMUM_SLOPE and project it onto
 * the range determined by SLOPE_RANGE (MAXIMUM_SLOPE - MINIMUM_SLOPE). We'll normalize this value to a value between
 * 0 and 1. and then project that on to the HUE range.
 */
QColor ProfilePainter::colorForSlope(const float slope) const {
    const float boundedSlope = qBound(MINIMUM_SLOPE, slope, MAXIMUM_SLOPE);
    const float normalizedSlope = (boundedSlope - MINIMUM_SLOPE) / SLOPE_RANGE;

    const int hueValue = MAXIMUM_HUE - static_cast<int>(std::round((normalizedSlope * SLOPE_RANGE * 10)));
    return QColor::fromHsv((hueValue >= 0)? hueValue: hueValue + 360, 255, 255);
}
