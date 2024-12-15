// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIGNALUSB_H
#define SIGNALUSB_H

#include <QThread>
#include <libusb-1.0/libusb.h>

class SignalUSB : public QThread
{
    Q_OBJECT
public:
    SignalUSB(QObject *parent = nullptr);
    ~SignalUSB() override;

    /*用于注册libusb回调*/
    static int LIBUSB_CALL static_usb_arrived_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *userdata);

    int  usb_arrived_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event);

protected:
    void run() override;

    bool isUsbPrinter(libusb_device *dev);

private:
    bool needExit;
};

#endif // SIGNALUSB_H
