/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2021 Ivan Podkurkov <podkiva2@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_GATTDESCRIPTORREMOTE_H
#define BLUEZQT_GATTDESCRIPTORREMOTE_H

#include <QObject>
#include <QMap>

#include "types.h"
#include "bluezqt_export.h"

namespace BluezQt
{

class GattCharacteristicRemote;
class PendingCall;

/**
 * @class BluezQt::GattDescriptorRemote gattdescriptorremote.h <BluezQt/GattDescriptorRemote>
 *
 * Bluetooth LE GATT descriptor.
 *
 * This class represents a Bluetooth LE GATT descriptor for the clients.
 */
class BLUEZQT_EXPORT GattDescriptorRemote : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString ubi READ ubi CONSTANT)
    Q_PROPERTY(QString uuid READ uuid NOTIFY uuidChanged)
    Q_PROPERTY(QByteArray value READ value NOTIFY valueChanged)
    Q_PROPERTY(QStringList flags READ flags NOTIFY flagsChanged)
    Q_PROPERTY(quint16 handle READ handle NOTIFY handleChanged)
    Q_PROPERTY(GattCharacteristicRemotePtr characteristic READ characteristic CONSTANT)


public:
    /**
     * Destroys a GattDescriptor object.
     */
    ~GattDescriptorRemote() override;

    /**
     * Returns a shared pointer from this.
     *
     * @return DevicePtr
     */
    GattDescriptorRemotePtr toSharedPtr() const;

    /**
     * Returns an UBI of the GATT descriptor.
     *
     * Example UBI: "/org/bluez/hci0/dev_40_79_6A_0C_39_75"
     *
     * @return UBI of descriptor
     */
    QString ubi() const;

    /**
     * Returns an uuid of the descriptor.
     *
     * @return uuid of the descriptor
     */
    QString uuid() const;

    /**
     * Returns an value of the descriptor.
     *
     * @return value of the descriptor
     */
    QByteArray value() const;

    /**
     * Returns flags the descriptor.
     *
     * @return flags of descriptor
     */
    QStringList flags() const;

    /**
     * Returns descriptor handle.
     *
     * @return qint16 descriptor handle
     */
    quint16 handle() const;

    /**
     * Sets the descriptor handle.
     *
     * @param handle descriptor handle
     * @return void pending call
     */
    PendingCall *setHandle(quint16 handle);

    /**
     * Returns a characteristic that owns that descriptor.
     *
     * @return characteristic of descriptor
     */
    GattCharacteristicRemotePtr characteristic() const;

public Q_SLOTS:
    /**
     * Read the value of the GATT descriptor.
     *
     * Issues a request to read the value of the descriptor and
     * returns the value if the	operation was successful.
     *
     * Possible errors: PendingCall::NotReady, PendingCall::Failed,
     *                  PendingCall::InProgress, PendingCall::AlreadyConnected
     *
     * @return QByteArray pending call
     */
    PendingCall *readValue(const QVariantMap &options);

    /**
     * Write the value of the GATT descriptor.
     *
     * Issues a request to write the value of the descriptor.
     *
     * Possible errors: PendingCall::NotReady, PendingCall::Failed,
     *                  PendingCall::InProgress, PendingCall::AlreadyConnected
     *
     * @return void pending call
     */
    PendingCall *writeValue(const QByteArray &value, const QVariantMap &options);

Q_SIGNALS:
    /**
     * Indicates that at least one of the descriptors's properties have changed.
     */
    void descriptorChanged(GattDescriptorRemotePtr descriptor);

    /**
     * Indicates that descriptor's uuid have changed.
     */
    void uuidChanged(const QString &uuid);

    /**
     * Indicates that descriptor's value have changed.
     */
    void valueChanged(const QByteArray value);

    /**
     * Indicates that descriptor's flags have changed.
     */
    void flagsChanged(QStringList flags);

    /**
     * Indicates that descriptor's handle have changed.
     */
    void handleChanged(quint16 handle);

private:
    BLUEZQT_NO_EXPORT explicit GattDescriptorRemote(const QString &path, const QVariantMap &properties, GattCharacteristicRemotePtr characteristic);

    const std::unique_ptr<class GattDescriptorRemotePrivate> d;

    friend class DevicePrivate;
    friend class GattServiceRemotePrivate;
    friend class GattCharacteristicRemotePrivate;
    friend class GattDescriptorRemotePrivate;
    friend class ManagerPrivate;
    friend class Adapter;
};

}  // namespace BluezQt

#endif // BLUEZQT_GATTDESCRIPTORREMOTE_H
