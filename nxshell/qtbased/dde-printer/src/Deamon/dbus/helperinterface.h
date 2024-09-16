// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HELPERINTERFACE_H
#define HELPERINTERFACE_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QTimer>

#include "zcupsmonitor.h"

class HelperInterface : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", SERVICE_INTERFACE_NAME)
public:
    explicit HelperInterface(CupsMonitor *pCupsMonitor, QObject *parent = nullptr);
    ~HelperInterface();

    void registerDBus();
    void unRegisterDBus();
signals:
    void signalJobStateChanged(int id, int state, const QString &message);
    void signalPrinterStateChanged(const QString &printer, int state, const QString &message);
    void signalPrinterDelete(const QString &printer);
    void signalPrinterAdd(const QString &printer);
    void deviceStatusChanged(const QString &defaultPrinterName, int status);
    void usbDeviceProcess();

public slots:
    //dbus接口
    bool isJobPurged(int id);
    QString getJobNotify(const QMap<QString, QVariant> &job);
    QString getStateString(int iState);
    void setTypeAndState(int status);

protected:
    void slotShowTrayIcon(bool bShow);
    void showJobsWindow();


private:
    CupsMonitor *m_pCupsMonitor;
    QSystemTrayIcon *m_pSystemTray;
    QTimer* m_timer;
};

#endif // HELPERINTERFACE_H
