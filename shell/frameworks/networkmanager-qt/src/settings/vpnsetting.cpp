/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "vpnsetting.h"
#include "nmdebug.h"
#include "vpnsetting_p.h"

NetworkManager::VpnSettingPrivate::VpnSettingPrivate()
    : name(NM_SETTING_VPN_SETTING_NAME)
    , persistent(false)
    , timeout(0)
{
}

NetworkManager::VpnSetting::VpnSetting()
    : Setting(Setting::Vpn)
    , d_ptr(new VpnSettingPrivate())
{
}

NetworkManager::VpnSetting::VpnSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new VpnSettingPrivate())
{
    setServiceType(other->serviceType());
    setUsername(other->username());
    setData(other->data());
    setSecrets(other->secrets());
    setPersistent(other->persistent());
    setTimeout(other->timeout());
}

NetworkManager::VpnSetting::~VpnSetting()
{
    delete d_ptr;
}

QString NetworkManager::VpnSetting::name() const
{
    Q_D(const VpnSetting);

    return d->name;
}

bool NetworkManager::VpnSetting::persistent() const
{
    Q_D(const VpnSetting);

    return d->persistent;
}

void NetworkManager::VpnSetting::setPersistent(bool persistent)
{
    Q_D(VpnSetting);

    d->persistent = persistent;
}

uint NetworkManager::VpnSetting::timeout() const
{
    Q_D(const VpnSetting);

    return d->timeout;
}

void NetworkManager::VpnSetting::setTimeout(uint timeout)
{
    Q_D(VpnSetting);

    d->timeout = timeout;
}

void NetworkManager::VpnSetting::setServiceType(const QString &type)
{
    Q_D(VpnSetting);

    d->serviceType = type;
}

QString NetworkManager::VpnSetting::serviceType() const
{
    Q_D(const VpnSetting);

    return d->serviceType;
}

void NetworkManager::VpnSetting::setUsername(const QString &username)
{
    Q_D(VpnSetting);

    d->username = username;
}

QString NetworkManager::VpnSetting::username() const
{
    Q_D(const VpnSetting);

    return d->username;
}

void NetworkManager::VpnSetting::setData(const NMStringMap &data)
{
    Q_D(VpnSetting);

    d->data = data;
}

NMStringMap NetworkManager::VpnSetting::data() const
{
    Q_D(const VpnSetting);

    return d->data;
}

void NetworkManager::VpnSetting::setSecrets(const NMStringMap &secrets)
{
    Q_D(VpnSetting);

    d->secrets = secrets;
}

NMStringMap NetworkManager::VpnSetting::secrets() const
{
    Q_D(const VpnSetting);

    return d->secrets;
}

void NetworkManager::VpnSetting::secretsFromMap(const QVariantMap &secrets)
{
    if (secrets.contains(QLatin1String(NM_SETTING_VPN_SECRETS))) {
        setSecrets(qdbus_cast<NMStringMap>(secrets.value(QLatin1String(NM_SETTING_VPN_SECRETS))));
    }
}

QVariantMap NetworkManager::VpnSetting::secretsToMap() const
{
    QVariantMap secretsMap;
    if (!secrets().isEmpty()) {
        secretsMap.insert(QLatin1String(NM_SETTING_VPN_SECRETS), QVariant::fromValue<NMStringMap>(secrets()));
    }

    // qCDebug(NMQT) << secretsMap;
    return secretsMap;
}

void NetworkManager::VpnSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_VPN_SERVICE_TYPE))) {
        setServiceType(setting.value(QLatin1String(NM_SETTING_VPN_SERVICE_TYPE)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_VPN_USER_NAME))) {
        setUsername(setting.value(QLatin1String(NM_SETTING_VPN_USER_NAME)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_VPN_DATA))) {
        setData(qdbus_cast<NMStringMap>(setting.value(QLatin1String(NM_SETTING_VPN_DATA))));
    }

    if (setting.contains(QLatin1String(NM_SETTING_VPN_SECRETS))) {
        setSecrets(qdbus_cast<NMStringMap>(setting.value(QLatin1String(NM_SETTING_VPN_SECRETS))));
    }

    if (setting.contains(QLatin1String(NM_SETTING_VPN_PERSISTENT))) {
        setPersistent(setting.value(QLatin1String(NM_SETTING_VPN_PERSISTENT)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_VPN_TIMEOUT))) {
        setTimeout(setting.value(QLatin1String(NM_SETTING_VPN_TIMEOUT)).toUInt());
    }
}

QVariantMap NetworkManager::VpnSetting::toMap() const
{
    QVariantMap setting;

    if (!serviceType().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_VPN_SERVICE_TYPE), serviceType());
    }

    if (!username().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_VPN_USER_NAME), username());
    }

    if (!data().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_VPN_DATA), QVariant::fromValue<NMStringMap>(data()));
    }

    if (!secrets().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_VPN_SECRETS), QVariant::fromValue<NMStringMap>(secrets()));
    }

    setting.insert(QLatin1String(NM_SETTING_VPN_PERSISTENT), persistent());
    setting.insert(QLatin1String(NM_SETTING_VPN_TIMEOUT), timeout());

    return setting;
}

void NetworkManager::VpnSetting::secretsFromStringMap(const NMStringMap &map)
{
    Q_D(VpnSetting);

    if (map.contains(QLatin1String("VpnSecrets"))) {
        const QStringList list = map.value(QLatin1String("VpnSecrets")).split("%SEP%");
        if (list.count() % 2 == 0) {
            for (int i = 0; i < list.count(); i += 2) {
                if (d->secrets.contains(list[i])) {
                    d->secrets.remove(list[i]);
                }
                d->secrets.insert(list[i], list[i + 1]);
            }
        }
    }
}

NMStringMap NetworkManager::VpnSetting::secretsToStringMap() const
{
    NMStringMap ret;
    QStringList list;
    QMap<QString, QString>::ConstIterator i = secrets().constBegin();
    while (i != secrets().constEnd()) {
        list << i.key() << i.value();
        ++i;
    }

    ret.insert(QLatin1String("VpnSecrets"), list.join("%SEP%"));
    return ret;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::VpnSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_VPN_SERVICE_TYPE << ": " << setting.serviceType() << '\n';
    dbg.nospace() << NM_SETTING_VPN_USER_NAME << ": " << setting.username() << '\n';
    dbg.nospace() << NM_SETTING_VPN_DATA << ": " << setting.data() << '\n';
    dbg.nospace() << NM_SETTING_VPN_PERSISTENT << ": " << setting.persistent() << '\n';
    dbg.nospace() << NM_SETTING_VPN_TIMEOUT << ": " << setting.timeout() << '\n';

    return dbg.maybeSpace();
}
