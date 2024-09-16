// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "proxycontroller.h"
#include "networkdbusproxy.h"

#include <QFile>
#include <QStandardPaths>

using namespace dde::network;

ProxyController::ProxyController(NetworkDBusProxy *networkInter, QObject *parent)
    : QObject(parent)
    , m_networkInter(networkInter)
    , m_proxyMothod(ProxyMethod::Init)
{
    Q_ASSERT(m_networkInter);
    // 设置成同步的方式，目的是为了在初始化的时候能正确的返回应用代理的数据，
    // 否则，如果是异步的方式，那么在初始化的时候查询应用代理的数据的时候，返回空
    // 后续ProxyChains的每个信号都会触发一次，将会触发多次appProxyChanged信号
    // 连接应用代理的相关的槽
    connect(m_networkInter, &NetworkDBusProxy::IPChanged, this, &ProxyController::onIPChanged);
    connect(m_networkInter, &NetworkDBusProxy::PasswordChanged, this, &ProxyController::onPasswordChanged);
    connect(m_networkInter, &NetworkDBusProxy::TypeChanged, this, &ProxyController::onTypeChanged);
    connect(m_networkInter, &NetworkDBusProxy::UserChanged, this, &ProxyController::onUserChanged);
    connect(m_networkInter, &NetworkDBusProxy::PortChanged, this, &ProxyController::onPortChanged);
    connect(m_networkInter, &NetworkDBusProxy::ProxyMethodChanged, this, &ProxyController::queryProxyMethod);
    // 初始化应用代理的相关的数据
    m_appProxyConfig.type = appProxyType(m_networkInter->type());
    m_appProxyConfig.ip = m_networkInter->iP();
    m_appProxyConfig.port = m_networkInter->port();
    m_appProxyConfig.username = m_networkInter->user();
    m_appProxyConfig.password = m_networkInter->password();
    // 判断是否存在proxychains4来决定是否存在应用代理
    m_appProxyExist = !QStandardPaths::findExecutable("proxychains4").isEmpty();
}

ProxyController::~ProxyController()
{
}

void ProxyController::setProxyMethod(const ProxyMethod &pm)
{
    // 设置代理模式，手动模式，自动模式和关闭代理
    QString methodName = convertProxyMethod(pm);
    m_networkInter->SetProxyMethod(methodName, this, SLOT(queryProxyMethod()));
}

void ProxyController::setProxyIgnoreHosts(const QString &hosts)
{
    m_networkInter->SetProxyIgnoreHosts(hosts, this, SLOT(queryProxyIgnoreHosts()));
}

void ProxyController::setAutoProxy(const QString &proxy)
{
    m_networkInter->SetAutoProxy(proxy, this, SLOT(queryAutoProxy()));
}

void ProxyController::setProxy(const SysProxyType &type, const QString &addr, const QString &port)
{
    QString uType = convertSysProxyType(type);
    switch (type) {
    case SysProxyType::Ftp:
        m_networkInter->SetProxy(uType, addr, port, this, SLOT(queryProxyDataByFtp()));
        break;
    case SysProxyType::Http:
        m_networkInter->SetProxy(uType, addr, port, this, SLOT(queryProxyDataByHttp()));
        break;
    case SysProxyType::Https:
        m_networkInter->SetProxy(uType, addr, port, this, SLOT(queryProxyDataByHttps()));
        break;
    case SysProxyType::Socks:
        m_networkInter->SetProxy(uType, addr, port, this, SLOT(queryProxyDataBySocks()));
        break;
    }
}

void ProxyController::setAppProxy(const AppProxyConfig &config)
{
    m_networkInter->Set(appProxyType(config.type), config.ip, config.port, config.username, config.password);
}

AppProxyConfig ProxyController::appProxy() const
{
    return m_appProxyConfig;
}

SysProxyConfig ProxyController::proxy(const SysProxyType &type) const
{
    // 根据列表的类型返回实际的配置
    for (SysProxyConfig config : m_sysProxyConfig) {
        if (config.type == type)
            return config;
    }

    return SysProxyConfig();
}

bool ProxyController::appProxyExist() const
{
    return m_appProxyExist;
}

void ProxyController::querySysProxyData()
{
    // 查询系统代理的数据
    m_sysProxyConfig.clear();

    static QStringList proxyTypes = {"http", "https", "ftp", "socks"};

    // 依次获取每种类型的数据，并填充到列表中
    for (QString type : proxyTypes)
        queryProxyDataByType(type);

    // 查询自动代理
    queryAutoProxy();
    // 查询代理模式
    queryProxyMethod();
    // 查询忽略的主机
    queryProxyIgnoreHosts();
}

void ProxyController::queryAutoProxy()
{
    QString autoProxyURL = m_networkInter->GetAutoProxy();
    if (m_autoProxyURL != autoProxyURL) {
        m_autoProxyURL = autoProxyURL;
        Q_EMIT autoProxyChanged(autoProxyURL);
    }
}

QString ProxyController::convertProxyMethod(const ProxyMethod &method)
{
    switch (method) {
    case ProxyMethod::Auto:    return "auto";
    case ProxyMethod::Manual:  return "manual";
    default: break;
    }

    return "none";
}

QString ProxyController::convertSysProxyType(const SysProxyType &type)
{
    switch (type) {
    case SysProxyType::Ftp:    return "ftp";
    case SysProxyType::Http:   return "http";
    case SysProxyType::Https:  return "https";
    case SysProxyType::Socks:  return "socks";
    }

    return "http";
}

SysProxyType ProxyController::convertSysProxyType(const QString &type)
{
    if (type == "ftp")
        return SysProxyType::Ftp;

    if (type == "http")
        return SysProxyType::Http;

    if (type == "https")
        return SysProxyType::Https;

    if (type == "socks")
        return SysProxyType::Socks;

    return SysProxyType::Http;
}

void ProxyController::queryProxyDataByFtp()
{
    queryProxyDataByType("ftp");
}

void ProxyController::queryProxyDataByHttp()
{
    queryProxyDataByType("http");
}

void ProxyController::queryProxyDataByHttps()
{
    queryProxyDataByType("https");
}

void ProxyController::queryProxyDataBySocks()
{
    queryProxyDataByType("socks");
}

void ProxyController::queryProxyDataByType(const QString &type)
{
    SysProxyType uType = convertSysProxyType(type);
    QStringList proxy = m_networkInter->GetProxy(type);
    if (proxy.size() != 2)
        return;
    bool finded = false;
    // 先查找原来是否存在响应的代理，如果存在，就直接更新最新的数据
    for (SysProxyConfig &conf : m_sysProxyConfig) {
        if (conf.type == uType) {
            QString url = proxy.at(0);
            uint port = proxy.at(1).toUInt();
            if (url != conf.url || port != conf.port) {
                conf.url = url;
                conf.port = port;
                Q_EMIT proxyChanged(conf);
            }
            finded = true;
            break;
        }
    }
    // 如果不存在，直接将数据放入内存中
    if (!finded) {
        SysProxyConfig proxyConfig;
        proxyConfig.url = proxy.at(0);
        proxyConfig.port = proxy.at(1).toUInt();
        proxyConfig.type = uType;
        m_sysProxyConfig << proxyConfig;
        Q_EMIT proxyChanged(proxyConfig);
    }
}

ProxyMethod ProxyController::convertProxyMethod(const QString &method)
{
    // 根据传入的字符串获取代理模式的枚举值
    if (method == "auto")
        return ProxyMethod::Auto;

    if (method == "manual")
        return ProxyMethod::Manual;

    return ProxyMethod::None;
}

void ProxyController::queryProxyMethod()
{
    // 查询代理模式
    ProxyMethod method = convertProxyMethod(m_networkInter->GetProxyMethod());
    if (method != m_proxyMothod) {
        m_proxyMothod = method;
        Q_EMIT proxyMethodChanged(method);
    }
}

void ProxyController::queryProxyIgnoreHosts()
{
    // 查询忽略的代理主机
    QString proxyIgnoreHosts = m_networkInter->GetProxyIgnoreHosts();
    if (m_proxyIgnoreHosts != proxyIgnoreHosts) {
        m_proxyIgnoreHosts = proxyIgnoreHosts;
        Q_EMIT proxyIgnoreHostsChanged(proxyIgnoreHosts);
    }
}

void ProxyController::onIPChanged(const QString &value)
{
    // 应用代理的IP发生变化
    if (value != m_appProxyConfig.ip) {
        m_appProxyConfig.ip = value;
        Q_EMIT appIPChanged(value);
    }
}

void ProxyController::onPasswordChanged(const QString &value)
{
    // 应用代理的密码发生变化
    if (value != m_appProxyConfig.password) {
        m_appProxyConfig.password = value;
        Q_EMIT appPasswordChanged(value);
    }
}

AppProxyType ProxyController::appProxyType(const QString &v)
{
    if (v == "http")
        return AppProxyType::Http;

    if (v == "socks4")
        return AppProxyType::Socks4;

    if (v == "socks5")
        return AppProxyType::Socks5;

    return AppProxyType::Http;
}

QString ProxyController::appProxyType(const AppProxyType &v)
{
    switch (v) {
    case AppProxyType::Http:     return "http";
    case AppProxyType::Socks4:   return "socks4";
    case AppProxyType::Socks5:   return "socks5";
    }

    return "http";
}

void ProxyController::onTypeChanged(const QString &value)
{
    // 应用代理类型发生变化
    AppProxyType t = appProxyType(value);
    if (t != m_appProxyConfig.type) {
        m_appProxyConfig.type = t;
        Q_EMIT appTypeChanged(t);
    }
}

void ProxyController::onUserChanged(const QString &value)
{
    // 应用代理用户名发生变化
    if (value != m_appProxyConfig.username) {
        m_appProxyConfig.username = value;
        Q_EMIT appUsernameChanged(value);
    }
}

void ProxyController::onPortChanged(uint value)
{
    // 应用代理端口发生变化
    if (value != m_appProxyConfig.port) {
        m_appProxyConfig.port = value;
        Q_EMIT appPortChanged(value);
    }
}
