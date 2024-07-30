/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "adslsetting.h"
#include "adslsetting_p.h"
#include <manager.h>

#include <QDebug>

NetworkManager::AdslSettingPrivate::AdslSettingPrivate()
    : name(NM_SETTING_ADSL_SETTING_NAME)
    , passwordFlags(Setting::None)
    , protocol(AdslSetting::UnknownProtocol)
    , encapsulation(AdslSetting::UnknownEncapsulation)
    , vpi(0)
    , vci(0)
{
}

NetworkManager::AdslSetting::AdslSetting()
    : Setting(Setting::Adsl)
    , d_ptr(new AdslSettingPrivate())
{
}

NetworkManager::AdslSetting::AdslSetting(const AdslSetting::Ptr &other)
    : Setting(other)
    , d_ptr(new AdslSettingPrivate())
{
    setUsername(other->username());
    setPassword(other->password());
    setProtocol(other->protocol());
    setPasswordFlags(other->passwordFlags());
    setProtocol(other->protocol());
    setEncapsulation(other->encapsulation());
    setVpi(other->vpi());
    setVci(other->vci());
}

NetworkManager::AdslSetting::~AdslSetting()
{
    delete d_ptr;
}

QString NetworkManager::AdslSetting::name() const
{
    Q_D(const AdslSetting);

    return d->name;
}

void NetworkManager::AdslSetting::setUsername(const QString &username)
{
    Q_D(AdslSetting);

    d->username = username;
}

QString NetworkManager::AdslSetting::username() const
{
    Q_D(const AdslSetting);

    return d->username;
}

void NetworkManager::AdslSetting::setPassword(const QString &password)
{
    Q_D(AdslSetting);

    d->password = password;
}

QString NetworkManager::AdslSetting::password() const
{
    Q_D(const AdslSetting);

    return d->password;
}

void NetworkManager::AdslSetting::setPasswordFlags(NetworkManager::Setting::SecretFlags flags)
{
    Q_D(AdslSetting);

    d->passwordFlags = flags;
}

NetworkManager::Setting::SecretFlags NetworkManager::AdslSetting::passwordFlags() const
{
    Q_D(const AdslSetting);

    return d->passwordFlags;
}

void NetworkManager::AdslSetting::setProtocol(NetworkManager::AdslSetting::Protocol protocol)
{
    Q_D(AdslSetting);

    d->protocol = protocol;
}

NetworkManager::AdslSetting::Protocol NetworkManager::AdslSetting::protocol() const
{
    Q_D(const AdslSetting);

    return d->protocol;
}

void NetworkManager::AdslSetting::setEncapsulation(NetworkManager::AdslSetting::Encapsulation encapsulation)
{
    Q_D(AdslSetting);

    d->encapsulation = encapsulation;
}

NetworkManager::AdslSetting::Encapsulation NetworkManager::AdslSetting::encapsulation() const
{
    Q_D(const AdslSetting);

    return d->encapsulation;
}

void NetworkManager::AdslSetting::setVpi(quint32 vpi)
{
    Q_D(AdslSetting);

    d->vpi = vpi;
}

quint32 NetworkManager::AdslSetting::vpi() const
{
    Q_D(const AdslSetting);

    return d->vpi;
}

void NetworkManager::AdslSetting::setVci(quint32 vci)
{
    Q_D(AdslSetting);

    d->vci = vci;
}

quint32 NetworkManager::AdslSetting::vci() const
{
    Q_D(const AdslSetting);

    return d->vci;
}

QStringList NetworkManager::AdslSetting::needSecrets(bool requestNew) const
{
    QStringList secrets;

    if ((password().isEmpty() || requestNew) && !passwordFlags().testFlag(NotRequired)) {
        secrets << QLatin1String(NM_SETTING_ADSL_PASSWORD);
    }

    return secrets;
}

void NetworkManager::AdslSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_ADSL_USERNAME))) {
        setUsername(setting.value(QLatin1String(NM_SETTING_ADSL_USERNAME)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_ADSL_PASSWORD))) {
        setPassword(setting.value(QLatin1String(NM_SETTING_ADSL_PASSWORD)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_ADSL_PASSWORD_FLAGS))) {
        setPasswordFlags((Setting::SecretFlags)setting.value(QLatin1String(NM_SETTING_ADSL_PASSWORD_FLAGS)).toInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_ADSL_PROTOCOL))) {
        const QString protocol = setting.value(QLatin1String(NM_SETTING_ADSL_PROTOCOL)).toString();

        if (protocol == QLatin1String(NM_SETTING_ADSL_PROTOCOL_PPPOA)) {
            setProtocol(Pppoa);
        } else if (protocol == QLatin1String(NM_SETTING_ADSL_PROTOCOL_PPPOE)) {
            setProtocol(Pppoe);
        } else if (protocol == QLatin1String(NM_SETTING_ADSL_PROTOCOL_IPOATM)) {
            setProtocol(Ipoatm);
        }
    }

    if (setting.contains(QLatin1String(NM_SETTING_ADSL_ENCAPSULATION))) {
        const QString encapsulation = setting.value(QLatin1String(NM_SETTING_ADSL_ENCAPSULATION)).toString();

        if (encapsulation == QLatin1String(NM_SETTING_ADSL_ENCAPSULATION_VCMUX)) {
            setEncapsulation(Vcmux);
        } else if (encapsulation == QLatin1String(NM_SETTING_ADSL_ENCAPSULATION_LLC)) {
            setEncapsulation(Llc);
        }
    }

    if (setting.contains(QLatin1String(NM_SETTING_ADSL_VPI))) {
        setVpi(setting.value(QLatin1String(NM_SETTING_ADSL_VPI)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_ADSL_VCI))) {
        setVci(setting.value(QLatin1String(NM_SETTING_ADSL_VCI)).toUInt());
    }
}

QVariantMap NetworkManager::AdslSetting::toMap() const
{
    QVariantMap setting;

    if (!username().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_ADSL_USERNAME), username());
    }

    if (!password().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_ADSL_PASSWORD), password());
    }

    if (passwordFlags() != None) {
        setting.insert(QLatin1String(NM_SETTING_ADSL_PASSWORD_FLAGS), (int)passwordFlags());
    }

    switch (protocol()) {
    case Pppoa:
        setting.insert(QLatin1String(NM_SETTING_ADSL_PROTOCOL), QLatin1String(NM_SETTING_ADSL_PROTOCOL_PPPOA));
        break;
    case Pppoe:
        setting.insert(QLatin1String(NM_SETTING_ADSL_PROTOCOL), QLatin1String(NM_SETTING_ADSL_PROTOCOL_PPPOE));
        break;
    case Ipoatm:
        setting.insert(QLatin1String(NM_SETTING_ADSL_PROTOCOL), QLatin1String(NM_SETTING_ADSL_PROTOCOL_IPOATM));
        break;
    case UnknownProtocol:
        break;
    }

    switch (encapsulation()) {
    case Vcmux:
        setting.insert(QLatin1String(NM_SETTING_ADSL_ENCAPSULATION), QLatin1String(NM_SETTING_ADSL_ENCAPSULATION_VCMUX));
        break;
    case Llc:
        setting.insert(QLatin1String(NM_SETTING_ADSL_ENCAPSULATION), QLatin1String(NM_SETTING_ADSL_ENCAPSULATION_LLC));
        break;
    case UnknownEncapsulation:
        break;
    }

    if (vpi()) {
        setting.insert(QLatin1String(NM_SETTING_ADSL_VPI), vpi());
    }

    if (vci()) {
        setting.insert(QLatin1String(NM_SETTING_ADSL_VCI), vci());
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::AdslSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_ADSL_USERNAME << ": " << setting.username() << '\n';
    dbg.nospace() << NM_SETTING_ADSL_PASSWORD << ": " << setting.password() << '\n';
    dbg.nospace() << NM_SETTING_ADSL_PASSWORD_FLAGS << ": " << setting.passwordFlags() << '\n';
    dbg.nospace() << NM_SETTING_ADSL_PROTOCOL << ": " << setting.protocol() << '\n';
    dbg.nospace() << NM_SETTING_ADSL_ENCAPSULATION << ": " << setting.encapsulation() << '\n';
    dbg.nospace() << NM_SETTING_ADSL_VPI << ": " << setting.vpi() << '\n';
    dbg.nospace() << NM_SETTING_ADSL_VCI << ": " << setting.vci() << '\n';

    return dbg.maybeSpace();
}
