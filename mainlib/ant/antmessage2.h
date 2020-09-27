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
#ifndef ANTMESSAGE2_H
#define ANTMESSAGE2_H

#include <array>
#include <memory>
#include <QtCore/QObject>

#include "antsensortype.h"

/* forward declarations */
class AntChannelEventMessage;

/** TODO: move this to a "better location".*/
enum class AntSportPeriod: quint16 {
    UNUSED = 0,
    HR = 8070,
    POWER = 8182,
    SMART_TRAINER = 8192,
    SPEED = 8118,
    CADENCE = 8102,
    SPEED_AND_CADENCE = 8086
};
/**
 * Class representing AntMessages.
 * @brief The AntMessage2 class
 */
class AntMessage2
{
public:
    /** creates an invalid AntMessage */
    AntMessage2();
    static const quint8 SYNC_BYTE = 0xA4;
    static const quint8 ANT_PLUS_NETWORK_NUMBER = 1;
    static const quint16 ANT_CHANNEL_FREQUENCY_BASE = 2400; // Mhz
    static const quint16 ANT_PLUS_CHANNEL_FREQUENCY = 2457; // Mhz

    /** message ids (in alphabetic order) */
    enum class AntMessageId: quint8 {
        INVALID = 0x0,
        ACKNOWLEDGED_MESSAGE = 0x4F,
        ASSIGN_CHANNEL = 0x42,
        BROADCAST_EVENT = 0x4e,
        CHANNEL_EVENT = 0x40,
        CLOSE_CHANNEL = 0x4c,
        OPEN_CHANNEL = 0x4b,
        REQUEST_MESSAGE = 0x4d,
        SET_CHANNEL_FREQUENCY = 0x45,
        SET_CHANNEL_ID = 0x51,
        SET_CHANNEL_PERIOD = 0x43,
        SET_NETWORK_KEY = 0x46,
        SET_SEARCH_TIMEOUT = 0x44,
        SYSTEM_RESET = 0x4A,
        UNASSIGN_CHANNEL = 0x41
    };

    bool isNull() const;
    QByteArray toBytes() const;
    QByteArray toHex() const;
    virtual QString toString() const;

    AntMessageId id() const;

    /** Create an AntMessage2 from bytes. The bytes should contain the complete message.
     * @return an invalid (null) unique_ptr if bytes did not contain a valid message
     */
    static std::unique_ptr<AntMessage2> createMessageFromBytes(const QByteArray& bytes);

    // static factory methods for different messages
    static AntMessage2 assignChannel(quint8 channelNumber, quint8 channelType = 0, quint8 networkNumber = ANT_PLUS_NETWORK_NUMBER);
    static AntMessage2 closeChannel(quint8 channelNumber);
    static AntMessage2 openChannel(quint8 channelNumber);
    static AntMessage2 requestMessage(quint8 channelNumber, AntMessageId messageId);
    static AntMessage2 setChannelFrequency(quint8 channelNumber, quint16 frequency = ANT_PLUS_CHANNEL_FREQUENCY);
    static AntMessage2 setChannelId(quint8 channelNumber, quint16 deviceId, indoorcycling::AntSensorType deviceType, quint8 transmissionType = 0u);
    /**
     * @brief setChannelPeriod set the channel messaging period.
     * @param channelNumber the channel number
     * @param messageRate the message rate. This is the rate in seconds * 32768. To get a rate of 4Hz, set the channel
     * period to 8192. 32768 / 4 = 8192. This period is determined by the sender, so look at the device profile to
     * get the correct period. For the ANT+ Heart Rate Profile, this should be set to 8070, for a rate of 4.06Hz.
     * @return a ANT+ message for setting a channel's messaging period.
     */
    static AntMessage2 setChannelPeriod(quint8 channelNumber, AntSportPeriod messagePeriod);
    /**
     * @brief Construct the ANT message to set the ant network key.
     * @param networkNumber the network to set the key for.
     * @param networkKey the network key, an 8 byte array.
     * @return the message.
     */
    static AntMessage2 setNetworkKey(quint8 networkNumber, const std::array<quint8, 8> &networkKey);
    static AntMessage2 setSearchTimeout(quint8 channelNumber, int seconds);
    static AntMessage2 setInfiniteSearchTimeout(quint8 channelNumber);
    static AntMessage2 systemReset();
    static AntMessage2 unassignChannel(quint8 channelNumber);

    /**
     * @brief return a pointer to this AntMessage as an AntChannelEventMessage. This fails (with an assert) if the
     * message is not an AntChannelEventMessage.
     * @return the message as an AntChannelEventMessage.
     */
    const AntChannelEventMessage* asChannelEventMessage() const;

    quint8 contentByte(int nr) const;
    quint16 contentShort(int index) const;

    AntMessage2(const AntMessageId id, const QByteArray& content);
protected:
    AntMessage2(const QByteArray& completeMessageBytes);

    const QByteArray& content() const;

private:
    quint8 computeChecksum(const QByteArray& bytes) const;

    AntMessageId _id;

    QByteArray _content;
};

/**
 * output an AntMessage2::AntMessageId as a string. This outputs the id as a hex string.
 */
const QString antMessageIdToString(const AntMessage2::AntMessageId messageId);

class AntChannelEventMessage: public AntMessage2
{
public:
    AntChannelEventMessage(const QByteArray& bytes);

    enum class MessageCode: quint8 {
        CHANNEL_IN_WRONG_STATE = 0x15,
        EVENT_CHANNEL_CLOSED = 0x07,
        EVENT_CHANNEL_COLLISION = 0x09,
        RESPONSE_NO_ERROR = 0x00,
        EVENT_RX_FAILED = 0x02,
        EVENT_RX_FAIL_GO_TO_SEARCH = 0x08,
        EVENT_RX_SEARCH_TIMEOUT = 0x01,
        EVENT_TRANSFER_TX_COMPLETED = 0x05,
        EVENT_TRANSFER_TX_FAILED = 0x06,
        EVENT_TX = 0x03
    };
    static const QString antMessageCodeToString(const MessageCode messageCode);
    quint8 channelNumber() const;
    AntMessageId messageId() const;
    MessageCode messageCode() const;

    virtual QString toString() const override;
private:
    quint8 _channelNumber;
    AntMessageId _messageId;
    MessageCode _messageCode;
};



/**
 * Set Channel ID message
 *
 * Bytes:
 * 0 channel number
 * 1-2 device numbers
 * 3 first bit (MSB) pairing bit.
 * 3 7 LSB bits device type id
 * 4 transmission type.
 */
class SetChannelIdMessage
{
public:
    SetChannelIdMessage(const AntMessage2& bytes);

    quint8 channelNumber() const;
    quint16 deviceNumber() const;
    bool pairing() const;
    quint8 deviceTypeId() const;
    quint8 transmissionType() const;

    const QByteArray& toBytes() const;
private:
    AntMessage2 _antMessage;
};

/**
 * A Broadcast message. Each channel can convert this message to a specific message (heartrate, power, etc..)
 */
class BroadCastMessage
{
public:
    /**
     * Construct a new BroadCastMessage. By default, an empty message is created. For an empty message, #isNull() will
     * return true.
     * @param antMessage the antMessage with the contents of the message.
     */
    BroadCastMessage(const AntMessage2& antMessage = AntMessage2());

    /**
     * Checks if this is an empty message.
     */
    bool isNull() const;
    /**
     * channel number of the message.
     */
    quint8 channelNumber() const;
    /**
     * date page of the message.
     */
    quint8 dataPage() const;
    /**
     * the actual ant message that was used to construct the broadcast message.
     */
    const AntMessage2 &antMessage() const;
    /**
     * a single byte from the content. Starts at 0 for the data page of a broad cast message.
     */
    quint8 contentByte(int byte) const;
    /**
     * a short (two bytes) from the content. Starts at 0 for the data page of a broad cast message.
     */
    quint16 contentShort(int startByteIndex) const;
protected:
    AntMessage2 _antMessage;
private:
    quint8 _channelNumber;
    quint8 _dataPage;
};

#endif // ANTMESSAGE2_H
