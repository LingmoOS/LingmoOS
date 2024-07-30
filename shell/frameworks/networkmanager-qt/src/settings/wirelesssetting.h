/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_WIRELESS_SETTING_H
#define NETWORKMANAGERQT_WIRELESS_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QSharedPointer>
#include <QStringList>

namespace NetworkManager
{
class WirelessSettingPrivate;

/**
 * Represents wireless setting
 */
class NETWORKMANAGERQT_EXPORT WirelessSetting : public Setting
{
public:
    typedef QSharedPointer<WirelessSetting> Ptr;
    typedef QList<Ptr> List;

    enum NetworkMode {
        Infrastructure,
        Adhoc,
        Ap,
    };

    enum FrequencyBand {
        Automatic,
        A,
        Bg,
    };

    enum PowerSave {
        PowerSaveDefault = 0,
        PowerSaveIgnore = 1,
        PowerSaveDisable = 2,
        PowerSaveEnable = 3,
    };

    WirelessSetting();
    explicit WirelessSetting(const Ptr &setting);
    ~WirelessSetting() override;

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

    /// @deprecated since NM 1.4.0, will internally use assigned-mac-address property
    void setClonedMacAddress(const QByteArray &address);
    QByteArray clonedMacAddress() const;

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

    /// @deprecated since NM 1.0.0
    void setSecurity(const QString &security);
    QString security() const;

    void setHidden(bool hidden);
    bool hidden() const;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    WirelessSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(WirelessSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const WirelessSetting &setting);

}

#endif // NETWORKMANAGERQT_WIRELESS_SETTING_H
