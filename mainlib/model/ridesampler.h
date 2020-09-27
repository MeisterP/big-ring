/*
 * Copyright (c) 2015 Ilja Booij (ibooij@gmail.com)
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
#ifndef RIDESAMPLER_H
#define RIDESAMPLER_H

#include <QtCore/QObject>
#include <QtCore/QTimer>

#include "ridefile.h"

class Simulation;

/**
 * Sample the simulation continously and keeps a record of all values (altitude, speed, power etc) in the ride.
 */
class RideSampler : public QObject
{
    Q_OBJECT
public:
    explicit RideSampler(const QString &rlvName, const QString &courseName, const Simulation &simulation, QObject *parent = 0);

    /** Get the ride file, with all values. This method can be called multiple times to get updates */
    const RideFile &rideFile() const;
public slots:
    void start();
    void stop();
private slots:
    void takeSample();
private:
    const Simulation &_simulation;
    QTimer _sampleTimer;
    RideFile _rideFile;
};

#endif // RIDESAMPLER_H
