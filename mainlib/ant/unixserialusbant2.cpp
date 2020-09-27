/*
 * Copyright (c) 2009 Mark Rages
 * Copyright (c) 2011 Mark Liversedge (liversedge@gmail.com)
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

#include "unixserialusbant.h"
#include <QtDebug>
#include <QtCore/QTime>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

namespace indoorcycling {

UnixSerialUsbAnt::UnixSerialUsbAnt(QObject *parent) : AntDevice(parent) {
  QSerialPortInfo garminSerialPortInfo = findGarminUsb1Stick();
  if (garminSerialPortInfo.isValid()) {
    openSerialConnection(garminSerialPortInfo);
    emit deviceReady();
  }
}

UnixSerialUsbAnt::~UnixSerialUsbAnt() {
  if (_serialPortConnection->isOpen()) {
    _serialPortConnection->close();
  }
}

bool UnixSerialUsbAnt::isValid() const {
  return _serialPortConnection->isOpen();
}

int UnixSerialUsbAnt::numberOfChannels() const { return 4; }

int UnixSerialUsbAnt::writeBytes(const QByteArray &bytes) {
  int totalWritten = 0;
  QByteArray bytesLeft(bytes);
  while (!bytesLeft.isEmpty()) {
    qint64 bytesWritten = _serialPortConnection->write(bytesLeft);
    if (bytesWritten < 0) {
      return bytesWritten;
    } else {
      totalWritten += bytesWritten;
      if (bytesWritten == bytesLeft.size()) {
        break;
      } else {
        bytesLeft = bytesLeft.mid(bytesWritten);
      }
    }
  }
  return totalWritten;
}

bool UnixSerialUsbAnt::isReady() const {
  return _serialPortConnection->isOpen();
}

void UnixSerialUsbAnt::readyRead() {
  emit bytesRead(_serialPortConnection->readAll());
}

QSerialPortInfo UnixSerialUsbAnt::findGarminUsb1Stick() {
  for (const QSerialPortInfo &serialPort : QSerialPortInfo::availablePorts()) {
    if (serialPort.vendorIdentifier() == indoorcycling::GARMIN_USB_VENDOR_ID &&
        serialPort.productIdentifier() ==
            indoorcycling::GARMIN_USB1_PRODUCT_ID) {
      qDebug() << "found garmin usb 1 stick at" << serialPort.systemLocation();
      return serialPort;
    }
  }
  return QSerialPortInfo();
}

void UnixSerialUsbAnt::openSerialConnection(
    const QSerialPortInfo &serialPortInfo) {
  _serialPortConnection = new QSerialPort(serialPortInfo);
  _serialPortConnection->setBaudRate(QSerialPort::Baud115200);
  _serialPortConnection->setDataBits(QSerialPort::Data8);
  _serialPortConnection->setParity(QSerialPort::NoParity);

  bool opened = _serialPortConnection->open(QIODevice::ReadWrite);
  if (opened) {
    _serialPortConnection->readAll();
    _serialPortConnection->flush();
    connect(_serialPortConnection, &QSerialPort::readyRead, this,
            &UnixSerialUsbAnt::readyRead);
  } else {
    qDebug() << "unable to open serial port device"
             << serialPortInfo.description();
  }
}
}
