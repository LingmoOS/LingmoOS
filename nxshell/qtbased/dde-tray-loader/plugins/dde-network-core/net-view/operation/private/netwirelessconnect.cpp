// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netwirelessconnect.h"

#include "configsetting.h"

#include <NetworkManagerQt/AccessPoint>
#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/Security8021xSetting>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/Utils>
#include <NetworkManagerQt/WirelessSecuritySetting>
#include <NetworkManagerQt/WirelessSetting>
#include <wirelessdevice.h>

#include <QDBusMetaType>

using namespace NetworkManager;

#define MANUAL 1

namespace dde {
namespace network {

NetWirelessConnect::NetWirelessConnect(dde::network::WirelessDevice *device, dde::network::AccessPoints *ap, QObject *parent)
    : QObject(parent)
    , m_device(device)
    , m_accessPoint(ap)
    , m_needUpdate(false)
    , m_needIdentify(false)
{
    qDBusRegisterMetaType<IpV6DBusAddress>();
    qDBusRegisterMetaType<IpV6DBusAddressList>();
}

NetWirelessConnect::~NetWirelessConnect() = default;

void NetWirelessConnect::setSsid(const QString &ssid)
{
    m_ssid = ssid;
    m_connectionSettings.clear();
}

void NetWirelessConnect::updateProp()
{
    if (!m_accessPoint) {
        return;
    }
    AccessPoint nmAp(m_accessPoint->path());
    AccessPoint::WpaFlags wpaFlags = nmAp.wpaFlags();
    AccessPoint::WpaFlags rsnFlags = nmAp.rsnFlags();
    m_needIdentify = (wpaFlags.testFlag(AccessPoint::WpaFlag::KeyMgmt8021x) || rsnFlags.testFlag(AccessPoint::WpaFlag::KeyMgmt8021x));
}

QString NetWirelessConnect::needSecrets()
{
    if (m_connectionSettings) {
        QList<Setting::SettingType> settingType = { Setting::SettingType::WirelessSecurity, Setting::SettingType::Security8021x };
        for (auto &&type : settingType) {
            NetworkManager::Setting::Ptr setting = m_connectionSettings->setting(type);
            if (setting) {
                const QStringList &secrets = setting->needSecrets();
                if (!secrets.isEmpty())
                    return setting->needSecrets().first();
            }
        }
    }
    if (m_accessPoint) {
        WirelessSecuritySetting::KeyMgmt keyMgmt = getKeyMgmtByAp(m_accessPoint);
        switch (keyMgmt) {
        case WirelessSecuritySetting::KeyMgmt::WpaPsk:
            return "psk";
        case WirelessSecuritySetting::KeyMgmt::Wep:
            return "wep-key0";
        default:
            break;
        }
    }
    return "password";
}

bool NetWirelessConnect::needInputPassword()
{
    return hasPassword();
}

bool NetWirelessConnect::needInputIdentify()
{
    return m_needIdentify;
}

bool NetWirelessConnect::passwordIsValid(const QString &password)
{
    WirelessSecuritySetting::KeyMgmt keyMgmt;
    if (m_connectionSettings) {
        WirelessSecuritySetting::Ptr wsSetting = m_connectionSettings->setting(Setting::SettingType::WirelessSecurity).staticCast<WirelessSecuritySetting>();
        keyMgmt = wsSetting->keyMgmt();
    } else {
        keyMgmt = getKeyMgmtByAp(m_accessPoint);
    }
    if (keyMgmt == WirelessSecuritySetting::KeyMgmt::Wep) {
        return wepKeyIsValid(password, WirelessSecuritySetting::WepKeyType::Passphrase);
    }
    return wpaPskIsValid(password);
}

WirelessSecuritySetting::KeyMgmt NetWirelessConnect::getKeyMgmtByAp(dde::network::AccessPoints *ap)
{
    if (nullptr == ap) {
        return WirelessSecuritySetting::WpaPsk;
    }
    AccessPoint::Ptr nmAp(new AccessPoint(ap->path()));
    AccessPoint::Capabilities capabilities = nmAp->capabilities();
    AccessPoint::WpaFlags wpaFlags = nmAp->wpaFlags();
    AccessPoint::WpaFlags rsnFlags = nmAp->rsnFlags();

    WirelessSecuritySetting::KeyMgmt keyMgmt = WirelessSecuritySetting::KeyMgmt::WpaNone;

    if (capabilities.testFlag(AccessPoint::Capability::Privacy) && !wpaFlags.testFlag(AccessPoint::WpaFlag::KeyMgmtPsk) && !wpaFlags.testFlag(AccessPoint::WpaFlag::KeyMgmt8021x)) {
        keyMgmt = WirelessSecuritySetting::KeyMgmt::Wep;
    }

#ifdef USE_DEEPIN_NMQT
    // 判断是否是wpa3加密的，因为wpa3加密方式，实际上是wpa2的扩展，所以其中会包含KeyMgmtPsk枚举值
    if (wpaFlags.testFlag(NetworkManager::AccessPoint::WpaFlag::KeyMgmtSAE) || rsnFlags.testFlag(NetworkManager::AccessPoint::WpaFlag::KeyMgmtSAE)) {
        keyMgmt = NetworkManager::WirelessSecuritySetting::KeyMgmt::SAE;
    }
#else
    if (wpaFlags.testFlag(NetworkManager::AccessPoint::WpaFlag::KeyMgmtSAE) || rsnFlags.testFlag(NetworkManager::AccessPoint::WpaFlag::KeyMgmtSAE)) {
        keyMgmt = NetworkManager::WirelessSecuritySetting::KeyMgmt::SAE;
    }
#endif

    if (wpaFlags.testFlag(AccessPoint::WpaFlag::KeyMgmt8021x) || rsnFlags.testFlag(AccessPoint::WpaFlag::KeyMgmt8021x)) {
        keyMgmt = WirelessSecuritySetting::KeyMgmt::WpaEap;
    }

    if (wpaFlags.testFlag(AccessPoint::WpaFlag::KeyMgmtEapSuiteB192) || rsnFlags.testFlag(AccessPoint::WpaFlag::KeyMgmtEapSuiteB192)) {
        keyMgmt = WirelessSecuritySetting::KeyMgmt::WpaEapSuiteB192;
    }

    if (wpaFlags.testFlag(AccessPoint::WpaFlag::KeyMgmtPsk) || rsnFlags.testFlag(AccessPoint::WpaFlag::KeyMgmtPsk)) {
        keyMgmt = WirelessSecuritySetting::KeyMgmt::WpaPsk;
    }
    return keyMgmt;
}

Security8021xSetting::EapMethod NetWirelessConnect::getEapMethod() const
{
    static NetworkManager::Security8021xSetting::EapMethod method = NetworkManager::Security8021xSetting::EapMethod::EapMethodUnknown;
    if (method != NetworkManager::Security8021xSetting::EapMethod::EapMethodUnknown)
        return method;

    static QMap<QString, NetworkManager::Security8021xSetting::EapMethod> eapMethods =
        {{"tls", NetworkManager::Security8021xSetting::EapMethod::EapMethodTls},
         {"leap", NetworkManager::Security8021xSetting::EapMethod::EapMethodLeap},
         {"fast", NetworkManager::Security8021xSetting::EapMethod::EapMethodFast},
         {"tls", NetworkManager::Security8021xSetting::EapMethod::EapMethodTtls},
         {"peap", NetworkManager::Security8021xSetting::EapMethod::EapMethodPeap}};

    method = eapMethods.value(ConfigSetting::instance()->wpaEapAuthen().toLower(), NetworkManager::Security8021xSetting::EapMethod::EapMethodPeap);
    return method;
}

Security8021xSetting::AuthMethod NetWirelessConnect::getAuthMethod() const
{
    static QMap<QString, NetworkManager::Security8021xSetting::AuthMethod> authMethods =
        {{QString("Pap"), NetworkManager::Security8021xSetting::AuthMethodPap},
        {QString("chap"), NetworkManager::Security8021xSetting::AuthMethodChap},
        {QString("mschap"), NetworkManager::Security8021xSetting::AuthMethodMschap},
        {QString("mschapv2"), NetworkManager::Security8021xSetting::AuthMethodMschapv2},
        {QString("gtc"), NetworkManager::Security8021xSetting::AuthMethodGtc},
        {QString("otp"), NetworkManager::Security8021xSetting::AuthMethodOtp},
        {QString("md5"), NetworkManager::Security8021xSetting::AuthMethodMd5},
        {QString("tls"), NetworkManager::Security8021xSetting::AuthMethodTls}};

    const QString authMethodName = ConfigSetting::instance()->wpaEapAuthmethod().toLower();
    return authMethods.value(authMethodName, NetworkManager::Security8021xSetting::AuthMethodGtc);
}

bool NetWirelessConnect::initConnection()
{
    // 隐藏网络直接创建settings
    if (m_accessPoint) {
        NetworkManager::Connection::Ptr conn;
        for (const ActiveConnection::Ptr &con : NetworkManager::activeConnections()) {
            if (con->type() != ConnectionSettings::ConnectionType::Wireless || con->id() != m_ssid)
                continue;

            NetworkManager::ConnectionSettings::Ptr connSettings = con->connection()->settings();
            NetworkManager::WirelessSetting::Ptr wSetting = connSettings->setting(NetworkManager::Setting::SettingType::Wireless).staticCast<NetworkManager::WirelessSetting>();
            if (wSetting.isNull())
                continue;

            QString settingMacAddress = wSetting->macAddress().toHex().toUpper();
            QString deviceMacAddress = m_device->realHwAdr().remove(":");
            if (!settingMacAddress.isEmpty() && settingMacAddress != deviceMacAddress)
                continue;

            conn = findConnectionByUuid(con->uuid());
            break;
        }

        if (conn.isNull()) {
            // 如果找到的conn是空的,当前的setting也需要重置
            m_connectionSettings.reset();
            for (auto item : m_device->items()) {
                if (item->connection()->ssid() != m_ssid)
                    continue;

                QString uuid = item->connection()->uuid();
                if (!uuid.isEmpty()) {
                    conn = findConnectionByUuid(uuid);
                    if (!conn.isNull() && conn->isValid()) {
                        break;
                    }
                }
            }
        }
        if (!conn.isNull() && conn->isValid()) {
            m_connectionSettings = conn->settings();
        }
    }
    // 没连接过的需要新建连接
    if (m_connectionSettings.isNull()) {
        qCInfo(DNC) << "Wireless connect init, the connection: " << m_connectionSettings << "is null";
        // 如果是企业版ap第一次连接时,在锁屏和greeter下需要发通知告知用户需要在控制中心连接,dock下跳转控制中心
        updateProp();
        m_connectionSettings = QSharedPointer<ConnectionSettings>(new ConnectionSettings(ConnectionSettings::ConnectionType::Wireless));
        // 创建uuid
        QString uuid = m_connectionSettings->createNewUuid();
        while (findConnectionByUuid(uuid)) {
            qint64 second = QDateTime::currentDateTime().toSecsSinceEpoch();
            uuid.replace(24, QString::number(second).length(), QString::number(second));
        }
        m_connectionSettings->setUuid(uuid);
        m_connectionSettings->setId(m_ssid);
        if (!m_accessPoint) {
            m_connectionSettings->setting(Setting::SettingType::Wireless).staticCast<WirelessSetting>()->setHidden(true);
        }
        qCInfo(DNC) << "Wireless connect init, create connect: ssid: " << m_ssid << ", uuid: " << uuid << ", access point: " << m_accessPoint;
        if (m_accessPoint) {
            WirelessSecuritySetting::Ptr wsSetting = m_connectionSettings->setting(Setting::WirelessSecurity).dynamicCast<WirelessSecuritySetting>();
            WirelessSecuritySetting::KeyMgmt keyMgmt = getKeyMgmtByAp(m_accessPoint);
            if (keyMgmt != WirelessSecuritySetting::KeyMgmt::WpaNone) {
                wsSetting->setKeyMgmt(keyMgmt);
                if (keyMgmt == WirelessSecuritySetting::KeyMgmt::Wep) {
                    wsSetting->setWepKeyFlags(Setting::None);
                } else if (keyMgmt == WirelessSecuritySetting::KeyMgmt::WpaPsk
#ifdef USE_DEEPIN_NMQT
                           || keyMgmt == WirelessSecuritySetting::KeyMgmt::SAE) {
#else
                           || keyMgmt == WirelessSecuritySetting::KeyMgmt::SAE) {
#endif
                    wsSetting->setPskFlags(Setting::None);
                }
                wsSetting->setInitialized(true);
            }
        }
        WirelessSetting::Ptr const wirelessSetting = m_connectionSettings->setting(Setting::Wireless).dynamicCast<WirelessSetting>();
        wirelessSetting->setSsid(m_ssid.toUtf8());
        wirelessSetting->setInitialized(true);
        m_needUpdate = true;
        if (m_needIdentify) {
            return false;
        }
    }
    return true;
}

void NetWirelessConnect::setPassword(const QString &password)
{
    WirelessSecuritySetting::Ptr const wsSetting = m_connectionSettings->setting(Setting::SettingType::WirelessSecurity).staticCast<WirelessSecuritySetting>();
    bool const isHidden = m_connectionSettings->setting(Setting::SettingType::Wireless).staticCast<WirelessSetting>()->hidden();
    WirelessSecuritySetting::KeyMgmt const keyMgmt = isHidden ? getKeyMgmtByAp(m_accessPoint) : wsSetting->keyMgmt();
    wsSetting->setKeyMgmt(keyMgmt);
    if (keyMgmt == WirelessSecuritySetting::KeyMgmt::Wep) {
        wsSetting->setWepKey0(password);
    } else if (keyMgmt == WirelessSecuritySetting::KeyMgmt::WpaPsk
#ifdef USE_DEEPIN_NMQT
               || keyMgmt == WirelessSecuritySetting::KeyMgmt::SAE) {
#else
               || keyMgmt == WirelessSecuritySetting::KeyMgmt::SAE) {
#endif
        wsSetting->setPsk(password);

        if (isHidden && keyMgmt == WirelessSecuritySetting::KeyMgmt::WpaPsk) {
            wsSetting->setAuthAlg(WirelessSecuritySetting::AuthAlg::Open);
        }
    }
    wsSetting->setInitialized(true);
    m_needUpdate = true;
}

QVariantMap NetWirelessConnect::setAuthen(const QVariantMap &param)
{
    NetworkManager::WirelessSecuritySetting::Ptr wsSetting = m_connectionSettings->setting(Setting::SettingType::WirelessSecurity).staticCast<WirelessSecuritySetting>();
    NetworkManager::WirelessSecuritySetting::KeyMgmt keyMgmt = getKeyMgmtByAp(m_accessPoint);
    wsSetting->setKeyMgmt(keyMgmt);
    if (keyMgmt == NetworkManager::WirelessSecuritySetting::KeyMgmt::Wep) {
        wsSetting->setWepKey0(param.value("wep-key0").toString());
    } else if (keyMgmt == NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaPsk || keyMgmt == NetworkManager::WirelessSecuritySetting::KeyMgmt::SAE) {
        wsSetting->setPsk(param.value("psk").toString());

        if (keyMgmt == NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaPsk) {
            wsSetting->setAuthAlg(NetworkManager::WirelessSecuritySetting::AuthAlg::Open);
        } else {
            wsSetting->setAuthAlg(NetworkManager::WirelessSecuritySetting::AuthAlg::None);
        }
    } else if (keyMgmt == NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaEap) {
        wsSetting->setAuthAlg(NetworkManager::WirelessSecuritySetting::AuthAlg::None);
        NetworkManager::Security8021xSetting::Ptr securitySetting =
                m_connectionSettings->setting(NetworkManager::Setting::SettingType::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();
        if (!securitySetting.isNull()) {
            QList<NetworkManager::Security8021xSetting::EapMethod> methods = securitySetting->eapMethods();
            if (methods.isEmpty()) {
                // 如果认证方式为空,说明是第一次进来,此时需要设置默认的认证方式
                securitySetting->setEapMethods(QList<NetworkManager::Security8021xSetting::EapMethod>() << getEapMethod());
            }
            NetworkManager::Security8021xSetting::AuthMethod authMethod = securitySetting->phase2AuthMethod();
            if (authMethod == NetworkManager::Security8021xSetting::AuthMethod::AuthMethodUnknown)
                securitySetting->setPhase2AuthMethod(getAuthMethod());
            securitySetting->setIdentity(param.value("identity").toString());
            const QList<NetworkManager::Security8021xSetting::EapMethod> eapMethods = securitySetting->eapMethods();
            NetworkManager::Security8021xSetting::EapMethod eapMethod = eapMethods.isEmpty() ? NetworkManager::Security8021xSetting::EapMethodTls : eapMethods.first();
            securitySetting->setEapMethods(QList<NetworkManager::Security8021xSetting::EapMethod>() << eapMethod);
            if (eapMethod == NetworkManager::Security8021xSetting::EapMethodTls) {
                securitySetting->setPrivateKeyPassword(param.value("private-key-password").toString());
            } else {
                securitySetting->setPassword(param.value("password").toString());
            }
            securitySetting->setInitialized(true);
        }
    }
    wsSetting->setInitialized(true);
    m_needUpdate = true;
    return QVariantMap();
}

/**
 * @brief NetWirelessConnect::hasPassword
 * @return 是否需要密码
 */
bool NetWirelessConnect::hasPassword()
{
    if ((m_accessPoint) && (m_connectionSettings) && m_accessPoint->secured()) {
        // 已有Connection则尝试直接连接
        NetworkManager::Connection::Ptr const conn = findConnectionByUuid(m_connectionSettings->uuid());
        return conn.isNull();
    }

    return (!m_accessPoint || m_accessPoint->secured());
}

void NetWirelessConnect::connectNetwork()
{
    activateConnection();
}

QVariantMap NetWirelessConnect::connectNetworkParam(const QVariantMap &param)
{
    QVariantMap ret = setAuthen(param);
    if (ret.isEmpty())
        activateConnection();
    return ret;
}

void NetWirelessConnect::activateConnection()
{
    // 断开连接会导致更改加密方式后，无法回连，详见bug192801
    NetworkManager::Connection::Ptr conn;
    QString const id = m_connectionSettings->id();
    ConnectionSettings::ConnectionType const type = m_connectionSettings->connectionType();
    for (const auto &it : NetworkManager::listConnections()) {
        if (type != it->settings()->connectionType() || id != it->name())
            continue;

        NetworkManager::WirelessSetting::Ptr const wSetting = it->settings()->setting(NetworkManager::Setting::SettingType::Wireless).staticCast<NetworkManager::WirelessSetting>();
        if (wSetting.isNull())
            continue;

        const QString settingMacAddress = wSetting->macAddress().toHex().toUpper();
        const QString deviceMacAddress = m_device->realHwAdr().remove(":").toUpper();
        if (!settingMacAddress.isEmpty() && settingMacAddress != deviceMacAddress)
            continue;

        m_connectionSettings->setUuid(it->uuid());
        conn = it;
        break;
    }

    QString accessPointPath;
    if (m_accessPoint) {
        accessPointPath = m_accessPoint->path();
    }
    if (conn.isNull()) {
        conn = findConnectionByUuid(m_connectionSettings->uuid());
    }

    // 默认设置Mac地址
    NetworkManager::WirelessSetting::Ptr wirelessSetting = m_connectionSettings->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
    if (wirelessSetting) {
        QString macAddress = m_device->realHwAdr();
        macAddress.remove(":");
        wirelessSetting->setMacAddress(QByteArray::fromHex(macAddress.toUtf8()));
        wirelessSetting->setInitialized(true);
    }

    // 工银瑞信的项目默认关闭自动连接的功能
    if (ConfigSetting::instance()->enableAccountNetwork())
        m_connectionSettings->setAutoconnect(false);

    if (conn.isNull()) {
        qCInfo(DNC) << "add and activate connection2 , device path: " << m_device->path() << "access point path: " << accessPointPath;
        QVariantMap options;
        options.insert("persist", "memory");
        options.insert("flags", MANUAL);
        addAndActivateConnection2(m_connectionSettings->toMap(), m_device->path(), accessPointPath, options).waitForFinished();
        return;
    }
    // 隐藏网络或设置过密码的需要update
    if (m_needUpdate) {
        m_needUpdate = false;
        QDBusPendingReply<> reply;
        auto originMap = conn->settings().data()->toMap();
        auto updateMap = m_connectionSettings->toMap();
        for (auto it = updateMap.begin(); it != updateMap.end(); it++) {
            originMap[it.key()] = it.value();
        }
        reply = conn->isUnsaved() ? conn->updateUnsaved(originMap) : conn->update(originMap);
        reply.waitForFinished();
        if (reply.isError()) {
            qCInfo(DNC) << "Error occurred while updating the connection, error: " << reply.error();
            return;
        }
    }
    if (m_accessPoint) {
        m_device->connectNetwork(id);
    } else {
        QVariantMap options;
        options.insert("flags", MANUAL);
        NetworkManager::activateConnection2(conn->path(), m_device->path(), accessPointPath, options);
    }
}

} // namespace network
} // namespace dde
