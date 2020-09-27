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

#include <QtDebug>
#include "simulation.h"
#include <math.h>
namespace {

/** Frontal area of a cyclist is around .5 m*m */
const float FRONTAL_AREA = 0.58f;
const float DRAG_COEFFICIENT = 0.63f;
const float AIR_DENSITY = 1.226f; // Sea level

const float MINIMUM_SPEED = 0.5f; // m/s

const QTime MAX_IDLE_TIME(0, 0, 5);


/** Calculate drag from wind resistance */
float calculateAeroDrag(const Cyclist& cyclist)
{
    return FRONTAL_AREA * DRAG_COEFFICIENT * AIR_DENSITY * cyclist.speed() * cyclist.speed() * .5;
}

const float GRAVITY_CONSTANT = 9.81f;
const float ROLLING_RESISTANCE_COEFFICIENT = 0.004;

float calculateGroundResistance(const Cyclist& cylist)
{
    return cylist.totalWeight() * GRAVITY_CONSTANT * ROLLING_RESISTANCE_COEFFICIENT;
}


float calculateGravityForce(const Cyclist& cyclist, float grade)
{
    return GRAVITY_CONSTANT * grade * 0.01 * cyclist.totalWeight();
}

}



Simulation::Simulation(SimulationSetting simulationSetting, Cyclist &cyclist, double powerForElevationCorrection, QObject *parent) :
    QObject(parent), _simulationSetting(simulationSetting),
    _lastElapsed(0), _simulationTime(0,0,0), _idleTime(),_cyclist(cyclist), _powerForElevationCorrection(powerForElevationCorrection)
{
    _simulationUpdateTimer.setInterval(1000 / 30);
    connect(&_simulationUpdateTimer, SIGNAL(timeout()), SLOT(simulationStep()));
}

Simulation::~Simulation()
{
    _simulationUpdateTimer.stop();
}

Cyclist &Simulation::cyclist() const
{
    return _cyclist;
}

bool Simulation::isPlaying() const
{
    return _simulationUpdateTimer.isActive();
}

QTime Simulation::runTime() const
{
    return _runTime;
}

void Simulation::play(bool play)
{
    if (play) {
        _simulationUpdateTimer.start();
        _simulationTime.restart();
        _lastElapsed = 0;
    } else {
        _simulationUpdateTimer.stop();
    }
    emit playing(play);
}

void Simulation::simulationStep()
{
    if (!_currentRlv.isValid())
        return;

    qint64 currentElapsed = _simulationTime.elapsed();
    qint64 elapsed = currentElapsed - _lastElapsed;
    _lastElapsed = currentElapsed;
    if (_cyclist.speed() > 0) {
        _runTime = _runTime.addMSecs(elapsed);
        emit runTimeChanged(_runTime);
    }

    float speed = calculateSpeed(elapsed);
    float distanceTravelled = (speed * elapsed) * 0.001;

    _cyclist.setSpeed(speed);
    _cyclist.setDistance(_cyclist.distance() + distanceTravelled);
    _cyclist.setDistanceTravelled(_cyclist.distanceTravelled() + distanceTravelled);
    _cyclist.setAltitude(_currentRlv.altitudeForDistance(_cyclist.distance()));
    _cyclist.setGeoPosition(_currentRlv.positionForDistance(_cyclist.distance()));


    emit slopeChanged(_currentRlv.slopeForDistance(_cyclist.distance()));
}

void Simulation::rlvSelected(RealLifeVideo rlv)
{
    reset();
    _currentRlv = rlv;
}

void Simulation::courseSelected(int courseNr)
{
    reset();
    if (courseNr == -1) {
        return;
    }
    if (!_currentRlv.isValid())
        return;

    const Course& course = _currentRlv.courses()[courseNr];
    courseSelected(course);
    _cyclist.setDistance(course.start());
}

void Simulation::courseSelected(const Course &course)
{
    reset();
    _cyclist.setDistance(course.start());
    _cyclist.setAltitude(_currentRlv.altitudeForDistance(course.start()));
}

void Simulation::setPower(int power)
{
    _cyclist.setPower(power);
}

void Simulation::setCadence(float cadenceRpm)
{
    _cyclist.setCadence(cadenceRpm);
}

void Simulation::setWheelSpeed(float wheelSpeedMetersPerSecond)
{
    _wheelSpeedMetersPerSecond = wheelSpeedMetersPerSecond;
}

void Simulation::setHeartRate(int heartRate)
{
    _cyclist.setHeartRate(heartRate);
}

float Simulation::calculateSpeed(quint64 timeDelta)
{
    if (_simulationSetting == SimulationSetting::DIRECT_SPEED) {
        return _wheelSpeedMetersPerSecond;
    }

    double power = (1.0 + _powerForElevationCorrection) * _cyclist.power();

    // Calculate speed from power.
    if (power < 1.0f) {
        // if there is no power input and current speed is zero, assume we have no input.
        if (_cyclist.speed() < MINIMUM_SPEED)
            return 0;

        // let the cyclist slow down and stop running after MAX_IDLE_TIME.
        _idleTime = _idleTime.addMSecs(timeDelta);

        if (_idleTime > MAX_IDLE_TIME)
            return 0;
    } else {
        _idleTime = QTime(0,0,0);
    }

    // if speed is very low, use cyclist weight, otherwise force gets very high. Is there
    // a better way to do this?
    const float force = (_cyclist.speed() > (MINIMUM_SPEED - 0.1)) ? power / _cyclist.speed() : _cyclist.totalWeight();

    const float resistantForce = calculateAeroDrag(_cyclist) +
            calculateGravityForce(_cyclist, _currentRlv.slopeForDistance(_cyclist.distance())) +
            calculateGroundResistance(_cyclist);
    const float resultingForce = force - resistantForce;

    const float accelaration = resultingForce / _cyclist.totalWeight();
    const float speedChange = accelaration * timeDelta * 0.001;
    const float speed = _cyclist.speed() + speedChange;

    // If there's power applied, always return at least MINIMUM_SPEED.
    return qMax(MINIMUM_SPEED, speed);
}

void Simulation::reset()
{
    play(false);
    _runTime = QTime(0, 0, 0);
    emit runTimeChanged(_runTime);
    _cyclist.setSpeed(0);
    _cyclist.setDistance(0);
    _cyclist.setDistanceTravelled(0);
}
