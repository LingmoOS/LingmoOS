// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DWIREDSETTING_H
#define DWIREDSETTING_H

#include "dnmsetting.h"
#include <QStringList>

DNETWORKMANAGER_BEGIN_NAMESPACE

class DWiredSettingPrivate;

class DWiredSetting : public DNMSetting
{
public:
    enum class PortType {
        Unknown = 0,
        Tp,
        Aui,
        Bnc,
        Mii,
    };

    enum class DuplexType {
        Unknown = 0,
        Half,
        Full,
    };

    enum class S390Nettype {
        Undefined = 0,
        Qeth,
        Lcs,
        Ctc,
    };

    enum class WakeOnLanFlag {
        Phy = 1 << 1,
        Unicast = 1 << 2,
        Multicast = 1 << 3,
        Broadcast = 1 << 4,
        Arp = 1 << 5,
        Magic = 1 << 6,
        Default = 1 << 0,
        Ignore = 1 << 15,
    };

    Q_DECLARE_FLAGS(WakeOnLanFlags, WakeOnLanFlag)
    Q_FLAGS(WakeOnLanFlag)

    DWiredSetting();
    explicit DWiredSetting(const QSharedPointer<DWiredSetting> &other);
    ~DWiredSetting() override = default;

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
    QScopedPointer<DWiredSettingPrivate> d_ptr;

private:
    Q_DECLARE_PRIVATE(DWiredSetting)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DWiredSetting::WakeOnLanFlags)

DNETWORKMANAGER_END_NAMESPACE

#endif
