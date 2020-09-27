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

#include "antdevice.h"
#include "antmessage2.h"
namespace {
const QByteArray PADDING(5, '\0');
}
namespace indoorcycling
{

bool AntDevice::writeAntMessage(const AntMessage2 &message)
{
    QByteArray paddedMessageBytes = message.toBytes() + PADDING;
    int nrOfBytesWritten = writeBytes(paddedMessageBytes);
    if (nrOfBytesWritten == paddedMessageBytes.length()) {
        return true;
    } else {
        qWarning("Tried to write a message of %d bytes, but only %d bytes written", paddedMessageBytes.length(), nrOfBytesWritten);
        return false;
    }
}

AntDevice::AntDevice(QObject *parent) :
    QObject(parent)
{
}

}
