/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_GATTMANAGER_H
#define BLUEZQT_GATTMANAGER_H

#include <QObject>

#include "bluezqt_export.h"

#include <memory>

namespace BluezQt
{
class GattApplication;
class PendingCall;

/**
 * @class BluezQt::GattManager GattManager.h <BluezQt/GattManager>
 *
 * Bluetooth GattManager.
 *
 * GATT Manager allows external applications to register GATT services and
 * profiles.
 *
 * Registering a profile allows applications to subscribe to *remote* services.
 * These must implement the GattProfile1 interface defined above.
 *
 * Registering a service allows applications to publish a *local* GATT service,
 * which then becomes available to remote devices. A GATT service is represented by
 * a D-Bus object hierarchy where the root node corresponds to a service and the
 * child nodes represent characteristics and descriptors that belong to that
 * service. Each node must implement one of GattService1, GattCharacteristic1,
 * or GattDescriptor1 interfaces described above, based on the attribute it
 * represents. Each node must also implement the standard D-Bus Properties
 * interface to expose their properties. These objects collectively represent a
 * GATT service definition.
 *
 * @see GattApplication
 */
class BLUEZQT_EXPORT GattManager : public QObject
{
    Q_OBJECT

public:
    /**
     * Destroys a GattManager object.
     */
    ~GattManager() override;

    /**
     * Registers a local GATT services hierarchy as described
     * above (GATT Server) and/or GATT profiles (GATT Client).
     *
     * The application object path together with the D-Bus
     * system bus connection ID define the identification of
     * the application registering a GATT based
     * service or profile.
     *
     * Possible errors: org.bluez.Error.InvalidArguments
     *             org.bluez.Error.AlreadyExists
     *
     * @param application application to be registered
     * @return void pending call
     */
    PendingCall *registerApplication(GattApplication *application);

    /**
     * This unregisters the services that has been
     * previously registered. The object path parameter
     * must match the same value that has been used
     * on registration.
     *
     * Possible errors: org.bluez.Error.InvalidArguments
     *             org.bluez.Error.DoesNotExist
     *
     * @param application application to be unregistered
     * @return void pending call
     */
    PendingCall *unregisterApplication(GattApplication *application);

private:
    BLUEZQT_NO_EXPORT explicit GattManager(const QString &path, QObject *parent = nullptr);

    std::unique_ptr<class GattManagerPrivate> const d;

    friend class AdapterPrivate;
};

} // namespace BluezQt

#endif
