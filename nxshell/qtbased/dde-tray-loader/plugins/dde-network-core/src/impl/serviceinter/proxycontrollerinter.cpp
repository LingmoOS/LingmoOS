// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "proxycontrollerinter.h"
#include "proxychains.h"

using namespace dde::network;

const static QString networkService     = "com.deepin.daemon.Network";
const static QString proxyChainsPath    = "/com/deepin/daemon/Network/ProxyChains";

ProxyController_Inter::ProxyController_Inter(QObject *parent)
    : ProxyController(parent)
    , m_chainsInter(new ProxyChains(networkService, proxyChainsPath, QDBusConnection::sessionBus(), this))
{
    // 设置成同步的方式，目的是为了在初始化的时候能正确的返回应用代理的数据，
    // 否则，如果是异步的方式，那么在初始化的时候查询应用代理的数据的时候，返回空
    // 后续ProxyChains的每个信号都会触发一次，将会触发多次appProxyChanged信号
    m_chainsInter->setSync(true);
    // 连接应用代理的相关的槽
    connect(m_chainsInter, &ProxyChains::IPChanged, this, &ProxyController_Inter::onIPChanged);
    connect(m_chainsInter, &ProxyChains::PasswordChanged, this, &ProxyController_Inter::onPasswordChanged);
    connect(m_chainsInter, &ProxyChains::TypeChanged, this, &ProxyController_Inter::onTypeChanged);
    connect(m_chainsInter, &ProxyChains::UserChanged, this, &ProxyController_Inter::onUserChanged);
    connect(m_chainsInter, &ProxyChains::PortChanged, this, &ProxyController_Inter::onPortChanged);
    connect(m_chainsInter, &ProxyChains::EnableChanged, this, &ProxyController::appEnableChanged);
    // 初始化应用代理的相关的数据
    onTypeChanged(m_chainsInter->type());
    onIPChanged(m_chainsInter->iP());
    onPortChanged(m_chainsInter->port());
    onUserChanged(m_chainsInter->user());
    onPasswordChanged(m_chainsInter->password());
}

void ProxyController_Inter::setAppProxy(const AppProxyConfig &config)
{
    m_chainsInter->Set(appProxyType(config.type), config.ip, config.port, config.username, config.password);
}

void ProxyController_Inter::setAppProxyEnabled(bool enabled)
{
    m_chainsInter->SetEnable(enabled);
}

bool ProxyController_Inter::appProxyEnabled() const
{
    return m_chainsInter->enable();
}
