/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2021 Ivan Podkurkov <podkiva2@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_GATTCHARACTERISTICREMOTE_H
#define BLUEZQT_GATTCHARACTERISTICREMOTE_H

#include "bluezqt_export.h"
#include "gattdescriptorremote.h"
#include "types.h"
#include <QList>
#include <QMap>
#include <QObject>
namespace BluezQt
{

class GattServiceRemote;
class PendingCall;

/**
 * @class BluezQt::GattCharacteristicRemote gattcharacteristicremote.h <BluezQt/GattCharacteristicRemote>
 *
 * Bluetooth LE GATT characteristic.
 *
 * This class represents a Bluetooth LE GATT characteristic for the clients.
 */
class BLUEZQT_EXPORT GattCharacteristicRemote : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString ubi READ ubi CONSTANT)
    Q_PROPERTY(QString uuid READ uuid NOTIFY uuidChanged)
    Q_PROPERTY(QByteArray value READ value NOTIFY valueChanged)
    Q_PROPERTY(bool writeAcquired READ isWriteAcquired NOTIFY writeAcquiredChanged)
    Q_PROPERTY(bool notifyAcquired READ isNotifyAcquired NOTIFY notifyAcquiredChanged)
    Q_PROPERTY(bool notifying READ isNotifying NOTIFY notifyingChanged)
    Q_PROPERTY(QStringList flags READ flags NOTIFY flagsChanged)
    Q_PROPERTY(quint16 handle READ handle NOTIFY handleChanged)
    Q_PROPERTY(quint16 MTU READ MTU NOTIFY MTUChanged)
    Q_PROPERTY(GattServiceRemotePtr service READ service CONSTANT)
    Q_PROPERTY(QList<GattDescriptorRemotePtr> descriptors READ descriptors NOTIFY descriptorsChanged)


public:
    /**
     * Destroys a GattCharacteristic object.
     */
    ~GattCharacteristicRemote() override;

    /**
     * Returns a shared pointer from this.
     *
     * @return DevicePtr
     */
    GattCharacteristicRemotePtr toSharedPtr() const;

    /**
     * Returns an UBI of the GATT characteristic.
     *
     * Example UBI: "/org/bluez/hci0/dev_40_79_6A_0C_39_75"
     *
     * @return UBI of device
     */
    QString ubi() const;

    /**
     * Returns an uuid of the characteristic.
     *
     * @return uuid of the characteristic
     */
    QString uuid() const;

    /**
     * Returns an value of the characteristic.
     *
     * @return value of the characteristic
     */
    QByteArray value() const;

    /**
     * Returns whether writeAcquired for the characteristic.
     *
     * @return true if write is acquired
     */
    bool isWriteAcquired() const;

    /**
     * Returns whether notifyAcquired for the characteristic.
     *
     * @return true if notify is acquired
     */
    bool isNotifyAcquired() const;

    /**
     * Returns whether the characteristic is notifying.
     *
     * @return true if notifying
     */
    bool isNotifying() const;

    /**
     * Returns flags the characteristic.
     *
     * @return flags of characteristic
     */
    QStringList flags() const;

    /**
     * Returns characteristic handle.
     *
     * @return qint16 characteristic handle
     */
    quint16 handle() const;

    /**
     * Sets the characteristic handle.
     *
     * @param handle characteristic handle
     * @return void pending call
     */
    PendingCall *setHandle(quint16 handle);

    /**
     * Returns characteristic MTU.
     *
     * @return qint16 characteristic MTU
     */
    quint16 MTU() const;

    /**
     * Returns a service that owns that characteristic.
     *
     * @return service of characteristic
     */
    GattServiceRemotePtr service() const;

    /**
     * Returns object paths representing the included
     * services of this service.
     *
     * @return Object paths of included services
     */
    QList<GattDescriptorRemotePtr> descriptors() const;

public Q_SLOTS:
    /**
     * Read the value of the GATT characteristic.
     *
     * Issues a request to read the value of the characteristic and
     * returns the value if the	operation was successful.
     *
     * Possible errors: PendingCall::NotReady, PendingCall::Failed,
     *                  PendingCall::InProgress, PendingCall::AlreadyConnected
     *
     * @return QByteArray pending call
     */
    PendingCall *readValue(const QVariantMap &options);

    /**
     * Write the value of the GATT characteristic.
     *
     * Issues a request to write the value of the characteristic.
     *
     * Possible errors: PendingCall::NotReady, PendingCall::Failed,
     *                  PendingCall::InProgress, PendingCall::AlreadyConnected
     *
     * @return void pending call
     */
    PendingCall *writeValue(const QByteArray &value, const QVariantMap &options);

    /**
     * Start notifying the value of the GATT characteristic.
     *
     * Starts a notification session from this characteristic if it supports
     * value notifications or indications.
     *
     * Possible errors: PendingCall::NotReady, PendingCall::Failed,
     *                  PendingCall::InProgress, PendingCall::AlreadyConnected
     *
     * @return void pending call
     */
    PendingCall *startNotify();

    /**
     * Stop notifying the value of the GATT characteristic.
     *
     * This method will cancel any previous StartNotify transaction.
     * Note that notifications from a characteristic are shared between
     * sessions thus calling StopNotify will release a single session.
     *
     * Possible errors: PendingCall::NotReady, PendingCall::Failed,
     *                  PendingCall::InProgress, PendingCall::AlreadyConnected
     *
     * @return void pending call
     */
    PendingCall *stopNotify();

    /**
     * Confirmation that value of the characteristic was received.
     *
     * This method doesn't expect a reply so it is just a confirmation
     * that value was received.
     *
     * Possible errors: PendingCall::NotReady, PendingCall::Failed,
     *                  PendingCall::InProgress, PendingCall::AlreadyConnected
     *
     * @return void pending call
     */
    PendingCall *confirm();

Q_SIGNALS:
    /**
     * Indicates that at least one of the characteristic's properties have changed.
     */
    void characteristicChanged(GattCharacteristicRemotePtr characteristic);

    /**
     * Indicates that a new descriptor was added (eg. found by connection).
     */
    void gattDescriptorAdded(GattDescriptorRemotePtr descriptor);

    /**
     * Indicates that characteristic descriptors list has changed
     */
    void descriptorsChanged(QList<GattDescriptorRemotePtr> descriptors);

    /**
     * Indicates that a descriptor was removed.
     */
    void gattDescriptorRemoved(GattDescriptorRemotePtr descriptor);

    /**
     * Indicates that at least one of the descriptor's properties have changed.
     */
    void gattDescriptorChanged(GattDescriptorRemotePtr descriptor);

    /**
     * Indicates that characteristic's uuid have changed.
     */
    void uuidChanged(const QString &uuid);

    /**
     * Indicates that characteristic's value have changed.
     */
    void valueChanged(const QByteArray value);

    /**
     * Indicates that characteristic's writeAcquired state have changed.
     */
    void writeAcquiredChanged(bool writeAcquired);

    /**
     * Indicates that characteristic's notifyAcquired state have changed.
     */
    void notifyAcquiredChanged(bool notifyAcquired);

    /**
     * Indicates that characteristic's notifying state have changed.
     */
    void notifyingChanged(bool notifying);

    /**
     * Indicates that characteristic's flags have changed.
     */
    void flagsChanged(QStringList flags);

    /**
     * Indicates that characteristic's handle have changed.
     */
    void handleChanged(quint16 handle);

    /**
     * Indicates that characteristic's MTU have changed.
     */
    void MTUChanged(quint16 MTU);

private:
    BLUEZQT_NO_EXPORT explicit GattCharacteristicRemote(const QString &path, const QVariantMap &properties, GattServiceRemotePtr service);

    const std::unique_ptr<class GattCharacteristicRemotePrivate> d;

    friend class DevicePrivate;
    friend class GattServiceRemotePrivate;
    friend class GattCharacteristicRemotePrivate;
    friend class ManagerPrivate;
    friend class Adapter;
};

} // namespace BluezQt

#endif // BLUEZQT_GATTCHARACTERISTICREMOTE_H
