// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTUNSETTING_H
#define DTUNSETTING_H

#include "dnmsetting.h"
#include <QString>

DNETWORKMANAGER_BEGIN_NAMESPACE

class DTunSettingPrivate;

class DTunSetting : public DNMSetting
{
public:
    enum class Mode {
        Tun,
        Tap,
    };

    DTunSetting();
    explicit DTunSetting(const QSharedPointer<DTunSetting> &other);
    ~DTunSetting() override = default;

    QString name() const override;

    void setGroup(const QString &group);
    QString group() const;

    void setMode(Mode mode);
    Mode mode() const;

    void setMultiQueue(bool multiQueue);
    bool multiQueue() const;

    void setOwner(const QString &owner);
    QString owner() const;

    void setPi(bool pi);
    bool pi() const;

    void setVnetHdr(bool vnetHdr);
    bool vnetHdr() const;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    QScopedPointer<DTunSettingPrivate> d_ptr;

private:
    Q_DECLARE_PRIVATE(DTunSetting)
};

DNETWORKMANAGER_END_NAMESPACE

#endif