#include "antspeedandcadencechannelhandler.h"

#include <QtCore/QtDebug>
#include <QtCore/QTime>

namespace indoorcycling
{
SpeedAndCadenceMessage::SpeedAndCadenceMessage(const AntMessage2 &antMessage):
    BroadCastMessage(antMessage)
{
    // empty
}

quint16 SpeedAndCadenceMessage::cadenceEventTime() const
{
    return antMessage().contentShort(1);
}

quint16 SpeedAndCadenceMessage::pedalRevolutions() const
{
    return antMessage().contentShort(3);
}

quint16 SpeedAndCadenceMessage::speedEventTime() const
{
    return antMessage().contentShort(5);
}

quint16 SpeedAndCadenceMessage::wheelRevolutions() const
{
    return antMessage().contentShort(7);
}


SpeedMessage::SpeedMessage(const AntMessage2 &antMessage):
    BroadCastMessage(antMessage)
{
    // empty
}

quint16 SpeedMessage::speedEventTime() const
{
    return antMessage().contentShort(5);
}

quint16 SpeedMessage::wheelRevolutions() const
{
    return antMessage().contentShort(7);
}


CadenceMessage::CadenceMessage(const AntMessage2 &antMessage):
    BroadCastMessage(antMessage)
{
    // empty
}

quint16 CadenceMessage::cadenceEventTime() const
{
    return antMessage().contentShort(5);
}

quint16 CadenceMessage::pedalRevolutions() const
{
    return antMessage().contentShort(7);
}

AntChannelHandler* AntSpeedAndCadenceChannelHandler::createCombinedSpeedAndCadenceChannelHandler(
        int channelNumber, QObject* parent)
{
    return  new AntSpeedAndCadenceChannelHandler(channelNumber, AntSensorType::SPEED_AND_CADENCE,
                                                     AntSportPeriod::SPEED_AND_CADENCE, parent);
}

AntChannelHandler* AntSpeedAndCadenceChannelHandler::createCadenceChannelHandler(int channelNumber, QObject* parent)
{
    return new AntSpeedAndCadenceChannelHandler(channelNumber, AntSensorType::CADENCE, AntSportPeriod::CADENCE, parent);
}

AntChannelHandler* AntSpeedAndCadenceChannelHandler::createSpeedChannelHandler(int channelNumber, QObject* parent)
{
    return new AntSpeedAndCadenceChannelHandler(channelNumber, AntSensorType::SPEED, AntSportPeriod::SPEED, parent);
}

AntSpeedAndCadenceChannelHandler::AntSpeedAndCadenceChannelHandler(int channelNumber, AntSensorType sensorType, AntSportPeriod period, QObject *parent):
    AntChannelHandler(channelNumber, sensorType, period, parent)
{
    // empty
}

void AntSpeedAndCadenceChannelHandler::handleBroadCastMessage(const BroadCastMessage &message)
{
    if (!_previousMessage.isNull()) {
        switch(sensorType()) {
        case AntSensorType::SPEED_AND_CADENCE:
            handleSpeedAndCadenceMessage(SpeedAndCadenceMessage(message.antMessage()));
            break;
        case AntSensorType::CADENCE:
            handleCadenceMessage(CadenceMessage(message.antMessage()));
            break;
        case AntSensorType::SPEED:
            handleSpeedMessage(SpeedMessage(message.antMessage()));
            break;
        default:
            qFatal("Wrong sensor type. Only Speed&Cadence, Cadence or Speed is possible!");
        }
    }
    _previousMessage = message;
}


void AntSpeedAndCadenceChannelHandler::handleSpeedAndCadenceMessage(const SpeedAndCadenceMessage &message)
{
    SpeedAndCadenceMessage previousMessage(_previousMessage.antMessage());
    calculateCadence(previousMessage.cadenceEventTime(), previousMessage.pedalRevolutions(),
                     message.cadenceEventTime(), message.pedalRevolutions());
    calculateSpeed(previousMessage.speedEventTime(), previousMessage.wheelRevolutions(),
                   message.speedEventTime(), message.wheelRevolutions());
}

void AntSpeedAndCadenceChannelHandler::handleCadenceMessage(const CadenceMessage &message)
{
    CadenceMessage previousMessage(_previousMessage.antMessage());
    calculateCadence(previousMessage.cadenceEventTime(), previousMessage.pedalRevolutions(),
                     message.cadenceEventTime(), message.pedalRevolutions());
}

void AntSpeedAndCadenceChannelHandler::handleSpeedMessage(const SpeedMessage &message)
{
    SpeedMessage previousMessage(_previousMessage.antMessage());
    calculateSpeed(previousMessage.speedEventTime(), previousMessage.wheelRevolutions(),
                     message.speedEventTime(), message.wheelRevolutions());
}

void AntSpeedAndCadenceChannelHandler::calculateSpeed(const quint16 previousTime,
                                                          const quint16 previousWheelRevolutions,
                                                          const quint16 currentTime,
                                                          const quint16 currentWheelRevolutions)
{
    quint16 time = currentTime - previousTime;
    quint16 revolutions = currentWheelRevolutions - previousWheelRevolutions;
    if (time) {
        float rpm = 1024*60*revolutions / static_cast<float>(time);
        qDebug() << QTime::currentTime().toString() << "wheel speed" << rpm << "=" << ((rpm / 60) * 2.070) * 3.6 << "km/h";
        emit sensorValue(SensorValueType::WHEEL_SPEED_RPM, sensorType(), QVariant::fromValue(rpm));
    }
}

void AntSpeedAndCadenceChannelHandler::calculateCadence(const quint16 previousTime,
                                                            const quint16 previousPedalRevolutions,
                                                            const quint16 currentTime,
                                                            const quint16 currentPedalRevolutions)
{
    quint16 time = currentTime - previousTime;
    quint16 revolutions = currentPedalRevolutions - previousPedalRevolutions;
    if (time) {
        float cadence = 1024 * 60 * revolutions / static_cast<float>(time);
        qDebug() << QTime::currentTime().toString() << "cadence" << cadence;
        emit sensorValue(SensorValueType::CADENCE_RPM, sensorType(), QVariant::fromValue(cadence));
    }
}
}
