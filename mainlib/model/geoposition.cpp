/*
 * Copyright (c) 2012-2015 Ilja Booij (ibooij@gmail.com)
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

#include "geoposition.h"

const GeoPosition GeoPosition::NULL_POSITION = GeoPosition();

GeoPosition::GeoPosition():
    GeoPosition(0, QGeoCoordinate())
{
    // empty
}

GeoPosition::GeoPosition(qreal distance, const QGeoCoordinate &coordinate) :
    _distance(distance), _coordinate(coordinate)
{
    // empty
}

qreal GeoPosition::distance() const
{
    return _distance;
}

const QGeoCoordinate &GeoPosition::coordinate() const
{
    return _coordinate;
}

double GeoPosition::latitude() const
{
    return _coordinate.latitude();
}

double GeoPosition::longitude() const
{
    return _coordinate.longitude();
}

double GeoPosition::altitude() const
{
    return _coordinate.altitude();
}

bool GeoPosition::isValid() const
{
    return _coordinate.isValid();
}

GeoPosition GeoPosition::interpolateBetween(const GeoPosition &position1, const GeoPosition &position2, const double distance)
{
    if (position1.coordinate() == position2.coordinate()) {
        return position1;
    }
    const double distanceBetweenPositions = position2.distance() - position1.distance();
    const double ratio = (distance - position1.distance()) / distanceBetweenPositions;

    const double latitudeDifference = position2.latitude() - position1.coordinate().latitude();
    const double longitudeDifference = position2.longitude() - position1.coordinate().longitude();

    const double interpolatedLatitude = position1.latitude() + ratio * latitudeDifference;
    const double interpolatedLongitude = position1.longitude() + ratio * longitudeDifference;

    return GeoPosition(distance, QGeoCoordinate(interpolatedLatitude, interpolatedLongitude));
}

GeoPosition GeoPosition::withAltitude(const double altitude) const
{
    QGeoCoordinate copy = coordinate();
    copy.setAltitude(altitude);
    return GeoPosition(distance(), copy);
}
