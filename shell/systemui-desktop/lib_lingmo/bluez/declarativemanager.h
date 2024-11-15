/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef DECLARATIVEMANAGER_H
#define DECLARATIVEMANAGER_H

#include <QHash>
#include <QQmlListProperty>

#include <BluezQt/Manager>

class DeclarativeDevice;
class DeclarativeAdapter;

class DeclarativeManager : public BluezQt::Manager
{
    Q_OBJECT
    Q_PROPERTY(DeclarativeAdapter *usableAdapter READ usableAdapter NOTIFY usableAdapterChanged)
    Q_PROPERTY(QQmlListProperty<DeclarativeAdapter> adapters READ declarativeAdapters NOTIFY adaptersChanged)
    Q_PROPERTY(QQmlListProperty<DeclarativeDevice> devices READ declarativeDevices NOTIFY devicesChanged)

public:
    explicit DeclarativeManager(QObject *parent = nullptr);

    DeclarativeAdapter *usableAdapter() const;
    QQmlListProperty<DeclarativeAdapter> declarativeAdapters();
    QQmlListProperty<DeclarativeDevice> declarativeDevices();

    DeclarativeAdapter *declarativeAdapterFromPtr(BluezQt::AdapterPtr ptr) const;
    DeclarativeDevice *declarativeDeviceFromPtr(BluezQt::DevicePtr ptr) const;

    QHash<QString, DeclarativeAdapter *> m_adapters;
    QHash<QString, DeclarativeDevice *> m_devices;

public Q_SLOTS:
    DeclarativeAdapter *adapterForAddress(const QString &address) const;
    DeclarativeAdapter *adapterForUbi(const QString &ubi) const;
    DeclarativeDevice *deviceForAddress(const QString &address) const;
    DeclarativeDevice *deviceForUbi(const QString &ubi) const;

Q_SIGNALS:
    void initFinished();
    void initError(const QString &errorText);
    void adapterAdded(DeclarativeAdapter *adapter);
    void adapterRemoved(DeclarativeAdapter *adapter);
    void adapterChanged(DeclarativeAdapter *adapter);
    void deviceAdded(DeclarativeDevice *device);
    void deviceRemoved(DeclarativeDevice *device);
    void deviceChanged(DeclarativeDevice *device);
    void usableAdapterChanged(DeclarativeAdapter *adapter);

    void adaptersChanged(QQmlListProperty<DeclarativeAdapter> adapters);
    void devicesChanged(QQmlListProperty<DeclarativeDevice> devices);

private Q_SLOTS:
    void initJobResult(BluezQt::InitManagerJob *job);

    void slotAdapterAdded(BluezQt::AdapterPtr adapter);
    void slotAdapterRemoved(BluezQt::AdapterPtr adapter);
    void slotDeviceAdded(BluezQt::DevicePtr device);
    void slotDeviceRemoved(BluezQt::DevicePtr device);
    void slotUsableAdapterChanged(BluezQt::AdapterPtr adapter);
};

#endif // DECLARATIVEMANAGER_H
