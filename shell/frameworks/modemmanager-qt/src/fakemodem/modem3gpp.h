/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_FAKE_MODEM_MODEM_3GPP_H
#define MODEMMANAGERQT_FAKE_MODEM_MODEM_3GPP_H

#include "generictypes.h"

#include <QDBusAbstractAdaptor>

class Modem3gpp : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.fakemodem.Modem.Modem3gpp")
public:
    explicit Modem3gpp(QObject *parent = nullptr);
    ~Modem3gpp() override;

    Q_PROPERTY(uint EnabledFacilityLocks READ enabledFacilityLocks)
    Q_PROPERTY(QString Imei READ imei)
    Q_PROPERTY(QString OperatorCode READ operatorCode)
    Q_PROPERTY(QString OperatorName READ operatorName)
    Q_PROPERTY(uint RegistrationState READ registrationState)
    Q_PROPERTY(uint SubscriptionState READ subscriptionState)

    uint enabledFacilityLocks() const;
    QString imei() const;
    QString operatorCode() const;
    QString operatorName() const;
    uint registrationState() const;
    uint subscriptionState() const;

    /* Not part of dbus interface */
    void setModemPath(const QString &path);
    void setEnableNotifications(bool enable);
    void setEnabledFacilityLocks(uint enabledFacilitiesLocks);
    void setImei(const QString &imei);
    void setOperatorCode(const QString &operatorCode);
    void setOperatorName(const QString &operatorName);
    void setRegistrationState(uint registrationState);
    void setSubscriptionState(uint subscriptionState);

    QVariantMap toMap() const;

public Q_SLOTS:
    Q_SCRIPTABLE void Register(const QString &operator_id);
    Q_SCRIPTABLE ModemManager::QVariantMapList Scan();

private:
    QString m_modemPath;
    bool m_enabledNotifications;
    uint m_enabledFacilityLocks;
    QString m_imei;
    QString m_operatorCode;
    QString m_operatorName;
    uint m_registrationState;
    uint m_subscriptionState;
};

#endif
