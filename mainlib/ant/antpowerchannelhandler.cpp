#include "antpowerchannelhandler.h"
#include <QtCore/QTime>
#include <QtCore/QtDebug>
#include <QtCore/QVariant>
namespace
{
const quint8 SENDING_POWER_CHANNEL_TRANSMISSION_TYPE = static_cast<quint8>(0x05);
}

namespace indoorcycling
{
PowerMessage::PowerMessage(const AntMessage2 &antMessage):
    BroadCastMessage(antMessage)
{
    // empty
}

AntMessage2 PowerMessage::createPowerMessage(quint8 channel, quint8 eventCount, quint8 cadence,
                                             quint16 accumulatedPower, quint16 instantaneousPower)
{
    QByteArray content;
    content += channel;
    content += static_cast<quint8>(0x10);
    content += eventCount;
    content += 0xFF; // no pedal balance available
    content += cadence;
    content += accumulatedPower & 0xFF;
    content += (accumulatedPower >> 8) & 0xFF;
    content += instantaneousPower & 0xFF;
    content += (instantaneousPower >> 8) & 0xFF;

    return AntMessage2(AntMessage2::AntMessageId::BROADCAST_EVENT, content);
}

bool PowerMessage::isPowerOnlyPage() const
{
    return dataPage() == POWER_ONLY_PAGE;
}

quint8 PowerMessage::eventCount() const
{
    return antMessage().contentByte(2);
}

bool PowerMessage::hasCadence() const
{
    return instantaneousCadence() != 0xFF;
}

quint8 PowerMessage::instantaneousCadence() const
{
    return antMessage().contentByte(4);
}

quint16 PowerMessage::accumulatedPower() const
{
    return antMessage().contentShort(5);
}

quint16 PowerMessage::instantaneousPower() const
{
    return antMessage().contentShort(7);
}

AntPowerSlaveChannelHandler::AntPowerSlaveChannelHandler(int channelNumber, QObject *parent) :
    AntChannelHandler(channelNumber, AntSensorType::POWER, AntSportPeriod::POWER, parent)
{
}

void AntPowerSlaveChannelHandler::handleBroadCastMessage(const BroadCastMessage &message)
{
    PowerMessage powerMessage(message.antMessage());
    if (powerMessage.isPowerOnlyPage()) {
        if (!_lastPowerMessage.isNull()) {
            if (_lastPowerMessage.eventCount() != powerMessage.eventCount()) {
                emit sensorValue(SensorValueType::POWER_WATT, AntSensorType::POWER,
                                 QVariant::fromValue(powerMessage.instantaneousPower()));
                if (powerMessage.hasCadence()) {
                    emit sensorValue(SensorValueType::CADENCE_RPM, AntSensorType::POWER,
                                     QVariant::fromValue(powerMessage.instantaneousCadence()));
                }
            }
        }
        _lastPowerMessage = powerMessage;
    }
}

AntPowerMasterChannelHandler::AntPowerMasterChannelHandler(int channelNumber, QObject *parent):
    AntMasterChannelHandler(channelNumber, AntSensorType::POWER, AntSportPeriod::POWER, parent),
    _updateTimer(new QTimer(this)), _eventCount(0u), _accumulatedPower(0u), _cadence(0u)
{
    setSensorDeviceNumber(1);
    _updateTimer->setTimerType(Qt::PreciseTimer);
    _updateTimer->setInterval(250);
    connect(_updateTimer, &QTimer::timeout, this, &AntPowerMasterChannelHandler::sendPowerUpdate);
}

void AntPowerMasterChannelHandler::setPower(quint16 power)
{
    _eventCount += 1;
    _instantaneousPower = power;
    _accumulatedPower += power;
}

void AntPowerMasterChannelHandler::sendSensorValue(const SensorValueType valueType, const QVariant &value)
{
    if (valueType == SensorValueType::POWER_WATT) {
        _eventCount += 1;
        _instantaneousPower = value.toInt();
        _accumulatedPower += _instantaneousPower;
    } else if (valueType == SensorValueType::CADENCE_RPM) {
        _cadence = value.toInt();
    }
}

quint8 AntPowerMasterChannelHandler::transmissionType() const
{
    return SENDING_POWER_CHANNEL_TRANSMISSION_TYPE;
}

void AntPowerMasterChannelHandler::channelOpened()
{
    qDebug() << "Power transmission channel opened";
    _updateTimer->start();
}

void AntPowerMasterChannelHandler::sendPowerUpdate()
{
    qDebug() << "sending power update" << _eventCount << _instantaneousPower << _accumulatedPower;
    emit antMessageGenerated(PowerMessage::createPowerMessage(channelNumber(), _eventCount, _cadence,
                                                              _accumulatedPower, _instantaneousPower));
}

}
