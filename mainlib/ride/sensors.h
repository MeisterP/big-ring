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
#ifndef SENSORS_H
#define SENSORS_H

#include <QtCore/QObject>
#include <QtCore/QTimer>

#include "config/sensorconfiguration.h"
#include "model/virtualpower.h"

namespace indoorcycling
{
class AntCentralDispatch;
class Sensors : public QObject
{
    Q_OBJECT
public:
    explicit Sensors(AntCentralDispatch* antCentralDispatch,
                     const NamedSensorConfigurationGroup& sensorConfigurationGroup,
                     QObject *parent = 0);
    virtual ~Sensors();

signals:
    void heartRateBpmMeasured(int heartRate);
    void powerWattsMeasured(int power);
    void cadenceRpmMeasured(float cadence);
    /** wheel speed in m/s */
    void wheelSpeedMpsMeasured(float speed);
public slots:
    void initialize();
private slots:
    void setSensorFound(AntSensorType channelType, int deviceNumber);
    void setSensorNotFound(AntSensorType channelType, int deviceNumber);
    void sensorValue(const SensorValueType sensorValueType, const AntSensorType sensorType,
                     const QVariant& sensorValue);
    void sendPowerUpdate();
private:
    void handleHeartRate(const QVariant& sensorValue);
    void handleCadence(const QVariant& sensorValue, const AntSensorType sensorType);
    void handlePower(const QVariant& sensorValue, const AntSensorType sensorType);
    void handleWheelSpeed(const QVariant& sensorValue);
    int calculatePower(const float wheelSpeedRpm) const;

    AntCentralDispatch* const _antCentralDispatch;
    const NamedSensorConfigurationGroup _sensorConfigurationGroup;

    QTimer* _updateTimer;
    VirtualPowerFunctionType _virtualPowerFunction;
    int _heartRateBpm;
    int _powerWatts;
    float _cadenceRpm;
    float _wheelSpeedRpm;

    bool _cadenceSensorPresent = false;
    bool _powerSensorPresent = false;
};
}
#endif // SENSORS_H
