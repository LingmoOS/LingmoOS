// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dwirelesssetting_p.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

DWirelessSettingPrivate::DWirelessSettingPrivate()
    : m_hidden(false)
    , m_channel(0)
    , m_rate(0)
    , m_txPower(0)
    , m_mtu(0)
    , m_mode(DWirelessSetting::NetworkMode::Infrastructure)
    , m_band(DWirelessSetting::FrequencyBand::Automatic)
    , m_macAddressRandomization(DWirelessSetting::MacAddressRandomization::Default)
    , m_powersave(DWirelessSetting::PowerSave::Default)
    , m_name(NM_SETTING_WIRELESS_SETTING_NAME)
{
}

DWirelessSetting::DWirelessSetting()
    : DNMSetting(DNMSetting::SettingType::Wireless)
    , d_ptr(new DWirelessSettingPrivate())
{
}

DWirelessSetting::DWirelessSetting(const QSharedPointer<DWirelessSetting> &other)
    : DNMSetting(other)
    , d_ptr(new DWirelessSettingPrivate)
{
    setSsid(other->ssid());
    setMode(other->mode());
    setBand(other->band());
    setChannel(other->channel());
    setBssid(other->bssid());
    setRate(other->rate());
    setTxPower(other->txPower());
    setMacAddress(other->macAddress());
    setGenerateMacAddressMask(other->generateMacAddressMask());
    setMacAddressBlacklist(other->macAddressBlacklist());
    setMacAddressRandomization(other->macAddressRandomization());
    setMtu(other->mtu());
    setSeenBssids(other->seenBssids());
    setPowerSave(other->powerSave());
    setHidden(other->hidden());
    setAssignedMacAddress(other->assignedMacAddress());
}

QString DWirelessSetting::name() const
{
    Q_D(const DWirelessSetting);

    return d->m_name;
}

void DWirelessSetting::setSsid(const QByteArray &ssid)
{
    Q_D(DWirelessSetting);

    d->m_ssid = ssid;
}

QByteArray DWirelessSetting::ssid() const
{
    Q_D(const DWirelessSetting);

    return d->m_ssid;
}

void DWirelessSetting::setMode(DWirelessSetting::NetworkMode mode)
{
    Q_D(DWirelessSetting);

    d->m_mode = mode;
}

DWirelessSetting::NetworkMode DWirelessSetting::mode() const
{
    Q_D(const DWirelessSetting);

    return d->m_mode;
}

void DWirelessSetting::setBand(DWirelessSetting::FrequencyBand band)
{
    Q_D(DWirelessSetting);

    d->m_band = band;
}

DWirelessSetting::FrequencyBand DWirelessSetting::band() const
{
    Q_D(const DWirelessSetting);

    return d->m_band;
}

void DWirelessSetting::setChannel(quint32 channel)
{
    Q_D(DWirelessSetting);

    d->m_channel = channel;
}

quint32 DWirelessSetting::channel() const
{
    Q_D(const DWirelessSetting);

    return d->m_channel;
}

void DWirelessSetting::setBssid(const QByteArray &bssid)
{
    Q_D(DWirelessSetting);

    d->m_bssid = bssid;
}

QByteArray DWirelessSetting::bssid() const
{
    Q_D(const DWirelessSetting);

    return d->m_bssid;
}

void DWirelessSetting::setRate(quint32 rate)
{
    Q_D(DWirelessSetting);

    d->m_rate = rate;
}

quint32 DWirelessSetting::rate() const
{
    Q_D(const DWirelessSetting);

    return d->m_rate;
}

void DWirelessSetting::setTxPower(quint32 power)
{
    Q_D(DWirelessSetting);

    d->m_txPower = power;
}

quint32 DWirelessSetting::txPower() const
{
    Q_D(const DWirelessSetting);

    return d->m_txPower;
}

void DWirelessSetting::setAssignedMacAddress(const QString &assignedMacAddress)
{
    Q_D(DWirelessSetting);

    d->m_assignedMacAddress = assignedMacAddress;
}

QString DWirelessSetting::assignedMacAddress() const
{
    Q_D(const DWirelessSetting);

    return d->m_assignedMacAddress;
}

void DWirelessSetting::setMacAddress(const QByteArray &address)
{
    Q_D(DWirelessSetting);

    d->m_macAddress = address;
}

QByteArray DWirelessSetting::macAddress() const
{
    Q_D(const DWirelessSetting);

    return d->m_macAddress;
}

void DWirelessSetting::setGenerateMacAddressMask(const QString &macAddressMask)
{
    Q_D(DWirelessSetting);

    d->m_generateMacAddressMask = macAddressMask;
}

QString DWirelessSetting::generateMacAddressMask() const
{
    Q_D(const DWirelessSetting);

    return d->m_generateMacAddressMask;
}

void DWirelessSetting::setMacAddressBlacklist(const QStringList &list)
{
    Q_D(DWirelessSetting);

    d->m_macAddressBlacklist = list;
}

QStringList DWirelessSetting::macAddressBlacklist() const
{
    Q_D(const DWirelessSetting);

    return d->m_macAddressBlacklist;
}

void DWirelessSetting::setMacAddressRandomization(DWirelessSetting::MacAddressRandomization randomization)
{
    Q_D(DWirelessSetting);

    d->m_macAddressRandomization = randomization;
}

DWirelessSetting::MacAddressRandomization DWirelessSetting::macAddressRandomization() const
{
    Q_D(const DWirelessSetting);

    return d->m_macAddressRandomization;
}

void DWirelessSetting::setMtu(quint32 mtu)
{
    Q_D(DWirelessSetting);

    d->m_mtu = mtu;
}

quint32 DWirelessSetting::mtu() const
{
    Q_D(const DWirelessSetting);

    return d->m_mtu;
}

void DWirelessSetting::setSeenBssids(const QStringList &list)
{
    Q_D(DWirelessSetting);

    d->m_seenBssids = list;
}

QStringList DWirelessSetting::seenBssids() const
{
    Q_D(const DWirelessSetting);

    return d->m_seenBssids;
}

void DWirelessSetting::setPowerSave(DWirelessSetting::PowerSave powersave)
{
    Q_D(DWirelessSetting);

    d->m_powersave = powersave;
}

DWirelessSetting::PowerSave DWirelessSetting::powerSave() const
{
    Q_D(const DWirelessSetting);

    return d->m_powersave;
}

void DWirelessSetting::setHidden(bool hidden)
{
    Q_D(DWirelessSetting);

    d->m_hidden = hidden;
}

bool DWirelessSetting::hidden() const
{
    Q_D(const DWirelessSetting);

    return d->m_hidden;
}

void DWirelessSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_SSID))) {
        setSsid(setting.value(QLatin1String(NM_SETTING_WIRELESS_SSID)).toByteArray());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_MODE))) {
        const QString mode = setting.value(QLatin1String(NM_SETTING_WIRELESS_MODE)).toString();
        if (mode == QLatin1String(NM_SETTING_WIRELESS_MODE_INFRA)) {
            setMode(NetworkMode::Infrastructure);
        } else if (mode == QLatin1String(NM_SETTING_WIRELESS_MODE_ADHOC)) {
            setMode(NetworkMode::Adhoc);
        } else if (mode == QLatin1String(NM_SETTING_WIRELESS_MODE_AP)) {
            setMode(NetworkMode::Ap);
        }
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_BAND))) {
        const QString band = setting.value(QLatin1String(NM_SETTING_WIRELESS_BAND)).toString();
        if (band == "a") {
            setBand(FrequencyBand::A);
        } else if (band == "bg") {
            setBand(FrequencyBand::Bg);
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

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_MAC_ADDRESS))) {
        setMacAddress(setting.value(QLatin1String(NM_SETTING_WIRELESS_MAC_ADDRESS)).toByteArray());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_GENERATE_MAC_ADDRESS_MASK))) {
        setGenerateMacAddressMask(setting.value(QLatin1String(NM_SETTING_WIRELESS_GENERATE_MAC_ADDRESS_MASK)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_MAC_ADDRESS_BLACKLIST))) {
        setMacAddressBlacklist(setting.value(QLatin1String(NM_SETTING_WIRELESS_MAC_ADDRESS_BLACKLIST)).toStringList());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_MAC_ADDRESS_RANDOMIZATION))) {
        setMacAddressRandomization(static_cast<DWirelessSetting::MacAddressRandomization>(
            setting.value(QLatin1String(NM_SETTING_WIRELESS_MAC_ADDRESS_RANDOMIZATION)).toUInt()));
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_MTU))) {
        setMtu(setting.value(QLatin1String(NM_SETTING_WIRELESS_MTU)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_SEEN_BSSIDS))) {
        setSeenBssids(setting.value(QLatin1String(NM_SETTING_WIRELESS_SEEN_BSSIDS)).toStringList());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_POWERSAVE))) {
        setPowerSave(
            static_cast<DWirelessSetting::PowerSave>(setting.value(QLatin1String(NM_SETTING_WIRELESS_POWERSAVE)).toUInt()));
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRELESS_HIDDEN))) {
        setHidden(setting.value(QLatin1String(NM_SETTING_WIRELESS_HIDDEN)).toBool());
    }
}

QVariantMap DWirelessSetting::toMap() const
{
    QVariantMap setting;

    if (!ssid().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SSID), ssid());
    }

    if (mode() == NetworkMode::Infrastructure) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_MODE), QLatin1String(NM_SETTING_WIRELESS_MODE_INFRA));
    } else if (mode() == NetworkMode::Adhoc) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_MODE), QLatin1String(NM_SETTING_WIRELESS_MODE_ADHOC));
    } else if (mode() == NetworkMode::Ap) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_MODE), QLatin1String(NM_SETTING_WIRELESS_MODE_AP));
    }

    if (band() != FrequencyBand::Automatic) {
        if (band() == FrequencyBand::A) {
            setting.insert(QLatin1String(NM_SETTING_WIRELESS_BAND), "a");
        } else if (band() == FrequencyBand::Bg) {
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

    if (!macAddress().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_MAC_ADDRESS), macAddress());
    }

    if (!generateMacAddressMask().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_GENERATE_MAC_ADDRESS_MASK), generateMacAddressMask());
    }

    if (!macAddressBlacklist().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_MAC_ADDRESS_BLACKLIST), macAddressBlacklist());
    }

    setting.insert(QLatin1String(NM_SETTING_WIRELESS_MAC_ADDRESS_RANDOMIZATION), QVariant::fromValue(macAddressRandomization()));

    if (mtu()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_MTU), mtu());
    }

    if (!seenBssids().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SEEN_BSSIDS), seenBssids());
    }

    setting.insert(QLatin1String(NM_SETTING_WIRELESS_POWERSAVE), QVariant::fromValue(powerSave()));

    if (hidden()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_HIDDEN), hidden());
    }

    return setting;
}

DNETWORKMANAGER_END_NAMESPACE
