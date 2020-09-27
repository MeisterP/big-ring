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
#include "antchannelhandler.h"

#include <QtCore/QMap>
#include <QtCore/QtDebug>
#include <QtCore/QTimer>

namespace {
const int SEARCH_TIMEOUT = 10; //seconds.
const int ACKNOWLEDGED_MESSAGE_QUEUE_SIZE = 4; // 1 acknowledged messages.
const int ACKNOWLEDGED_MESSAGE_TIMEOUT = 1000; // milliseconds

using indoorcycling::AntChannelHandler;
const QMap<AntChannelHandler::ChannelState,QString> CHANNEL_STATE_STRINGS(
{
            {AntChannelHandler::ChannelState::CLOSED, "CHANNEL_CLOSED"},
            {AntChannelHandler::ChannelState::ASSIGNED, "CHANNEL_ASSIGNED"},
            {AntChannelHandler::ChannelState::ID_SET, "CHANNEL_ID_SET"},
            {AntChannelHandler::ChannelState::FREQUENCY_SET, "CHANNEL_FREQUENCY_SET"},
            {AntChannelHandler::ChannelState::PERIOD_SET, "CHANNEL_PERIOD_SET"},
            {AntChannelHandler::ChannelState::TIMEOUT_SET, "CHANNEL_SEARCH_TIMEOUT_SET"},
            {AntChannelHandler::ChannelState::OPENED, "CHANNEL_OPENED"},
            {AntChannelHandler::ChannelState::SEARCHING, "CHANNEL_SEARCHING"},
            {AntChannelHandler::ChannelState::TRACKING, "CHANNEL_TRACKING"},
            {AntChannelHandler::ChannelState::UNASSIGNED, "CHANNEL_UNASSIGNED"}
});
}
namespace indoorcycling {
AntChannelHandler::AntChannelHandler(const int channelNumber, const AntSensorType sensorType,
                                     AntSportPeriod channelPeriod, QObject *parent) :
    QObject(parent), _channelNumber(channelNumber), _deviceNumber(0), _sensorType(sensorType),
    _channelPeriod(channelPeriod),_state(ChannelState::CLOSED)
{
    _acknowledgedMessageTimer.setSingleShot(true);
    _acknowledgedMessageTimer.setInterval(ACKNOWLEDGED_MESSAGE_TIMEOUT);
    connect(&_acknowledgedMessageTimer, &QTimer::timeout, this, &AntChannelHandler::transferTxFailed);
}

quint8 AntChannelHandler::channelNumber() const
{
    return _channelNumber;
}

bool AntChannelHandler::isMasterNode() const
{
    return false;
}

quint8 AntChannelHandler::transmissionType() const
{
    return static_cast<quint8>(0u);
}

void AntChannelHandler::channelOpened()
{
    // empty.
}

QString AntChannelHandler::channelIdString() const
{
    return QString("Channel #%1, device type %2, device #%3:")
            .arg(_channelNumber).arg(static_cast<int>(_sensorType)).arg(_deviceNumber);
}

AntChannelHandler::~AntChannelHandler()
{
    qDebug() << "Deleting AntChannelHandler for channel type" << ANT_SENSOR_TYPE_STRINGS[_sensorType] << static_cast<int>(_sensorType);
}

AntSensorType AntChannelHandler::sensorType() const
{
    return _sensorType;
}

int AntChannelHandler::sensorDeviceNumber() const
{
    return _deviceNumber;
}

void AntChannelHandler::setSensorDeviceNumber(int deviceNumber)
{
    _deviceNumber = deviceNumber;
}

void AntChannelHandler::initialize()
{
    quint8 channelType = (isMasterNode()) ? 0x10 : 0x0;
    emit antMessageGenerated(AntMessage2::assignChannel(_channelNumber, channelType));
    setState(ChannelState::ASSIGNED);
}

void AntChannelHandler::close()
{
    qDebug() << QString("Closing channel #%1").arg(_channelNumber);
    setState(ChannelState::CLOSED);
    while (!_acknowledgedMessagesToSend.empty()) {
        _acknowledgedMessagesToSend.pop();
    }
    emit antMessageGenerated(AntMessage2::closeChannel(_channelNumber));
}

void AntChannelHandler::handleChannelEvent(const AntChannelEventMessage &message)
{
    switch (message.messageCode()) {
    case AntChannelEventMessage::MessageCode::RESPONSE_NO_ERROR:
        advanceState(message.messageId());
        break;
    case AntChannelEventMessage::MessageCode::EVENT_RX_SEARCH_TIMEOUT:
        setState(ChannelState::SEARCH_TIMEOUT);
        emit searchTimeout(_channelNumber, _sensorType);
        emit antMessageGenerated(AntMessage2::unassignChannel(_channelNumber));
        setState(ChannelState::UNASSIGNED);
        break;
    case AntChannelEventMessage::MessageCode::EVENT_RX_FAILED:
        qDebug() << "RX Failure on channel" << _channelNumber;
        break;
    case AntChannelEventMessage::MessageCode::EVENT_RX_FAIL_GO_TO_SEARCH:
        qDebug() << _channelNumber << "Too many messages missed, going to search mode";
        setState(ChannelState::SEARCHING);
        break;
    case AntChannelEventMessage::MessageCode::EVENT_CHANNEL_CLOSED:
        qDebug() << "Channel closed by ANT+ stick.";
        if (_state != ChannelState::UNASSIGNED) {
            emit antMessageGenerated(AntMessage2::unassignChannel(_channelNumber));
            setState(ChannelState::UNASSIGNED);
        }
        break;
    case AntChannelEventMessage::MessageCode::EVENT_TX:
        // message has been sent.
        break;
    case AntChannelEventMessage::MessageCode::EVENT_TRANSFER_TX_COMPLETED:
        transferTxCompleted();
        break;
    case AntChannelEventMessage::MessageCode::EVENT_TRANSFER_TX_FAILED:
        transferTxFailed();
        break;
    default:
        qDebug() << "unhandled message" << message.toString();
    }
}

void AntChannelHandler::handleBroadcastEvent(const BroadCastMessage &broadcastMessage)
{
    // we'll check if there are any acknowledged message to send. If so we'll send one now.
    sendNextAcknowledgedMessage();
    if (_state == ChannelState::SEARCHING) {
        handleFirstBroadCastMessage(broadcastMessage);
    } else if (_state == ChannelState::TRACKING) {
        handleBroadCastMessage(broadcastMessage);
    } else {
        qDebug() << "channel" << _channelNumber << "Did not expect a broad cast message in state"
                 << CHANNEL_STATE_STRINGS[_state]
                    << QString("data page = %1").arg(QString::number(broadcastMessage.dataPage()));
    }
}

void AntChannelHandler::handleChannelIdEvent(const SetChannelIdMessage &channelIdMessage)
{
    _deviceNumber = channelIdMessage.deviceNumber();
    setState(ChannelState::TRACKING);
    emit sensorFound(_channelNumber, _sensorType, _deviceNumber);
    emit antMessageGenerated(AntMessage2::setInfiniteSearchTimeout(_channelNumber));
}

void AntChannelHandler::setState(AntChannelHandler::ChannelState state)
{
    _state = state;
    emit stateChanged(_state);
}

void AntChannelHandler::assertMessageId(const AntMessage2::AntMessageId expected, const AntMessage2::AntMessageId actual)
{
    Q_ASSERT_X(expected == actual, "AntChannelHandler::assertMessageId",
               qPrintable(QString("expected message = %1, but was %2")
                          .arg(antMessageIdToString(expected))
                          .arg(antMessageIdToString(actual))));
    if (expected != actual) {
        QString message = QString("%1 Expected messageid in state %2 was 0x%3, but it was 0x%4.")
                .arg(channelIdString())
                .arg(CHANNEL_STATE_STRINGS[_state]).arg(antMessageIdToString(expected))
                .arg(antMessageIdToString(actual));
        qWarning("%s", qPrintable(message));
    }
}


void AntChannelHandler::queueAcknowledgedMessage(const AntMessage2 &message)
{
    if (_acknowledgedMessagesToSend.size() > ACKNOWLEDGED_MESSAGE_QUEUE_SIZE) {
        qDebug() << channelIdString() << "Queueing Acknowledged message, queue is full. Removing oldest message from queue";
        _acknowledgedMessagesToSend.pop();
    }
    _acknowledgedMessagesToSend.push(message);
    if (_acknowledgedMessagesToSend.size() > 1) {
        qDebug() << channelIdString() << "Queued Acknowledged message, queue size" << _acknowledgedMessagesToSend.size();
    }
}

/**
 * Send thee next Acknowledged message, if there's any to send. This should only be called after a broadcastmessage
 * has been received. When a message is sent, _acknowledgedMessageTimer is started to be able to deal with the fact
 * that no acknowledgement (SUCCESS or FAILURE) will be received.
 */
void AntChannelHandler::sendNextAcknowledgedMessage()
{
    if (!_acknowledgedMessageInFlight && !_acknowledgedMessagesToSend.empty()) {
        qDebug() << channelIdString() << "sending acknowledged message";
        _acknowledgedMessageInFlight = true;
        _acknowledgedMessageTimer.start();
        emit antMessageGenerated(_acknowledgedMessagesToSend.front());
    }
}

/**
 * An acknowledged message transfer has been completed. Remove the message from the queue. If there are any more
 * messages on the queue, they will be sent after the next broadcast message has been received.
 */
void AntChannelHandler::transferTxCompleted()
{
    _acknowledgedMessageTimer.stop();
    _acknowledgedMessageInFlight = false;
    if (!_acknowledgedMessagesToSend.empty()) {
        _acknowledgedMessagesToSend.pop();
        qDebug() << "dequeued Acknowledged message, queue size" << _acknowledgedMessagesToSend.size();
    }
}

/**
 * An acknowledged message transfer has failed. The last acknowledged message will be sent
 * again.
 */
void AntChannelHandler::transferTxFailed()
{
    _acknowledgedMessageTimer.stop();
    qDebug() << "An acknowledged message failed. Trying it again";
    _acknowledgedMessageInFlight = false;
    sendNextAcknowledgedMessage();
}

void AntChannelHandler::advanceState(const AntMessage2::AntMessageId messageId)
{
    switch (_state) {
    case ChannelState::ASSIGNED:
        assertMessageId(AntMessage2::AntMessageId::ASSIGN_CHANNEL, messageId);
        emit antMessageGenerated(AntMessage2::setChannelId(_channelNumber,
                                                           _deviceNumber, _sensorType, transmissionType()));
        setState(ChannelState::ID_SET);
        break;
    case ChannelState::ID_SET:
        assertMessageId(AntMessage2::AntMessageId::SET_CHANNEL_ID, messageId);
        emit antMessageGenerated(AntMessage2::setChannelFrequency(_channelNumber));
        setState(ChannelState::FREQUENCY_SET);
        break;
    case ChannelState::FREQUENCY_SET:
        assertMessageId(AntMessage2::AntMessageId::SET_CHANNEL_FREQUENCY, messageId);
        emit antMessageGenerated(AntMessage2::setChannelPeriod(_channelNumber, _channelPeriod));
        setState(ChannelState::PERIOD_SET);
        break;
    case ChannelState::PERIOD_SET:
        assertMessageId(AntMessage2::AntMessageId::SET_CHANNEL_PERIOD, messageId);
        if (isMasterNode()) {
            emit antMessageGenerated(AntMessage2::openChannel(_channelNumber));
            setState(ChannelState::OPENED);
        } else {
            emit antMessageGenerated(AntMessage2::setSearchTimeout(_channelNumber, SEARCH_TIMEOUT));
            setState(ChannelState::TIMEOUT_SET);
        }
        break;
    case ChannelState::TIMEOUT_SET:
        assertMessageId(AntMessage2::AntMessageId::SET_SEARCH_TIMEOUT, messageId);
        emit antMessageGenerated(AntMessage2::openChannel(_channelNumber));
        setState(ChannelState::OPENED);
        break;
    case ChannelState::OPENED:
        assertMessageId(AntMessage2::AntMessageId::OPEN_CHANNEL, messageId);
        setState(ChannelState::SEARCHING);
        channelOpened();
        break;
    case ChannelState::CLOSED:
        assertMessageId(AntMessage2::AntMessageId::CLOSE_CHANNEL, messageId);
        // we have to wait for a "channel closed message from the ant+ adapter
        break;
    case ChannelState::UNASSIGNED:
        assertMessageId(AntMessage2::AntMessageId::UNASSIGN_CHANNEL, messageId);
        emit unassigned(_channelNumber);
        qDebug() << channelIdString() << "Channel unassigned. Can be deleted";
        break;
    case ChannelState::TRACKING:
        assertMessageId(AntMessage2::AntMessageId::SET_SEARCH_TIMEOUT, messageId);
        qDebug() << channelIdString() << "search timeout set after acquiring sensor.";
        break;
    default:
        qDebug() << channelIdString() << "Unhandled state" << CHANNEL_STATE_STRINGS[_state];
    }
}

/**
 * When the first broad cast message from a sensor is received, we'll ask
 * the sensor for it's id.
 */
void AntChannelHandler::handleFirstBroadCastMessage(const BroadCastMessage&)
{
    qDebug() << channelIdString() <<
                QString("channel %1: First broadcast message received, requesting sensor id")
                .arg(_channelNumber);
    emit antMessageGenerated(AntMessage2::requestMessage(_channelNumber,
                                                         AntMessage2::AntMessageId::SET_CHANNEL_ID));
    setState(ChannelState::TRACKING);
}

AntMasterChannelHandler::AntMasterChannelHandler(int channelNumber, const AntSensorType sensorType,
                                                 AntSportPeriod channelPeriod, QObject *parent):
    AntChannelHandler(channelNumber, sensorType, channelPeriod, parent)
{
    // empty
}

void AntMasterChannelHandler::handleBroadCastMessage(const BroadCastMessage &)
{
    qDebug() << channelIdString() << QString("received broadcast message on master channel #%1").arg(channelNumber());
}

}
