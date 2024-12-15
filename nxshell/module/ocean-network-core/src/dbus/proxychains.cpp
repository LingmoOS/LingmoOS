// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "proxychains.h"

/*
 * Implementation of interface class __ProxyChains
 */

class __ProxyChainsPrivate
{
public:
   __ProxyChainsPrivate() = default;

    // begin member variables
    bool Enable;
    QString IP;
    QString Password;
    uint Port;
    QString Type;
    QString User;

public:
    QMap<QString, QDBusPendingCallWatcher *> m_processingCalls;
    QMap<QString, QList<QVariant>> m_waittingCalls;
};

ProxyChains::ProxyChains(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : Dtk::Core::DDBusInterface(service, path, staticInterfaceName(), connection, parent)
    , d_ptr(new __ProxyChainsPrivate)
{
    // connect(this, &ProxyChains::propertyChanged, this, &ProxyChains::onPropertyChanged);
}

ProxyChains::~ProxyChains()
{
    qDeleteAll(d_ptr->m_processingCalls.values());
    delete d_ptr;
}

void ProxyChains::setSync(bool sync)
{

}

void ProxyChains::onPropertyChanged(const QString &propName, const QVariant &value)
{
    if (propName == QStringLiteral("Enable"))
    {
        const bool &Enable = qvariant_cast<bool>(value);
        if (d_ptr->Enable != Enable)
        {
            d_ptr->Enable = Enable;
            Q_EMIT EnableChanged(d_ptr->Enable);
        }
        return;
    }

    if (propName == QStringLiteral("IP"))
    {
        const QString &IP = qvariant_cast<QString>(value);
        if (d_ptr->IP != IP)
        {
            d_ptr->IP = IP;
            Q_EMIT IPChanged(d_ptr->IP);
        }
        return;
    }

    if (propName == QStringLiteral("Password"))
    {
        const QString &Password = qvariant_cast<QString>(value);
        if (d_ptr->Password != Password)
        {
            d_ptr->Password = Password;
            Q_EMIT PasswordChanged(d_ptr->Password);
        }
        return;
    }

    if (propName == QStringLiteral("Port"))
    {
        const uint &Port = qvariant_cast<uint>(value);
        if (d_ptr->Port != Port)
        {
            d_ptr->Port = Port;
            Q_EMIT PortChanged(d_ptr->Port);
        }
        return;
    }

    if (propName == QStringLiteral("Type"))
    {
        const QString &Type = qvariant_cast<QString>(value);
        if (d_ptr->Type != Type)
        {
            d_ptr->Type = Type;
            Q_EMIT TypeChanged(d_ptr->Type);
        }
        return;
    }

    if (propName == QStringLiteral("User"))
    {
        const QString &User = qvariant_cast<QString>(value);
        if (d_ptr->User != User)
        {
            d_ptr->User = User;
            Q_EMIT UserChanged(d_ptr->User);
        }
        return;
    }

    qWarning() << "On property changed，property not handle: " << propName;
    return;
}

bool ProxyChains::enable()
{
    return qvariant_cast<bool>(DDBusInterface::property("Enable"));
}

QString ProxyChains::iP()
{
    return qvariant_cast<QString>(DDBusInterface::property("IP"));
}

QString ProxyChains::password()
{
    return qvariant_cast<QString>(DDBusInterface::property("Password"));
}

uint ProxyChains::port()
{
    return qvariant_cast<uint>(DDBusInterface::property("Port"));
}

QString ProxyChains::type()
{
    return qvariant_cast<QString>(DDBusInterface::property("Type"));
}

QString ProxyChains::user()
{
    return qvariant_cast<QString>(DDBusInterface::property("User"));
}

void ProxyChains::CallQueued(const QString &callName, const QList<QVariant> &args)
{
    if (d_ptr->m_waittingCalls.contains(callName))
    {
        d_ptr->m_waittingCalls[callName] = args;
        return;
    }
    if (d_ptr->m_processingCalls.contains(callName))
    {
        d_ptr->m_waittingCalls.insert(callName, args);
    } else {
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(asyncCallWithArgumentList(callName, args));
        connect(watcher, &QDBusPendingCallWatcher::finished, this, &ProxyChains::onPendingCallFinished);
        d_ptr->m_processingCalls.insert(callName, watcher);
    }
}

void ProxyChains::onPendingCallFinished(QDBusPendingCallWatcher *w)
{
    w->deleteLater();
    const auto callName = d_ptr->m_processingCalls.key(w);
    Q_ASSERT(!callName.isEmpty());
    if (callName.isEmpty())
        return;
    d_ptr->m_processingCalls.remove(callName);
    if (!d_ptr->m_waittingCalls.contains(callName))
        return;
    const auto args = d_ptr->m_waittingCalls.take(callName);
    CallQueued(callName, args);
}
