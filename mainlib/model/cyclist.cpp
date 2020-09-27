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

#include "cyclist.h"

Cyclist::Cyclist(const qreal userWeight, const qreal bikeWeight, QObject *parent) :
    QObject(parent), _userWeight(userWeight), _bikeWeight(bikeWeight)
{
}

void Cyclist::setSpeed(float speed) {
    _speed = speed;
    emit speedChanged(speed);
}

void Cyclist::setGeoPosition(const GeoPosition &position)
{
    _geoPosition = position;
    emit geoPositionChanged(_geoPosition);
}

void Cyclist::setDistance(float distance) {
    _distance = distance;
    emit distanceChanged(distance);
}

void Cyclist::setDistanceTravelled(float distanceTravelled)
{
    _distanceTravelled = distanceTravelled;
    emit distanceTravelledChanged(distanceTravelled);
}

void Cyclist::setAltitude(float altitude)
{
    _altitude = altitude;
    emit altitudeChanged(altitude);
}


float Cyclist::distance() const
{
    return _distance;
}

float Cyclist::distanceTravelled() const
{
    return _distanceTravelled;
}

float Cyclist::altitude() const
{
    return _altitude;
}

float Cyclist::speed() const
{
    return _speed;
}

int Cyclist::heartRate() const
{
    return _heartRate;
}

int Cyclist::power() const
{
    return _power;
}

GeoPosition Cyclist::geoPosition() const
{
    return _geoPosition;
}

int Cyclist::cadence() const
{
    return _cadence;
}

qreal Cyclist::userWeight() const
{
    return _userWeight;
}

qreal Cyclist::bikeWeight() const
{
    return _bikeWeight;
}

qreal Cyclist::totalWeight() const
{
    return _userWeight + _bikeWeight;
}

void Cyclist::setHeartRate(int heartRate)
{
    _heartRate = heartRate;
    emit heartRateChanged(heartRate);
}

void Cyclist::setCadence(int cadence)
{
    _cadence = cadence;
    emit cadenceChanged(cadence);
}

void Cyclist::setPower(int power)
{
    _power = power;
    emit powerChanged(power);
}
