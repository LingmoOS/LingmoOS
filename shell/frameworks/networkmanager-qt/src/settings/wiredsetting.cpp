/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "wiredsetting.h"
#include "manager.h"
#include "wiredsetting_p.h"

#include <QDebug>

#define NM_SETTING_WIRED_ASSIGNED_MAC_ADDRESS "assigned-mac-address"

NetworkManager::WiredSettingPrivate::WiredSettingPrivate()
    : name(NM_SETTING_WIRED_SETTING_NAME)
    , port(NetworkManager::WiredSetting::UnknownPort)
    , speed(0)
    , duplex(NetworkManager::WiredSetting::UnknownDuplexType)
    , mtu(0)
    , s390NetType(NetworkManager::WiredSetting::Undefined)
    , wakeOnLan(NetworkManager::WiredSetting::WakeOnLanDefault)
{
    if (NetworkManager::checkVersion(1, 6, 0)) {
        autoNegotiate = false;
    } else {
        autoNegotiate = true;
    }
}

NetworkManager::WiredSetting::WiredSetting()
    : Setting(Setting::Wired)
    , d_ptr(new WiredSettingPrivate())
{
}

NetworkManager::WiredSetting::WiredSetting(const WiredSetting::Ptr &other)
    : Setting(other)
    , d_ptr(new WiredSettingPrivate())
{
    setPort(other->port());
    setSpeed(other->speed());
    setDuplexType(other->duplexType());
    setAutoNegotiate(other->autoNegotiate());
    setGenerateMacAddressMask(other->generateMacAddressMask());
    setMacAddress(other->macAddress());
    setClonedMacAddress(other->clonedMacAddress());
    setMacAddressBlacklist(other->macAddressBlacklist());
    setMtu(other->mtu());
    setS390Subchannels(other->s390Subchannels());
    setS390NetType(other->s390NetType());
    setS390Options(other->s390Options());
    setWakeOnLan(other->wakeOnLan());
    setWakeOnLanPassword(other->wakeOnLanPassword());
    setAssignedMacAddress(other->assignedMacAddress());
}

NetworkManager::WiredSetting::~WiredSetting()
{
    delete d_ptr;
}

QString NetworkManager::WiredSetting::name() const
{
    Q_D(const WiredSetting);

    return d->name;
}

void NetworkManager::WiredSetting::setPort(NetworkManager::WiredSetting::PortType port)
{
    Q_D(WiredSetting);

    d->port = port;
}

NetworkManager::WiredSetting::PortType NetworkManager::WiredSetting::port() const
{
    Q_D(const WiredSetting);

    return d->port;
}

void NetworkManager::WiredSetting::setSpeed(quint32 speed)
{
    Q_D(WiredSetting);

    d->speed = speed;
}

quint32 NetworkManager::WiredSetting::speed() const
{
    Q_D(const WiredSetting);

    return d->speed;
}

void NetworkManager::WiredSetting::setDuplexType(NetworkManager::WiredSetting::DuplexType type)
{
    Q_D(WiredSetting);

    d->duplex = type;
}

NetworkManager::WiredSetting::DuplexType NetworkManager::WiredSetting::duplexType() const
{
    Q_D(const WiredSetting);

    return d->duplex;
}

void NetworkManager::WiredSetting::setAutoNegotiate(bool autoNegotiate)
{
    Q_D(WiredSetting);

    d->autoNegotiate = autoNegotiate;
}

bool NetworkManager::WiredSetting::autoNegotiate() const
{
    Q_D(const WiredSetting);

    return d->autoNegotiate;
}

QString NetworkManager::WiredSetting::generateMacAddressMask() const
{
    Q_D(const WiredSetting);

    return d->generateMacAddressMask;
}

void NetworkManager::WiredSetting::setGenerateMacAddressMask(const QString &mask)
{
    Q_D(WiredSetting);

    d->generateMacAddressMask = mask;
}

void NetworkManager::WiredSetting::setMacAddress(const QByteArray &address)
{
    Q_D(WiredSetting);

    d->macAddress = address;
}

QByteArray NetworkManager::WiredSetting::macAddress() const
{
    Q_D(const WiredSetting);

    return d->macAddress;
}

void NetworkManager::WiredSetting::setClonedMacAddress(const QByteArray &address)
{
    Q_D(WiredSetting);

    d->clonedMacAddress = address;
}

QByteArray NetworkManager::WiredSetting::clonedMacAddress() const
{
    Q_D(const WiredSetting);

    return d->clonedMacAddress;
}

void NetworkManager::WiredSetting::setMacAddressBlacklist(const QStringList &list)
{
    Q_D(WiredSetting);

    d->macAddressBlacklist = list;
}

QStringList NetworkManager::WiredSetting::macAddressBlacklist() const
{
    Q_D(const WiredSetting);

    return d->macAddressBlacklist;
}

void NetworkManager::WiredSetting::setMtu(quint32 mtu)
{
    Q_D(WiredSetting);

    d->mtu = mtu;
}

quint32 NetworkManager::WiredSetting::mtu() const
{
    Q_D(const WiredSetting);

    return d->mtu;
}

void NetworkManager::WiredSetting::setS390Subchannels(const QStringList &channels)
{
    Q_D(WiredSetting);

    d->s390Subchannels = channels;
}

QStringList NetworkManager::WiredSetting::s390Subchannels() const
{
    Q_D(const WiredSetting);

    return d->s390Subchannels;
}

void NetworkManager::WiredSetting::setS390NetType(NetworkManager::WiredSetting::S390Nettype type)
{
    Q_D(WiredSetting);

    d->s390NetType = type;
}

NetworkManager::WiredSetting::S390Nettype NetworkManager::WiredSetting::s390NetType() const
{
    Q_D(const WiredSetting);

    return d->s390NetType;
}

void NetworkManager::WiredSetting::setS390Options(const QMap<QString, QString> &options)
{
    Q_D(WiredSetting);

    d->s390Options = options;
}

QMap<QString, QString> NetworkManager::WiredSetting::s390Options() const
{
    Q_D(const WiredSetting);

    return d->s390Options;
}

NetworkManager::WiredSetting::WakeOnLanFlags NetworkManager::WiredSetting::wakeOnLan() const
{
    Q_D(const WiredSetting);

    return d->wakeOnLan;
}

void NetworkManager::WiredSetting::setWakeOnLan(NetworkManager::WiredSetting::WakeOnLanFlags wol)
{
    Q_D(WiredSetting);

    d->wakeOnLan = wol;
}

QString NetworkManager::WiredSetting::wakeOnLanPassword() const
{
    Q_D(const WiredSetting);

    return d->wakeOnLanPassword;
}

void NetworkManager::WiredSetting::setWakeOnLanPassword(const QString &password)
{
    Q_D(WiredSetting);

    d->wakeOnLanPassword = password;
}

void NetworkManager::WiredSetting::setAssignedMacAddress(const QString &assignedMacAddress)
{
    Q_D(WiredSetting);

    d->assignedMacAddress = assignedMacAddress;
}

QString NetworkManager::WiredSetting::assignedMacAddress() const
{
    Q_D(const WiredSetting);

    return d->assignedMacAddress;
}

void NetworkManager::WiredSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_WIRED_PORT))) {
        const QString port = setting.value(QLatin1String(NM_SETTING_WIRED_PORT)).toString();

        if (port == "tp") {
            setPort(Tp);
        } else if (port == "aui") {
            setPort(Aui);
        } else if (port == "bnc") {
            setPort(Bnc);
        } else if (port == "mii") {
            setPort(Mii);
        }
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRED_SPEED))) {
        setSpeed(setting.value(QLatin1String(NM_SETTING_WIRED_SPEED)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRED_DUPLEX))) {
        const QString duplex = setting.value(QLatin1String(NM_SETTING_WIRED_DUPLEX)).toString();

        if (duplex == "half") {
            setDuplexType(Half);
        } else if (duplex == "full") {
            setDuplexType(Full);
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

    if (setting.contains(QLatin1String(NM_SETTING_WIRED_CLONED_MAC_ADDRESS))) {
        setClonedMacAddress(setting.value(QLatin1String(NM_SETTING_WIRED_CLONED_MAC_ADDRESS)).toByteArray());
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
            setS390NetType(Qeth);
        } else if (nettype == "lcs") {
            setS390NetType(Lcs);
        } else if (nettype == "ctc") {
            setS390NetType(Ctc);
        }
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRED_S390_OPTIONS))) {
        QMap<QString, QString> tmp;
        if (setting.value(QLatin1String(NM_SETTING_WIRED_S390_OPTIONS)).canConvert<QDBusArgument>()) {
            QDBusArgument arg = setting.value(QLatin1String(NM_SETTING_WIRED_S390_OPTIONS)).value<QDBusArgument>();
            tmp = qdbus_cast<NMStringMap>(arg);
        } else {
            tmp = setting.value(QLatin1String(NM_SETTING_WIRED_S390_OPTIONS)).value<NMStringMap>();
        }
        setS390Options(tmp);
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRED_WAKE_ON_LAN))) {
        setWakeOnLan((NetworkManager::WiredSetting::WakeOnLanFlags)setting.value(QLatin1String(NM_SETTING_WIRED_WAKE_ON_LAN)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRED_WAKE_ON_LAN_PASSWORD))) {
        setWakeOnLanPassword(setting.value(QLatin1String(NM_SETTING_WIRED_WAKE_ON_LAN_PASSWORD)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIRED_ASSIGNED_MAC_ADDRESS))) {
        setAssignedMacAddress(setting.value(QLatin1String(NM_SETTING_WIRED_ASSIGNED_MAC_ADDRESS)).toString());
    }
}

QVariantMap NetworkManager::WiredSetting::toMap() const
{
    QVariantMap setting;

    switch (port()) {
    case Tp:
        setting.insert(QLatin1String(NM_SETTING_WIRED_PORT), "tp");
        break;
    case Aui:
        setting.insert(QLatin1String(NM_SETTING_WIRED_PORT), "aui");
        break;
    case Bnc:
        setting.insert(QLatin1String(NM_SETTING_WIRED_PORT), "bnc");
        break;
    case Mii:
        setting.insert(QLatin1String(NM_SETTING_WIRED_PORT), "mii");
        break;
    case UnknownPort: // Make GCC hapy
        break;
    }

    // When autonegotiation is set we have to avoid setting speed and duplex, while
    // when autonegotiation is disabled, we have to specify both
    if (autoNegotiate()) {
        setting.insert(QLatin1String(NM_SETTING_WIRED_AUTO_NEGOTIATE), autoNegotiate());
    } else {
        setting.insert(QLatin1String(NM_SETTING_WIRED_AUTO_NEGOTIATE), autoNegotiate());
        setting.insert(QLatin1String(NM_SETTING_WIRED_SPEED), speed());

        switch (duplexType()) {
        case Half:
            setting.insert(QLatin1String(NM_SETTING_WIRED_DUPLEX), "half");
            break;
        case Full:
            setting.insert(QLatin1String(NM_SETTING_WIRED_DUPLEX), "full");
            break;
        case UnknownDuplexType:
            break;
        }
    }

    if (!generateMacAddressMask().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRED_GENERATE_MAC_ADDRESS_MASK), generateMacAddressMask());
    }

    if (!macAddress().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRED_MAC_ADDRESS), macAddress());
    }
    if (!clonedMacAddress().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRED_CLONED_MAC_ADDRESS), clonedMacAddress());
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
    case Qeth:
        setting.insert(QLatin1String(NM_SETTING_WIRED_S390_NETTYPE), "qeth");
        break;
    case Lcs:
        setting.insert(QLatin1String(NM_SETTING_WIRED_S390_NETTYPE), "lcs");
        break;
    case Ctc:
        setting.insert(QLatin1String(NM_SETTING_WIRED_S390_NETTYPE), "ctc");
        break;
    case Undefined:
        break;
    }

    if (!s390Options().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRED_S390_OPTIONS), QVariant::fromValue(s390Options()));
    }

    setting.insert(QLatin1String(NM_SETTING_WIRED_WAKE_ON_LAN), (uint)wakeOnLan());

    if (!wakeOnLanPassword().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRED_WAKE_ON_LAN_PASSWORD), wakeOnLanPassword());
    }

    if (!assignedMacAddress().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRED_ASSIGNED_MAC_ADDRESS), assignedMacAddress());
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::WiredSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_WIRED_PORT << ": " << setting.port() << '\n';
    dbg.nospace() << NM_SETTING_WIRED_SPEED << ": " << setting.speed() << '\n';
    dbg.nospace() << NM_SETTING_WIRED_DUPLEX << ": " << setting.duplexType() << '\n';
    dbg.nospace() << NM_SETTING_WIRED_AUTO_NEGOTIATE << ": " << setting.autoNegotiate() << '\n';
    dbg.nospace() << NM_SETTING_WIRED_GENERATE_MAC_ADDRESS_MASK << ": " << setting.generateMacAddressMask() << '\n';
    dbg.nospace() << NM_SETTING_WIRED_MAC_ADDRESS << ": " << setting.macAddress() << '\n';
    dbg.nospace() << NM_SETTING_WIRED_CLONED_MAC_ADDRESS << ": " << setting.clonedMacAddress() << '\n';
    dbg.nospace() << NM_SETTING_WIRED_MAC_ADDRESS_BLACKLIST << ": " << setting.macAddressBlacklist() << '\n';
    dbg.nospace() << NM_SETTING_WIRED_MTU << ": " << setting.mtu() << '\n';
    dbg.nospace() << NM_SETTING_WIRED_S390_SUBCHANNELS << ": " << setting.s390Subchannels() << '\n';
    dbg.nospace() << NM_SETTING_WIRED_S390_NETTYPE << ": " << setting.s390NetType() << '\n';
    dbg.nospace() << NM_SETTING_WIRED_S390_OPTIONS << ": " << setting.s390Options() << '\n';
    dbg.nospace() << NM_SETTING_WIRED_WAKE_ON_LAN << ": " << setting.wakeOnLan() << '\n';
    dbg.nospace() << NM_SETTING_WIRED_WAKE_ON_LAN_PASSWORD << ": " << setting.wakeOnLanPassword() << '\n';
    dbg.nospace() << NM_SETTING_WIRED_ASSIGNED_MAC_ADDRESS << ": " << setting.assignedMacAddress() << '\n';    

    return dbg.maybeSpace();
}
