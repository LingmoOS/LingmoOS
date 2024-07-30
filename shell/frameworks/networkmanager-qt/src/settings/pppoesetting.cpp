/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "pppoesetting.h"
#include "pppoesetting_p.h"

#include <QDebug>

NetworkManager::PppoeSettingPrivate::PppoeSettingPrivate()
    : name(NM_SETTING_PPPOE_SETTING_NAME)
    , passwordFlags(Setting::None)
{
}

NetworkManager::PppoeSetting::PppoeSetting()
    : Setting(Setting::Pppoe)
    , d_ptr(new PppoeSettingPrivate())
{
}

NetworkManager::PppoeSetting::PppoeSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new PppoeSettingPrivate())
{
    setService(other->service());
    setUsername(other->username());
    setPassword(other->password());
    setPasswordFlags(other->passwordFlags());
}

NetworkManager::PppoeSetting::~PppoeSetting()
{
    delete d_ptr;
}

QString NetworkManager::PppoeSetting::name() const
{
    Q_D(const PppoeSetting);

    return d->name;
}

void NetworkManager::PppoeSetting::setService(const QString &service)
{
    Q_D(PppoeSetting);

    d->service = service;
}

QString NetworkManager::PppoeSetting::service() const
{
    Q_D(const PppoeSetting);

    return d->service;
}

void NetworkManager::PppoeSetting::setParent(const QString &parent)
{
    Q_D(PppoeSetting);

    d->parent = parent;
}

QString NetworkManager::PppoeSetting::parent() const
{
    Q_D(const PppoeSetting);

    return d->parent;
}

void NetworkManager::PppoeSetting::setUsername(const QString &username)
{
    Q_D(PppoeSetting);

    d->username = username;
}

QString NetworkManager::PppoeSetting::username() const
{
    Q_D(const PppoeSetting);

    return d->username;
}

void NetworkManager::PppoeSetting::setPassword(const QString &password)
{
    Q_D(PppoeSetting);

    d->password = password;
}

QString NetworkManager::PppoeSetting::password() const
{
    Q_D(const PppoeSetting);

    return d->password;
}

void NetworkManager::PppoeSetting::setPasswordFlags(NetworkManager::Setting::SecretFlags flags)
{
    Q_D(PppoeSetting);

    d->passwordFlags = flags;
}

NetworkManager::Setting::SecretFlags NetworkManager::PppoeSetting::passwordFlags() const
{
    Q_D(const PppoeSetting);

    return d->passwordFlags;
}

QStringList NetworkManager::PppoeSetting::needSecrets(bool requestNew) const
{
    QStringList secrets;

    if ((password().isEmpty() || requestNew) && !passwordFlags().testFlag(Setting::NotRequired)) {
        secrets << QLatin1String(NM_SETTING_PPPOE_PASSWORD);
    }

    return secrets;
}

void NetworkManager::PppoeSetting::secretsFromMap(const QVariantMap &secrets)
{
    if (secrets.contains(QLatin1String(NM_SETTING_PPPOE_PASSWORD))) {
        setPassword(secrets.value(QLatin1String(NM_SETTING_PPPOE_PASSWORD)).toString());
    }
}

QVariantMap NetworkManager::PppoeSetting::secretsToMap() const
{
    QVariantMap secrets;

    if (!password().isEmpty()) {
        secrets.insert(QLatin1String(NM_SETTING_PPPOE_PASSWORD), password());
    }

    return secrets;
}

void NetworkManager::PppoeSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_PPPOE_SERVICE))) {
        setService(setting.value(QLatin1String(NM_SETTING_PPPOE_SERVICE)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_PPPOE_USERNAME))) {
        setUsername(setting.value(QLatin1String(NM_SETTING_PPPOE_USERNAME)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_PPPOE_PASSWORD))) {
        setPassword(setting.value(QLatin1String(NM_SETTING_PPPOE_PASSWORD)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_PPPOE_PASSWORD_FLAGS))) {
        setPasswordFlags((Setting::SecretFlags)setting.value(QLatin1String(NM_SETTING_PPPOE_PASSWORD_FLAGS)).toUInt());
    }
}

QVariantMap NetworkManager::PppoeSetting::toMap() const
{
    QVariantMap setting;

    if (!service().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_PPPOE_SERVICE), service());
    }

    if (!parent().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_PPPOE_PARENT), parent());
    }

    if (!username().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_PPPOE_USERNAME), username());
    }

    if (!password().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_PPPOE_PASSWORD), password());
    }

    if (passwordFlags()) {
        setting.insert(QLatin1String(NM_SETTING_PPPOE_PASSWORD_FLAGS), (int)passwordFlags());
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::PppoeSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_PPPOE_SERVICE << ": " << setting.service() << '\n';
    dbg.nospace() << NM_SETTING_PPPOE_PARENT << ": " << setting.parent() << '\n';
    dbg.nospace() << NM_SETTING_PPPOE_USERNAME << ": " << setting.username() << '\n';
    dbg.nospace() << NM_SETTING_PPPOE_PASSWORD << ": " << setting.password() << '\n';
    dbg.nospace() << NM_SETTING_PPPOE_PASSWORD_FLAGS << ": " << setting.passwordFlags() << '\n';

    return dbg.maybeSpace();
}
