/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_DEVICE_P_H
#define BLUEZQT_DEVICE_P_H

#include <QObject>
#include <QStringList>

#include "bluezdevice1.h"
#include "bluezqt_dbustypes.h"
#include "dbusproperties.h"
#include "types.h"

namespace BluezQt
{
typedef org::bluez::Device1 BluezDevice;
typedef org::freedesktop::DBus::Properties DBusProperties;

class DevicePrivate : public QObject
{
    Q_OBJECT

public:
    explicit DevicePrivate(const QString &path, const QVariantMap &properties, const AdapterPtr &adapter);

    void init(const QVariantMap &properties);

    void interfacesAdded(const QString &path, const QVariantMapMap &interfaces);
    void interfacesRemoved(const QString &path, const QStringList &interfaces);

    void addGattService(const QString &gattServicePath, const QVariantMap &properties);
    void removeGattService(const QString &gattServicePath);

    QDBusPendingReply<> setDBusProperty(const QString &name, const QVariant &value);
    void propertiesChanged(const QString &path, const QString &interface, const QVariantMap &changed, const QStringList &invalidated);
    void namePropertyChanged(const QString &value);
    void aliasPropertyChanged(const QString &value);
    void addressPropertyChanged(const QString &value);
    void classPropertyChanged(quint32 value);
    void serviceDataChanged(const QHash<QString, QByteArray> &value);

    QWeakPointer<Device> q;
    BluezDevice *m_bluezDevice;
    DBusProperties *m_dbusProperties;

    QString m_address;
    QString m_name;
    QString m_alias;
    quint32 m_deviceClass;
    quint16 m_appearance;
    QString m_icon;
    bool m_paired;
    bool m_trusted;
    bool m_blocked;
    bool m_legacyPairing;
    qint16 m_rssi;
    ManData m_manufacturerData;
    bool m_servicesResolved;
    bool m_connected;
    QStringList m_uuids;
    QString m_modalias;
    QHash<QString, QByteArray> m_serviceData;
    BatteryPtr m_battery;
    InputPtr m_input;
    MediaPlayerPtr m_mediaPlayer;
    MediaTransportPtr m_mediaTransport;
    QList<GattServiceRemotePtr> m_services;
    AdapterPtr m_adapter;
};

} // namespace BluezQt

#endif // BLUEZQT_DEVICE_P_H
