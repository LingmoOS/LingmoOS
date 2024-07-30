/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_MACSECSETTING_H
#define NETWORKMANAGERQT_MACSECSETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

namespace NetworkManager
{
class MacsecSettingPrivate;

/**
 * Represents Macsec setting
 */
class NETWORKMANAGERQT_EXPORT MacsecSetting : public Setting
{
public:
    typedef QSharedPointer<MacsecSetting> Ptr;
    typedef QList<Ptr> List;

    enum Mode {
        Psk,
        Eap,
    };

    enum Validation {
        Disable,
        Check,
        Strict,
    };

    MacsecSetting();
    explicit MacsecSetting(const Ptr &other);
    ~MacsecSetting() override;

    QString name() const override;

    void setEncrypt(bool encrypt);
    bool encrypt() const;

    void setMkaCak(const QString &mkaCak);
    QString mkaCak() const;

    void setMkaCkn(const QString &mkaCkn);
    QString mkaCkn() const;

    void setMode(Mode mode);
    Mode mode() const;

    void setParent(const QString &parent);
    QString parent() const;

    void setPort(qint32 port);
    qint32 port() const;

    void setSendSci(bool sendSci);
    bool sendSci() const;

    void setValidation(Validation validation);
    Validation validation() const;

    void setMkaCakFlags(Setting::SecretFlags flags);
    Setting::SecretFlags mkaCakFlags() const;

    QStringList needSecrets(bool requestNew = false) const override;

    void secretsFromMap(const QVariantMap &secrets) override;

    QVariantMap secretsToMap() const override;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    MacsecSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(MacsecSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const MacsecSetting &setting);

}

#endif // NETWORKMANAGERQT_MACSECSETTING_H
