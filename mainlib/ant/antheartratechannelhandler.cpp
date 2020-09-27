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
#include "antheartratechannelhandler.h"
#include <QtCore/QtDebug>
#include <QtCore/QTime>
#include <QtCore/QVariant>

namespace
{
const quint8 HEARTRATE_MASTER_CHANNEL_TRANSMISSION_TYPE = static_cast<quint8>(0x01);
}
namespace indoorcycling
{
HeartRateMessage::HeartRateMessage(const AntMessage2 &antMessage): BroadCastMessage(antMessage)
{
    if (!isNull()) {
        _measurementTime = antMessage.contentShort(5);
        _heartBeatCount = antMessage.contentByte(7);
        _computedHeartRate = antMessage.contentByte(8);
    }
}

AntMessage2 HeartRateMessage::createHeartRateMessage(quint8 channelNumber, bool toggleHigh, quint16 measurementTime,
                                                     quint8 heartBeatCount, quint8 computedHeartRate)
{
    QByteArray content;
    content += channelNumber;
    if (toggleHigh) {
        content += 0x80;
    } else {
        quint8 zero = 0x0;
        content += zero;
    }
    content += 0xFF;
    content += 0xFF;
    content += 0xFF;
    content += measurementTime & 0xFF;
    content += (measurementTime >> 8) & 0xFF;
    content += heartBeatCount;
    content += computedHeartRate;

    return AntMessage2(AntMessage2::AntMessageId::BROADCAST_EVENT, content);
}

quint16 HeartRateMessage::measurementTime() const
{
    return _measurementTime;
}

quint8 HeartRateMessage::heartBeatCount() const
{
    return _heartBeatCount;
}

quint8 HeartRateMessage::computedHeartRate() const
{
    return _computedHeartRate;
}


AntHeartRateChannelHandler::AntHeartRateChannelHandler(int channelNumber, QObject *parent):
    AntChannelHandler(channelNumber, AntSensorType::HEART_RATE, AntSportPeriod::HR, parent)
{
    // empty
}

void AntHeartRateChannelHandler::handleBroadCastMessage(const BroadCastMessage &message)
{
    HeartRateMessage heartRateMessage(message.antMessage());
    if (_lastMessage.isNull() || heartRateMessage.measurementTime() != _lastMessage.measurementTime()) {
        qDebug() << QTime::currentTime().toString() << "HR:" << heartRateMessage.computedHeartRate();
        emit sensorValue(SensorValueType::HEARTRATE_BPM, sensorType(),
                         QVariant::fromValue(heartRateMessage.computedHeartRate()));
    }
    _lastMessage = heartRateMessage;
}

AntHeartRateMasterChannelHandler::AntHeartRateMasterChannelHandler(int channelNumber, QObject *parent):
    AntMasterChannelHandler(channelNumber, AntSensorType::HEART_RATE, AntSportPeriod::HR, parent),
    _updateTimer(new QTimer(this)), _updateCounter(0), _heartRate(0), _lastEventTime(0), _lastNrOfHeartBeats(0)
{
    _updateTimer->setInterval(250);
    connect(_updateTimer, &QTimer::timeout, this, &AntHeartRateMasterChannelHandler::sendMessage);
}

void AntHeartRateMasterChannelHandler::sendSensorValue(const SensorValueType valueType, const QVariant &value)
{
    if (valueType == SensorValueType::HEARTRATE_BPM) {
        _heartRate = value.toInt();
    }
}

quint8 AntHeartRateMasterChannelHandler::transmissionType() const
{
    return HEARTRATE_MASTER_CHANNEL_TRANSMISSION_TYPE;
}

void AntHeartRateMasterChannelHandler::channelOpened()
{
    _updateTimer->start();
}

void AntHeartRateMasterChannelHandler::sendMessage()
{
    int elapsed = _lastSendTime.restart();
    quint16 elapsedIn1024 = static_cast<quint16>(qRound(elapsed * 1.024));
    _lastEventTime += elapsedIn1024;
    _lastNrOfHeartBeats += (_heartRate / 4);

    bool toggle = ((_updateCounter / 4) > 0);
    _updateCounter = (_updateCounter + 1) % 8;
    AntMessage2 hrMessage = HeartRateMessage::createHeartRateMessage(channelNumber(), toggle, _lastEventTime, _lastNrOfHeartBeats, _heartRate);
    emit antMessageGenerated(hrMessage);
}

}
