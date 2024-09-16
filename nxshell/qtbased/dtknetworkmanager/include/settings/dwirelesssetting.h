// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DWIRELESSSETTING_H
#define DWIRELESSSETTING_H

#include "dnmsetting.h"
#include <QSharedPointer>
#include <QStringList>

DNETWORKMANAGER_BEGIN_NAMESPACE

class DWirelessSettingPrivate;

class DWirelessSetting : public DNMSetting
{
public:
    enum class NetworkMode {
        Infrastructure = 0,
        Adhoc,
        Ap,
    };

    enum class FrequencyBand {
        Automatic = 0,
        A,
        Bg,
    };

    enum class PowerSave {
        Default = 0,
        Ignore,
        Disable,
        Enable,
    };

    enum class MacAddressRandomization {
        Default = 0,
        Never,
        Always,
    };

    DWirelessSetting();
    explicit DWirelessSetting(const QSharedPointer<DWirelessSetting> &other);
    ~DWirelessSetting() override = default;

    QString name() const override;

    void setSsid(const QByteArray &ssid);
    QByteArray ssid() const;

    void setMode(NetworkMode mode);
    NetworkMode mode() const;

    void setBand(FrequencyBand band);
    FrequencyBand band() const;

    void setChannel(quint32 channel);
    quint32 channel() const;

    void setBssid(const QByteArray &bssid);
    QByteArray bssid() const;

    void setRate(quint32 rate);
    quint32 rate() const;

    void setTxPower(quint32 power);
    quint32 txPower() const;

    void setAssignedMacAddress(const QString &assignedMacAddress);
    QString assignedMacAddress() const;

    void setMacAddress(const QByteArray &address);
    QByteArray macAddress() const;

    void setGenerateMacAddressMask(const QString &macAddressMask);
    QString generateMacAddressMask() const;

    void setMacAddressBlacklist(const QStringList &list);
    QStringList macAddressBlacklist() const;

    void setMacAddressRandomization(MacAddressRandomization randomization);
    MacAddressRandomization macAddressRandomization() const;

    void setMtu(quint32 mtu);
    quint32 mtu() const;

    void setSeenBssids(const QStringList &list);
    QStringList seenBssids() const;

    void setPowerSave(PowerSave powersave);
    PowerSave powerSave() const;

    void setHidden(bool hidden);
    bool hidden() const;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    QScopedPointer<DWirelessSettingPrivate> d_ptr;

private:
    Q_DECLARE_PRIVATE(DWirelessSetting)
};

DNETWORKMANAGER_END_NAMESPACE

Q_DECLARE_METATYPE(DTK_DNETWORKMANAGER_NAMESPACE::DWirelessSetting::MacAddressRandomization)
Q_DECLARE_METATYPE(DTK_DNETWORKMANAGER_NAMESPACE::DWirelessSetting::PowerSave)

#endif
