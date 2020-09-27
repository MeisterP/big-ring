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
#include "antmessage2.h"
#include <QtCore/QtDebug>

namespace {
const double MESSAGING_PERIOD_BASE = 32768.0;

const QMap<AntChannelEventMessage::MessageCode,QString> EVENT_CHANNEL_MESSAGES (
{
            {AntChannelEventMessage::MessageCode::RESPONSE_NO_ERROR, "NO_ERROR"},
            {AntChannelEventMessage::MessageCode::EVENT_CHANNEL_CLOSED, "CHANNEL CLOSED"},
            {AntChannelEventMessage::MessageCode::EVENT_CHANNEL_COLLISION, "CHANNEL COLLISION"},
            {AntChannelEventMessage::MessageCode::CHANNEL_IN_WRONG_STATE, "WRONG_STATE"},
            {AntChannelEventMessage::MessageCode::EVENT_RX_FAILED, "RX_FAIL"},
            {AntChannelEventMessage::MessageCode::EVENT_RX_SEARCH_TIMEOUT, "SEARCH_TIMEOUT"}
});
}


AntMessage2::AntMessage2(const QByteArray &completeMessageBytes):
    AntMessage2(static_cast<AntMessageId>(completeMessageBytes[2]),
    completeMessageBytes.mid(3, completeMessageBytes[1]))
{
    // empty
}

AntMessage2::AntMessage2(const AntMessageId id, const QByteArray& content):
    _id(id), _content(content)
{
    // empty
}

const QByteArray &AntMessage2::content() const
{
    return _content;
}

quint8 AntMessage2::computeChecksum(const QByteArray &bytes) const
{
    quint8 checksum = 0;
    for (int i = 0; i < bytes.length(); ++i) {
        checksum ^= bytes[i];
    }
    return checksum;
}

quint8 AntMessage2::contentByte(int nr) const
{
    Q_ASSERT_X(!isNull(), "AntMessage2::contentByte", "requesting content for a NULL message.");
    return static_cast<quint8>(_content[nr]);
}

quint16 AntMessage2::contentShort(int index) const
{
    Q_ASSERT_X(!isNull(), "AntMessage2::contentShort", "requesting content for a NULL message.");
    quint8 lowByte = _content[index];
    quint8 highByte = _content[index + 1];

    return ((highByte << 8) & 0xFF00) + lowByte;
}

AntMessage2::AntMessage2():
    AntMessage2(AntMessageId::INVALID, QByteArray())
{

}

bool AntMessage2::isNull() const
{
    return _id == AntMessageId::INVALID;
}

QByteArray AntMessage2::toBytes() const
{
    QByteArray bytes;
    bytes.append(SYNC_BYTE);
    bytes.append(static_cast<quint8>(_content.length()));
    bytes.append(static_cast<quint8>(_id));
    bytes.append(_content);
    bytes.append(computeChecksum(bytes));

    return bytes;
}

QByteArray AntMessage2::toHex() const
{
    return toBytes().toHex();
}

QString AntMessage2::toString() const
{
    switch(_id) {
    case AntMessageId::ASSIGN_CHANNEL:
        return QString("Assign Channel, Channel %1, Channel Type %2, Network Number %3").arg(contentByte(0)).arg(contentByte(1)).arg(contentByte(2));
    case AntMessageId::BROADCAST_EVENT:
        return QString("Broadcast event, Channel %1").arg(contentByte(0));
    case AntMessageId::CLOSE_CHANNEL:
        return QString("Close Channel (0x4c), Channel #%1").arg(contentByte(0));
    case AntMessageId::OPEN_CHANNEL:
        return QString("Open Channel (0x4b), Channel %1").arg(contentByte(0));
    case AntMessageId::REQUEST_MESSAGE:
        return QString("Request Message, Channel %1, Message Id %2").arg(contentByte(0)).arg(QString::number(contentByte(1), 16));
    case AntMessageId::SET_CHANNEL_FREQUENCY:
        return QString("Set Channel Frequency, Channel %1, Frequency %2Mhz").arg(contentByte(0)).arg(contentByte(1) + ANT_CHANNEL_FREQUENCY_BASE);
    case AntMessageId::SET_CHANNEL_ID:
        return QString("Set Channel Id, Channel %1, Device Nr %2, "
                       "Device Type %3, Transmission Type %4").arg(contentByte(0)).arg(contentShort(1))
                .arg(contentByte(3)).arg(contentByte(4));
    case AntMessageId::SET_CHANNEL_PERIOD:
        return QString("Set Channel Period, Channel %1, Period %2Hz (%3)").arg(contentByte(0))
                .arg(QString::number(MESSAGING_PERIOD_BASE / contentShort(1), 'f', 2)).arg(contentShort(1));
    case AntMessageId::SET_NETWORK_KEY:
        return QString("Set Network Key, Network %1, Key: %2").arg(contentByte(0)).arg(QString(_content.mid(1).toHex()));
    case AntMessageId::SET_SEARCH_TIMEOUT:
    {
        QString timeoutString = (contentByte(1) == 0xFF) ? "INFINITE":
                                                           QString("%1s").arg(QString::number(qRound(contentByte(1) * 2.5)));
        return QString("Set Search Timeout, Channel %1, Timeout %2").arg(contentByte(0)).arg(timeoutString);
    }
    case AntMessageId::SYSTEM_RESET:
        return "Sytem Reset";
    case AntMessageId::UNASSIGN_CHANNEL:
        return QString("Unassign Channel, Channel #%1").arg(contentByte(0));
    default:
        return QString("Unknown message %1").arg(QString::number(static_cast<quint8>(_id), 16));
    }
}

AntMessage2::AntMessageId AntMessage2::id() const
{
    return _id;
}

AntMessage2 AntMessage2::systemReset()
{
    static const char data[] = { 0x00, 0x00 };
    return AntMessage2(AntMessageId::SYSTEM_RESET, QByteArray::fromRawData(data, 1));
}

AntMessage2 AntMessage2::setNetworkKey(quint8 networkNumber, const std::array<quint8, 8>& networkKey)
{
    QByteArray content;
    content += networkNumber;
    for(const quint8 byte: networkKey) {
        content += byte;
    }
    return AntMessage2(AntMessageId::SET_NETWORK_KEY, content);
}

AntMessage2 AntMessage2::setSearchTimeout(quint8 channelNumber, int seconds)
{
    QByteArray content;
    content += channelNumber;
    quint8 timeout = static_cast<quint8>(qRound(seconds / 2.5));
    content += timeout;
    return AntMessage2(AntMessageId::SET_SEARCH_TIMEOUT, content);
}

AntMessage2 AntMessage2::setInfiniteSearchTimeout(quint8 channelNumber)
{
    QByteArray content;
    content += channelNumber;
    quint8 timeout = 0xFF;
    content += timeout;
    return AntMessage2(AntMessageId::SET_SEARCH_TIMEOUT, content);
}

AntMessage2 AntMessage2::unassignChannel(quint8 channelNumber)
{
    QByteArray array;
    array += channelNumber;

    return AntMessage2(AntMessageId::UNASSIGN_CHANNEL, array);
}

const AntChannelEventMessage* AntMessage2::asChannelEventMessage() const
{
    const AntChannelEventMessage* antChannelEventMessage = dynamic_cast<const AntChannelEventMessage*>(this);
    Q_ASSERT_X(antChannelEventMessage, "AntMessage2::asChannelEventMessage()", "bad cast to ChannelEventMessage");
    return antChannelEventMessage;
}

AntMessage2 AntMessage2::assignChannel(quint8 channelNumber, quint8 channelType, quint8 networkNumber)
{
    QByteArray data;
    data += channelNumber;
    data += channelType;
    data += networkNumber;

    return AntMessage2(AntMessageId::ASSIGN_CHANNEL, data);
}

AntMessage2 AntMessage2::closeChannel(quint8 channelNumber)
{
    QByteArray content;
    content += channelNumber;
    return AntMessage2(AntMessageId::CLOSE_CHANNEL, content);
}

AntMessage2 AntMessage2::openChannel(quint8 channelNumber)
{
    QByteArray content;
    content += channelNumber;

    return AntMessage2(AntMessageId::OPEN_CHANNEL, content);
}

AntMessage2 AntMessage2::requestMessage(quint8 channelNumber, AntMessage2::AntMessageId messageId)
{
    QByteArray content;
    content += channelNumber;
    content += static_cast<quint8>(messageId);

    return AntMessage2(AntMessageId::REQUEST_MESSAGE, content);
}

AntMessage2 AntMessage2::setChannelFrequency(quint8 channelNumber, quint16 frequency)
{
    QByteArray data;
    data += channelNumber;
    quint8 frequencyOffset = static_cast<quint8>(frequency - ANT_CHANNEL_FREQUENCY_BASE);
    data += frequencyOffset;
    return AntMessage2(AntMessageId::SET_CHANNEL_FREQUENCY, data);
}

AntMessage2 AntMessage2::setChannelId(quint8 channelNumber, quint16 deviceId, indoorcycling::AntSensorType deviceType, quint8 transmissionType)
{
    QByteArray array;
    array += channelNumber;
    array += deviceId & 0xFF;
    array += (deviceId >> 8) & 0xFF;
    array += static_cast<quint8>(deviceType);
    array += transmissionType;

    return AntMessage2(AntMessageId::SET_CHANNEL_ID, array);
}

AntMessage2 AntMessage2::setChannelPeriod(quint8 channelNumber, AntSportPeriod messagePeriod)
{
    QByteArray content;
    content += channelNumber;

    quint16 messageRate = static_cast<quint16>(messagePeriod);
    content += messageRate & 0xFF;
    content += (messageRate >> 8) & 0xFF;

    return AntMessage2(AntMessageId::SET_CHANNEL_PERIOD, content);
}


AntChannelEventMessage::AntChannelEventMessage(const QByteArray &bytes):
    AntMessage2(AntMessageId::CHANNEL_EVENT, bytes.mid(3))
{
    _channelNumber = content()[0];
    _messageId = static_cast<AntMessageId>(content()[1]);
    quint8 messageCode = content()[2];
    _messageCode = static_cast<MessageCode>(messageCode);
}

const QString AntChannelEventMessage::antMessageCodeToString(const AntChannelEventMessage::MessageCode messageCode)
{
    return QString("0x%1").arg(QString::number(static_cast<quint8>(messageCode), 16));
}

quint8 AntChannelEventMessage::channelNumber() const
{
    return _channelNumber;
}

AntMessage2::AntMessageId AntChannelEventMessage::messageId() const
{
    return _messageId;
}

AntChannelEventMessage::MessageCode AntChannelEventMessage::messageCode() const
{
    return _messageCode;
}

QString AntChannelEventMessage::toString() const
{
    QString channelEventString = EVENT_CHANNEL_MESSAGES.value(_messageCode, "UNKNOWN");
    return QString("Channel Event %1 (%2), Channel %3, Message %4").arg(channelEventString)
            .arg(antMessageCodeToString(_messageCode))
            .arg(contentByte(0)).arg(antMessageIdToString(messageId()));
}


std::unique_ptr<AntMessage2> AntMessage2::createMessageFromBytes(const QByteArray &bytes)
{
    const AntMessageId messageId = static_cast<AntMessageId>(bytes[2]);
    switch(messageId) {
    case AntMessage2::AntMessageId::CHANNEL_EVENT:
        return std::unique_ptr<AntMessage2>(new AntChannelEventMessage(bytes));
    default:
        return std::unique_ptr<AntMessage2>(new AntMessage2(bytes));
    }

    return std::unique_ptr<AntMessage2>();
}

BroadCastMessage::BroadCastMessage(const AntMessage2 &antMessage):
    _antMessage(antMessage)
{
    if (!antMessage.isNull()) {
        _channelNumber = antMessage.contentByte(0);
        _dataPage = antMessage.contentByte(1);
    }
}

bool BroadCastMessage::isNull() const
{
    return _antMessage.isNull();
}

quint8 BroadCastMessage::channelNumber() const
{
    return _channelNumber;
}

quint8 BroadCastMessage::dataPage() const
{
    return _dataPage;
}

const AntMessage2 &BroadCastMessage::antMessage() const
{
    return _antMessage;
}

quint8 BroadCastMessage::contentByte(int byte) const
{
    return _antMessage.contentByte(byte + 1);
}

quint16 BroadCastMessage::contentShort(int startByteIndex) const
{
    return _antMessage.contentShort(startByteIndex + 1);
}

SetChannelIdMessage::SetChannelIdMessage(const AntMessage2& antMessage):
    _antMessage(antMessage)
{
    // empty
}

quint8 SetChannelIdMessage::channelNumber() const
{
    return _antMessage.contentByte(0);
}

quint16 SetChannelIdMessage::deviceNumber() const
{
    return _antMessage.contentShort(1);
}

bool SetChannelIdMessage::pairing() const
{
    return _antMessage.contentByte(3) & 0x80;
}

quint8 SetChannelIdMessage::deviceTypeId() const
{
    return _antMessage.contentByte(3) & 0x7F;
}

quint8 SetChannelIdMessage::transmissionType() const
{
    return _antMessage.contentByte(4);
}


const QString antMessageIdToString(const AntMessage2::AntMessageId messageId)
{
    return QString("0x%1").arg(QString::number(static_cast<quint8>(messageId), 16));
}
