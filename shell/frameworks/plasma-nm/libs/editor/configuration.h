/*
    SPDX-FileCopyrightText: 2017 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_CONFIGURATION_H
#define PLASMA_NM_CONFIGURATION_H

#include "plasmanm_editor_export.h"

#include <NetworkManagerQt/Manager>
#include <QMutex>
#include <QObject>

class PLASMANM_EDITOR_EXPORT Configuration : public QObject
{
    Q_OBJECT
public:
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

    static Configuration &self();

Q_SIGNALS:
    void airplaneModeEnabledChanged();
    void manageVirtualConnectionsChanged(bool manage);

private:
    Configuration() = default;
    static QMutex sMutex;
};

#endif // PLAMA_NM_CONFIGURATION_H
