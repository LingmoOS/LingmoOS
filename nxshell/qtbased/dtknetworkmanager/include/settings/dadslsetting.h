// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DADSLSETTING_H
#define DADSLSETTING_H

#include "dnmsetting.h"
#include <QString>

DNETWORKMANAGER_BEGIN_NAMESPACE

class DAdslSettingPrivate;

class DAdslSetting : public DNMSetting
{
public:
    enum class Protocol {
        Unknown = 0,
        Pppoa,
        Pppoe,
        Ipoatm,
    };
    enum class Encapsulation {
        Unknown = 0,
        Vcmux,
        Llc,
    };

    DAdslSetting();
    explicit DAdslSetting(const QSharedPointer<DAdslSetting> &other);
    ~DAdslSetting() override = default;

    QString name() const override;

    void setUsername(const QString &username);
    QString username() const;

    void setPassword(const QString &password);
    QString password() const;

    void setPasswordFlags(SecretFlags flags);
    SecretFlags passwordFlags() const;

    void setProtocol(Protocol protocol);
    Protocol protocol() const;

    void setEncapsulation(Encapsulation encapsulation);
    Encapsulation encapsulation() const;

    void setVpi(quint32 vpi);
    quint32 vpi() const;

    void setVci(quint32 vci);
    quint32 vci() const;

    QStringList needSecrets(bool requestNew = false) const override;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    QScopedPointer<DAdslSettingPrivate> d_ptr;

private:
    Q_DECLARE_PRIVATE(DAdslSetting)
};

DNETWORKMANAGER_END_NAMESPACE

#endif
