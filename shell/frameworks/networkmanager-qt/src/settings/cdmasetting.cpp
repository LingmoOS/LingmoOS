/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "cdmasetting.h"
#include "cdmasetting_p.h"

#include <QDebug>

NetworkManager::CdmaSettingPrivate::CdmaSettingPrivate()
    : name(NM_SETTING_CDMA_SETTING_NAME)
    , passwordFlags(Setting::None)
{
}

NetworkManager::CdmaSetting::CdmaSetting()
    : Setting(Setting::Cdma)
    , d_ptr(new CdmaSettingPrivate())
{
}

NetworkManager::CdmaSetting::CdmaSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new CdmaSettingPrivate())
{
    setUsername(other->username());
    setNumber(other->number());
    setPassword(other->password());
    setPasswordFlags(other->passwordFlags());
}

NetworkManager::CdmaSetting::~CdmaSetting()
{
    delete d_ptr;
}

QString NetworkManager::CdmaSetting::name() const
{
    Q_D(const CdmaSetting);

    return d->name;
}

void NetworkManager::CdmaSetting::setNumber(const QString &number)
{
    Q_D(CdmaSetting);

    d->number = number;
}

QString NetworkManager::CdmaSetting::number() const
{
    Q_D(const CdmaSetting);

    return d->number;
}

void NetworkManager::CdmaSetting::setUsername(const QString &username)
{
    Q_D(CdmaSetting);

    d->username = username;
}

QString NetworkManager::CdmaSetting::username() const
{
    Q_D(const CdmaSetting);

    return d->username;
}

void NetworkManager::CdmaSetting::setPassword(const QString &password)
{
    Q_D(CdmaSetting);

    d->password = password;
}

QString NetworkManager::CdmaSetting::password() const
{
    Q_D(const CdmaSetting);

    return d->password;
}

void NetworkManager::CdmaSetting::setPasswordFlags(NetworkManager::Setting::SecretFlags flags)
{
    Q_D(CdmaSetting);

    d->passwordFlags = flags;
}

NetworkManager::Setting::SecretFlags NetworkManager::CdmaSetting::passwordFlags() const
{
    Q_D(const CdmaSetting);

    return d->passwordFlags;
}

QStringList NetworkManager::CdmaSetting::needSecrets(bool requestNew) const
{
    QStringList secrets;
    if (!username().isEmpty()) {
        if ((password().isEmpty() || requestNew) && !passwordFlags().testFlag(NotRequired)) {
            secrets << QLatin1String(NM_SETTING_CDMA_PASSWORD);
        }
    }

    return secrets;
}

void NetworkManager::CdmaSetting::secretsFromMap(const QVariantMap &secrets)
{
    if (secrets.contains(QLatin1String(NM_SETTING_CDMA_PASSWORD))) {
        setPassword(secrets.value(QLatin1String(NM_SETTING_CDMA_PASSWORD)).toString());
    }
}

QVariantMap NetworkManager::CdmaSetting::secretsToMap() const
{
    QVariantMap secrets;

    if (!password().isEmpty()) {
        secrets.insert(QLatin1String(NM_SETTING_CDMA_PASSWORD), password());
    }

    return secrets;
}

void NetworkManager::CdmaSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_CDMA_NUMBER))) {
        setNumber(setting.value(QLatin1String(NM_SETTING_CDMA_NUMBER)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_CDMA_USERNAME))) {
        setUsername(setting.value(QLatin1String(NM_SETTING_CDMA_USERNAME)).toString());
    }

    // SECRETS
    if (setting.contains(QLatin1String(NM_SETTING_CDMA_PASSWORD))) {
        setPassword(setting.value(QLatin1String(NM_SETTING_CDMA_PASSWORD)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_CDMA_PASSWORD_FLAGS))) {
        setPasswordFlags((Setting::SecretFlags)setting.value(QLatin1String(NM_SETTING_CDMA_PASSWORD_FLAGS)).toInt());
    }
}

QVariantMap NetworkManager::CdmaSetting::toMap() const
{
    QVariantMap setting;

    if (!number().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_CDMA_NUMBER), number());
    }

    if (!username().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_CDMA_USERNAME), username());
    }

    // SECRETS
    if (!password().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_CDMA_PASSWORD), password());
    }

    if (passwordFlags() != None) {
        setting.insert(QLatin1String(NM_SETTING_CDMA_PASSWORD_FLAGS), (int)passwordFlags());
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::CdmaSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_CDMA_NUMBER << ": " << setting.number() << '\n';
    dbg.nospace() << NM_SETTING_CDMA_USERNAME << ": " << setting.username() << '\n';
    dbg.nospace() << NM_SETTING_CDMA_PASSWORD << ": " << setting.password() << '\n';
    dbg.nospace() << NM_SETTING_CDMA_PASSWORD_FLAGS << ": " << setting.passwordFlags() << '\n';

    return dbg.maybeSpace();
}
