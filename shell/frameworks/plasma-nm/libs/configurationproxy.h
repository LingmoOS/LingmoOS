/*
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef CONFIGURATIONPROXY_H
#define CONFIGURATIONPROXY_H

#include "plasmanm_internal_export.h"

#include "configuration.h"
#include <QObject>
#include <QPointer>

#include <qqmlregistration.h>

class PLASMANM_INTERNAL_EXPORT ConfigurationProxy : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_NAMED_ELEMENT(Configuration)
    QML_SINGLETON
    Q_PROPERTY(bool unlockModemOnDetection READ unlockModemOnDetection WRITE setUnlockModemOnDetection)
    Q_PROPERTY(bool manageVirtualConnections READ manageVirtualConnections WRITE setManageVirtualConnections NOTIFY manageVirtualConnectionsChanged)
    Q_PROPERTY(bool airplaneModeEnabled READ airplaneModeEnabled WRITE setAirplaneModeEnabled NOTIFY airplaneModeEnabledChanged)
    Q_PROPERTY(QString hotspotName READ hotspotName WRITE setHotspotName)
    Q_PROPERTY(QString hotspotPassword READ hotspotPassword WRITE setHotspotPassword)
    Q_PROPERTY(QString hotspotConnectionPath READ hotspotConnectionPath WRITE setHotspotConnectionPath)

    // Readonly constant property, as this value should only be set by the platform
    Q_PROPERTY(bool showPasswordDialog READ showPasswordDialog CONSTANT)
public:
    ConfigurationProxy(QObject *parent = nullptr);
    bool unlockModemOnDetection() const;
    void setUnlockModemOnDetection(bool unlock);

    bool manageVirtualConnections() const;
    void setManageVirtualConnections(bool manage);

    bool airplaneModeEnabled() const;
    void setAirplaneModeEnabled(bool enabled);

    QString hotspotName() const;
    void setHotspotName(const QString &name);

    QString hotspotPassword() const;
    void setHotspotPassword(const QString &password);

    QString hotspotConnectionPath() const;
    void setHotspotConnectionPath(const QString &path);

    bool showPasswordDialog() const;

    bool systemConnectionsByDefault() const;

Q_SIGNALS:
    void airplaneModeEnabledChanged();
    void manageVirtualConnectionsChanged(bool manage);

private:
    const QPointer<Configuration> mConfiguration;
};

#endif // CONFIGURATIONPROXY_H
