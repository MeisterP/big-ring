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

#ifndef GEOPOSITION_H
#define GEOPOSITION_H

#include <QtPositioning/QGeoCoordinate>

class GeoPosition
{
public:
    /**
     * Construct an empty, invalid, GeoPosition object
     */
    explicit GeoPosition();
    /**
     * @brief construct a GeoPosition object from a distance and a coodinate.
     * @param distance distance on the track (from the start)
     * @param coordinate the coordinate.
     */
    explicit GeoPosition(qreal distance, const QGeoCoordinate &coordinate);

    /** distance from the start */
    qreal distance() const;
    /** the coordinate of the position. Will be invalid if the isValid() method returns false */
    const QGeoCoordinate &coordinate() const;
    /** latitude of the position */
    double latitude() const;
    /** longitude of the position */
    double longitude() const;
    /** altitude of the position */
    double altitude() const;
    /** true if the position is valid */
    bool isValid() const;

    /** A NULL position with an invalid coordinate. */
    static const GeoPosition NULL_POSITION;

    /**
     * @brief interpolate between two positions.
     *
     * @param position1 first position
     * @param position2 second position
     * @param distance distance of the position we want to interpolate. Value should be between position1.distance() and position2.distance().
     */
    static GeoPosition interpolateBetween(const GeoPosition &position1, const GeoPosition &position2, const double distance);

    /**
     * Construct a copy of the same GeoPosition with a different altitude
     *
     * @param altitude the altitude.
     * @return the GeoPosition with a new altitude.
     */
    GeoPosition withAltitude(const double altitude) const;
private:
    qreal _distance;
    QGeoCoordinate _coordinate;
};

#endif // GEOPOSITION_H
