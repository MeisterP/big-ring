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

#include "usb2antdevice.h"
#include <QtDebug>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QThread>

extern "C" {
#ifdef Q_OS_LINUX
#include "thirdparty/libusb-compat/usb.h"
#endif
#ifdef Q_OS_WIN
#include <lusb0_usb.h>
#endif
}

namespace
{
const QVector<int> VALID_PRODUCT_IDS({ indoorcycling::GARMIN_USB1_PRODUCT_ID,
                                       indoorcycling::GARMIN_USB2_PRODUCT_ID,
                                       indoorcycling::OEM_USB2_PRODUCT_ID });
bool usbInitialized = false;

/**
 * @brief initialize the usb library. This function can be called multiple times, but will only "work" once.
 */
void initializeUsb();
/**
 * @brief find an ANT+ Stick on the USB ports.
 * @return the usb_device found, or nullptr if nothing found.
 */
static struct usb_device* findAntStick();
/**
 * @brief reset the ant stick. We'll use this before connecting.
 * @param antStick
 */
void resetAntStick(struct usb_device* antStick);
/**
 * @brief open a connection to the ANT+ stick and initialize communication interfaces.
 * @return the full USB configuration.
 */
std::unique_ptr<indoorcycling::Usb2DeviceConfiguration> openAntStick();
/**
 * @brief initialize communication interfaces for the ant stick.
 * @return the USB configuration.
 */
std::unique_ptr<indoorcycling::Usb2DeviceConfiguration> findUsbInterface(usb_config_descriptor &config_descriptor);

#ifdef Q_OS_LINUX
/** On Linux, we might have to detach the kernel driver. */
int detachKernelDriver(struct usb_dev_handle& usbDeviceHandle, int interface);
#endif
}

namespace indoorcycling
{

AntDeviceType findAntDeviceType()
{
    struct usb_device* device = findAntStick();
    if (device) {
        switch(device->descriptor.idProduct) {
        case GARMIN_USB1_PRODUCT_ID:
            return AntDeviceType::USB_1;
        case GARMIN_USB2_PRODUCT_ID:
        case OEM_USB2_PRODUCT_ID:
            return AntDeviceType::USB_2;
        }
    }
    return AntDeviceType::NONE;
}

/**
 * @brief struct containing the usb configuration that we'll use for reading and writing.
 */
struct Usb2DeviceConfiguration
{
    QMutex mutex;
    struct usb_dev_handle* deviceHandle;
    int readEndpoint;
    int writeEndpoint;
    int interface;
};

Usb2AntDevice::Usb2AntDevice(QObject *parent) :
    AntDevice(parent), _deviceConfiguration(openAntStick()), _workerThread(new QThread(this)),
    _workerReady(false)
{
    if (!_deviceConfiguration) {
        qWarning("Unable to open ANT+ stick correctly");
    }
    _worker = new Usb2AntDeviceWorker(_deviceConfiguration.get());
    _worker->moveToThread(_workerThread);

    connect(_worker, &Usb2AntDeviceWorker::bytesRead, this, &Usb2AntDevice::bytesRead);
    connect(_worker, &Usb2AntDeviceWorker::workerReady, this, &Usb2AntDevice::workerReady);
    connect(this, &Usb2AntDevice::doWrite, _worker, &Usb2AntDeviceWorker::write);
    connect(_workerThread, &QThread::started, _worker, &Usb2AntDeviceWorker::initialize);

    _workerThread->start();
}

Usb2AntDevice::~Usb2AntDevice() {
    qDebug() << "Usb2AntDevice stopping worker";
    _workerThread->quit();
    _workerThread->wait(1000);

    QMutexLocker locker(&_deviceConfiguration->mutex);
    if (_deviceConfiguration) {
        usb_release_interface(_deviceConfiguration->deviceHandle, _deviceConfiguration->interface);
        usb_close(_deviceConfiguration->deviceHandle);
    }
    qDebug() << "Usb2AntDevice stopped worker";
}

bool Usb2AntDevice::isValid() const
{
    return (_deviceConfiguration.get() != nullptr);
}

int Usb2AntDevice::numberOfChannels() const
{
    return 8;
}

int Usb2AntDevice::writeBytes(const QByteArray &bytes)
{
    emit doWrite(bytes);
    return bytes.size();
}

bool Usb2AntDevice::isReady() const
{
    return _workerReady;
}

void Usb2AntDevice::workerReady()
{
    _workerReady = true;
    emit deviceReady();
}

Usb2AntDeviceWorker::Usb2AntDeviceWorker(Usb2DeviceConfiguration *deviceConfiguration, QObject* parent):
    QObject(parent), _deviceConfiguration(deviceConfiguration)
{
    // empty
}

void Usb2AntDeviceWorker::initialize()
{
    qDebug() << "initializing worker";
    // try to empty the buffer of the device, if needed. We do not want any bytes that are read now to be
    // emitted, so we'll block signals when doing this initial read.
    blockSignals(true);
    read();
    blockSignals(false);
    _readTimer = new QTimer(this);
    _readTimer->setInterval(50);
    connect(_readTimer, &QTimer::timeout, this, &Usb2AntDeviceWorker::read);
    _readTimer->start();
    qDebug() << "initializing worker finished";
    emit workerReady();
}

void Usb2AntDeviceWorker::read()
{
    if (!_deviceConfiguration) {
        qWarning("Trying to write without a connection to a USB device");
        return;
    }
    QMutexLocker lock(&_deviceConfiguration->mutex);
    QByteArray bytes;
    bool bytesAvailable = true;
    int loopNr = 0;
    while(bytesAvailable) {
        if (loopNr > 0) {
            qDebug() << "loopNr" << loopNr;
        }
        QByteArray buffer(64, 0);

        int nrOfBytesRead = usb_bulk_read(_deviceConfiguration->deviceHandle, _deviceConfiguration->readEndpoint, buffer.data(), buffer.size(), 10);
        if (nrOfBytesRead <= 0) {
#ifdef Q_OS_WIN
            // for some reason, on Windows we get a -116 error code after a timeout. Just accept it.
            if (nrOfBytesRead != -116) {
#else
            if (nrOfBytesRead != -ETIMEDOUT) {
#endif
                qDebug() << "usb returns" << nrOfBytesRead << usb_strerror();
            }
            bytesAvailable = false;
        } else {
            bytes.append(buffer.left(nrOfBytesRead));
            bytesAvailable = (nrOfBytesRead == buffer.size());
            buffer.clear();
        }
        loopNr += 1;
    }
    if (!bytes.isEmpty()) {
        emit bytesRead(bytes);
    }
}

void Usb2AntDeviceWorker::write(const QByteArray &bytes)
{
    if (!_deviceConfiguration) {
        qWarning("Trying to read without a connection to a USB device");
    }
    QMutexLocker lock(&_deviceConfiguration->mutex);
    int written;
#ifdef Q_OS_WIN
    const char* constdata = reinterpret_cast<const char*>(bytes.data());
    char* data = const_cast<char*>(constdata);
    written = usb_interrupt_write(_deviceConfiguration->deviceHandle, _deviceConfiguration->writeEndpoint, data, bytes.size(), 10);
#else
    written = usb_bulk_write(_deviceConfiguration->deviceHandle, _deviceConfiguration->writeEndpoint, bytes.data(), bytes.size(), 10);
    if (written < 0) {
        qWarning("usb error: %s", usb_strerror());
    }
#endif
    emit bytesWritten(written);
}

} // end namespace indoorcycling

namespace
{

void initializeUsb()
{
    if (!usbInitialized) {
        usb_set_debug(0);
        usb_init();

        usbInitialized = true;
    }
}

struct usb_device *findAntStick()
{
    initializeUsb();
    usb_find_busses();
    usb_find_devices();

    struct usb_bus* bus;
    struct usb_device* device;

    for (bus = usb_get_busses(); bus; bus = bus->next) {
        for (device = bus->devices; device; device = device->next) {
            if (device->descriptor.idVendor == indoorcycling::GARMIN_USB_VENDOR_ID &&
                    VALID_PRODUCT_IDS.contains(device->descriptor.idProduct)) {
                qDebug() << "findAntStick(): returning device";
                return device;
            }
        }
    }
    qDebug() << "findAntStick(): returning null pointer";
    return nullptr;
}

void resetAntStick(struct usb_device *antStick)
{
#ifdef Q_OS_LINUX
    struct usb_dev_handle* antStickHandle;
    if ((antStickHandle = usb_open(antStick))) {
        usb_reset(antStickHandle);
        usb_close(antStickHandle);
    } else {
        qWarning("Unable to open and reset ANT stick");
    }
#else
    Q_UNUSED(antStick);
#endif
}


std::unique_ptr<indoorcycling::Usb2DeviceConfiguration> openAntStick()
{
    struct usb_dev_handle* deviceHandle;

    struct usb_device* device = findAntStick();
    if (device) {
        resetAntStick(device);
        deviceHandle = usb_open(device);
        if (deviceHandle && device->descriptor.bNumConfigurations) {
            std::unique_ptr<indoorcycling::Usb2DeviceConfiguration> deviceConfiguration = findUsbInterface(*(&device->config[0]));
            if (deviceConfiguration) {
                deviceConfiguration->deviceHandle = deviceHandle;
                int rc;
#ifdef Q_OS_LINUX
                detachKernelDriver(*deviceHandle, deviceConfiguration->interface);
#endif
                rc = usb_set_configuration(deviceHandle, 1);
                if (rc < 0) {
                    qDebug()<<"usb_set_configuration Error: "<< usb_strerror();
                }

                rc = usb_claim_interface(deviceHandle, deviceConfiguration->interface);
                if (rc < 0) qDebug()<<"usb_claim_interface Error: "<< usb_strerror();

                usb_clear_halt(deviceHandle, deviceConfiguration->writeEndpoint);
                usb_clear_halt(deviceHandle, deviceConfiguration->readEndpoint);

                qDebug() << "USB2 Ant Stick connected";
                return deviceConfiguration;
            }
        }
    } else {
        qWarning("Unable to find an ANT+ USB2 stick to open");
    }

    return nullptr;
}

std::unique_ptr<indoorcycling::Usb2DeviceConfiguration> findUsbInterface(usb_config_descriptor& config_descriptor)
{
    if (!config_descriptor.bNumInterfaces) {
        qWarning("No interfaces found on USB device");
        return nullptr;
    }
    if (!config_descriptor.interface[0].num_altsetting) {
        qWarning("No alternative interfaces found on USB device");
        return nullptr;
    }

    std::unique_ptr<indoorcycling::Usb2DeviceConfiguration> deviceConfiguration(new indoorcycling::Usb2DeviceConfiguration);

    struct usb_interface_descriptor intf = config_descriptor.interface[0].altsetting[0];

    if (intf.bNumEndpoints != 2) {
        qWarning("The USB device needs to have a read and a write endpoint");
        return nullptr;
    }
    deviceConfiguration->interface = intf.bInterfaceNumber;

    for (int i = 0 ; i < 2; i++) {
        if (intf.endpoint[i].bEndpointAddress & USB_ENDPOINT_DIR_MASK)
            deviceConfiguration->readEndpoint = intf.endpoint[i].bEndpointAddress;
        else
            deviceConfiguration->writeEndpoint = intf.endpoint[i].bEndpointAddress;
    }

    if (deviceConfiguration->readEndpoint < 0) {
        qWarning("USB device read end point not found.");
        return nullptr;
    }
    if (deviceConfiguration->writeEndpoint < 0) {
        qWarning("USB device write end point not found.");
        return nullptr;
    }

    return deviceConfiguration;
}

#ifdef Q_OS_LINUX
int detachKernelDriver(usb_dev_handle &usbDeviceHandle, int interface)
{
    QByteArray driverNameBuffer(128, '0');
    if (usb_get_driver_np(&usbDeviceHandle, interface, driverNameBuffer.data(), driverNameBuffer.size()) >= 0) {
        qDebug() << "We need to detach the kernel driver with name:" << QString(driverNameBuffer);
        if (usb_detach_kernel_driver_np(&usbDeviceHandle, interface) < 0) {
            qWarning("Unable to detach kernel driver for usb ANT+ stick: %s", usb_strerror());
            return -1;
        } else {
            qDebug() << "Kernel driver detached, we can now connect";
        }
    } else {
        qDebug() << "There is no kernel driver attached to the ANT+ Stick. We can connect to it.";
    }
    return 0;
}
#endif

}

