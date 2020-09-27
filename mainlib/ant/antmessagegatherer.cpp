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

#include "antmessage2.h"
#include "antmessagegatherer.h"

AntMessageGatherer::AntMessageGatherer(QObject *parent) :
    QObject(parent)
{
}

void AntMessageGatherer::submitBytes(QByteArray bytes)
{
    _bytesReceived.append(bytes);
    int length;
    QByteArray messageBytes;
    while(!_bytesReceived.isEmpty()) {
        quint8 currentByte = _bytesReceived[0];
        _bytesReceived.remove(0, 1);
        if (messageBytes.isEmpty()) {
            if (currentByte == AntMessage2::SYNC_BYTE) {
                messageBytes.append(currentByte);
            }
        } else {
            if (messageBytes.size() == 1) {
                length = (int) currentByte;
                messageBytes.append(currentByte)
                        .append(_bytesReceived.left(2 + length));
                _bytesReceived.remove(0, 2 + length);
                if (messageIsTooShort(messageBytes, length)) {
                    _bytesReceived.prepend(messageBytes);
                    return;
                } else if (checksumIsOk(messageBytes)) {
                    emit antMessageReceived(messageBytes);
                }
                messageBytes.clear();
            }
        }
    }
}


bool AntMessageGatherer::messageIsTooShort(const QByteArray &messageBytes, int length)
{
    // Message length is SYNC_BYTE, LENGTH, TYPE, DATA, CHECKSUM
    return ((4 + length) > messageBytes.length());
}

bool AntMessageGatherer::checksumIsOk(const QByteArray &messageBytes)
{
    quint8 checksum = messageBytes.right(1)[0];
    quint8 calculatedChecksum = 0;
    foreach(const char byte, messageBytes.left(messageBytes.length() - 1)) {
        calculatedChecksum ^= byte;
    }
    return (checksum == calculatedChecksum);
}
