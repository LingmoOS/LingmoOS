/*
 *   SPDX-FileCopyrightText: 2001 Matthias Hoelzer-Kluepfel <mhk@caldera.de>
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __USB_DEVICES_H__
#define __USB_DEVICES_H__

#include <QList>
#include <QString>

#include <libusb.h>

class USBDB;

class USBDevice
{
public:
    USBDevice(libusb_device *dev, struct libusb_device_descriptor &dev_desc);

    ~USBDevice();

    int level() const
    {
        return _level;
    }
    int device() const
    {
        return _device;
    }
    int parent() const
    {
        return _parent;
    }
    int bus() const
    {
        return _bus;
    }
    QString product();

    QString dump();

    static QList<USBDevice *> &devices()
    {
        return _devices;
    }
    static USBDevice *find(int bus, int device);
    static bool load();
    static void clear();

private:
    static QList<USBDevice *> _devices;
    static libusb_context *_context;

    static USBDB *_db;

    int _bus, _level, _parent, _port, _device, _channels;
    float _speed;

    QString _manufacturer, _product, _serial;

    unsigned int _verMajor, _verMinor;
    uint8_t _class, _sub, _prot;
    unsigned int _maxPacketSize;

    uint16_t _vendorID, _prodID;

    void collectDataSys(libusb_device *dev);
};

#endif
