/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_CDMA_SETTINGS_H
#define NETWORKMANAGERQT_CDMA_SETTINGS_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QString>

namespace NetworkManager
{
class CdmaSettingPrivate;

/**
 * Represents cdma setting
 */
class NETWORKMANAGERQT_EXPORT CdmaSetting : public Setting
{
public:
    typedef QSharedPointer<CdmaSetting> Ptr;
    typedef QList<Ptr> List;
    CdmaSetting();
    explicit CdmaSetting(const Ptr &other);
    ~CdmaSetting() override;

    QString name() const override;

    void setNumber(const QString &number);
    QString number() const;

    void setUsername(const QString &username);
    QString username() const;

    void setPassword(const QString &password);
    QString password() const;

    void setPasswordFlags(SecretFlags flags);
    SecretFlags passwordFlags() const;

    QStringList needSecrets(bool requestNew = false) const override;

    void secretsFromMap(const QVariantMap &secrets) override;

    QVariantMap secretsToMap() const override;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    CdmaSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(CdmaSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const CdmaSetting &setting);

}

#endif // NETWORKMANAGERQT_CDMA_SETTINGS_H
