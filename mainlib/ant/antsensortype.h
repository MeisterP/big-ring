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
#ifndef ANTCHANNELTYPE_H
#define ANTCHANNELTYPE_H

#include <QtCore/QMap>
#include <QtCore/QString>

namespace indoorcycling
{
enum class AntSensorType: quint8 {
    HEART_RATE = 0x78,
    POWER = 0x0B,
    SMART_TRAINER = 0x11,
    SPEED = 0x7B,
    CADENCE = 0x7A,
    SPEED_AND_CADENCE = 0x79
};

inline uint qHash(const AntSensorType antSensorType)
{
    return static_cast<uint>(antSensorType);
}

const QMap<AntSensorType,QString> ANT_SENSOR_TYPE_STRINGS(
        {
            {AntSensorType::CADENCE, "Cadence"},
            {AntSensorType::HEART_RATE, "HeartRate"},
            {AntSensorType::POWER, "Power"},
            {AntSensorType::SMART_TRAINER, "ANT+ Smart Trainer"},
            {AntSensorType::SPEED, "Speed"},
            {AntSensorType::SPEED_AND_CADENCE,"Speed And Cadence"}
        });

enum class SensorValueType {
    HEARTRATE_BPM,
    POWER_WATT,
    CADENCE_RPM,
    WHEEL_SPEED_RPM
};


const QMap<SensorValueType,QString> SENSOR_VALUE_TYPE_STRINGS(
        {
            {SensorValueType::HEARTRATE_BPM, "BPM"},
            {SensorValueType::POWER_WATT, "W"},
            {SensorValueType::CADENCE_RPM, "RPM"},
            {SensorValueType::WHEEL_SPEED_RPM, "RPM"},
        });
}
#endif // ANTCHANNELTYPE_H
