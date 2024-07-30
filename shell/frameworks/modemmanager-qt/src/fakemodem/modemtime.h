/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_FAKE_MODEM_MODEM_TIME_H
#define MODEMMANAGERQT_FAKE_MODEM_MODEM_TIME_H

#include "generictypes.h"

#include <QDBusAbstractAdaptor>

class ModemTime : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.fakemodem.Modem.Time")
public:
    explicit ModemTime(QObject *parent = nullptr);
    ~ModemTime() override;

    Q_PROPERTY(QVariantMap NetworkTimezone READ networkTimezone)

    QVariantMap networkTimezone() const;

    /* Not part of dbus interface */
    void setModemPath(const QString &path);
    void setEnableNotifications(bool enable);
    void setNetworkTimezone(const QVariantMap &timezone);

    QVariantMap toMap() const;

public Q_SLOTS:
    Q_SCRIPTABLE QString GetNetworkTime();

Q_SIGNALS: // SIGNALS
    Q_SCRIPTABLE void NetworkTimeChanged(const QString &time);

private:
    QString m_modemPath;
    bool m_enabledNotifications;
    QVariantMap m_timezone;
};

#endif
