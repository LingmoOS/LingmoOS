// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dadslsetting_p.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

DAdslSettingPrivate::DAdslSettingPrivate()
    : m_passwordFlags(DNMSetting::SecretFlagType::None)
    , m_protocol(DAdslSetting::Protocol::Unknown)
    , m_encapsulation(DAdslSetting::Encapsulation::Unknown)
    , m_vpi(0)
    , m_vci(0)
    , m_name(NM_SETTING_ADSL_SETTING_NAME)
{
}

DAdslSetting::DAdslSetting()
    : DNMSetting(DNMSetting::SettingType::Adsl)
    , d_ptr(new DAdslSettingPrivate())
{
}

DAdslSetting::DAdslSetting(const QSharedPointer<DAdslSetting> &other)
    : DNMSetting(other)
    , d_ptr(new DAdslSettingPrivate())
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

QString DAdslSetting::name() const
{
    Q_D(const DAdslSetting);

    return d->m_name;
}

void DAdslSetting::setUsername(const QString &username)
{
    Q_D(DAdslSetting);

    d->m_username = username;
}

QString DAdslSetting::username() const
{
    Q_D(const DAdslSetting);

    return d->m_username;
}

void DAdslSetting::setPassword(const QString &password)
{
    Q_D(DAdslSetting);

    d->m_password = password;
}

QString DAdslSetting::password() const
{
    Q_D(const DAdslSetting);

    return d->m_password;
}

void DAdslSetting::setPasswordFlags(DNMSetting::SecretFlags flags)
{
    Q_D(DAdslSetting);

    d->m_passwordFlags = flags;
}

DNMSetting::SecretFlags DAdslSetting::passwordFlags() const
{
    Q_D(const DAdslSetting);

    return d->m_passwordFlags;
}

void DAdslSetting::setProtocol(DAdslSetting::Protocol protocol)
{
    Q_D(DAdslSetting);

    d->m_protocol = protocol;
}

DAdslSetting::Protocol DAdslSetting::protocol() const
{
    Q_D(const DAdslSetting);

    return d->m_protocol;
}

void DAdslSetting::setEncapsulation(DAdslSetting::Encapsulation encapsulation)
{
    Q_D(DAdslSetting);

    d->m_encapsulation = encapsulation;
}

DAdslSetting::Encapsulation DAdslSetting::encapsulation() const
{
    Q_D(const DAdslSetting);

    return d->m_encapsulation;
}

void DAdslSetting::setVpi(quint32 vpi)
{
    Q_D(DAdslSetting);

    d->m_vpi = vpi;
}

quint32 DAdslSetting::vpi() const
{
    Q_D(const DAdslSetting);

    return d->m_vpi;
}

void DAdslSetting::setVci(quint32 vci)
{
    Q_D(DAdslSetting);

    d->m_vci = vci;
}

quint32 DAdslSetting::vci() const
{
    Q_D(const DAdslSetting);

    return d->m_vci;
}

QStringList DAdslSetting::needSecrets(bool requestNew) const
{
    QStringList secrets;

    if ((password().isEmpty() or requestNew) and !passwordFlags().testFlag(DNMSetting::SecretFlagType::NotRequired)) {
        secrets << QLatin1String(NM_SETTING_ADSL_PASSWORD);
    }

    return secrets;
}

void DAdslSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_ADSL_USERNAME))) {
        setUsername(setting.value(QLatin1String(NM_SETTING_ADSL_USERNAME)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_ADSL_PASSWORD))) {
        setPassword(setting.value(QLatin1String(NM_SETTING_ADSL_PASSWORD)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_ADSL_PASSWORD_FLAGS))) {
        setPasswordFlags((DNMSetting::SecretFlags)setting.value(QLatin1String(NM_SETTING_ADSL_PASSWORD_FLAGS)).toInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_ADSL_PROTOCOL))) {
        const QString protocol = setting.value(QLatin1String(NM_SETTING_ADSL_PROTOCOL)).toString();

        if (protocol == QLatin1String(NM_SETTING_ADSL_PROTOCOL_PPPOA)) {
            setProtocol(Protocol::Pppoa);
        } else if (protocol == QLatin1String(NM_SETTING_ADSL_PROTOCOL_PPPOE)) {
            setProtocol(Protocol::Pppoe);
        } else if (protocol == QLatin1String(NM_SETTING_ADSL_PROTOCOL_IPOATM)) {
            setProtocol(Protocol::Ipoatm);
        }
    }

    if (setting.contains(QLatin1String(NM_SETTING_ADSL_ENCAPSULATION))) {
        const QString encapsulation = setting.value(QLatin1String(NM_SETTING_ADSL_ENCAPSULATION)).toString();

        if (encapsulation == QLatin1String(NM_SETTING_ADSL_ENCAPSULATION_VCMUX)) {
            setEncapsulation(Encapsulation::Vcmux);
        } else if (encapsulation == QLatin1String(NM_SETTING_ADSL_ENCAPSULATION_LLC)) {
            setEncapsulation(Encapsulation::Llc);
        }
    }

    if (setting.contains(QLatin1String(NM_SETTING_ADSL_VPI))) {
        setVpi(setting.value(QLatin1String(NM_SETTING_ADSL_VPI)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_ADSL_VCI))) {
        setVci(setting.value(QLatin1String(NM_SETTING_ADSL_VCI)).toUInt());
    }
}

QVariantMap DAdslSetting::toMap() const
{
    QVariantMap setting;

    if (!username().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_ADSL_USERNAME), username());
    }

    if (!password().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_ADSL_PASSWORD), password());
    }

    if (!passwordFlags().testFlag(DNMSetting::SecretFlagType::None)) {
        setting.insert(QLatin1String(NM_SETTING_ADSL_PASSWORD_FLAGS), static_cast<int>(passwordFlags()));
    }

    switch (protocol()) {
        case Protocol::Pppoa:
            setting.insert(QLatin1String(NM_SETTING_ADSL_PROTOCOL), QLatin1String(NM_SETTING_ADSL_PROTOCOL_PPPOA));
            break;
        case Protocol::Pppoe:
            setting.insert(QLatin1String(NM_SETTING_ADSL_PROTOCOL), QLatin1String(NM_SETTING_ADSL_PROTOCOL_PPPOE));
            break;
        case Protocol::Ipoatm:
            setting.insert(QLatin1String(NM_SETTING_ADSL_PROTOCOL), QLatin1String(NM_SETTING_ADSL_PROTOCOL_IPOATM));
            break;
        case Protocol::Unknown:
            break;
    }

    switch (encapsulation()) {
        case Encapsulation::Vcmux:
            setting.insert(QLatin1String(NM_SETTING_ADSL_ENCAPSULATION), QLatin1String(NM_SETTING_ADSL_ENCAPSULATION_VCMUX));
            break;
        case Encapsulation::Llc:
            setting.insert(QLatin1String(NM_SETTING_ADSL_ENCAPSULATION), QLatin1String(NM_SETTING_ADSL_ENCAPSULATION_LLC));
            break;
        case Encapsulation::Unknown:
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

DNETWORKMANAGER_END_NAMESPACE
