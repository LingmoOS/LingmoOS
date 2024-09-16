// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dwiredsetting_p.h"
#include <QDBusArgument>

#define NM_SETTING_WIRED_ASSIGNED_MAC_ADDRESS "assigned-mac-address"

DNETWORKMANAGER_BEGIN_NAMESPACE

DWiredSettingPrivate::DWiredSettingPrivate()
    : m_autoNegotiate(false)
    , m_speed(0)
    , m_mtu(0)
    , m_port(DWiredSetting::PortType::Unknown)
    , m_duplex(DWiredSetting::DuplexType::Unknown)
    , m_s390NetType(DWiredSetting::S390Nettype::Undefined)
    , m_wakeOnLan(DWiredSetting::WakeOnLanFlag::Default)
    , m_name(NM_SETTING_WIRED_SETTING_NAME)
{
}

DWiredSetting::DWiredSetting()
    : DNMSetting(DNMSetting::SettingType::Wired)
    , d_ptr(new DWiredSettingPrivate())
{
}

DWiredSetting::DWiredSetting(const QSharedPointer<DWiredSetting> &other)
    : DNMSetting(other)
    , d_ptr(new DWiredSettingPrivate())
{
    setPort(other->port());
    setSpeed(other->speed());
    setDuplexType(other->duplexType());
    setAutoNegotiate(other->autoNegotiate());
    setGenerateMacAddressMask(other->generateMacAddressMask());
    setMacAddress(other->macAddress());
    setMacAddressBlacklist(other->macAddressBlacklist());
    setMtu(other->mtu());
    setS390Subchannels(other->s390Subchannels());
    setS390NetType(other->s390NetType());
    setS390Options(other->s390Options());
    setWakeOnLan(other->wakeOnLan());
    setWakeOnLanPassword(other->wakeOnLanPassword());
    setAssignedMacAddress(other->assignedMacAddress());
}

QString NetworkManager::DWiredSetting::name() const
{
    Q_D(const DWiredSetting);

    return d->m_name;
}

void NetworkManager::DWiredSetting::setPort(NetworkManager::DWiredSetting::PortType port)
{
    Q_D(DWiredSetting);

    d->m_port = port;
}

NetworkManager::DWiredSetting::PortType NetworkManager::DWiredSetting::port() const
{
    Q_D(const DWiredSetting);

    return d->m_port;
}

void NetworkManager::DWiredSetting::setSpeed(quint32 speed)
{
    Q_D(DWiredSetting);

    d->m_speed = speed;
}

quint32 NetworkManager::DWiredSetting::speed() const
{
    Q_D(const DWiredSetting);

    return d->m_speed;
}

void NetworkManager::DWiredSetting::setDuplexType(NetworkManager::DWiredSetting::DuplexType type)
{
    Q_D(DWiredSetting);

    d->m_duplex = type;
}

NetworkManager::DWiredSetting::DuplexType NetworkManager::DWiredSetting::duplexType() const
{
    Q_D(const DWiredSetting);

    return d->m_duplex;
}

void NetworkManager::DWiredSetting::setAutoNegotiate(bool autoNegotiate)
{
    Q_D(DWiredSetting);

    d->m_autoNegotiate = autoNegotiate;
}

bool NetworkManager::DWiredSetting::autoNegotiate() const
{
    Q_D(const DWiredSetting);

    return d->m_autoNegotiate;
}

QString NetworkManager::DWiredSetting::generateMacAddressMask() const
{
    Q_D(const DWiredSetting);

    return d->m_generateMacAddressMask;
}

void NetworkManager::DWiredSetting::setGenerateMacAddressMask(const QString &mask)
{
    Q_D(DWiredSetting);

    d->m_generateMacAddressMask = mask;
}

void NetworkManager::DWiredSetting::setMacAddress(const QByteArray &address)
{
    Q_D(DWiredSetting);

    d->m_macAddress = address;
}

QByteArray NetworkManager::DWiredSetting::macAddress() const
{
    Q_D(const DWiredSetting);

    return d->m_macAddress;
}

void NetworkManager::DWiredSetting::setMacAddressBlacklist(const QStringList &list)
{
    Q_D(DWiredSetting);

    d->m_macAddressBlacklist = list;
}

QStringList NetworkManager::DWiredSetting::macAddressBlacklist() const
{
    Q_D(const DWiredSetting);

    return d->m_macAddressBlacklist;
}

void NetworkManager::DWiredSetting::setMtu(quint32 mtu)
{
    Q_D(DWiredSetting);

    d->m_mtu = mtu;
}

quint32 NetworkManager::DWiredSetting::mtu() const
{
    Q_D(const DWiredSetting);

    return d->m_mtu;
}

void NetworkManager::DWiredSetting::setS390Subchannels(const QStringList &channels)
{
    Q_D(DWiredSetting);

    d->m_s390Subchannels = channels;
}

QStringList NetworkManager::DWiredSetting::s390Subchannels() const
{
    Q_D(const DWiredSetting);

    return d->m_s390Subchannels;
}

void NetworkManager::DWiredSetting::setS390NetType(NetworkManager::DWiredSetting::S390Nettype type)
{
    Q_D(DWiredSetting);

    d->m_s390NetType = type;
}

NetworkManager::DWiredSetting::S390Nettype NetworkManager::DWiredSetting::s390NetType() const
{
    Q_D(const DWiredSetting);

    return d->m_s390NetType;
}

void NetworkManager::DWiredSetting::setS390Options(const QMap<QString, QString> &options)
{
    Q_D(DWiredSetting);

    d->m_s390Options = options;
}

QMap<QString, QString> NetworkManager::DWiredSetting::s390Options() const
{
    Q_D(const DWiredSetting);

    return d->m_s390Options;
}

NetworkManager::DWiredSetting::WakeOnLanFlags NetworkManager::DWiredSetting::wakeOnLan() const
{
    Q_D(const DWiredSetting);

    return d->m_wakeOnLan;
}

void NetworkManager::DWiredSetting::setWakeOnLan(NetworkManager::DWiredSetting::WakeOnLanFlags wol)
{
    Q_D(DWiredSetting);

    d->m_wakeOnLan = wol;
}

QString NetworkManager::DWiredSetting::wakeOnLanPassword() const
{
    Q_D(const DWiredSetting);

    return d->m_wakeOnLanPassword;
}

void NetworkManager::DWiredSetting::setWakeOnLanPassword(const QString &password)
{
    Q_D(DWiredSetting);

    d->m_wakeOnLanPassword = password;
}

void NetworkManager::DWiredSetting::setAssignedMacAddress(const QString &assignedMacAddress)
{
    Q_D(DWiredSetting);

    d->m_assignedMacAddress = assignedMacAddress;
}

QString NetworkManager::DWiredSetting::assignedMacAddress() const
{
    Q_D(const DWiredSetting);

    return d->m_assignedMacAddress;
}

void NetworkManager::DWiredSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_WIRED_PORT))) {
        const QString port = setting.value(QLatin1String(NM_SETTING_WIRED_PORT)).toString();

        if (port == "tp") {
            setPort(PortType::Tp);
        } else if (port == "aui") {
            setPort(PortType::Aui);
        } else if (port == "bnc") {
            setPort(PortType::Bnc);
        } else if (port == "mii") {
            setPort(PortType::Mii);
        }
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRED_SPEED))) {
        setSpeed(setting.value(QLatin1String(NM_SETTING_WIRED_SPEED)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRED_DUPLEX))) {
        const QString duplex = setting.value(QLatin1String(NM_SETTING_WIRED_DUPLEX)).toString();

        if (duplex == "half") {
            setDuplexType(DuplexType::Half);
        } else if (duplex == "full") {
            setDuplexType(DuplexType::Full);
        }
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRED_AUTO_NEGOTIATE))) {
        setAutoNegotiate(setting.value(QLatin1String(NM_SETTING_WIRED_AUTO_NEGOTIATE)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRED_GENERATE_MAC_ADDRESS_MASK))) {
        setGenerateMacAddressMask(setting.value(QLatin1String(NM_SETTING_WIRED_GENERATE_MAC_ADDRESS_MASK)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRED_MAC_ADDRESS))) {
        setMacAddress(setting.value(QLatin1String(NM_SETTING_WIRED_MAC_ADDRESS)).toByteArray());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRED_MAC_ADDRESS_BLACKLIST))) {
        setMacAddressBlacklist(setting.value(QLatin1String(NM_SETTING_WIRED_MAC_ADDRESS_BLACKLIST)).toStringList());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRED_MTU))) {
        setMtu(setting.value(QLatin1String(NM_SETTING_WIRED_MTU)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRED_S390_SUBCHANNELS))) {
        setS390Subchannels(setting.value(QLatin1String(NM_SETTING_WIRED_S390_SUBCHANNELS)).toStringList());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRED_S390_NETTYPE))) {
        const QString nettype = setting.value(QLatin1String(NM_SETTING_WIRED_S390_NETTYPE)).toString();

        if (nettype == "qeth") {
            setS390NetType(S390Nettype::Qeth);
        } else if (nettype == "lcs") {
            setS390NetType(S390Nettype::Lcs);
        } else if (nettype == "ctc") {
            setS390NetType(S390Nettype::Ctc);
        }
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRED_S390_OPTIONS))) {
        QMap<QString, QString> tmp;
        if (setting.value(QLatin1String(NM_SETTING_WIRED_S390_OPTIONS)).canConvert<QDBusArgument>()) {
            auto arg = setting.value(QLatin1String(NM_SETTING_WIRED_S390_OPTIONS)).value<QDBusArgument>();
            tmp = qdbus_cast<QMap<QString, QString>>(arg);
        } else {
            tmp = setting.value(QLatin1String(NM_SETTING_WIRED_S390_OPTIONS)).value<QMap<QString, QString>>();
        }
        setS390Options(tmp);
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRED_WAKE_ON_LAN))) {
        setWakeOnLan(
            (NetworkManager::DWiredSetting::WakeOnLanFlags)setting.value(QLatin1String(NM_SETTING_WIRED_WAKE_ON_LAN)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRED_WAKE_ON_LAN_PASSWORD))) {
        setWakeOnLanPassword(setting.value(QLatin1String(NM_SETTING_WIRED_WAKE_ON_LAN_PASSWORD)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRED_ASSIGNED_MAC_ADDRESS))) {
        setAssignedMacAddress(setting.value(QLatin1String(NM_SETTING_WIRED_ASSIGNED_MAC_ADDRESS)).toString());
    }
}

QVariantMap NetworkManager::DWiredSetting::toMap() const
{
    QVariantMap setting;

    switch (port()) {
        case PortType::Tp:
            setting.insert(QLatin1String(NM_SETTING_WIRED_PORT), "tp");
            break;
        case PortType::Aui:
            setting.insert(QLatin1String(NM_SETTING_WIRED_PORT), "aui");
            break;
        case PortType::Bnc:
            setting.insert(QLatin1String(NM_SETTING_WIRED_PORT), "bnc");
            break;
        case PortType::Mii:
            setting.insert(QLatin1String(NM_SETTING_WIRED_PORT), "mii");
            break;
        case PortType::Unknown:
            break;
    }

    if (autoNegotiate()) {
        setting.insert(QLatin1String(NM_SETTING_WIRED_AUTO_NEGOTIATE), autoNegotiate());
    } else {
        setting.insert(QLatin1String(NM_SETTING_WIRED_AUTO_NEGOTIATE), autoNegotiate());
        setting.insert(QLatin1String(NM_SETTING_WIRED_SPEED), speed());

        switch (duplexType()) {
            case DuplexType::Half:
                setting.insert(QLatin1String(NM_SETTING_WIRED_DUPLEX), "half");
                break;
            case DuplexType::Full:
                setting.insert(QLatin1String(NM_SETTING_WIRED_DUPLEX), "full");
                break;
            case DuplexType::Unknown:
                break;
        }
    }

    if (!generateMacAddressMask().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRED_GENERATE_MAC_ADDRESS_MASK), generateMacAddressMask());
    }

    if (!macAddress().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRED_MAC_ADDRESS), macAddress());
    }
    if (!macAddressBlacklist().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRED_MAC_ADDRESS_BLACKLIST), macAddressBlacklist());
    }
    if (mtu()) {
        setting.insert(QLatin1String(NM_SETTING_WIRED_MTU), mtu());
    }
    if (!s390Subchannels().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRED_S390_SUBCHANNELS), s390Subchannels());
    }

    switch (s390NetType()) {
        case S390Nettype::Qeth:
            setting.insert(QLatin1String(NM_SETTING_WIRED_S390_NETTYPE), "qeth");
            break;
        case S390Nettype::Lcs:
            setting.insert(QLatin1String(NM_SETTING_WIRED_S390_NETTYPE), "lcs");
            break;
        case S390Nettype::Ctc:
            setting.insert(QLatin1String(NM_SETTING_WIRED_S390_NETTYPE), "ctc");
            break;
        case S390Nettype::Undefined:
            break;
    }

    if (!s390Options().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRED_S390_OPTIONS), QVariant::fromValue(s390Options()));
    }

    setting.insert(QLatin1String(NM_SETTING_WIRED_WAKE_ON_LAN), static_cast<uint>(wakeOnLan()));

    if (!wakeOnLanPassword().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRED_WAKE_ON_LAN_PASSWORD), wakeOnLanPassword());
    }

    if (!assignedMacAddress().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRED_ASSIGNED_MAC_ADDRESS), assignedMacAddress());
    }

    return setting;
}

DNETWORKMANAGER_END_NAMESPACE
