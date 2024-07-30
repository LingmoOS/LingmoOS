/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_GATTAPPLICATION_H
#define BLUEZQT_GATTAPPLICATION_H

#include <QObject>

#include "bluezqt_export.h"

#include <memory>

class QDBusObjectPath;

namespace BluezQt
{
/**
 * @class BluezQt::GattApplication GattApplication.h <BluezQt/GattApplication>
 *
 * Bluetooth GattApplication.
 *
 * This class represents a Bluetooth GattApplication, which is the root node of
 * a GATT object hierarchy. Its child nodes can be GattServices,
 * GattCharacteristics and GattDescriptors that belong to that GattApplication.
 * The object path prefix for GattApplications is freely definable and its
 * children's paths follow the application path hierarchy automatically, while
 * all instances are enumerated automatically as well.
 *
 * Object path: [variable prefix]/appXX/serviceYY/charZZ
 *
 */
class BLUEZQT_EXPORT GattApplication : public QObject
{
    Q_OBJECT

public:
    /**
     * Creates a new GattApplication object with default object path prefix.
     *
     * Object path: /org/kde/bluezqt/appXX/serviceYY/charZZ
     *
     * @param parent
     */
    explicit GattApplication(QObject *parent = nullptr);

    /**
     * Creates a new GattApplication object with custom object path prefix.
     *
     * Object path: [objectPathPrefix]/appXX/serviceYY/charZZ
     *
     * @param objectPathPrefix
     * @param parent
     */
    explicit GattApplication(const QString &objectPathPrefix, QObject *parent = nullptr);

    /**
     * Destroys a GattApplication object.
     */
    ~GattApplication() override;

private:
    /**
     * D-Bus object path of the GATT application.
     *
     * The path where the GATT application will be registered.
     *
     * @note You must provide valid object path!
     *
     * @return object path of GATT application
     */
    virtual QDBusObjectPath objectPath() const;

    std::unique_ptr<class GattApplicationPrivate> const d;

    friend class GattManager;
    friend class GattService;
    friend class ObjectManagerAdaptor;
};

} // namespace BluezQt

#endif
