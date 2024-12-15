// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dnotificationappconfig.h"
#include "dnotificationappconfig_p.h"
#include "dbus/dnotificationinterface.h"
#include <QDebug>

DNOTIFICATIONS_BEGIN_NAMESPACE

DNotificationAppConfigPrivate::DNotificationAppConfigPrivate(const QString &id, DNotificationAppConfig *parent)
    : q_ptr(parent)
    , m_dNoticationInter(new DNotificationInterface(this))
    , m_id(id)
{
    connect(m_dNoticationInter, &DNotificationInterface::appInfoChanged, [this] (const QString &id, AppNotificationConfigItem item, QVariant var) {
        Q_Q(DNotificationAppConfig);
        if (id != m_id) {
            return;
        }
        if (item == EnableNotification) {
            Q_EMIT q->notificationEnabledChanged(m_id, var.toBool());
        } else if (item == EnablePreview) {
            Q_EMIT q->previewEnabledChanged(m_id, var.toBool());
        } else if (item == EnableSound) {
            Q_EMIT q->soundEnabledChanged(m_id, var.toBool());
        } else if (item == ShowInNotificationCenter) {
            Q_EMIT q->showInNotificationCenterEnabledChanged(m_id, var.toBool());
        } else if (item == LockScreenShowNofitication) {
            Q_EMIT q->showInLockScreenEnabledChanged(m_id, var.toBool());
        }
    });
}

DNotificationAppConfig::DNotificationAppConfig(const QString &id, QObject *parent)
    : QObject(parent)
    , d_ptr(new DNotificationAppConfigPrivate(id, this))
{
}

DNotificationAppConfig::~DNotificationAppConfig() {}

QString DNotificationAppConfig::appName() const
{
    Q_D(const DNotificationAppConfig);
    auto reply = d->m_dNoticationInter->appInfo(d->m_id, AppName);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return QString();
    } else {
        return reply.value().variant().toString();
    }
}

QString DNotificationAppConfig::appIcon() const
{
    Q_D(const DNotificationAppConfig);
    auto reply = d->m_dNoticationInter->appInfo(d->m_id, AppIcon);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return QString();
    } else {
        return reply.value().variant().toString();
    }
}

bool DNotificationAppConfig::notificationEnabled() const
{
    Q_D(const DNotificationAppConfig);
    auto reply = d->m_dNoticationInter->appInfo(d->m_id, EnableNotification);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return false;
    } else {
        return reply.value().variant().toBool();
    }
}

DExpected<void> DNotificationAppConfig::setNotificationEnabled(const bool enabled)
{
    Q_D(DNotificationAppConfig);
    QDBusVariant dbusVar(enabled);
    auto reply = d->m_dNoticationInter->setAppInfo(d->m_id, EnableNotification, dbusVar);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    }
    Q_EMIT notificationEnabledChanged(d->m_id, enabled);
    return {};
}

bool DNotificationAppConfig::previewEnabled() const
{
    Q_D(const DNotificationAppConfig);
    auto reply = d->m_dNoticationInter->appInfo(d->m_id, EnablePreview);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return false;
    } else {
        return reply.value().variant().toBool();
    }
}

DExpected<void> DNotificationAppConfig::setPreviewEnabled(const bool enabled)
{
    Q_D(DNotificationAppConfig);
    QDBusVariant dbusVar(enabled);
    auto reply = d->m_dNoticationInter->setAppInfo(d->m_id, EnablePreview, dbusVar);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    }
    Q_EMIT previewEnabledChanged(d->m_id, enabled);
    return {};
}

bool DNotificationAppConfig::soundEnabled() const
{
    Q_D(const DNotificationAppConfig);
    auto reply = d->m_dNoticationInter->appInfo(d->m_id, EnableSound);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return false;
    } else {
        return reply.value().variant().toBool();
    }
}

DExpected<void> DNotificationAppConfig::setSoundEnabled(const bool enabled)
{
    Q_D(DNotificationAppConfig);
    QDBusVariant dbusVar(enabled);
    auto reply = d->m_dNoticationInter->setAppInfo(d->m_id, EnableSound, dbusVar);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    }
    Q_EMIT soundEnabledChanged(d->m_id, enabled);
    return {};
}

bool DNotificationAppConfig::showInNotificationCenterEnabled() const
{
    Q_D(const DNotificationAppConfig);
    auto reply = d->m_dNoticationInter->appInfo(d->m_id, ShowInNotificationCenter);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return false;
    } else {
        return reply.value().variant().toBool();
    }
}

DExpected<void> DNotificationAppConfig::setShowInNotificationCenterEnabled(const bool enabled)
{
    Q_D(DNotificationAppConfig);
    QDBusVariant dbusVar(enabled);
    auto reply = d->m_dNoticationInter->setAppInfo(d->m_id, ShowInNotificationCenter, dbusVar);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    }
    Q_EMIT showInNotificationCenterEnabledChanged(d->m_id, enabled);
    return {};
}

bool DNotificationAppConfig::showInLockScreenEnabled() const
{
    Q_D(const DNotificationAppConfig);
    auto reply = d->m_dNoticationInter->appInfo(d->m_id, LockScreenShowNofitication);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return false;
    } else {
        return reply.value().variant().toBool();
    }
}

DExpected<void> DNotificationAppConfig::setShowInLockScreenEnabled(const bool enabled)
{
    Q_D(DNotificationAppConfig);
    QDBusVariant dbusVar(enabled);
    auto reply = d->m_dNoticationInter->setAppInfo(d->m_id, LockScreenShowNofitication, dbusVar);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    }
    Q_EMIT showInLockScreenEnabledChanged(d->m_id, enabled);
    return {};
}
DNOTIFICATIONS_END_NAMESPACE
