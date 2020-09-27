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
#include "ridesampler.h"

#include "ridegui/run.h"

#include <QtCore/QtDebug>
#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>


namespace {
// We sample every second.
const int SAMPLE_INTERVAL_MS = 1000;
}

RideSampler::RideSampler(const QString &rlvName, const QString &courseName, const Simulation &simulation, QObject *parent):
    QObject(parent), _simulation(simulation), _rideFile(rlvName, courseName)
{
    _sampleTimer.setInterval(SAMPLE_INTERVAL_MS);
    connect(&_sampleTimer, &QTimer::timeout, this, &RideSampler::takeSample);
}

const RideFile &RideSampler::rideFile() const
{
    return _rideFile;
}

void RideSampler::start()
{
    _sampleTimer.start();
}

void RideSampler::stop()
{
    _sampleTimer.stop();
}

void RideSampler::takeSample()
{
    const Cyclist &cyclist = _simulation.cyclist();
    RideFile::Sample sample = { _simulation.runTime(),
                                cyclist.altitude(),
                                cyclist.cadence(),
                                cyclist.distanceTravelled(),
                                cyclist.heartRate(),
                                cyclist.power(),
                                cyclist.speed(),
                                cyclist.geoPosition()};
    _rideFile.addSample(sample);
}

