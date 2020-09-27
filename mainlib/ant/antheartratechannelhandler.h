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
#ifndef ANTHEARTRATECHANNELHANDLER_H
#define ANTHEARTRATECHANNELHANDLER_H

#include <QtCore/QObject>
#include <QtCore/QTime>
#include <QtCore/QTimer>

#include "antchannelhandler.h"
namespace indoorcycling {

/**
 * HeartRateMessage is an ANT+ Broadcast message.
 *
 * There are several data pages in the ANT+ Heart Rate profile.
 * All Data Pages hold 8 bytes of content.
 *
 * Byte 0 contains the channel
 * Byte 1 contains the data page.
 * Byte 2-4 are different for different pages. We will not use them here.
 * Bytes 5 & 6 contain the measurement time in 1/1024s as an unsigned short (16 bits). (this wraps every 64 seconds)
 * Byte 7 contains the a count of heart beat events. This wraps every 255 counts. It can be used to check for missed
 * events.
 * Byte 8 contains the heart rate as computed by the sensor.
 */
class HeartRateMessage: public BroadCastMessage
{
public:
    /**
     * Create a HeartRateMessage
     * @param antMessage the ANT+ broadcast message.
     * @return An HeartRateMessage.
     */
    HeartRateMessage(const AntMessage2& antMessage = AntMessage2());

    static AntMessage2 createHeartRateMessage(quint8 channelNumber, bool toggleHigh, quint16 measurementTime,
                                              quint8 heartBeatCount, quint8 computedHeartRate);

    /**
     * Represents the time of the last valid heart beat event.
     */
    quint16 measurementTime() const;
    /**
     * A single byte value which increments with each heart beat event.
     */
    quint8 heartBeatCount() const;
    /**
     * Instantaneous heart rate.
     */
    quint8 computedHeartRate() const;
private:
    quint16 _measurementTime;
    quint8 _heartBeatCount;
    quint8 _computedHeartRate;
};

class AntHeartRateChannelHandler : public AntChannelHandler
{
    Q_OBJECT
public:
    explicit AntHeartRateChannelHandler(int channelNumber, QObject* parent);
    virtual ~AntHeartRateChannelHandler() {}

protected:
    virtual void handleBroadCastMessage(const BroadCastMessage& message) override;
private:
    HeartRateMessage _lastMessage;
};

/**
 * Implements a ANT+ Heart rate Master Channel Handler. This channel is used for transmitting messages so they can
 * be picked up by an ANT+ slave, like a head unit, which can display the values.
 */
class AntHeartRateMasterChannelHandler : public AntMasterChannelHandler
{
    Q_OBJECT
public:
    explicit AntHeartRateMasterChannelHandler(int channelNumber, QObject* parent = nullptr);
    virtual ~AntHeartRateMasterChannelHandler() {}

    /**
     * Send a new sensor value over the channel.
     * @param valueType the type of value. For HeartRate channels this is always SensorValueType::SENSOR_TYPE_HR
     * @param value the value. For heart rate channels, this should be of type integer.
     */
    virtual void sendSensorValue(const SensorValueType valueType, const QVariant &value) override;
protected:
    virtual quint8 transmissionType() const override;
    virtual void channelOpened() override;
private slots:
    void sendMessage();
private:
    QTimer* const _updateTimer;
    QTime _lastSendTime;

    int _updateCounter;
    quint8 _heartRate;
    quint16 _lastEventTime;
    quint8 _lastNrOfHeartBeats;

};
}
#endif // ANTHEARTRATECHANNELHANDLER_H
