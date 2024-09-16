// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dnotificationdndmodeconfig.h"
#include "dnotificationdndmodeconfig_p.h"
#include "dbus/dnotificationinterface.h"
#include <QDebug>

DNOTIFICATIONS_BEGIN_NAMESPACE

DNotificationDNDModeConfigPrivate::DNotificationDNDModeConfigPrivate(const QString &name, DNotificationDNDModeConfig *q)
    : DAbstractNotificationModeConfigPrivate(name, q)
    , m_dNoticationInter(new DNotificationInterface(this))
{

}

DNotificationDNDModeConfigPrivate::~DNotificationDNDModeConfigPrivate()
{

}


DNotificationDNDModeConfig::DNotificationDNDModeConfig(const QString &name, QObject *parent)
    : DAbstractNotificationModeConfig(*new DNotificationDNDModeConfigPrivate(name, this), parent)
{
}

DNotificationDNDModeConfig::~DNotificationDNDModeConfig()
{

}

bool DNotificationDNDModeConfig::enabled() const
{
    Q_D(const DNotificationDNDModeConfig);
    auto reply = d->m_dNoticationInter->systemInformation(DNDMode);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return false;
    } else {
        return reply.value().variant().toBool();
    }
}

DExpected<void> DNotificationDNDModeConfig::setEnabled(bool enabled)
{
    Q_D(DNotificationDNDModeConfig);
    QDBusVariant dbusVar(enabled);
    auto reply = d->m_dNoticationInter->setSystemInfo(DNDMode, dbusVar);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    }
    Q_EMIT enabledChanged(enabled);
    return {};
}

bool DNotificationDNDModeConfig::DNDModeInLockScreenEnabled() const
{
    Q_D(const DNotificationDNDModeConfig);
    auto reply = d->m_dNoticationInter->systemInformation(LockScreenOpenDNDMode);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return false;
    } else {
        return reply.value().variant().toBool();
    }
}

DExpected<void> DNotificationDNDModeConfig::setDNDModeInLockScreenEnabled(bool enabled)
{
    Q_D(DNotificationDNDModeConfig);
    QDBusVariant dbusVar(enabled);
    auto reply = d->m_dNoticationInter->setSystemInfo(LockScreenOpenDNDMode, dbusVar);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    }
    Q_EMIT DNDModeInLockScreenEnabledChanged(enabled);
    return {};
}

bool DNotificationDNDModeConfig::openByTimeIntervalEnabled() const
{
    Q_D(const DNotificationDNDModeConfig);
    auto reply = d->m_dNoticationInter->systemInformation(OpenByTimeInterval);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return false;
    } else {
        return reply.value().variant().toBool();
    }
}

DExpected<void> DNotificationDNDModeConfig::setOpenByTimeIntervalEnabled(bool enabled)
{
    Q_D(DNotificationDNDModeConfig);
    QDBusVariant dbusVar(enabled);
    auto reply = d->m_dNoticationInter->setSystemInfo(OpenByTimeInterval, dbusVar);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};;
    }
    Q_EMIT openByTimeIntervalEnabledChanged(enabled);
    return {};
}

QString DNotificationDNDModeConfig::startTime() const
{
    Q_D(const DNotificationDNDModeConfig);
    auto reply = d->m_dNoticationInter->systemInformation(StartTime);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return QString();
    } else {
        return reply.value().variant().toString();
    }
}

DExpected<void> DNotificationDNDModeConfig::setStartTime(const QString &startTime)
{
    Q_D(DNotificationDNDModeConfig);
    QDBusVariant dbusVar(startTime);
    auto reply = d->m_dNoticationInter->setSystemInfo(StartTime, dbusVar);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    }
    Q_EMIT startTimeChanged(startTime);
    return {};
}

QString DNotificationDNDModeConfig::endTime() const
{
    Q_D(const DNotificationDNDModeConfig);
    auto reply = d->m_dNoticationInter->systemInformation(EndTime);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return QString();
    } else {
        return reply.value().variant().toString();
    }
}

DExpected<void> DNotificationDNDModeConfig::setEndTime(const QString &endTime)
{
    Q_D(DNotificationDNDModeConfig);
    QDBusVariant dbusVar(endTime);
    auto reply = d->m_dNoticationInter->setSystemInfo(EndTime, dbusVar);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    }
    Q_EMIT endTimeChanged(endTime);
    return {};
}

bool DNotificationDNDModeConfig::showIconEnabled() const
{
    Q_D(const DNotificationDNDModeConfig);
    auto reply = d->m_dNoticationInter->systemInformation(ShowIcon);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return false;
    } else {
        return reply.value().variant().toBool();
    }
}

DExpected<void> DNotificationDNDModeConfig::setShowIconEnabled(bool enabled)
{
    Q_D(DNotificationDNDModeConfig);
    QDBusVariant dbusVar(enabled);
    auto reply = d->m_dNoticationInter->setSystemInfo(ShowIcon, dbusVar);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    }
    Q_EMIT showIconEnabledChanged(enabled);
    return {};
}
DNOTIFICATIONS_END_NAMESPACE
