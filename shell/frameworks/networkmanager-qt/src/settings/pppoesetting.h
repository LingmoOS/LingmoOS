/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_PPPOE_SETTING_H
#define NETWORKMANAGERQT_PPPOE_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QString>

namespace NetworkManager
{
class PppoeSettingPrivate;

/**
 * Represents pppoe setting
 */
class NETWORKMANAGERQT_EXPORT PppoeSetting : public Setting
{
public:
    typedef QSharedPointer<PppoeSetting> Ptr;
    typedef QList<Ptr> List;
    PppoeSetting();
    explicit PppoeSetting(const Ptr &other);
    ~PppoeSetting() override;

    QString name() const override;

    void setService(const QString &service);
    QString service() const;

    void setParent(const QString &parent);
    QString parent() const;

    void setUsername(const QString &username);
    QString username() const;

    void setPassword(const QString &password);
    QString password() const;

    void setPasswordFlags(Setting::SecretFlags flags);
    Setting::SecretFlags passwordFlags() const;

    QStringList needSecrets(bool requestNew = false) const override;

    void secretsFromMap(const QVariantMap &secrets) override;

    QVariantMap secretsToMap() const override;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    PppoeSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(PppoeSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const PppoeSetting &setting);

}

#endif // NETWORKMANAGERQT_PPPOE_SETTING_H
