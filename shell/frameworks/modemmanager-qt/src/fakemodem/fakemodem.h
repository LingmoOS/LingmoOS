/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_FAKE_MODEM_H
#define MODEMMANAGERQT_FAKE_MODEM_H

#include "bearer.h"
#include "modem.h"
#include "objectmanager.h"

#include <QDBusObjectPath>
#include <QObject>

class FakeModem : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.fakemodem")
public:
    explicit FakeModem(QObject *parent = nullptr);
    ~FakeModem() override;

    /* Not part of DBus interface */
    void addModem(Modem *modem);
    void removeModem(Modem *modem);
    void addBearer(Bearer *bearer);
    void removeBearer(Bearer *bearer);
    void addInterfaces(const QDBusObjectPath &object_path, const ModemManager::MMVariantMapMap &interfaces_and_properties);
    void removeInterfaces(const QDBusObjectPath &object_path, const QStringList &interfaces);

public Q_SLOTS:
    Q_SCRIPTABLE void ScanDevices();
    Q_SCRIPTABLE void SetLogging(const QString &level);

private:
    /* Not part of DBus interface */
    int m_bearerCounter;
    int m_modemCounter;
    QMap<QDBusObjectPath, Bearer *> m_bearers;
    QMap<QDBusObjectPath, Modem *> m_modems;
    ObjectManager *m_objectManager;
};

#endif
