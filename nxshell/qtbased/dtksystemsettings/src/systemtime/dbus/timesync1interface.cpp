// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "timesync1interface.h"

#include <qdbusconnection.h>
#include <qdbusextratypes.h>
#include <qstring.h>
#include <qvariant.h>

DSYSTEMTIME_BEGIN_NAMESPACE
TimeSync1Interface::TimeSync1Interface(QObject *parent)
    : QObject(parent)
{
#ifdef USE_FAKE_INTERFACE
    static const QString &Service = QStringLiteral("com.deepin.daemon.FakeTimeSync1");
    static const QString &Path = QStringLiteral("/com/deepin/daemon/FakeTimeSync1");
    static const QString &Interface = QStringLiteral("com.deepin.daemon.FakeTimeSync1");
    QDBusConnection connection = QDBusConnection::sessionBus();
#else
    static const QString &Service = QStringLiteral("org.freedesktop.timesync1");
    static const QString &Interface = QStringLiteral("org.freedesktop.timesync1.Manager");
    static const QString &Path = "/org/freedesktop/timesync1";
    QDBusConnection connection = QDBusConnection::systemBus();
#endif
    m_inter = new DDBusInterface(Service, Path, Interface, connection, this);
    // register
    Address_p::registerMetaType();
    Message_p::registerMetaType();
}

TimeSync1Interface::~TimeSync1Interface() { }

// properties

QStringList TimeSync1Interface::fallbackNTPServers() const
{
    return qdbus_cast<QStringList>(m_inter->property("FallbackNTPServers"));
}

QStringList TimeSync1Interface::linkNTPServers() const
{
    return qdbus_cast<QStringList>(m_inter->property("LinkNTPServers"));
}

QStringList TimeSync1Interface::systemNTPServers() const
{
    return qdbus_cast<QStringList>(m_inter->property("SystemNTPServers"));
}

qint64 TimeSync1Interface::frequency() const
{
    return qdbus_cast<qint64>(m_inter->property("Frequency"));
}

QString TimeSync1Interface::serverName() const
{
    return qdbus_cast<QString>(m_inter->property("ServerName"));
}

Address_p TimeSync1Interface::serverAddress() const
{
    return qdbus_cast<Address_p>(m_inter->property("ServerAddress"));
}

Message_p TimeSync1Interface::NTPMessage() const
{
    return qdbus_cast<Message_p>(m_inter->property("NTPMessage"));
}

quint64 TimeSync1Interface::pollIntervalMaxUSec() const
{
    return qdbus_cast<quint64>(m_inter->property("PollIntervalMaxUSec"));
}

quint64 TimeSync1Interface::pollIntervalMinUSec() const
{
    return qdbus_cast<quint64>(m_inter->property("PollIntervalMinUSec"));
}

quint64 TimeSync1Interface::pollIntervalUSec() const
{
    return qdbus_cast<quint64>(m_inter->property("PollIntervalUSec"));
}

quint64 TimeSync1Interface::rootDistanceMaxUSec() const
{
    return qdbus_cast<quint64>(m_inter->property("RootDistanceMaxUSec"));
}

DSYSTEMTIME_END_NAMESPACE