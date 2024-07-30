/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "wirelesssetting.h"
#include "wirelesssetting_p.h"

#include "utils.h"

// define the deprecated&dropped values
#define NM_SETTING_WIRELESS_SEC "security"
#define NM_SETTING_WIRELESS_ASSIGNED_MAC_ADDRESS "assigned-mac-address"

#include <QDebug>

NetworkManager::WirelessSettingPrivate::WirelessSettingPrivate()
    : name(NM_SETTING_WIRELESS_SETTING_NAME)
    , mode(NetworkManager::WirelessSetting::Infrastructure)
    , band(NetworkManager::WirelessSetting::Automatic)
    , channel(0)
    , rate(0)
    , txPower(0)
    , mtu(0)
    , hidden(false)
    , macAddressRandomization(NetworkManager::Setting::MacAddressRandomizationDefault)
    , powersave(NetworkManager::WirelessSetting::PowerSaveDefault)
{
}

NetworkManager::WirelessSetting::WirelessSetting()
    : Setting(Setting::Wireless)
    , d_ptr(new WirelessSettingPrivate())
{
}

NetworkManager::WirelessSetting::WirelessSetting(const Ptr &setting)
    : Setting(setting)
    , d_ptr(new WirelessSettingPrivate)
{
    setSsid(setting->ssid());
    setMode(setting->mode());
    setBand(setting->band());
    setChannel(setting->channel());
    setBssid(setting->bssid());
    setRate(setting->rate());
    setTxPower(setting->txPower());
    setMacAddress(setting->macAddress());
    setGenerateMacAddressMask(setting->generateMacAddressMask());
    setClonedMacAddress(setting->clonedMacAddress());
    setMacAddressBlacklist(setting->macAddressBlacklist());
    setMacAddressRandomization(setting->macAddressRandomization());
    setMtu(setting->mtu());
    setSeenBssids(setting->seenBssids());
    setPowerSave(setting->powerSave());
    setSecurity(setting->security());
    setHidden(setting->hidden());
    setAssignedMacAddress(setting->assignedMacAddress());
}

NetworkManager::WirelessSetting::~WirelessSetting()
{
    delete d_ptr;
}

QString NetworkManager::WirelessSetting::name() const
{
    Q_D(const WirelessSetting);

    return d->name;
}

void NetworkManager::WirelessSetting::setSsid(const QByteArray &ssid)
{
    Q_D(WirelessSetting);

    d->ssid = ssid;
}

QByteArray NetworkManager::WirelessSetting::ssid() const
{
    Q_D(const WirelessSetting);

    return d->ssid;
}

void NetworkManager::WirelessSetting::setMode(NetworkManager::WirelessSetting::NetworkMode mode)
{
    Q_D(WirelessSetting);

    d->mode = mode;
}

NetworkManager::WirelessSetting::NetworkMode NetworkManager::WirelessSetting::mode() const
{
    Q_D(const WirelessSetting);

    return d->mode;
}

void NetworkManager::WirelessSetting::setBand(NetworkManager::WirelessSetting::FrequencyBand band)
{
    Q_D(WirelessSetting);

    d->band = band;
}

NetworkManager::WirelessSetting::FrequencyBand NetworkManager::WirelessSetting::band() const
{
    Q_D(const WirelessSetting);

    return d->band;
}

void NetworkManager::WirelessSetting::setChannel(quint32 channel)
{
    Q_D(WirelessSetting);

    d->channel = channel;
}

quint32 NetworkManager::WirelessSetting::channel() const
{
    Q_D(const WirelessSetting);

    return d->channel;
}

void NetworkManager::WirelessSetting::setBssid(const QByteArray &bssid)
{
    Q_D(WirelessSetting);

    d->bssid = bssid;
}

QByteArray NetworkManager::WirelessSetting::bssid() const
{
    Q_D(const WirelessSetting);

    return d->bssid;
}

void NetworkManager::WirelessSetting::setRate(quint32 rate)
{
    Q_D(WirelessSetting);

    d->rate = rate;
}

quint32 NetworkManager::WirelessSetting::rate() const
{
    Q_D(const WirelessSetting);

    return d->rate;
}

void NetworkManager::WirelessSetting::setTxPower(quint32 power)
{
    Q_D(WirelessSetting);

    d->txPower = power;
}

quint32 NetworkManager::WirelessSetting::txPower() const
{
    Q_D(const WirelessSetting);

    return d->txPower;
}

void NetworkManager::WirelessSetting::setAssignedMacAddress(const QString &assignedMacAddress)
{
    Q_D(WirelessSetting);

    d->assignedMacAddress = assignedMacAddress;
}

QString NetworkManager::WirelessSetting::assignedMacAddress() const
{
    Q_D(const WirelessSetting);

    return d->assignedMacAddress;
}

void NetworkManager::WirelessSetting::setMacAddress(const QByteArray &address)
{
    Q_D(WirelessSetting);

    d->macAddress = address;
}

QByteArray NetworkManager::WirelessSetting::macAddress() const
{
    Q_D(const WirelessSetting);

    return d->macAddress;
}

void NetworkManager::WirelessSetting::setClonedMacAddress(const QByteArray &address)
{
    Q_D(WirelessSetting);

    d->assignedMacAddress = NetworkManager::macAddressAsString(address);
}

QByteArray NetworkManager::WirelessSetting::clonedMacAddress() const
{
    Q_D(const WirelessSetting);

    return NetworkManager::macAddressFromString(d->assignedMacAddress.toUtf8());
}

void NetworkManager::WirelessSetting::setGenerateMacAddressMask(const QString &macAddressMask)
{
    Q_D(WirelessSetting);

    d->generateMacAddressMask = macAddressMask;
}

QString NetworkManager::WirelessSetting::generateMacAddressMask() const
{
    Q_D(const WirelessSetting);

    return d->generateMacAddressMask;
}

void NetworkManager::WirelessSetting::setMacAddressBlacklist(const QStringList &list)
{
    Q_D(WirelessSetting);

    d->macAddressBlacklist = list;
}

QStringList NetworkManager::WirelessSetting::macAddressBlacklist() const
{
    Q_D(const WirelessSetting);

    return d->macAddressBlacklist;
}

void NetworkManager::WirelessSetting::setMacAddressRandomization(NetworkManager::Setting::MacAddressRandomization randomization)
{
    Q_D(WirelessSetting);

    d->macAddressRandomization = randomization;
}

NetworkManager::Setting::MacAddressRandomization NetworkManager::WirelessSetting::macAddressRandomization() const
{
    Q_D(const WirelessSetting);

    return d->macAddressRandomization;
}

void NetworkManager::WirelessSetting::setMtu(quint32 mtu)
{
    Q_D(WirelessSetting);

    d->mtu = mtu;
}

quint32 NetworkManager::WirelessSetting::mtu() const
{
    Q_D(const WirelessSetting);

    return d->mtu;
}

void NetworkManager::WirelessSetting::setSeenBssids(const QStringList &list)
{
    Q_D(WirelessSetting);

    d->seenBssids = list;
}

QStringList NetworkManager::WirelessSetting::seenBssids() const
{
    Q_D(const WirelessSetting);

    return d->seenBssids;
}

void NetworkManager::WirelessSetting::setPowerSave(NetworkManager::WirelessSetting::PowerSave powersave)
{
    Q_D(WirelessSetting);

    d->powersave = powersave;
}

NetworkManager::WirelessSetting::PowerSave NetworkManager::WirelessSetting::powerSave() const
{
    Q_D(const WirelessSetting);

    return d->powersave;
}

void NetworkManager::WirelessSetting::setSecurity(const QString &security)
{
    Q_D(WirelessSetting);

    d->security = security;
}

QString NetworkManager::WirelessSetting::security() const
{
    Q_D(const WirelessSetting);

    return d->security;
}

void NetworkManager::WirelessSetting::setHidden(bool hidden)
{
    Q_D(WirelessSetting);

    d->hidden = hidden;
}

bool NetworkManager::WirelessSetting::hidden() const
{
    Q_D(const WirelessSetting);

    return d->hidden;
}

void NetworkManager::WirelessSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_SSID))) {
        setSsid(setting.value(QLatin1String(NM_SETTING_WIRELESS_SSID)).toByteArray());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_MODE))) {
        const QString mode = setting.value(QLatin1String(NM_SETTING_WIRELESS_MODE)).toString();
        if (mode == QLatin1String(NM_SETTING_WIRELESS_MODE_INFRA)) {
            setMode(Infrastructure);
        } else if (mode == QLatin1String(NM_SETTING_WIRELESS_MODE_ADHOC)) {
            setMode(Adhoc);
        } else if (mode == QLatin1String(NM_SETTING_WIRELESS_MODE_AP)) {
            setMode(Ap);
        }
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_BAND))) {
        const QString band = setting.value(QLatin1String(NM_SETTING_WIRELESS_BAND)).toString();
        if (band == "a") {
            setBand(A);
        } else if (band == "bg") {
            setBand(Bg);
        }
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_CHANNEL))) {
        setChannel(setting.value(QLatin1String(NM_SETTING_WIRELESS_CHANNEL)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_BSSID))) {
        setBssid(setting.value(QLatin1String(NM_SETTING_WIRELESS_BSSID)).toByteArray());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_RATE))) {
        setRate(setting.value(QLatin1String(NM_SETTING_WIRELESS_RATE)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_TX_POWER))) {
        setTxPower(setting.value(QLatin1String(NM_SETTING_WIRELESS_TX_POWER)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_ASSIGNED_MAC_ADDRESS))) {
        setAssignedMacAddress(setting.value(QLatin1String(NM_SETTING_WIRELESS_ASSIGNED_MAC_ADDRESS)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_MAC_ADDRESS))) {
        setMacAddress(setting.value(QLatin1String(NM_SETTING_WIRELESS_MAC_ADDRESS)).toByteArray());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_GENERATE_MAC_ADDRESS_MASK))) {
        setGenerateMacAddressMask(setting.value(QLatin1String(NM_SETTING_WIRELESS_GENERATE_MAC_ADDRESS_MASK)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_CLONED_MAC_ADDRESS))) {
        setClonedMacAddress(setting.value(QLatin1String(NM_SETTING_WIRELESS_CLONED_MAC_ADDRESS)).toByteArray());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_MAC_ADDRESS_BLACKLIST))) {
        setMacAddressBlacklist(setting.value(QLatin1String(NM_SETTING_WIRELESS_MAC_ADDRESS_BLACKLIST)).toStringList());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_MAC_ADDRESS_RANDOMIZATION))) {
        setMacAddressRandomization(
            static_cast<Setting::MacAddressRandomization>(setting.value(QLatin1String(NM_SETTING_WIRELESS_MAC_ADDRESS_RANDOMIZATION)).toUInt()));
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_MTU))) {
        setMtu(setting.value(QLatin1String(NM_SETTING_WIRELESS_MTU)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_SEEN_BSSIDS))) {
        setSeenBssids(setting.value(QLatin1String(NM_SETTING_WIRELESS_SEEN_BSSIDS)).toStringList());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_POWERSAVE))) {
        setPowerSave(static_cast<WirelessSetting::PowerSave>(setting.value(QLatin1String(NM_SETTING_WIRELESS_POWERSAVE)).toUInt()));
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_SEC))) {
        setSecurity(setting.value(QLatin1String(NM_SETTING_WIRELESS_SEC)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_HIDDEN))) {
        setHidden(setting.value(QLatin1String(NM_SETTING_WIRELESS_HIDDEN)).toBool());
    }
}

QVariantMap NetworkManager::WirelessSetting::toMap() const
{
    QVariantMap setting;

    if (!ssid().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SSID), ssid());
    }

    if (mode() == Infrastructure) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_MODE), QLatin1String(NM_SETTING_WIRELESS_MODE_INFRA));
    } else if (mode() == Adhoc) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_MODE), QLatin1String(NM_SETTING_WIRELESS_MODE_ADHOC));
    } else if (mode() == Ap) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_MODE), QLatin1String(NM_SETTING_WIRELESS_MODE_AP));
    }

    if (band() != Automatic) {
        if (band() == A) {
            setting.insert(QLatin1String(NM_SETTING_WIRELESS_BAND), "a");
        } else if (band() == Bg) {
            setting.insert(QLatin1String(NM_SETTING_WIRELESS_BAND), "bg");
        }
    }

    if (channel()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_CHANNEL), channel());
    }

    if (!bssid().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_BSSID), bssid());
    }

    if (rate()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_RATE), rate());
    }

    if (txPower()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_TX_POWER), txPower());
    }

    if (!assignedMacAddress().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_ASSIGNED_MAC_ADDRESS), assignedMacAddress());
    }

    if (!macAddress().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_MAC_ADDRESS), macAddress());
    }

    if (!generateMacAddressMask().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_GENERATE_MAC_ADDRESS_MASK), generateMacAddressMask());
    }

    if (!macAddressBlacklist().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_MAC_ADDRESS_BLACKLIST), macAddressBlacklist());
    }

    setting.insert(QLatin1String(NM_SETTING_WIRELESS_MAC_ADDRESS_RANDOMIZATION), macAddressRandomization());

    if (mtu()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_MTU), mtu());
    }

    if (!seenBssids().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SEEN_BSSIDS), seenBssids());
    }

    setting.insert(QLatin1String(NM_SETTING_WIRELESS_POWERSAVE), powerSave());

    if (!security().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SEC), security());
    }

    if (hidden()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_HIDDEN), hidden());
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::WirelessSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_WIRELESS_SSID << ": " << setting.ssid() << '\n';
    dbg.nospace() << NM_SETTING_WIRELESS_MODE << ": " << setting.mode() << '\n';
    dbg.nospace() << NM_SETTING_WIRELESS_BAND << ": " << setting.band() << '\n';
    dbg.nospace() << NM_SETTING_WIRELESS_CHANNEL << ": " << setting.channel() << '\n';
    dbg.nospace() << NM_SETTING_WIRELESS_BSSID << ": " << setting.bssid() << '\n';
    dbg.nospace() << NM_SETTING_WIRELESS_RATE << ": " << setting.rate() << '\n';
    dbg.nospace() << NM_SETTING_WIRELESS_TX_POWER << ": " << setting.txPower() << '\n';
    dbg.nospace() << NM_SETTING_WIRELESS_ASSIGNED_MAC_ADDRESS << ": " << setting.assignedMacAddress() << '\n';
    dbg.nospace() << NM_SETTING_WIRELESS_MAC_ADDRESS << ": " << setting.macAddress() << '\n';
    dbg.nospace() << NM_SETTING_WIRELESS_GENERATE_MAC_ADDRESS_MASK << ": " << setting.generateMacAddressMask() << '\n';
    dbg.nospace() << NM_SETTING_WIRELESS_MAC_ADDRESS_BLACKLIST << ": " << setting.macAddressBlacklist() << '\n';
    dbg.nospace() << NM_SETTING_WIRELESS_MAC_ADDRESS_RANDOMIZATION << ": " << setting.macAddressRandomization() << '\n';
    dbg.nospace() << NM_SETTING_WIRELESS_MTU << ": " << setting.mtu() << '\n';
    dbg.nospace() << NM_SETTING_WIRELESS_SEEN_BSSIDS << ": " << setting.seenBssids() << '\n';
    dbg.nospace() << NM_SETTING_WIRELESS_POWERSAVE << ": " << setting.powerSave() << '\n';
    dbg.nospace() << NM_SETTING_WIRELESS_SEC << ": " << setting.security() << '\n';
    dbg.nospace() << NM_SETTING_WIRELESS_HIDDEN << ": " << setting.hidden() << '\n';

    return dbg.maybeSpace();
}
