// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dvpnsetting_p.h"
#include <QDBusArgument>

DNETWORKMANAGER_BEGIN_NAMESPACE

DVpnSettingPrivate::DVpnSettingPrivate()
    : m_persistent(false)
    , m_timeout(0)
    , m_name(NM_SETTING_VPN_SETTING_NAME)
{
}

DVpnSetting::DVpnSetting()
    : DNMSetting(DNMSetting::SettingType::Vpn)
    , d_ptr(new DVpnSettingPrivate())
{
}

DVpnSetting::DVpnSetting(const QSharedPointer<DVpnSetting> &other)
    : DNMSetting(other)
    , d_ptr(new DVpnSettingPrivate())
{
    setServiceType(other->serviceType());
    setUsername(other->username());
    setData(other->data());
    setSecrets(other->secrets());
    setPersistent(other->persistent());
    setTimeout(other->timeout());
}

QString DVpnSetting::name() const
{
    Q_D(const DVpnSetting);

    return d->m_name;
}

bool DVpnSetting::persistent() const
{
    Q_D(const DVpnSetting);

    return d->m_persistent;
}

void DVpnSetting::setPersistent(bool persistent)
{
    Q_D(DVpnSetting);

    d->m_persistent = persistent;
}

uint DVpnSetting::timeout() const
{
    Q_D(const DVpnSetting);

    return d->m_timeout;
}

void DVpnSetting::setTimeout(uint timeout)
{
    Q_D(DVpnSetting);

    d->m_timeout = timeout;
}

void DVpnSetting::setServiceType(const QString &type)
{
    Q_D(DVpnSetting);

    d->m_serviceType = type;
}

QString DVpnSetting::serviceType() const
{
    Q_D(const DVpnSetting);

    return d->m_serviceType;
}

void DVpnSetting::setUsername(const QString &username)
{
    Q_D(DVpnSetting);

    d->m_username = username;
}

QString DVpnSetting::username() const
{
    Q_D(const DVpnSetting);

    return d->m_username;
}

void DVpnSetting::setData(const QMap<QString, QString> &data)
{
    Q_D(DVpnSetting);

    d->m_data = data;
}

QMap<QString, QString> DVpnSetting::data() const
{
    Q_D(const DVpnSetting);

    return d->m_data;
}

void DVpnSetting::setSecrets(const QMap<QString, QString> &secrets)
{
    Q_D(DVpnSetting);

    d->m_secrets = secrets;
}

QMap<QString, QString> DVpnSetting::secrets() const
{
    Q_D(const DVpnSetting);

    return d->m_secrets;
}

void DVpnSetting::mapToSecrets(const QVariantMap &secrets)
{
    if (secrets.contains(QLatin1String(NM_SETTING_VPN_SECRETS))) {
        setSecrets(qdbus_cast<QMap<QString, QString>>(secrets.value(QLatin1String(NM_SETTING_VPN_SECRETS))));
    }
}

QVariantMap DVpnSetting::secretsToMap() const
{
    QVariantMap secretsMap;
    if (!secrets().isEmpty()) {
        secretsMap.insert(QLatin1String(NM_SETTING_VPN_SECRETS), QVariant::fromValue<QMap<QString, QString>>(secrets()));
    }

    // qCDebug(NMQT) << secretsMap;
    return secretsMap;
}

void DVpnSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_VPN_SERVICE_TYPE))) {
        setServiceType(setting.value(QLatin1String(NM_SETTING_VPN_SERVICE_TYPE)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_VPN_USER_NAME))) {
        setUsername(setting.value(QLatin1String(NM_SETTING_VPN_USER_NAME)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_VPN_DATA))) {
        setData(qdbus_cast<QMap<QString, QString>>(setting.value(QLatin1String(NM_SETTING_VPN_DATA))));
    }

    if (setting.contains(QLatin1String(NM_SETTING_VPN_SECRETS))) {
        setSecrets(qdbus_cast<QMap<QString, QString>>(setting.value(QLatin1String(NM_SETTING_VPN_SECRETS))));
    }

    if (setting.contains(QLatin1String(NM_SETTING_VPN_PERSISTENT))) {
        setPersistent(setting.value(QLatin1String(NM_SETTING_VPN_PERSISTENT)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_VPN_TIMEOUT))) {
        setTimeout(setting.value(QLatin1String(NM_SETTING_VPN_TIMEOUT)).toUInt());
    }
}

QVariantMap DVpnSetting::toMap() const
{
    QVariantMap setting;

    if (!serviceType().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_VPN_SERVICE_TYPE), serviceType());
    }

    if (!username().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_VPN_USER_NAME), username());
    }

    if (!data().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_VPN_DATA), QVariant::fromValue<QMap<QString, QString>>(data()));
    }

    if (!secrets().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_VPN_SECRETS), QVariant::fromValue<QMap<QString, QString>>(secrets()));
    }

    setting.insert(QLatin1String(NM_SETTING_VPN_PERSISTENT), persistent());
    setting.insert(QLatin1String(NM_SETTING_VPN_TIMEOUT), timeout());

    return setting;
}

void DVpnSetting::stringMapTosecrets(const QMap<QString, QString> &map)
{
    Q_D(DVpnSetting);

    if (map.contains(QLatin1String("VpnSecrets"))) {
        const QStringList list = map.value(QLatin1String("VpnSecrets")).split("%SEP%");
        if (list.count() % 2 == 0) {
            for (int i = 0; i < list.count(); i += 2) {
                if (d->m_secrets.contains(list[i])) {
                    d->m_secrets.remove(list[i]);
                }
                d->m_secrets.insert(list[i], list[i + 1]);
            }
        }
    }
}

QMap<QString, QString> DVpnSetting::secretsToStringMap() const
{
    QMap<QString, QString> ret;
    QStringList list;
    QMap<QString, QString>::ConstIterator i = secrets().constBegin();
    while (i != secrets().constEnd()) {
        list << i.key() << i.value();
        ++i;
    }

    ret.insert(QLatin1String("VpnSecrets"), list.join("%SEP%"));
    return ret;
}

DNETWORKMANAGER_END_NAMESPACE
