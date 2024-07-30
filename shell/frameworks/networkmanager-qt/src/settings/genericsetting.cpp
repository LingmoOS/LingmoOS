/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "genericsetting.h"
#include "genericsetting_p.h"

#include "nmdebug.h"

NetworkManager::GenericSettingPrivate::GenericSettingPrivate()
    : name(NM_SETTING_GENERIC_SETTING_NAME)
{
}

NetworkManager::GenericSetting::GenericSetting()
    : Setting(Setting::Generic)
    , d_ptr(new GenericSettingPrivate())
{
}

NetworkManager::GenericSetting::GenericSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new GenericSettingPrivate())
{
}

NetworkManager::GenericSetting::~GenericSetting()
{
    delete d_ptr;
}

QString NetworkManager::GenericSetting::name() const
{
    Q_D(const GenericSetting);

    return d->name;
}

void NetworkManager::GenericSetting::fromMap(const QVariantMap &setting)
{
    Q_UNUSED(setting);
}

QVariantMap NetworkManager::GenericSetting::toMap() const
{
    QVariantMap setting;

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::GenericSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    return dbg.maybeSpace();
}
