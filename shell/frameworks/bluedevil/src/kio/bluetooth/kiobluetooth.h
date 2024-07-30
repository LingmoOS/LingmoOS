/*  This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2010 Eduardo Robles Elvira <edulix@gmail.com>
    SPDX-FileCopyrightText: 2010 Rafael Fernández López <ereslibre@kde.org>
    SPDX-FileCopyrightText: 2010 UFO Coders <info@ufocoders.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kdedbluedevil.h"

#include <QLoggingCategory>
#include <QObject>
#include <QUrl>

#include <KIO/WorkerBase>

/**
 * @short This class implements a bluetooth kioworker that list devices and their services.
 */
class KioBluetoothPrivate;

class KioBluetooth : public QObject, public KIO::WorkerBase
{
    Q_OBJECT

public:
    KioBluetooth(const QByteArray &pool, const QByteArray &app);

    struct Service {
        QString name;
        QString icon;
        QString mimetype;
        QString uuid;
    };

    /**
     * As our kio does not perform any service action, but just list devices and their services, the
     * get function shall not do much other than setting a mimetype and returning some data that
     * could be useful for the mimetype handler.
     */
    KIO::WorkerResult get(const QUrl &url) override;

    /**
     * List current directory. There are two types of current directories in this kio:
     *
     * 1. First type, is the root dir, bluetooth:/. This directory is unique, and lists the remote
     *    devices that our default bluetooth adapter sees.
     * 2. Remote device directory (something like bluetoth:/00_12_34_56_6d_34). This directory lists
     *    the services provided by the given remote device.
     */
    KIO::WorkerResult listDir(const QUrl &url) override;

    KIO::WorkerResult stat(const QUrl &url) override;

    /**
     * As at the momento we don't handle more than one level url paths, @p setHost has not much
     * difference with @p listDir
     *
     */
    void setHost(const QString &hostname, quint16 port, const QString &user, const QString &pass) override;

    /**
     * Returns a list of supported service names corresponding to the given uuids list. If an uuid is
     * not found in the uuids list, it is not added to the list of service names.
     */
    QList<Service> getSupportedServices(const QStringList &uuids);

    /**
     * Called by @p Bluetooth::listDir to create a "Received Files" folder entry.
     */
    void listDownload();

    /**
     * Called by @p Bluetooth::listDir when listing root dir, bluetooth:/.
     */
    void listDevices();

    /**
     * Called by @p Bluetooth::listDir when listing a remote device (something like
     * bluetoth:/00_12_34_56_6d_34) services.
     */
    [[nodiscard]] KIO::WorkerResult listRemoteDeviceServices();

public Q_SLOTS:
    void listDevice(const DeviceInfo device);

private:
    /**
     * This is set to true when @p setHost is called to list a given remote device, like for example
     * 00:2a:5E:8e:6e:f5. If listing the remote devices (bluetooth:/ uri), it's set back to false.
     */
    bool m_hasCurrentHost;

    /**
     * This is set in @p setHost when it's called to list a given remote device like for example
     * 00:2a:5E:8e:6e:f5. We don't directly set @p currentHost in @p setHost because libbludevil might not
     * have ready the remote bluetooth device yet ready at that time (it.s being created by the call
     * to @p Solid::Control::BluetoothDevice::createBluetoothRemoteDevice .
     */
    QString m_currentHostname;

    /**
     * Uppercase colon separated address (ex. 00:2A:5E:8E:6E:F5)
     */
    QString m_currentHostAddress;

    /**
     * This is an array containing as key the uuid and as value the name of the service that the
     * given uuid represents, and a representative icon. It only contains the supported service names.
     */
    QMap<QString, Service> m_supportedServices;

    /**
     * KDED DBus interface, used to communicate to the daemon since we need some status (like connected)
     */
    org::kde::BlueDevil *m_kded;
};

Q_DECLARE_LOGGING_CATEGORY(BLUETOOTH)
