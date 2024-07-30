/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2022 Pontus Sjögren
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef GATTDESCRIPTOR_H
#define GATTDESCRIPTOR_H

#include <QDBusObjectPath>
#include <QObject>

#include "bluezqt_export.h"

#include <memory>

namespace BluezQt
{

class GattCharacteristic;

/**
 * Bluetooth GATT Descriptor
 *
 * GATT Descriptors contain additional information and attributes of a GATT characteristic.

 * @since 6.0
 */
class BLUEZQT_EXPORT GattDescriptor : public QObject
{
    Q_OBJECT

public:
    /**
     * Convenience method to create a User Description for the given charactersitic.
     *
     * @param description The User Description the characteristic should have
     * @param characteristic The characteristic to assign the descriptor to
     * @return A pointer to the created descriptor
     */
    static GattDescriptor *createUserDescription(const QString &description, GattCharacteristic *characteristic);

    /**
     * Creates a GattDescriptor with the given UUID.
     *
     * @param uuid UUID of the descriptor
     * @param parent Parent characteristic
     */
    GattDescriptor(const QString &uuid, GattCharacteristic *parent);

    /**
     * Creates a GattDescriptor with the given UUID and flags.
     *
     * @param uuid UUID of the descriptor
     * @param flags The flags of the descriptor
     * @param parent Parent characteristic
     */
    GattDescriptor(const QString &uuid, const QStringList &flags, GattCharacteristic *parent);

    /**
     * Creates a GattDescriptor with the given UUID, flags and initial value.
     *
     * @param uuid UUID of the descriptor
     * @param flags The flags of the descriptor
     * @param initialValue The value of the descriptor
     * @param parent Parent characteristic
     */
    GattDescriptor(const QString &uuid, const QStringList &flags, const QByteArray &initialValue, GattCharacteristic *parent);

    /**
     * Destroys the GattDescriptor.
     */
    ~GattDescriptor() override;

    /**
     * Reads the current value of the descriptor.
     *
     * @return A QByteArray representing the current value
     */
    QByteArray readValue();

    /**
     * Writes the value of the descriptor.
     *
     * @param value A QByteArray representing the new value
     */
    void writeValue(const QByteArray &value);

    /**
     * Returns the UUID of the descriptor.
     *
     * @return A QString representing the UUID
     */
    QString uuid() const;

    /**
     * Return the DBus object path of the parent characteristic.
     *
     * @return A QDBusObjectPath representing the DBus object path
     */
    QDBusObjectPath characteristic() const;

    /**
     * Return the flags of the descriptor.
     *
     * @return A QStringList representing the flags
     */
    QStringList flags() const;

protected:
    virtual QDBusObjectPath objectPath() const;

private:
    std::unique_ptr<class GattDescriptorPrivate> const d;

    friend class GattManager;
    friend class GattApplicationPrivate;
};

}

#endif
