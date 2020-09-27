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

#ifndef USBEXPRESSANTDEVICE_H
#define USBEXPRESSANTDEVICE_H

/** This file should only be compiled on Windows */
#ifdef Q_OS_WIN32

#include "antdevice.h"
#include <QObject>

#include <windows.h>
extern "C" {
#include <SiUSBXp.h>
}

namespace indoorcycling
{
/**
 * @brief ANT+ device that supports ANT+ USB1 Sticks. USB1 sticks use the CP210x USB to UART Bridge. These USB
 * sticks can be accessed using serial over USB communication. This is the Windows version, that uses the
 * USBXpress® Development Tools from http://www.silabs.com/products/mcu/Pages/USBXpress.aspx.
 */
class UsbExpressAntDevice: public AntDevice
{
    Q_OBJECT
public:
    UsbExpressAntDevice(QObject* parent = 0);
    virtual ~UsbExpressAntDevice();

    virtual int numberOfChannels() const;
    virtual bool isValid() const;
    virtual int writeBytes(QByteArray &bytes);
    virtual QByteArray readBytes();

private:
    static int findDevice();
    bool openConnection();

    Qt::HANDLE _deviceHandle;
};
}
#endif

#endif // USBEXPRESSANTDEVICE_H
