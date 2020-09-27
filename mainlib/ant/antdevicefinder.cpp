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

#include "antdevicefinder.h"
#ifdef Q_OS_WIN
//#include "usbexpressantdevice.h"
#else
#include "unixserialusbant.h"
#endif
#include "usb2antdevice.h"
#include <QtDebug>

namespace indoorcycling {

AntDeviceFinder::AntDeviceFinder(QObject *parent) :
    QObject(parent)
{
    // empty
}

AntDeviceFinder::~AntDeviceFinder()
{
    // empty
}

std::unique_ptr<AntDevice> AntDeviceFinder::openAntDevice()
{
    AntDeviceType type = findAntDeviceType();
    switch(type) {
    case AntDeviceType::USB_1:
#ifdef Q_OS_LINUX
        return std::unique_ptr<AntDevice>(new UnixSerialUsbAnt);
#else
//        return std::unique_ptr<AntDevice>(new UsbExpressAntDevice);
#endif
    case AntDeviceType::USB_2:
        return std::unique_ptr<AntDevice>(new Usb2AntDevice);
    default:
        return std::unique_ptr<AntDevice>();
  }
}
}
