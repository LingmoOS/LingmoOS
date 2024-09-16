// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBLUETOOTHREQUEST_H
#define DBLUETOOTHREQUEST_H

#include "dbluetoothtypes.h"
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDebug>
#include <QStringBuilder>

DBLUETOOTH_BEGIN_NAMESPACE

template <typename T = void>
class DRequest
{
public:
    DRequest() = default;
    ~DRequest() = default;

    explicit DRequest(RequestDest dest, const QDBusMessage &msg)
        : m_dest(dest)
        , m_msg(msg){};

    explicit DRequest(RequestDest dest, QDBusMessage &&msg)
        : m_dest(dest)
        , m_msg(std::move(msg))
    {
    }

    DRequest(const DRequest &other)
        : m_dest(other.m_dest)
        , m_msg(other.m_msg)
    {
    }

    DRequest(DRequest &&other)
        : m_dest(other.m_dest)
        , m_msg(std::move(other.m_msg))
    {
    }

    DRequest &operator=(const DRequest &other)
    {
        m_dest = other.m_dest;
        m_msg = other.m_msg;
        return *this;
    }

    DRequest &operator=(DRequest &&other) noexcept
    {
        m_dest = other.m_dest;
        m_msg = std::move(other.m_msg);
        return *this;
    }

    bool accept(const T &value) const
    {
        QDBusMessage reply{m_msg.createReply(QVariant::fromValue<T>(value))};
        return sendDBusMessage(reply);
    }

    bool reject() const
    {
        const QDBusMessage &reply =
            m_msg.createErrorReply(interfaceName() % QStringLiteral(".Rejected"), QStringLiteral("Rejected"));
        return sendDBusMessage(reply);
    }

    bool cancel() const
    {
        const QDBusMessage &reply =
            m_msg.createErrorReply(interfaceName() % QStringLiteral(".Canceled"), QStringLiteral("Canceled"));
        return sendDBusMessage(reply);
    }

private:
    QString interfaceName() const
    {
        switch (m_dest) {
            case RequestDest::OrgBluezAgent:
                return QStringLiteral("org.bluez.Agent1");
            case RequestDest::OrgBluezObexAgent:
                return QStringLiteral("org.bluez.obex.Agent1");
            default: {
                qWarning() << "unexpected bluez interface";
                return QString{};
            }
        }
    }

    bool sendDBusMessage(const QDBusMessage &msg) const
    {
#ifdef USE_FAKE_INTERFACE
        const auto &Connection = QDBusConnection::sessionBus();
#else
        const auto &Connection = QDBusConnection::systemBus();
#endif
        switch (m_dest) {
            case RequestDest::OrgBluezAgent:
                return Connection.send(msg);
            case RequestDest::OrgBluezObexAgent:
                return QDBusConnection::sessionBus().send(msg);
            default: {
                qWarning() << "unexpected bluez interface";
                return false;
            }
        }
    }

    RequestDest m_dest;
    QDBusMessage m_msg;
};

template <>
class DRequest<void>
{
public:
    DRequest() = default;
    ~DRequest() = default;

    explicit DRequest(RequestDest dest, const QDBusMessage &msg)
        : m_dest(dest)
        , m_msg(msg){};

    explicit DRequest(RequestDest dest, QDBusMessage &&msg)
        : m_dest(dest)
        , m_msg(std::move(msg))
    {
    }

    DRequest(const DRequest &other)
        : m_dest(other.m_dest)
        , m_msg(other.m_msg)
    {
    }

    DRequest(DRequest &&other)
        : m_dest(other.m_dest)
        , m_msg(std::move(other.m_msg))
    {
    }

    DRequest &operator=(const DRequest &other)
    {
        m_dest = other.m_dest;
        m_msg = other.m_msg;
        return *this;
    }

    DRequest &operator=(DRequest &&other) noexcept
    {
        m_dest = other.m_dest;
        m_msg = std::move(other.m_msg);
        return *this;
    }

    bool accept() const
    {
        const QDBusMessage &reply{m_msg.createReply()};
        return sendDBusMessage(reply);
    }

    bool reject() const
    {
        const QDBusMessage &reply =
            m_msg.createErrorReply(interfaceName() % QStringLiteral(".Rejected"), QStringLiteral("Rejected"));
        return sendDBusMessage(reply);
    }

    bool cancel() const
    {
        const QDBusMessage &reply =
            m_msg.createErrorReply(interfaceName() % QStringLiteral(".Canceled"), QStringLiteral("Canceled"));
        return sendDBusMessage(reply);
    }

private:
    QString interfaceName() const
    {
        switch (m_dest) {
            case RequestDest::OrgBluezAgent:
                return QStringLiteral("org.bluez.Agent1");
            case RequestDest::OrgBluezObexAgent:
                return QStringLiteral("org.bluez.obex.Agent1");
            default: {
                qWarning() << "unexpected bluez interface";
                return QString{};
            }
        }
    }

    bool sendDBusMessage(const QDBusMessage &msg) const
    {
#ifdef USE_FAKE_INTERFACE
        const auto &Connection = QDBusConnection::sessionBus();
#else
        const auto &Connection = QDBusConnection::systemBus();
#endif
        switch (m_dest) {
            case RequestDest::OrgBluezAgent:
                return Connection.send(msg);
            case RequestDest::OrgBluezObexAgent:
                return QDBusConnection::sessionBus().send(msg);
            default: {
                qWarning() << "unexpected bluez interface";
                return false;
            }
        }
    }

    RequestDest m_dest;
    QDBusMessage m_msg;
};

DBLUETOOTH_END_NAMESPACE

#endif
