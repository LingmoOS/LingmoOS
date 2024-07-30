// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020-2021 Harald Sitter <sitter@kde.org>

#ifndef SMARTMONITOR_H
#define SMARTMONITOR_H

#include <QHash>
#include <QObject>
#include <QTimer>

#include <functional>
#include <memory>

class AbstractSMARTCtl;
class Device;
class DeviceNotifier;

class SMARTMonitor : public QObject
{
    Q_OBJECT
    friend class SMARTMonitorTest;

public:
    explicit SMARTMonitor(std::unique_ptr<AbstractSMARTCtl> ctl, std::unique_ptr<DeviceNotifier> deviceNotifier, QObject *parent = nullptr);
    void start();

    QList<Device *> devices() const;

Q_SIGNALS:
    void deviceAdded(Device *device);
    void deviceRemoved(Device *device);

private Q_SLOTS:
    void removeUDI(const QString &udi);
    void reloadData();
    void onSMARTCtlFinished(const QString &devicePath, const QJsonDocument &document, const QString &textDocument);

private:
    void addDevice(Device *device);

    QTimer m_reloadTimer;
    const std::unique_ptr<AbstractSMARTCtl> m_ctl;
    const std::unique_ptr<DeviceNotifier> m_deviceNotifier;
    QHash<QString, Device *> m_pendingDevices; // waiting for smartctl to return
    QList<Device *> m_devices; // monitored smart devices
};

#endif // SMARTMONITOR_H
