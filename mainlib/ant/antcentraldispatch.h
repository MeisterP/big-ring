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
#ifndef ANTCENTRALDISPATCH_H
#define ANTCENTRALDISPATCH_H

#include <functional>
#include <memory>
#include <vector>

#include <QtCore/QFile>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QTimer>
#include <QtCore/QVector>

#include "antsensortype.h"
#include "util/util.h"

class AntChannelEventMessage;
class AntMessageGatherer;
class AntMessage2;
class BroadCastMessage;
class SetChannelIdMessage;


#include "antdevice.h"
#include "antsensortype.h"

namespace indoorcycling {
class AntChannelHandler;
class AntMasterChannelHandler;
class AntHeartRateMasterChannelHandler;
class AntPowerMasterChannelHandler;
class AntSmartTrainerChannelHandler;

/**
 * The Main ANT+ class that manages the connection with the ANT+ usb stick and through that stick, to the ANT+
 * sensors that are found.
 */
class AntCentralDispatch : public QObject
{
    Q_OBJECT
public:
    explicit AntCentralDispatch(QObject *parent = 0);

    /**
     * Check if an ANT+ adapter is present.
     */
    bool antAdapterPresent() const;
    /**
     * check if the ANT+ system has been initialized. After this, channels can be opened.
     */
    bool isInitialized() const;

    /**
     * Search for a sensor of a certain type, for instance a Heart Rate Monitor.
     */
    bool searchForSensorType(AntSensorType channelType);
    /**
     * Search for a sensor of a certain type, with a specific device number. Use this if you want to pair with
     * a specific instance of a sensor type.
     */
    bool searchForSensor(AntSensorType channelType, int deviceNumber);
    /**
     * Check if all ANT+ channels are closed.
     */
    bool areAllChannelsClosed() const;
signals:
    /** signal emitted when scanning for an usb stick is finished. @param found indicates whether or not an ANT+ usb
     * stick was found.
     */
    void antUsbStickScanningFinished(bool found);
    /**
     * Signal emitted when the connection to the ANT+ stick and the ANT+ network has been set up. After this, we
     * can try to connect to or search for ANT+ sensors. If success is false, we have not been able to initialize the
     * ANT+ connection.
     */
    void initializationFinished(bool success);
    /**
     * emitted when the search for a certain sensor type is started.
     */
    void searchStarted(AntSensorType channelType, int channelNumber);
    /**
     * emitted when an ANT+ sensor is found. @param channelType denotes the type of sensor, @param deviceNumber the
     * number of the device itself.
     */
    void sensorFound(AntSensorType channelType, int deviceNumber);
    /**
     * Emitted when a search for sensor has timed out.
     * @param channelType type of the channel that was not found
     * @param deviceNumber, if not 0 (zero), number of the device that was not found.
     */
    void sensorNotFound(AntSensorType channelType, int deviceNumber);

    /** emitted when a sensor value if measured by one of the sensors
     * @param sensorValueType type of sensor value
     * @param sensorValueType type of sensor
     * @param sensorValue the value ifself.
    */
    void sensorValue(const SensorValueType sensorValueType, const AntSensorType sensorType,
                     const QVariant& sensorValue);
    /**
     * emitted when a channel is closed
     */
    void channelClosed(int channelNumber, const AntSensorType sensorType);
    /**
     * emitted when all channels are closed
     */
    void allChannelsClosed();
public slots:
    /**
     * Initialize the connection to the ANT+ stick. After calling this, listen for the signal initializationFinished(bool)
     * to check if the connection was successful.
     *
     * If no ANT+ stick can be found or opened, initialization will be retried until it's found.
     */
    void initialize();

    /**
      Close all channels
     */
    void closeAllChannels();

    /**
     * open a master (transmitting) channel of a certain type, returns true if opening succeeded.
     */
    bool openMasterChannel(AntSensorType sensorType);
    /**
     * Send value to a master channel
     */
    bool sendSensorValue(const SensorValueType sensorValueType, const AntSensorType sensorType,
                         const QVariant& sensorValue);

    /**
     * Set the weight of the cyclist
     */
    void setWeight(const qreal cyclistWeightInKilograms, const qreal bikeWeightInKilograms);
    /**
     * Set the slope in grades. This will only have an effect if a Smart Trainer (FE-C) channel is open.
     */
    void setSlope(const qreal slopeInPercent);
private slots:
    void messageFromAntUsbStick(const QByteArray& bytes);
    /**
     * Reset ANT+ USB Stick.
     */
    void resetAntSystem();
    /**
     * Set ANT+ Network Key
     */
    void sendNetworkKey();
    /**
     * This slot is called when an Ant Channel is set to a certain device number.
     */
    void setChannelInfo(int channelNumber, AntSensorType sensorType, int sensorDeviceNumber);
    /**
     * slot called when a search is timed out.
     */
    void searchTimedOut(int channelType, AntSensorType sensorType);
    /**
     * handle a new sensor value from one of the sensors.
     */
    void handleSensorValue(const SensorValueType sensorValueType, const AntSensorType sensorType,
                     const QVariant& sensorValue);
    /**
      * handle that communication on a channel is finished.
      */
    void handleChannelUnassigned(int channelNumber);
private:
    /**
     * Start scanning for an ANT+ usb stick. When scanning is finished, antUsbStickScanningFinished(AntDeviceType) is emitted.
     */
    void scanForAntUsbStick();

    /**
     * find a channel for an AntSensorType. If no channel is configured for the type, this returns
     * a nullptr.
     */
    AntChannelHandler *findChannelForSensorType(const AntSensorType &sensorType);

    /**
     * find free channel
     */
    int findFreeChannel();
    /**
      Create a new channel
     */
    AntChannelHandler* createChannel(int channelNumber, AntSensorType& sensorType);

    /**
     * Send a message to the USB ANT+ Stick.
     */
    void sendAntMessage(const AntMessage2& message);

    /**
     * handle a channel event message.
     */
    void handleChannelEvent(const AntChannelEventMessage& channelEventMessage);
    /**
     * handle a broad cast message
     */
    void handleBroadCastMessage(const BroadCastMessage& broadCastMessage);

    /**
     * handle channel id mesage.
     */
    void handleChannelIdMessage(const SetChannelIdMessage& channelIdMessage);

    enum class AntMessageIO {
        INPUT, OUTPUT
    };
    /**
     * Log ANT+ message
     */
    void logAntMessage(const AntMessageIO io, const AntMessage2& message);

    /** function template for sending a message to a channel. */
    template <class T>
    bool sendToChannel(const T& message, std::function<void(AntChannelHandler&, const T&)> sendFunction);

    std::unique_ptr<AntDevice> _antUsbStick;
    bool _initialized;
    AntMessageGatherer* const _antMessageGatherer;

    std::vector<qobject_unique_ptr<AntChannelHandler>> _channels;
    QMap<AntSensorType,QPointer<AntMasterChannelHandler>> _masterChannels;
    QPointer<AntSmartTrainerChannelHandler> _smartTrainerChannelHandler;
    QPointer<AntPowerMasterChannelHandler> _powerTransmissionChannelHandler;
    QPointer<AntHeartRateMasterChannelHandler> _heartRateMasterChannelhandler;
    QTimer* const _initializationTimer;
    QFile _logFile;
};
}

#endif // ANTCENTRALDISPATCH_H
