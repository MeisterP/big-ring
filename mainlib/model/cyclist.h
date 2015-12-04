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

#ifndef CYCLIST_H
#define CYCLIST_H

#include <QObject>

class Cyclist : public QObject
{
    Q_OBJECT
public:
    explicit Cyclist(const qreal userWeight, const qreal bikeWeight, QObject *parent = 0);
    void setDistance(float distance);
    void setDistanceTravelled(float distanceTravelled);
    void setSpeed(float speed);

    float distance() const;
    float distanceTravelled() const;
    float speed() const;
    int heartRate() const;
    int cadence() const;
    int power() const;

    qreal userWeight() const;
    qreal bikeWeight() const;
    qreal totalWeight() const;

public slots:
    void setHeartRate(int heartRate);
    void setCadence(int cadence);
    void setPower(int power);

signals:
    void heartRateChanged(int heartRate);
    void cadenceChanged(int cadence);
    void powerChanged(int power);

    void speedChanged(float speed);
    /** distance on track changed */
    void distanceChanged(float distance);
    /** distance travelled from start of course */
    void distanceTravelledChanged(float distanceTravelled);

private:
    const qreal _userWeight;
    const qreal _bikeWeight;
    int _heartRate;
    int _cadence;
    int _power;

    float _speed;
    float _distance;
    float _distanceTravelled;
};

#endif // CYCLIST_H
