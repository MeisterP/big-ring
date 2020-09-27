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
#include "sensorconfiguration.h"

#include <QtCore/QSettings>

namespace indoorcycling {


SensorConfiguration::SensorConfiguration()
{
    // empty
}

SensorConfiguration::SensorConfiguration(AntSensorType sensorType, int deviceNumber):
    _sensorType(sensorType), _deviceNumber(deviceNumber)
{
    // empty
}

AntSensorType SensorConfiguration::sensorType() const
{
    return _sensorType;
}

int SensorConfiguration::deviceNumber() const
{
    return _deviceNumber;
}

NamedSensorConfigurationGroup::NamedSensorConfigurationGroup()
{
    // empty
}

NamedSensorConfigurationGroup::NamedSensorConfigurationGroup(
        const QString &name,
        const QMap<AntSensorType, SensorConfiguration> &sensorConfigurations,
        SimulationSetting simulationSetting):
    _name(name), _sensorConfigurations(sensorConfigurations), _simulationSetting(simulationSetting)
{
    // empty
}

const QString &NamedSensorConfigurationGroup::name() const
{
    return _name;
}

const QMap<AntSensorType,SensorConfiguration>& NamedSensorConfigurationGroup::sensorConfigurations() const
{
    return _sensorConfigurations;
}

SimulationSetting NamedSensorConfigurationGroup::simulationSetting() const
{
    return _simulationSetting;
}

int NamedSensorConfigurationGroup::fixedPower() const
{
    return _fixedPower;
}

void NamedSensorConfigurationGroup::setFixedPower(int watts)
{
    _fixedPower = watts;
}

VirtualPowerTrainer NamedSensorConfigurationGroup::trainer() const
{
    return _trainer;
}

void NamedSensorConfigurationGroup::setTrainer(VirtualPowerTrainer trainer)
{
    _trainer = trainer;
}

int NamedSensorConfigurationGroup::wheelCircumferenceInMM() const
{
    return _wheelCircumferenceInMM;;
}

void NamedSensorConfigurationGroup::setWheelCircumferenceInMM(int mm)
{
    _wheelCircumferenceInMM = mm;
}

const NamedSensorConfigurationGroup NamedSensorConfigurationGroup::selectedConfigurationGroup()
{
    QSettings settings;
    settings.beginGroup("Sensor_Configuration");
    QString name = settings.value("selectedConfiguration").toString();

    return readFromSettings().value(name);
}

void NamedSensorConfigurationGroup::saveSelectedConfigurationGroup(const QString &name)
{
    QSettings settings;
    settings.beginGroup("Sensor_Configuration");
    return settings.setValue("selectedConfiguration", QVariant::fromValue(name));
}

void NamedSensorConfigurationGroup::addNamedSensorConfigurationGroup(NamedSensorConfigurationGroup &group)
{
    QSettings settings;
    settings.beginGroup("Sensor_Configuration");
    settings.beginGroup("configurations");
    settings.beginGroup(group.name());
    settings.beginWriteArray("sensors", group.sensorConfigurations().size());
    int i = 0;
    for (const auto& configuration: group.sensorConfigurations()) {
        settings.setArrayIndex(i++);
        settings.setValue("sensorType",
                          QVariant::fromValue(static_cast<int>(configuration.sensorType())));
        settings.setValue("deviceNumber", configuration.deviceNumber());
    }
    settings.endArray();
    settings.setValue("simulationSetting", QVariant::fromValue(
                          static_cast<int>(group.simulationSetting())));
    if (group.simulationSetting() == SimulationSetting::FIXED_POWER) {
        settings.setValue("fixedPower", QVariant::fromValue(group.fixedPower()));
    } else if (group.simulationSetting() == SimulationSetting::VIRTUAL_POWER) {
        settings.setValue("trainer", QVariant::fromValue(static_cast<int>(group.trainer())));
        settings.setValue("wheelCircumference", QVariant::fromValue(static_cast<int>(group.wheelCircumferenceInMM())));
    } else if (group.simulationSetting() == SimulationSetting::DIRECT_SPEED) {
        settings.setValue("wheelCircumference", QVariant::fromValue(static_cast<int>(group.wheelCircumferenceInMM())));
    }
}

void NamedSensorConfigurationGroup::removeConfigurationGroup(const QString &name)
{
    QSettings settings;
    settings.beginGroup("Sensor_Configuration");
    settings.beginGroup("configurations");
    settings.remove(name);
}

QMap<QString, NamedSensorConfigurationGroup> NamedSensorConfigurationGroup::readFromSettings()
{
    QSettings settings;
    settings.beginGroup("Sensor_Configuration");
    settings.beginGroup("configurations");
    const QStringList configurationNames = settings.childGroups();

    QMap<QString,NamedSensorConfigurationGroup> namedConfigurationGroups;
    for (QString configurationName: configurationNames) {
        namedConfigurationGroups[configurationName] = readSingleGroupFromSettings(settings, configurationName);;;
    }
    settings.endGroup();

    return namedConfigurationGroups;
}

NamedSensorConfigurationGroup NamedSensorConfigurationGroup::readSingleGroupFromSettings(QSettings& settings,
                                                                                         const QString& configurationName)
{
    QMap<AntSensorType,SensorConfiguration> sensorConfigurations;
    settings.beginGroup(configurationName);
    int size = settings.beginReadArray("sensors");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        AntSensorType sensorType =
                static_cast<AntSensorType>(settings.value("sensorType").toInt());
        int deviceNumber = settings.value("deviceNumber").toInt();
        sensorConfigurations.insert(sensorType, SensorConfiguration(sensorType, deviceNumber));
    }
    settings.endArray();

    SimulationSetting simulationSetting =
            static_cast<SimulationSetting>(settings.value("simulationSetting").toInt());
    NamedSensorConfigurationGroup group(configurationName, sensorConfigurations,
                                        simulationSetting);
    if (simulationSetting == SimulationSetting::FIXED_POWER) {
        group.setFixedPower(settings.value("fixedPower").toInt());
    } else if (simulationSetting == SimulationSetting::VIRTUAL_POWER) {
        group.setTrainer(static_cast<VirtualPowerTrainer>(
                             settings.value("trainer").toInt()));
        group.setWheelCircumferenceInMM(settings.value("wheelCircumference").toInt());
    } else if (simulationSetting == SimulationSetting::DIRECT_SPEED) {
        group.setWheelCircumferenceInMM(settings.value("wheelCircumference").toInt());
    }
    settings.endGroup();

    return group;
}
}
