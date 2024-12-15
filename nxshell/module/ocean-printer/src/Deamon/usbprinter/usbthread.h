// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef USBTHREAD_H
#define USBTHREAD_H

#include <QThread>
#include <QMutex>

#include "zdevicemanager.h"

#include <libusb-1.0/libusb.h>


class USBThread : public QThread
{
    Q_OBJECT
public:
    USBThread(QObject *parent = nullptr);
    ~USBThread() override;
    void getUsbDevice();
    void processArrivedUSBDevice();

private:
    libusb_device *m_currentUSBDevice;
    QList<libusb_device *> m_usbDeviceList;
    QMutex m_mutex;
    TDeviceInfo m_deviceInfo;

    QMap<uint, QString> m_pendingNotificationsMap;
    QString m_configingPrinterName;

private:
    void getDriver();

    void nextConfiguration();

private slots:
    bool addArrivedUSBPrinter();
    void notificationActionInvoked(uint id, const QString &msg);
    void addingJobFinished(int status);
signals:
    void newUSBDeviceArrived();
    void startGetDriver();
    void workFinished();
    /*通知前端当前正在配置的打印机状态变化*/
    void deviceStatusChanged(const QString &defaultPrinterName, int status);

protected:
   void run() override;
};

#endif // USBTHREAD_H
