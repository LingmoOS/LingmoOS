/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_FAKE_MODEM_MODEM_3GPP_USSD_H
#define MODEMMANAGERQT_FAKE_MODEM_MODEM_3GPP_USSD_H

#include "generictypes.h"

#include <QDBusAbstractAdaptor>

class Modem3gppUssd : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.fakemodem.Modem.Modem3gpp.Ussd")
public:
    explicit Modem3gppUssd(QObject *parent = nullptr);
    ~Modem3gppUssd() override;

    Q_PROPERTY(QString NetworkNotification READ networkNotification)
    Q_PROPERTY(QString NetworkRequest READ networkRequest)
    Q_PROPERTY(uint State READ state)

    QString networkNotification() const;
    QString networkRequest() const;
    uint state() const;

    /* Not part of dbus interface */
    void setModemPath(const QString &path);
    void setEnableNotifications(bool enable);
    void setNetworkNotification(const QString &networkNotification);
    void setNetworkRequest(const QString &networkRequest);
    void setState(uint state);

    QVariantMap toMap() const;

public Q_SLOTS:
    Q_SCRIPTABLE void Cancel();
    Q_SCRIPTABLE QString Initiate(const QString &command);
    Q_SCRIPTABLE QString Respond(const QString &response);

private:
    QString m_modemPath;
    bool m_enabledNotifications;
    QString m_networkNotification;
    QString m_networkRequest;
    uint m_state;
};

#endif
