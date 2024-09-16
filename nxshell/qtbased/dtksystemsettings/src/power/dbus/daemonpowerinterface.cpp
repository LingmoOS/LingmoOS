// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daemonpowerinterface.h"

#ifdef USE_FAKE_INTERFACE
static const QString &Service = QStringLiteral("com.deepin.daemon.FakePower");
static const QString &Path = QStringLiteral("/com/deepin/daemon/FakePower");
static const QString &Interface = QStringLiteral("com.deepin.daemon.FakePower");
#  define DBUS_CON QDBusConnection::sessionBus()
#else
static const QString &Service = QStringLiteral("com.deepin.daemon.Power");
static const QString &Path = QStringLiteral("/com/deepin/daemon/Power");
static const QString &Interface = QStringLiteral("com.deepin.daemon.Power");
#  define DBUS_CON QDBusConnection::sessionBus()
#endif

DPOWER_BEGIN_NAMESPACE
DaemonPowerInterface::DaemonPowerInterface(QObject *parent)
    : QObject(parent)
{
    m_inter.reset(new DDBusInterface(Service, Path, Interface, DBUS_CON, this));
}

DPOWER_END_NAMESPACE
