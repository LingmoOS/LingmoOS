// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsystemproxy_p.h"
#include "dnmutils.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DUnexpected;
using DCORE_NAMESPACE::emplace_tag;

DSystemProxyPrivate::DSystemProxyPrivate(DSystemProxy *parent)
    : q_ptr(parent)
    , m_sysProxy(new DSystemProxyInterface(this))
{
}

DSystemProxy::DSystemProxy(QObject *parent)
    : QObject(parent)
    , d_ptr(new DSystemProxyPrivate(this))
{
}

DSystemProxy::~DSystemProxy() = default;

DExpected<QString> DSystemProxy::autoProxy() const
{
    Q_D(const DSystemProxy);
    auto reply = d->m_sysProxy->getAutoProxy();
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return reply.value();
}

DExpected<SystemProxyAddr> DSystemProxy::proxy(const ProxyType type) const
{
    Q_D(const DSystemProxy);
    auto reply = d->m_sysProxy->getProxy(enumToProxyType(type));
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return SystemProxyAddr{reply.argumentAt<1>().toUInt(), reply.argumentAt<0>()};
}

DExpected<QString> DSystemProxy::proxyIgnoreHosts() const
{
    Q_D(const DSystemProxy);
    auto reply = d->m_sysProxy->getProxyIgnoreHosts();
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return reply.value();
}

DExpected<SystemProxyMethod> DSystemProxy::proxyMethod() const
{
    Q_D(const DSystemProxy);
    auto reply = d->m_sysProxy->getProxyMethod();
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return proxyMethodToEnum(reply.value());
}

DExpected<void> DSystemProxy::setAutoProxy(const QUrl &pac) const
{
    Q_D(const DSystemProxy);
    if (pac.scheme().isEmpty())
        return DUnexpected{emplace_tag::USE_EMPLACE, -1, "url scheme syntax error"};
    auto reply = d->m_sysProxy->setAutoProxy(pac.toString());
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return {};
}

DExpected<void> DSystemProxy::setProxy(const ProxyType type, const SystemProxyAddr &addr) const
{
    Q_D(const DSystemProxy);
    auto reply = d->m_sysProxy->setProxy(enumToProxyType(type), addr.host, QString::number(addr.port));
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return {};
}

DExpected<void> DSystemProxy::setProxyIgnoreHosts(const QString &hosts) const
{
    Q_D(const DSystemProxy);
    auto reply = d->m_sysProxy->setProxyIgnoreHosts(hosts);
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return {};
}

DExpected<void> DSystemProxy::setProxyMethod(const SystemProxyMethod method) const
{
    Q_D(const DSystemProxy);
    auto reply = d->m_sysProxy->setProxyMethod(enumToProxyMethod(method));
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return {};
}

DNETWORKMANAGER_END_NAMESPACE
