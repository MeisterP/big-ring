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

#ifndef UNIXSERIALUSBANT_H
#define UNIXSERIALUSBANT_H

#include <QByteArray>
#include <QObject>

#include <QtSerialPort/QSerialPort>

#include "antdevice.h"

namespace indoorcycling
{

/**
 * @brief ANT+ device that supports ANT+ USB1 Sticks. USB1 sticks use the CP210x USB to UART Bridge. These USB
 * sticks can be accessed using serial over USB communication.
 */
class UnixSerialUsbAnt : public AntDevice
{
    Q_OBJECT
public:
    explicit UnixSerialUsbAnt(QObject *parent = 0);
    virtual ~UnixSerialUsbAnt();

    virtual bool isValid() const override;

    virtual int numberOfChannels() const override;

    virtual int writeBytes(const QByteArray& bytes) override;

    virtual bool isReady() const override;
signals:

private slots:
    void readyRead();
private:
    QSerialPortInfo findGarminUsb1Stick();
    void openSerialConnection(const QSerialPortInfo& serialPortInfo);

    int openConnection();

    QSerialPort* _serialPortConnection;
};
}
#endif // UNIXSERIALUSBANT_H
