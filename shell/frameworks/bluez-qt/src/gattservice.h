/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_GATTSERVICE_H
#define BLUEZQT_GATTSERVICE_H

#include "bluezqt_export.h"
#include "types.h"

#include <QDBusObjectPath>

#include <memory>

namespace BluezQt
{
/**
 * @class BluezQt::GattService GattService.h <BluezQt/GattService>
 *
 * Bluetooth GattService.
 *
 * This class represents a Bluetooth GattService.
 */
class GattApplication;
class BLUEZQT_EXPORT GattService : public QObject
{
    Q_OBJECT

public:
    /**
     * Creates a new GattService object.
     *
     * @param parent
     */
    explicit GattService(const QString &uuid, bool isPrimary, GattApplication *parent);

    /**
     * Destroys a GattService object.
     */
    ~GattService() override;

    /**
     * 128-bit service UUID.
     *
     * @return uuid of gatt service
     */
    QString uuid() const;

    /**
     * Indicates whether or not this GATT service is a
     * primary service. If false, the service is secondary.
     *
     * @return true if gatt service is primary
     */
    bool isPrimary() const;

protected:
    /**
     * D-Bus object path of the GattService.
     *
     * The path where the GattService will be registered.
     *
     * @note You must provide valid object path!
     *
     * @return object path of GattService
     */
    virtual QDBusObjectPath objectPath() const;

private:
    std::unique_ptr<class GattServicePrivate> const d;

    friend class GattApplicationPrivate;
    friend class GattCharacterisiticPrivate;
    friend class GattCharacteristicAdaptor;
    friend class GattManager;
};

} // namespace BluezQt

#endif
