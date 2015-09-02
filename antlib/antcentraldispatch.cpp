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
#include "antcentraldispatch.h"

#include "antchannelhandler.h"
#include "antdevicefinder.h"
#include "antheartratechannelhandler.h"
#include "antmessage2.h"
#include "antmessagegatherer.h"
#include "antpowerchannelhandler.h"
#include "antspeedandcadencechannelhandler.h"

#include <QtCore/QtDebug>

namespace {
const int INITIALIZATION_TIMEOUT = 1000; // ms

// According to the ANT specification, we should wait at least 500ms after sending
// the System Reset message. To be on the safe side, we'll wait 600ms.
const int ANT_RESET_SYSTEM_TIMEOUT = 600; // ms.

// ANT+ Network
const int ANT_PLUS_NETWORK_NUMBER = 1;
// ANT+ Network Key
const std::array<quint8,8> ANT_PLUS_NETWORK_KEY = { {0xB9, 0xA5, 0x21, 0xFB, 0xBD, 0x72, 0xC3, 0x45} };
}
namespace indoorcycling {

AntCentralDispatch::AntCentralDispatch(QObject *parent) :
    QObject(parent), _initialized(false), _antMessageGatherer(new AntMessageGatherer(this)),
    _powerTransmissionChannelHandler(nullptr), _initializationTimer(new QTimer(this))
{
    connect(_antMessageGatherer, &AntMessageGatherer::antMessageReceived, this,
            &AntCentralDispatch::messageFromAntUsbStick);
    _initializationTimer->setInterval(INITIALIZATION_TIMEOUT);
    _initializationTimer->setSingleShot(true);
    connect(_initializationTimer, &QTimer::timeout, _initializationTimer, [this]() {
        emit initializationFinished(false);
    });
}

bool AntCentralDispatch::antAdapterPresent() const
{
    return _antUsbStick.get();
}

bool AntCentralDispatch::isInitialized() const
{
    return _initialized;
}

bool AntCentralDispatch::searchForSensorType(AntSensorType channelType)
{
    return searchForSensor(channelType, 0);
}

bool AntCentralDispatch::searchForSensor(AntSensorType channelType, int deviceNumber)
{
    Q_ASSERT_X((_antUsbStick), "AntCentralDispatch::searchForSensor", "usb stick not initialized");
    int channelNumber = findFreeChannel();
    if (channelNumber == _antUsbStick->numberOfChannels()) {
        return false;
    }

    // create and insert new channel
    AntChannelHandler* channel = createChannel(channelNumber, channelType);
    if (deviceNumber != 0) {
        channel->setSensorDeviceNumber(deviceNumber);
    }

    connect(channel, &AntChannelHandler::sensorFound, this, &AntCentralDispatch::setChannelInfo);
    connect(channel, &AntChannelHandler::sensorValue, this, &AntCentralDispatch::handleSensorValue);
    connect(channel, &AntChannelHandler::antMessageGenerated, this, &AntCentralDispatch::sendAntMessage);
    connect(channel, &AntChannelHandler::searchTimeout, this, &AntCentralDispatch::searchTimedOut);
    connect(channel, &AntChannelHandler::finished, this, &AntCentralDispatch::handleChannelFinished);

    channel->initialize();
    _channels[channelNumber] = channel;

    emit searchStarted(channelType, channelNumber);
    return true;
}

bool AntCentralDispatch::areAllChannelsClosed() const
{
    auto channelEmpty = [](AntChannelHandler* channelPtr) {
        return channelPtr == nullptr;
    };

    return std::all_of(_channels.begin(), _channels.end(), channelEmpty);
}

void AntCentralDispatch::initialize()
{
    qDebug() << "AntCentralDispatch::initialize()";
    scanForAntUsbStick();
    if (!_antUsbStick) {
        qDebug() << "AntCentralDispatch::initialize() failed";
        emit initializationFinished(false);

        // try it again after a second.
        QTimer::singleShot(1000, this, SLOT(initialize()));
        return;
    }
    _initializationTimer->start();
    _channels.resize(_antUsbStick->numberOfChannels());
    if (_antUsbStick->isReady()) {
        resetAntSystem();
    } else {
        connect(_antUsbStick.get(), &indoorcycling::AntDevice::deviceReady,
                this, &AntCentralDispatch::resetAntSystem);
    }
}

void AntCentralDispatch::closeAllChannels()
{
    qDebug() << "Closing all channels";
    for(AntChannelHandler* handler: _channels) {
        if (handler) {
            handler->close();
        }
    }
    _powerTransmissionChannelHandler = nullptr;
}

bool AntCentralDispatch::openMasterChannel(AntSensorType sensorType)
{
    if (_masterChannels.contains(sensorType)) {
        qDebug() << ANT_SENSOR_TYPE_STRINGS[sensorType] << " master channel already exists";
        return false;
    }
    int channelNumber = findFreeChannel();
    if (channelNumber == _antUsbStick->numberOfChannels()) {
        qDebug() << "All channels occupied";
        return false;
    }
    AntMasterChannelHandler *channel;
    switch (sensorType) {
    case AntSensorType::HEART_RATE:
        channel = new AntHeartRateMasterChannelHandler(channelNumber, this);
        break;
    case AntSensorType::POWER:
        channel = new AntPowerMasterChannelHandler(channelNumber, this);
        break;
    default:
        return false;
    }
    _channels[channelNumber] = channel;
    _masterChannels[sensorType] = channel;

    connect(channel, &AntChannelHandler::antMessageGenerated, this, &AntCentralDispatch::sendAntMessage);
    connect(channel, &AntChannelHandler::finished, this, &AntCentralDispatch::handleChannelFinished);

    channel->initialize();

    return true;

}

bool AntCentralDispatch::sendSensorValue(const SensorValueType sensorValueType, const AntSensorType sensorType, const QVariant &sensorValue)
{
    if (!_masterChannels.contains(sensorType)) {
        return false;
    }
    AntMasterChannelHandler* channel = _masterChannels[sensorType];
    channel->sendSensorValue(sensorValueType, sensorValue);
    return true;
}

void AntCentralDispatch::messageFromAntUsbStick(const QByteArray &bytes)
{
    std::unique_ptr<AntMessage2> antMessage = AntMessage2::createMessageFromBytes(bytes);
    switch(antMessage->id()) {
    case AntMessage2::AntMessageId::CHANNEL_EVENT:
        handleChannelEvent(*antMessage->asChannelEventMessage());
        break;
    case AntMessage2::AntMessageId::BROADCAST_EVENT:
        handleBroadCastMessage(BroadCastMessage(*antMessage));
        break;
    case AntMessage2::AntMessageId::SET_CHANNEL_ID:
        handleChannelIdMessage(SetChannelIdMessage(*antMessage));
        break;
    default:
        qDebug() << "unhandled ANT+ message" << antMessage->toString();
    }
}

void AntCentralDispatch::scanForAntUsbStick()
{
    AntDeviceFinder deviceFinder;
    _antUsbStick = deviceFinder.openAntDevice();
    if (_antUsbStick) {
        connect(_antUsbStick.get(), &indoorcycling::AntDevice::bytesRead, _antMessageGatherer,
                &AntMessageGatherer::submitBytes);
    }
    emit antUsbStickScanningFinished(_antUsbStick.get());

}

int AntCentralDispatch::findFreeChannel()
{
    int channelNumber;
    // search for first non-occupied channel.
    for (channelNumber = 0; channelNumber < _antUsbStick->numberOfChannels(); ++channelNumber) {
        if (!_channels[channelNumber]) {
            break;
        }
    }
    return channelNumber;
}

AntChannelHandler* AntCentralDispatch::createChannel(int channelNumber, AntSensorType &sensorType)
{
    switch (sensorType) {
    case AntSensorType::CADENCE:
        return AntSpeedAndCadenceChannelHandler::createCadenceChannelHandler(channelNumber, this);
    case AntSensorType::HEART_RATE:
        return new AntHeartRateChannelHandler(channelNumber, this);
    case AntSensorType::POWER:
        return new AntPowerSlaveChannelHandler(channelNumber, this);
    case AntSensorType::SPEED:
        return AntSpeedAndCadenceChannelHandler::createSpeedChannelHandler(channelNumber, this);
    case AntSensorType::SPEED_AND_CADENCE:
        return AntSpeedAndCadenceChannelHandler::createCombinedSpeedAndCadenceChannelHandler(channelNumber, this);
    default:
        qFatal("Unknown sensor type %d", sensorType);
        return nullptr;
    }
}

void AntCentralDispatch::resetAntSystem()
{
    sendAntMessage(AntMessage2::systemReset());
    QTimer::singleShot(ANT_RESET_SYSTEM_TIMEOUT, this, SLOT(sendNetworkKey()));
}

void AntCentralDispatch::sendNetworkKey()
{
    sendAntMessage(AntMessage2::setNetworkKey(ANT_PLUS_NETWORK_NUMBER, ANT_PLUS_NETWORK_KEY));
}

void AntCentralDispatch::setChannelInfo(int, AntSensorType sensorType, int sensorDeviceNumber)
{
    emit sensorFound(sensorType, sensorDeviceNumber);
}

void AntCentralDispatch::searchTimedOut(int channelNumber, AntSensorType)
{
    qDebug() << "Search timed out for channel" << channelNumber;
    emit sensorNotFound(_channels[channelNumber]->sensorType());
}

void AntCentralDispatch::handleSensorValue(const SensorValueType sensorValueType,
                                           const AntSensorType sensorType,
                                           const QVariant &value)
{
    emit sensorValue(sensorValueType, sensorType, value);
}

void AntCentralDispatch::handleChannelFinished(int channelNumber)
{
    Q_ASSERT_X(_channels[channelNumber] != nullptr, "AntCentralDispatch::handleChannelFinished",
               "getting channel finished for empty channel number.");
    AntChannelHandler* handler = _channels[channelNumber];
    _channels[channelNumber] = nullptr;

    emit channelClosed(channelNumber, handler->sensorType());
    handler->deleteLater();
    if (areAllChannelsClosed()) {
        emit allChannelsClosed();
    }
}

void AntCentralDispatch::sendAntMessage(const AntMessage2 &message)
{
    Q_ASSERT_X(_antUsbStick.get(), "AntCentralDispatch::sendAntMessage", "usb stick should be present.");
    if (_antUsbStick) {
        qDebug() << "Sending ANT Message:" << message.toString();
        _antUsbStick->writeAntMessage(message);
    }
}

void AntCentralDispatch::handleChannelEvent(const AntChannelEventMessage &channelEventMessage)
{
    if (channelEventMessage.messageId() == AntMessage2::AntMessageId::SET_NETWORK_KEY) {
        _initializationTimer->stop();
        _initialized = (channelEventMessage.messageCode() == AntChannelEventMessage::MessageCode::RESPONSE_NO_ERROR);
        emit initializationFinished(_initialized);
    } else {
        bool sent = sendToChannel<AntChannelEventMessage>(channelEventMessage,
                                                          [](indoorcycling::AntChannelHandler& channel,
                                                          const AntChannelEventMessage& msg) {
            channel.handleChannelEvent(msg);
        });
        if (!sent) {
            qDebug() << "Unhandled channel event" << channelEventMessage.toString();
        }
    }
}

void AntCentralDispatch::handleBroadCastMessage(const BroadCastMessage &broadCastMessage)
{
    bool sent = sendToChannel<BroadCastMessage>(broadCastMessage, [](indoorcycling::AntChannelHandler& channel,
                                                const BroadCastMessage& msg) {
        channel.handleBroadcastEvent(msg);
    });
    if (!sent) {
        qDebug() << "Unhandled broad cast event for channel" << broadCastMessage.channelNumber();
    }
}

void AntCentralDispatch::handleChannelIdMessage(const SetChannelIdMessage &channelIdMessage)
{
    bool sent = sendToChannel<SetChannelIdMessage>(channelIdMessage,
                                                   [](indoorcycling::AntChannelHandler& channel,
                                                   const SetChannelIdMessage& msg) {
        channel.handleChannelIdEvent(msg);
    });
    if (!sent) {
        qDebug() << "Unhandled set channel id event for channel" << channelIdMessage.channelNumber();
    }
}

}

