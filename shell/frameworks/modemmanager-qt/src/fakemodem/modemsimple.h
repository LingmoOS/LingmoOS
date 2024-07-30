/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_FAKE_MODEM_MODEM_SIMPLE_H
#define MODEMMANAGERQT_FAKE_MODEM_MODEM_SIMPLE_H

#include "generictypes.h"

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

class ModemSimple : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.fakemodem.Modem.Simple")
public:
    explicit ModemSimple(QObject *parent = nullptr);
    ~ModemSimple() override;

public Q_SLOTS:
    Q_SCRIPTABLE QDBusObjectPath Connect(const QVariantMap &properties);
    Q_SCRIPTABLE void Disconnect(const QDBusObjectPath &bearer);
    Q_SCRIPTABLE QVariantMap GetStatus();
};

#endif
