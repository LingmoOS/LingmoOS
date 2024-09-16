// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DGENERICSETTING_H
#define DGENERICSETTING_H

#include "dnmsetting.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DGenericSettingPrivate;

class DGenericSetting : public DNMSetting
{
public:
    DGenericSetting();
    explicit DGenericSetting(QSharedPointer<DGenericSetting> &other);
    explicit DGenericSetting(const QSharedPointer<DGenericSetting> &other);
    ~DGenericSetting() override = default;

    QString name() const override;

    QVariantMap toMap() const override;

    void fromMap(const QVariantMap &map) override;

protected:
    QScopedPointer<DGenericSettingPrivate> d_ptr;

private:
    Q_DECLARE_PRIVATE(DGenericSetting)
};

DNETWORKMANAGER_END_NAMESPACE
#endif
