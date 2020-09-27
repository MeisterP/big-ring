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

#ifndef SIMULATION_H
#define SIMULATION_H

#include <QtCore/QObject>
#include <QtCore/QTime>
#include <QtCore/QTimer>

#include "cyclist.h"
#include "reallifevideo.h"

enum class SimulationSetting {
    FIXED_POWER,
    DIRECT_POWER,
    VIRTUAL_POWER,
    DIRECT_SPEED
};

class Simulation : public QObject
{
    Q_OBJECT
public:
    explicit Simulation(SimulationSetting simulationSetting, Cyclist& cyclist,
                        double powerForElevationCorrection,
                        QObject *parent = 0);
    virtual ~Simulation();

    Cyclist& cyclist() const;
    bool isPlaying() const;
    QTime runTime() const;
signals:
    void slopeChanged(float slope);
    void runTimeChanged(QTime& runTime);

    void playing(bool playing);

public slots:
    void play(bool play);
    void simulationStep();
    void rlvSelected(RealLifeVideo rlv);
    void courseSelected(int courseNr);
    void courseSelected(const Course& course);

    void setPower(int power);
    void setCadence(float cadenceRpm);
    void setWheelSpeed(float wheelSpeedMetersPerSecond);
    void setHeartRate(int heartRate);

private:
    float calculateSpeed(quint64 timeDelta);
    void reset();

    const SimulationSetting _simulationSetting;
    /** in case of indoorcycling::SimulationSetting::DIRECT_SPEED, we store the
     * wheel speed and use it directly, without calculating it from power */
    float _wheelSpeedMetersPerSecond;
    qint64 _lastElapsed;
    QTime _runTime;
    QTime _simulationTime;
    QTime _idleTime;
    Cyclist& _cyclist;
    const double _powerForElevationCorrection;
    RealLifeVideo _currentRlv;
    QTimer _simulationUpdateTimer;
};

#endif // SIMULATION_H
