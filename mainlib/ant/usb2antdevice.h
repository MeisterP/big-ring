/*
 * Copyright (c) 2012-2015 Ilja Booij (ibooij@gmail.com)
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

#ifndef USB2ANTDEVICE_H
#define USB2ANTDEVICE_H
#include "antdevice.h"

#include <memory>

#include <QtCore/QThread>
#include <QtCore/QTimer>

namespace indoorcycling
{

struct Usb2DeviceConfiguration;

/**
 * @brief find a ANT+ Usb stick and report it's type
 * @return the type of USB ANT+ Stick, or ANT_DEVICE_NONE if none found.
 */
AntDeviceType findAntDeviceType();

/**
 * We use a worker which runs on different thread to perform the actual
 * reading and writing. We do this to make sure that we don't block Qt's
 * event loop. We could be blocking it, because we're using libusb's
 * synchronous interface.
 * Instead, reading and writing takes place on it's own thread. Communication
 * between the main thread and the worker thread is done using signals and
 * slots.
 */
class Usb2AntDeviceWorker: public QObject
{
    Q_OBJECT
public:
    Usb2AntDeviceWorker(Usb2DeviceConfiguration* deviceConfiguration, QObject *parent = 0);
public slots:
    void initialize();
    void read();
    void write(const QByteArray& bytes);
signals:
    void workerReady();
    void bytesWritten(int written);
    void bytesRead(const QByteArray& bytes);
private:
    QTimer* _readTimer;
    Usb2DeviceConfiguration* _deviceConfiguration;
};

/**
 * @brief class used for connecting to USB2 ANT+ devices. This implementation uses libusb-0.1
 */
class Usb2AntDevice : public AntDevice
{
    Q_OBJECT
public:
    explicit Usb2AntDevice(QObject *parent = 0);
    virtual ~Usb2AntDevice();
    virtual bool isValid() const override;
    virtual int numberOfChannels() const override;
    virtual int writeBytes(const QByteArray& bytes) override;
    virtual bool isReady() const override;
signals:
    void doWrite(const QByteArray& bytes);
private slots:
    void workerReady();
private:
    const std::unique_ptr<Usb2DeviceConfiguration> _deviceConfiguration;
    Usb2AntDeviceWorker* _worker;
    QThread* _workerThread;
    bool _workerReady;
};
}
#endif // USB2ANTDEVICE_H
