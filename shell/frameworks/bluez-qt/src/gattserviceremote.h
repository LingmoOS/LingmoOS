/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2021 Ivan Podkurkov <podkiva2@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_GATTSERVICEREMOTE_H
#define BLUEZQT_GATTSERVICEREMOTE_H

#include <QObject>
#include <QMap>
#include <QDBusObjectPath>

#include "types.h"
#include "bluezqt_export.h"

namespace BluezQt
{

class Device;
class PendingCall;

/**
 * @class BluezQt::GattService gattservice.h <BluezQt/GattService>
 *
 * Bluetooth LE GATT service.
 *
 * This class represents a Bluetooth LE GATT service.
 */
class BLUEZQT_EXPORT GattServiceRemote : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString uuid READ uuid NOTIFY uuidChanged)
    Q_PROPERTY(bool primary READ isPrimary NOTIFY primaryChanged)
    Q_PROPERTY(DevicePtr device READ device CONSTANT)
    Q_PROPERTY(QList<QDBusObjectPath> includes READ includes NOTIFY includesChanged)
    Q_PROPERTY(quint16 handle READ handle WRITE setHandle NOTIFY handleChanged)
    Q_PROPERTY(QList<GattCharacteristicRemotePtr> characteristics READ characteristics NOTIFY characteristicsChanged)

public:
    /**
     * Destroys a GattService object.
     */
    ~GattServiceRemote() override;

    /**
     * Returns a shared pointer from this.
     *
     * @return GattServicePtr
     */
    GattServiceRemotePtr toSharedPtr() const;

    /**
     * Returns an UBI of the gatt service.
     *
     * Example UBI: "/org/bluez/hci0/dev_40_79_6A_0C_39_75/service01"
     *
     * @return UBI of gatt service
     */
    QString ubi() const;

    /**
     * Returns an uuid of the service.
     *
     * @return uuid of the service
     */
    QString uuid() const;

    /**
     * Returns whether the service is primary.
     *
     * @return true if service is primary
     */
    bool isPrimary() const;

    /**
     * Returns a device that has this service.
     *
     * @return device of service
     */
    DevicePtr device() const;

    /**
     * Returns object paths representing the included
     * services of this service.
     *
     * @return Object paths of included services
     */
    QList<QDBusObjectPath> includes() const;

    /**
     * Returns service handle.
     *
     * @return qint16 service handle
     */
    quint16 handle() const;

    /**
     * Sets the service handle.
     *
     * @param handle service handle
     * @return void pending call
     */
    PendingCall *setHandle(quint16 handle);

    /**
     * Returns object paths representing the included
     * services of this service.
     *
     * @return Object paths of included services
     */
    QList<GattCharacteristicRemotePtr> characteristics() const;

Q_SIGNALS:
    /**
     * Indicates that at least one of the service's properties have changed.
     */
    void serviceChanged(GattServiceRemotePtr service);

    /**
     * Indicates that a new characteristic was added (eg. found by connection).
     */
    void gattCharacteristicAdded(GattCharacteristicRemotePtr characteristic);

    /**
     * Indicates that service characteristics list has changed
     */
    void characteristicsChanged(QList<GattCharacteristicRemotePtr> characteristics);

    /**
     * Indicates that a characteristic was removed.
     */
    void gattCharacteristicRemoved(GattCharacteristicRemotePtr characteristic);

    /**
     * Indicates that at least one of the characteristic's properties have changed.
     */
    void gattCharacteristicChanged(GattCharacteristicRemotePtr characteristic);

    /**
     * Indicates that services's uuid have changed.
     */
    void uuidChanged(const QString &uuid);

    /**
     * Indicates that services's primary state have changed.
     */
    void primaryChanged(bool primary);

    /**
     * Indicates that services's handle have changed.
     */
    void handleChanged(quint16 handle);

    /**
     * Indicates that object paths representing the included
     * services have changed.
     */
    void includesChanged(const QList<QDBusObjectPath> &includes);

private:
    BLUEZQT_NO_EXPORT explicit GattServiceRemote(const QString &path, const QVariantMap &properties, DevicePtr device);

    const std::unique_ptr<class GattServiceRemotePrivate> d;

    friend class GattServiceRemotePrivate;
    friend class DevicePrivate;
    friend class ManagerPrivate;
    friend class Adapter;
};

} // namespace BluezQt

#endif // BLUEZQT_GATTSERVICEREMOTE_H
