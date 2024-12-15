// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "systemd1_unit_interface.h"

#include "dbus_common.h"

using namespace dbus::common;

/*
 * Implementation of interface class Systemd1UnitInterface
 */

// Constructor
Systemd1UnitInterface::Systemd1UnitInterface(const QString &service,
                                             const QString &path,
                                             const QDBusConnection &connection,
                                             QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
    , m_DBusPropIface(new DBusPropertiesInterface(service, path, connection))
{
}

// Destructor
Systemd1UnitInterface::~Systemd1UnitInterface()
{
    m_DBusPropIface->deleteLater();
}

// Get unit Id
QPair<ErrorContext, QString> Systemd1UnitInterface::getId() const
{
    // error context
    ErrorContext ec;
    // unit id
    QString id {};

    // dbus interface property call: Id
    auto reply = m_DBusPropIface->Get(staticInterfaceName(), "Id");
    ec = reply.first;
    if (ec)
        return {ec, id};
    QDBusMessage msg = reply.second;

    // check reply
    if (msg.type() == QDBusMessage::ErrorMessage) {
        return {ec, id};
    } else {
        Q_ASSERT(msg.type() == QDBusMessage::ReplyMessage);
        QDBusVariant v = qvariant_cast<QDBusVariant>(msg.arguments()[0]);
        // check return type
        if (v.variant().type() == QVariant::String) {
            id = qvariant_cast<QString>(v.variant());
        }
    }

    return {ec, id};
}
// Get unit's load state
QPair<ErrorContext, QString> Systemd1UnitInterface::getLoadState() const
{
    // error context
    ErrorContext ec;
    // unit's load state
    QString state {};

    // dbus interface property call: LoadState
    auto reply = m_DBusPropIface->Get(staticInterfaceName(), "LoadState");
    ec = reply.first;
    if (ec)
        return {ec, state};
    QDBusMessage msg = reply.second;

    // check reply
    if (msg.type() == QDBusMessage::ErrorMessage) {
        return {ec, state};
    } else {
        Q_ASSERT(msg.type() == QDBusMessage::ReplyMessage);
        QDBusVariant v = qvariant_cast<QDBusVariant>(msg.arguments()[0]);
        // check return type
        if (v.variant().type() == QVariant::String) {
            state = qvariant_cast<QString>(v.variant());
        }
    }

    return {ec, state};
}
// Get unit's active state
QPair<ErrorContext, QString> Systemd1UnitInterface::getActiveState() const
{
    // error context
    ErrorContext ec;
    // unit's active state
    QString state {};

    // dbus interface property call: ActiveState
    auto reply = m_DBusPropIface->Get(staticInterfaceName(), "ActiveState");
    ec = reply.first;
    if (ec)
        return {ec, state};
    QDBusMessage msg = reply.second;

    // check reply
    if (msg.type() == QDBusMessage::ErrorMessage) {
        return {ec, state};
    } else {
        Q_ASSERT(msg.type() == QDBusMessage::ReplyMessage);
        QDBusVariant v = qvariant_cast<QDBusVariant>(msg.arguments()[0]);
        // check return type
        if (v.variant().type() == QVariant::String) {
            state = qvariant_cast<QString>(v.variant());
        }
    }

    return {ec, state};
}
// Get unit's sub state
QPair<ErrorContext, QString> Systemd1UnitInterface::getSubState() const
{
    // error context
    ErrorContext ec;
    // unit's sub state
    QString sub {};

    // dbus interface property call: SubState
    auto reply = m_DBusPropIface->Get(staticInterfaceName(), "SubState");
    ec = reply.first;
    if (ec)
        return {ec, sub};
    QDBusMessage msg = reply.second;

    // check reply
    if (msg.type() == QDBusMessage::ErrorMessage) {
        return {ec, sub};
    } else {
        Q_ASSERT(msg.type() == QDBusMessage::ReplyMessage);
        QDBusVariant v = qvariant_cast<QDBusVariant>(msg.arguments()[0]);
        // check return type
        if (v.variant().type() == QVariant::String) {
            sub = qvariant_cast<QString>(v.variant());
        }
    }

    return {ec, sub};
}
// Get unit's description
QPair<ErrorContext, QString> Systemd1UnitInterface::getDescription() const
{
    // error context
    ErrorContext ec;
    // unit's description
    QString desc {};

    // dbus interface property call: Description
    auto reply = m_DBusPropIface->Get(staticInterfaceName(), "Description");
    ec = reply.first;
    if (ec)
        return {ec, desc};
    QDBusMessage msg = reply.second;

    // check reply
    if (msg.type() == QDBusMessage::ErrorMessage) {
        return {ec, desc};
    } else {
        Q_ASSERT(msg.type() == QDBusMessage::ReplyMessage);
        QDBusVariant v = qvariant_cast<QDBusVariant>(msg.arguments()[0]);
        // check return type
        if (v.variant().type() == QVariant::String) {
            desc = qvariant_cast<QString>(v.variant());
        }
    }

    return {ec, desc};
}
// Get unit's canReload property
QPair<ErrorContext, bool> Systemd1UnitInterface::canReload() const
{
    // error context
    ErrorContext ec;
    // unit's canReload property
    bool re {false};

    // dbus interface property call: CanReload
    auto reply = m_DBusPropIface->Get(staticInterfaceName(), "CanReload");
    ec = reply.first;
    if (ec)
        return {ec, re};
    QDBusMessage msg = reply.second;

    // check reply
    if (msg.type() == QDBusMessage::ErrorMessage) {
        return {ec, re};
    } else {
        Q_ASSERT(msg.type() == QDBusMessage::ReplyMessage);
        QDBusVariant v = qvariant_cast<QDBusVariant>(msg.arguments()[0]);
        // check return type
        if (v.variant().type() == QVariant::Bool) {
            re = qvariant_cast<bool>(v.variant());
        }
    }

    return {ec, re};
}
// Get unit's canStart property
QPair<ErrorContext, bool> Systemd1UnitInterface::canStart() const
{
    // error context
    ErrorContext ec;
    // unit's canStart property
    bool re {false};

    // dbus interface property call: CanStart
    auto reply = m_DBusPropIface->Get(staticInterfaceName(), "CanStart");
    ec = reply.first;
    if (ec)
        return {ec, re};
    QDBusMessage msg = reply.second;

    // check reply
    if (msg.type() == QDBusMessage::ErrorMessage) {
        return {ec, re};
    } else {
        Q_ASSERT(msg.type() == QDBusMessage::ReplyMessage);
        QDBusVariant v = qvariant_cast<QDBusVariant>(msg.arguments()[0]);
        // check return type
        if (v.variant().type() == QVariant::Bool) {
            re = qvariant_cast<bool>(v.variant());
        }
    }

    return {ec, re};
}
// Get unit's canStop property
QPair<ErrorContext, bool> Systemd1UnitInterface::canStop() const
{
    // error context
    ErrorContext ec;
    // unit's canStop property
    bool re {false};

    // dbus interface property call: CanStop
    auto reply = m_DBusPropIface->Get(staticInterfaceName(), "CanStop");
    ec = reply.first;
    if (ec)
        return {ec, re};
    QDBusMessage msg = reply.second;

    // check reply
    if (msg.type() == QDBusMessage::ErrorMessage) {
        return {ec, re};
    } else {
        Q_ASSERT(msg.type() == QDBusMessage::ReplyMessage);
        QDBusVariant v = qvariant_cast<QDBusVariant>(msg.arguments()[0]);
        // check return type
        if (v.variant().type() == QVariant::Bool) {
            re = qvariant_cast<bool>(v.variant());
        }
    }

    return {ec, re};
}
