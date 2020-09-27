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
#include "sensors.h"

#include "ant/antcentraldispatch.h"
#include "config/bigringsettings.h"
#include <QtCore/QDebug>
#include <QtCore/QtMath>
#include <QtCore/QVariant>

namespace {
const int FIXED_POWER_UPDATE_INTERVAL = 250; // ms
const int SEARCH_RETRY_INTERVAL = 1000; // ms
}
namespace indoorcycling {


Sensors::Sensors(AntCentralDispatch* antCentralDispatch,
                 const NamedSensorConfigurationGroup &sensorConfigurationGroup,
                 QObject *parent) :
    QObject(parent), _antCentralDispatch(antCentralDispatch),
    _sensorConfigurationGroup(sensorConfigurationGroup)
{
    if (_sensorConfigurationGroup.simulationSetting() == SimulationSetting::FIXED_POWER ||
        _sensorConfigurationGroup.simulationSetting() == SimulationSetting::VIRTUAL_POWER) {
        _virtualPowerFunction = indoorcycling::virtualPowerFunctionForTrainer(
                    _sensorConfigurationGroup.trainer());
        _updateTimer = new QTimer(this);
        _updateTimer->setInterval(FIXED_POWER_UPDATE_INTERVAL);
        connect(_updateTimer, &QTimer::timeout, this, &Sensors::sendPowerUpdate);
    }
    connect(_antCentralDispatch, &AntCentralDispatch::sensorFound, this, &Sensors::setSensorFound);
    connect(_antCentralDispatch, &AntCentralDispatch::sensorNotFound, this, &Sensors::setSensorNotFound);
    connect(_antCentralDispatch, &AntCentralDispatch::sensorValue, this, &Sensors::sensorValue);
}

Sensors::~Sensors()
{
    _antCentralDispatch->closeAllChannels();
}

void Sensors::initialize()
{
    if (_antCentralDispatch->antAdapterPresent()) {
        for (SensorConfiguration configuration: _sensorConfigurationGroup.sensorConfigurations()) {
            _antCentralDispatch->searchForSensor(configuration.sensorType(), configuration.deviceNumber());
        }
    }
    if (_sensorConfigurationGroup.simulationSetting() == SimulationSetting::FIXED_POWER) {
        _updateTimer->start();
        QTimer::singleShot(0, this, SLOT(sendPowerUpdate()));
    }
}

void Sensors::setSensorFound(AntSensorType sensorType, int)
{
    switch(sensorType) {
    case AntSensorType::CADENCE:
    case AntSensorType::SPEED_AND_CADENCE:
        _cadenceSensorPresent = true;
        break;
    case AntSensorType::POWER:
        _powerSensorPresent = true;
        break;
    default:
        ; // noop
    }
}

/**
 * @brief If a sensor is not found, we'll retry the search.
 * @param channelType the type of sensor that was not found.
 * @param deviceNumber the device number that was not found.
 */
void Sensors::setSensorNotFound(AntSensorType channelType, int deviceNumber)
{
    qDebug() << "Sensor of type" << ANT_SENSOR_TYPE_STRINGS[channelType] << "with device number" << deviceNumber << "not found. Retrying";
    QTimer::singleShot(SEARCH_RETRY_INTERVAL, this, [this, channelType, deviceNumber]() {
        _antCentralDispatch->searchForSensor(channelType, deviceNumber);
    });
}

void Sensors::sensorValue(const SensorValueType sensorValueType,
                          const AntSensorType sensorType, const QVariant &sensorValue)
{
    switch (sensorValueType) {
    case SensorValueType::HEARTRATE_BPM:
        handleHeartRate(sensorValue);
        break;
    case SensorValueType::CADENCE_RPM:
        handleCadence(sensorValue, sensorType);
        break;
    case SensorValueType::POWER_WATT:
        handlePower(sensorValue, sensorType);
        break;
    case SensorValueType::WHEEL_SPEED_RPM:
        handleWheelSpeed(sensorValue);
        break;
    }
}

void Sensors::sendPowerUpdate()
{
    if (_sensorConfigurationGroup.simulationSetting() == SimulationSetting::FIXED_POWER) {
        const int fixedPower = _sensorConfigurationGroup.fixedPower();
        const int withRandomPower = fixedPower + (qrand() % 40) - 20;
        emit powerWattsMeasured(withRandomPower);
    } else if (_sensorConfigurationGroup.simulationSetting() == SimulationSetting::VIRTUAL_POWER) {
        qDebug() << "no speed/power input, setting power to 0W.";
        _powerWatts = 0;
        emit powerWattsMeasured(_powerWatts);
    }
}

void Sensors::handleHeartRate(const QVariant &sensorValue)
{
    _heartRateBpm = sensorValue.toInt();
    emit heartRateBpmMeasured(_heartRateBpm);
}

/**
 * handle the receival of a cadence value. If we are connected to a cadence or speed/cadence sensor, we will
 * only consider cadence messages from those kind of sensors, as we deem them more reliable then the values from
 * power sensors or smart trainers. If we don't have a cadence sensor, but do have a power sensor, we'll only consider
 * values from the power sensor. If we don't have cadence or power sensors, we'll just use the value from the Smart Trainer.
 * @param sensorValue the value
 * @param sensorType the sensor type used for measuring the value.
 */
void Sensors::handleCadence(const QVariant &sensorValue, const AntSensorType sensorType)
{
    bool useValue;
    if (_cadenceSensorPresent) {
        useValue = (sensorType == AntSensorType::CADENCE || sensorType == AntSensorType::SPEED_AND_CADENCE);
    } else if (_powerSensorPresent) {
        useValue = sensorType == AntSensorType::POWER;
    } else { // this must be a value from a smart trainer.
        useValue = true;
    }
    if (useValue) {
        _cadenceRpm = sensorValue.toFloat();
        emit cadenceRpmMeasured(_cadenceRpm);
    }
}

/**
 * Handle the receival of a power value.
 *
 * If we have a power sensor we will only consider value from power sensors as power sensors are more accurate and
 * reliable then Smart Trainers.
 *
 * @param sensorValue the power value.
 * @param sensorType the sensor type used for measuring the value.
 */
void Sensors::handlePower(const QVariant &sensorValue, const AntSensorType sensorType)
{
    bool useValue;
    if (_powerSensorPresent) {
        useValue = sensorType == AntSensorType::POWER;
    } else {
        useValue = true;
    }

    if (useValue) {
        _powerWatts = sensorValue.toInt();
        emit powerWattsMeasured(_powerWatts);
    }
}

void Sensors::handleWheelSpeed(const QVariant &sensorValue)
{
    _wheelSpeedRpm = sensorValue.toInt();
    if (_sensorConfigurationGroup.simulationSetting() == SimulationSetting::DIRECT_SPEED) {
        float wheelSpeedMps = _wheelSpeedRpm * 2.096 / 60.0;
        emit wheelSpeedMpsMeasured(wheelSpeedMps);
    } else if (_sensorConfigurationGroup.simulationSetting() == SimulationSetting::VIRTUAL_POWER) {
        emit powerWattsMeasured(calculatePower(_wheelSpeedRpm));
        _updateTimer->start();
    }
}

int Sensors::calculatePower(const float wheelSpeedRpm) const
{
    float wheelCircumferenceInM = _sensorConfigurationGroup.wheelCircumferenceInMM() * 0.001;
    float wheelSpeedMps = wheelSpeedRpm * wheelCircumferenceInM / 60.0;
    float virtualPower = _virtualPowerFunction(wheelSpeedMps);
    qDebug() << QString("Virtual Power For %1 meters per second = %2W")
                .arg(wheelSpeedMps, 2, 'f')
                .arg(virtualPower, 2, 'f');
    return static_cast<int>(virtualPower);
}
}
