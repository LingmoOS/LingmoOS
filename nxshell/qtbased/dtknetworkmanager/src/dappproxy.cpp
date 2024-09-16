// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dappproxy_p.h"
#include "dnmutils.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DUnexpected;
using DCORE_NAMESPACE::emplace_tag;

DAppProxyPrivate::DAppProxyPrivate(DAppProxy *parent)
    : q_ptr(parent)
    , m_appProxy(new DAppProxyInterface(this))
{
}

DAppProxy::DAppProxy(QObject *parent)
    : QObject(parent)
    , d_ptr(new DAppProxyPrivate(this))
{
}

DAppProxy::~DAppProxy() = default;

QHostAddress DAppProxy::IP() const
{
    Q_D(const DAppProxy);
    return QHostAddress(d->m_appProxy->IP());
}

QByteArray DAppProxy::password() const
{
    Q_D(const DAppProxy);
    return d->m_appProxy->password().toUtf8();
}

ProxyType DAppProxy::type() const
{
    Q_D(const DAppProxy);
    return proxyTypeToEnum(d->m_appProxy->type());
}

QByteArray DAppProxy::user() const
{
    Q_D(const DAppProxy);
    return d->m_appProxy->user().toUtf8();
}

quint32 DAppProxy::port() const
{
    Q_D(const DAppProxy);
    return d->m_appProxy->port();
}

DExpected<void> DAppProxy::set(
    const ProxyType type, const QByteArray &ip, const quint32 port, const QByteArray &user, const QByteArray &password) const
{
    Q_D(const DAppProxy);
    auto reply = d->m_appProxy->set(enumToProxyType(type), ip, port, user, password);
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return {};
}

DNETWORKMANAGER_END_NAMESPACE
