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

#include <memory>
#include "antchannelhandler.h"
#include "antdevicefinder.h"
#include "antheartratechannelhandler.h"
#include "antmessage2.h"
#include "antmessagegatherer.h"
#include "antpowerchannelhandler.h"
#include "antsmarttrainerchannelhandler.h"
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
    _powerTransmissionChannelHandler(nullptr), _initializationTimer(new QTimer(this)),
    _logFile("ant.log")
{
    connect(_antMessageGatherer, &AntMessageGatherer::antMessageReceived, this,
            &AntCentralDispatch::messageFromAntUsbStick);
    _initializationTimer->setInterval(INITIALIZATION_TIMEOUT);
    _initializationTimer->setSingleShot(true);
    connect(_initializationTimer, &QTimer::timeout, _initializationTimer, [this]() {
        emit initializationFinished(false);
    });

    if (!_logFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning("Unable to open ant log file.");
    }
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
    if (findChannelForSensorType(channelType)) {
        qDebug() << "There's already a channel open for sensor type" << ANT_SENSOR_TYPE_STRINGS[channelType]
                    << ", not opening a new one";
        return false;
    }

    int channelNumber = findFreeChannel();
    if (channelNumber == _antUsbStick->numberOfChannels()) {
        return false;
    }

    // create and insert new channel
    AntChannelHandler* channel = createChannel(channelNumber, channelType);
    if (deviceNumber != 0) {
        channel->setSensorDeviceNumber(deviceNumber);
    }

    if (channelType == AntSensorType::SMART_TRAINER) {
        _smartTrainerChannelHandler = dynamic_cast<AntSmartTrainerChannelHandler*>(channel);
    }

    connect(channel, &AntChannelHandler::sensorFound, this, &AntCentralDispatch::setChannelInfo);
    connect(channel, &AntChannelHandler::sensorValue, this, &AntCentralDispatch::handleSensorValue);
    connect(channel, &AntChannelHandler::antMessageGenerated, this, &AntCentralDispatch::sendAntMessage);
    connect(channel, &AntChannelHandler::searchTimeout, this, &AntCentralDispatch::searchTimedOut);
    connect(channel, &AntChannelHandler::unassigned, this, &AntCentralDispatch::handleChannelUnassigned);

    channel->initialize();
    _channels[channelNumber] = make_qobject_unique(channel);

    emit searchStarted(channelType, channelNumber);
    return true;
}

bool AntCentralDispatch::areAllChannelsClosed() const
{
    return std::all_of(_channels.begin(), _channels.end(), [](const qobject_unique_ptr<AntChannelHandler> &channelPtr) {
        return channelPtr == nullptr;
    });
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
    _channels.resize(_antUsbStick->numberOfChannels());
    _initializationTimer->start();
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
    for(const auto &handler: _channels) {
        if (handler) {
            handler->close();
        }
    }
    _powerTransmissionChannelHandler = nullptr;
    _smartTrainerChannelHandler = nullptr;
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
    _channels[channelNumber] = make_qobject_unique(static_cast<AntChannelHandler*>(channel));
    _masterChannels[sensorType] = channel;

    connect(channel, &AntChannelHandler::antMessageGenerated, this, &AntCentralDispatch::sendAntMessage);
    connect(channel, &AntChannelHandler::unassigned, this, &AntCentralDispatch::handleChannelUnassigned);

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

void AntCentralDispatch::setWeight(const qreal cyclistWeightInKilograms, const qreal bikeWeightInKilograms)
{
    if (_smartTrainerChannelHandler) {
        _smartTrainerChannelHandler->setWeight(cyclistWeightInKilograms, bikeWeightInKilograms);
    }
}

void AntCentralDispatch::setSlope(const qreal slopeInPercent)
{
    if (_smartTrainerChannelHandler) {
        _smartTrainerChannelHandler->setSlope(slopeInPercent);
    }
}

void AntCentralDispatch::messageFromAntUsbStick(const QByteArray &bytes)
{
    std::unique_ptr<AntMessage2> antMessage = AntMessage2::createMessageFromBytes(bytes);

    logAntMessage(AntMessageIO::INPUT, *antMessage);
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

AntChannelHandler *AntCentralDispatch::findChannelForSensorType(const AntSensorType &sensorType)
{
    for(const auto &handler: _channels) {
        if (handler && handler->sensorType() == sensorType) {
            return handler.get();
        }
    }
    return nullptr;
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
    qDebug() << "creating new channel, # =" << channelNumber << "type =" << ANT_SENSOR_TYPE_STRINGS[sensorType];
     switch (sensorType) {
    case AntSensorType::CADENCE:
        return AntSpeedAndCadenceChannelHandler::createCadenceChannelHandler(channelNumber, this);
    case AntSensorType::HEART_RATE:
        return new AntHeartRateChannelHandler(channelNumber, this);
    case AntSensorType::POWER:
        return new AntPowerSlaveChannelHandler(channelNumber, this);
    case AntSensorType::SMART_TRAINER:
        return new AntSmartTrainerChannelHandler(channelNumber, this);
    case AntSensorType::SPEED:
        return AntSpeedAndCadenceChannelHandler::createSpeedChannelHandler(channelNumber, this);
    case AntSensorType::SPEED_AND_CADENCE:
        return AntSpeedAndCadenceChannelHandler::createCombinedSpeedAndCadenceChannelHandler(channelNumber, this);
    default:
        qFatal("Unknown sensor type %d", static_cast<int>(sensorType));
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
    const AntSensorType sensorType = _channels[channelNumber]->sensorType();
    const int deviceNumber = _channels[channelNumber]->sensorDeviceNumber();

    emit sensorNotFound(sensorType, deviceNumber);
}

void AntCentralDispatch::handleSensorValue(const SensorValueType sensorValueType,
                                           const AntSensorType sensorType,
                                           const QVariant &value)
{
    emit sensorValue(sensorValueType, sensorType, value);
}

void AntCentralDispatch::handleChannelUnassigned(int channelNumber)
{
    Q_ASSERT_X(_channels[channelNumber] != nullptr, "AntCentralDispatch::handleChannelUnassigned",
               "getting channel unassigned for empty channel number.");
    const auto handler = std::move(_channels[channelNumber]);
    _channels[channelNumber] = make_qobject_unique<AntChannelHandler>();

    emit channelClosed(channelNumber, handler->sensorType());

    if (areAllChannelsClosed()) {
        emit allChannelsClosed();
    }
}

void AntCentralDispatch::logAntMessage(const AntMessageIO io, const AntMessage2 &message)
{
    if (_logFile.isWritable()) {
        const QString inOrOut = (io == AntMessageIO::INPUT) ? "IN" : "OUT";
        const QString logMessage = QString("%1:\t%2\t%3\n")
                .arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
                .arg(inOrOut)
                .arg(QString(message.toHex()));

        _logFile.write(logMessage.toUtf8());
        _logFile.flush();
    }
}

void AntCentralDispatch::sendAntMessage(const AntMessage2 &message)
{
    Q_ASSERT_X(_antUsbStick.get(), "AntCentralDispatch::sendAntMessage", "usb stick should be present.");
    if (_antUsbStick) {
        if (_logFile.isOpen()) {
            logAntMessage(AntMessageIO::OUTPUT, message);
        } else {
            qDebug() << "Log file is not open";
        }
        qDebug() << "Sending ANT Message:" << message.toString();
        _antUsbStick->writeAntMessage(message);
    }
}

template <class T>
bool AntCentralDispatch::sendToChannel(const T& message, std::function<void(indoorcycling::AntChannelHandler&,const T&)> sendFunction)
{
    quint8 channelNumber = message.channelNumber();
    const auto &channelHandlerPtr = _channels[channelNumber];
    if (channelHandlerPtr) {
        sendFunction(*channelHandlerPtr, message);
        return true;
    } else {
        return false;
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

