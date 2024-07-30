/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_WIRED_SETTING_H
#define NETWORKMANAGERQT_WIRED_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QStringList>

namespace NetworkManager
{
class WiredSettingPrivate;

/**
 * Represents wired setting
 */
class NETWORKMANAGERQT_EXPORT WiredSetting : public Setting
{
public:
    typedef QSharedPointer<WiredSetting> Ptr;
    typedef QList<Ptr> List;
    enum PortType {
        UnknownPort = 0,
        Tp,
        Aui,
        Bnc,
        Mii,
    };

    enum DuplexType {
        UnknownDuplexType = 0,
        Half,
        Full,
    };

    enum S390Nettype {
        Undefined = 0,
        Qeth,
        Lcs,
        Ctc,
    };

    enum WakeOnLanFlag {
        WakeOnLanPhy = 1 << 1,
        WakeOnLanUnicast = 1 << 2,
        WakeOnLanMulticast = 1 << 3,
        WakeOnLanBroadcast = 1 << 4,
        WakeOnLanArp = 1 << 5,
        WakeOnLanMagic = 1 << 6,
        /* Special values */
        WakeOnLanDefault = 1 << 0,
        WakeOnLanIgnore = 1 << 15,
    };
    Q_DECLARE_FLAGS(WakeOnLanFlags, WakeOnLanFlag)
    Q_FLAGS(WakeOnLanFlag)

    WiredSetting();
    explicit WiredSetting(const Ptr &other);
    ~WiredSetting() override;

    QString name() const override;

    void setPort(PortType port);
    PortType port() const;

    void setSpeed(quint32 speed);
    quint32 speed() const;

    void setDuplexType(DuplexType type);
    DuplexType duplexType() const;

    void setAutoNegotiate(bool autoNegotiate);
    bool autoNegotiate() const;

    QString generateMacAddressMask() const;
    void setGenerateMacAddressMask(const QString &mask);

    void setMacAddress(const QByteArray &address);
    QByteArray macAddress() const;

    void setClonedMacAddress(const QByteArray &address);
    QByteArray clonedMacAddress() const;

    void setMacAddressBlacklist(const QStringList &list);
    QStringList macAddressBlacklist() const;

    void setMtu(quint32 mtu);
    quint32 mtu() const;

    void setS390Subchannels(const QStringList &channels);
    QStringList s390Subchannels() const;

    void setS390NetType(S390Nettype type);
    S390Nettype s390NetType() const;

    void setS390Options(const QMap<QString, QString> &options);
    QMap<QString, QString> s390Options() const;

    WakeOnLanFlags wakeOnLan() const;
    void setWakeOnLan(WakeOnLanFlags wol);

    QString wakeOnLanPassword() const;
    void setWakeOnLanPassword(const QString &password);

    QString assignedMacAddress() const;
    void setAssignedMacAddress(const QString &assignedMacAddress);

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    WiredSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(WiredSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const WiredSetting &setting);

Q_DECLARE_OPERATORS_FOR_FLAGS(WiredSetting::WakeOnLanFlags)

}

#endif // NETWORKMANAGERQT_WIRED_SETTING_H
