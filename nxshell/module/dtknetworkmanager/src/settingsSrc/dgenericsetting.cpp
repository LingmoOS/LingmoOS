// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dgenericsetting_p.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

DGenericSettingPrivate::DGenericSettingPrivate()
    : m_name(NM_SETTING_GENERIC_SETTING_NAME)
{
}

DGenericSetting::DGenericSetting()
    : DNMSetting(DNMSetting::SettingType::Generic)
    , d_ptr(new DGenericSettingPrivate())
{
}

DGenericSetting::DGenericSetting(const QSharedPointer<DGenericSetting> &other)
    : DNMSetting(other)
    , d_ptr(new DGenericSettingPrivate())
{
}

QString DGenericSetting::name() const
{
    Q_D(const DGenericSetting);

    return d->m_name;
}

QVariantMap DGenericSetting::toMap() const
{
    return {};
}

void DGenericSetting::fromMap(const QVariantMap &map)
{
    Q_UNUSED(map)
}

DNETWORKMANAGER_END_NAMESPACE
