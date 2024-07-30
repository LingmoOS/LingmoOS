/*
    SPDX-FileCopyrightText: Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "usersetting.h"
#include "usersetting_p.h"

#include <QDebug>

#if !NM_CHECK_VERSION(1, 8, 0)
#define NM_SETTING_USER_SETTING_NAME "user"
#define NM_SETTING_USER_DATA "data"
#endif

NetworkManager::UserSettingPrivate::UserSettingPrivate()
    : name(NM_SETTING_USER_SETTING_NAME)
{
}

NetworkManager::UserSetting::UserSetting()
    : Setting(Setting::User)
    , d_ptr(new UserSettingPrivate())
{
}

NetworkManager::UserSetting::UserSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new UserSettingPrivate())
{
    setData(other->data());
}

NetworkManager::UserSetting::~UserSetting()
{
    delete d_ptr;
}

QString NetworkManager::UserSetting::name() const
{
    Q_D(const UserSetting);

    return d->name;
}

void NetworkManager::UserSetting::addData(const QString &data, const QString &value)
{
    Q_D(UserSetting);

    d->data.insert(data, value);
}

void NetworkManager::UserSetting::setData(const NMStringMap &data)
{
    Q_D(UserSetting);

    d->data = data;
}

NMStringMap NetworkManager::UserSetting::data() const
{
    Q_D(const UserSetting);

    return d->data;
}

void NetworkManager::UserSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_USER_DATA))) {
        setData(qdbus_cast<NMStringMap>(setting.value(QLatin1String(NM_SETTING_USER_DATA))));
    }
}

QVariantMap NetworkManager::UserSetting::toMap() const
{
    QVariantMap setting;

    if (!data().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_USER_DATA), QVariant::fromValue<NMStringMap>(data()));
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::UserSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_USER_DATA << ": " << setting.data() << '\n';

    return dbg.maybeSpace();
}
