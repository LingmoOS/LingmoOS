// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DVPNSETTING_H
#define DVPNSETTING_H

#include "dnmsetting.h"
#include <QString>

DNETWORKMANAGER_BEGIN_NAMESPACE

class DVpnSettingPrivate;

class DVpnSetting : public DNMSetting
{
public:
    DVpnSetting();
    explicit DVpnSetting(const QSharedPointer<DVpnSetting> &other);
    ~DVpnSetting() override = default;

    QString name() const override;

    bool persistent() const;
    void setPersistent(bool persistent);

    void setServiceType(const QString &type);
    QString serviceType() const;

    uint timeout() const;
    void setTimeout(uint timeout);

    void setUsername(const QString &username);
    QString username() const;

    void setData(const QMap<QString, QString> &data);
    QMap<QString, QString> data() const;

    void setSecrets(const QMap<QString, QString> &secrets);
    QMap<QString, QString> secrets() const;

    void mapToSecrets(const QVariantMap &secrets) override;
    QVariantMap secretsToMap() const override;

    void stringMapTosecrets(const QMap<QString, QString> &map) override;
    QMap<QString, QString> secretsToStringMap() const override;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    QScopedPointer<DVpnSettingPrivate> d_ptr;

private:
    Q_DECLARE_PRIVATE(DVpnSetting)
};

DNETWORKMANAGER_END_NAMESPACE

#endif