//SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
//SPDX-License-Identifier: GPL-3.0-or-later

#include "systeminfodbusproxy.h"

#include <qdbusreply.h>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingReply>
#include <QMetaObject>

const QString HostnameService = QStringLiteral("org.freedesktop.hostname1");
const QString HostnamePath = QStringLiteral("/org/freedesktop/hostname1");
const QString HostnameInterface = QStringLiteral("org.freedesktop.hostname1");

const QString LicenseInfoService = QStringLiteral("com.lingmo.license");
const QString LicenseInfoPath = QStringLiteral("/com/lingmo/license/Info");
const QString LicenseInfoInterface = QStringLiteral("com.lingmo.license.Info");

const QString LicenseActivatorService = QStringLiteral("com.lingmo.license.activator");
const QString LicenseActivatorPath = QStringLiteral("/com/lingmo/license/activator");
const QString LicenseActivatorInterface = QStringLiteral("com.lingmo.license.activator");

const QString PropertiesInterface = QStringLiteral("org.freedesktop.DBus.Properties");
const QString PropertiesChanged = QStringLiteral("PropertiesChanged");

const QString &UserexperienceService = QStringLiteral("com.lingmo.userexperience.Daemon");
const QString &UserexperiencePath = QStringLiteral("/com/lingmo/userexperience/Daemon");
const QString &UserexperienceInterface = QStringLiteral("com.lingmo.userexperience.Daemon");

const QString &SystemInfoService = QStringLiteral("com.lingmo.system.SystemInfo");
const QString &SystemInfoPath = QStringLiteral("/com/lingmo/system/SystemInfo");
const QString &SystemInfoInterface = QStringLiteral("com.lingmo.system.SystemInfo");

const QString &TimedateService = QStringLiteral("com.lingmo.daemon.Timedate");
const QString &TimedatePath = QStringLiteral("/com/lingmo/daemon/Timedate");
const QString &TimedateInterface = QStringLiteral("com.lingmo.daemon.Timedate");

const QString &TimeZoneService = QStringLiteral("org.freedesktop.timedate1");
const QString &TimeZonePath = QStringLiteral("/org/freedesktop/timedate1");
const QString &TimeZoneInterface = QStringLiteral("org.freedesktop.timedate1");

SystemInfoDBusProxy::SystemInfoDBusProxy(QObject *parent)
    : QObject(parent)
    , m_hostname1Inter(new DDBusInterface(HostnameService, HostnamePath, HostnameInterface, QDBusConnection::systemBus(), this))
    , m_licenseInfoInter(new DDBusInterface(LicenseInfoService, LicenseInfoPath, LicenseInfoInterface, QDBusConnection::systemBus(), this))
    , m_licenseActivatorInter(new DDBusInterface(LicenseActivatorService, LicenseActivatorPath, LicenseActivatorInterface, QDBusConnection::sessionBus(), this))
    , m_userexperienceInter(new DDBusInterface(UserexperienceService, UserexperiencePath, UserexperienceInterface, QDBusConnection::sessionBus(), this))
    , m_systemInfo(new DDBusInterface(SystemInfoService, SystemInfoPath, SystemInfoInterface, QDBusConnection::systemBus(), this))
    , m_timedateInter(new DDBusInterface(TimedateService, TimedatePath, TimedateInterface, QDBusConnection::sessionBus(), this))
    , m_timeZoneInter(new DDBusInterface(TimeZoneService, TimeZonePath, TimeZoneInterface, QDBusConnection::systemBus(), this))
{
}

QString SystemInfoDBusProxy::staticHostname()
{
    return qvariant_cast<QString>(m_hostname1Inter->property("StaticHostname"));
}

void SystemInfoDBusProxy::setStaticHostname(const QString &value)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(value) << QVariant::fromValue(true);
    m_hostname1Inter->asyncCallWithArgumentList("SetStaticHostname", argumentList);
}

void SystemInfoDBusProxy::setStaticHostname(const QString &value, QObject *receiver, const char *member, const char *errorSlot)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(value) << QVariant::fromValue(true);
    m_hostname1Inter->callWithCallback("SetStaticHostname", argumentList, receiver, member, errorSlot);
}

int SystemInfoDBusProxy::authorizationState()
{
    return qvariant_cast<int>(m_licenseInfoInter->property("AuthorizationState"));
}

void SystemInfoDBusProxy::setAuthorizationState(const int value)
{
    m_licenseInfoInter->setProperty("AuthorizationState", QVariant::fromValue(value));
}

QString SystemInfoDBusProxy::timezone()
{
    return qvariant_cast<QString>(m_timeZoneInter->property("Timezone"));
}

void SystemInfoDBusProxy::setTimezone(const QString &value)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(value) << QVariant::fromValue(true);
    m_hostname1Inter->asyncCallWithArgumentList("SetTimezone", argumentList);
}

int SystemInfoDBusProxy::shortDateFormat()
{
    return qvariant_cast<int>(m_timedateInter->property("ShortDateFormat"));
}

void SystemInfoDBusProxy::Enable(const bool value)
{
    m_userexperienceInter->asyncCallWithArgumentList("Enable", { value });
}

bool SystemInfoDBusProxy::IsEnabled()
{
    QDBusReply<bool> reply = m_userexperienceInter->call(QStringLiteral("IsEnabled"));
    if (reply.isValid())
        return reply.value();
    return false;
}

qulonglong SystemInfoDBusProxy::memorySize()
{
    return m_systemInfo->property("MemorySize").toULongLong();
}

void SystemInfoDBusProxy::Show()
{
    m_licenseActivatorInter->asyncCall("Show");
}
