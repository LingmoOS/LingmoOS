/*
    SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "powermanagement_inhibition.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusServiceWatcher>

static const QString s_solidPowerService = QStringLiteral("org.kde.Solid.PowerManagement.PolicyAgent");
static const QString s_solidPath = QStringLiteral("/org/kde/Solid/PowerManagement/PolicyAgent");

Q_DECLARE_METATYPE(QList<InhibitionInfo>)
Q_DECLARE_METATYPE(InhibitionInfo)

PowerManagementInhibition::PowerManagementInhibition(QObject *parent)
    : QObject(parent)
    , m_solidPowerServiceWatcher(new QDBusServiceWatcher(s_solidPowerService,
                                                         QDBusConnection::sessionBus(),
                                                         QDBusServiceWatcher::WatchForUnregistration | QDBusServiceWatcher::WatchForRegistration))
{
    qDBusRegisterMetaType<QList<InhibitionInfo>>();
    qDBusRegisterMetaType<InhibitionInfo>();

    connect(m_solidPowerServiceWatcher, &QDBusServiceWatcher::serviceUnregistered, this, [this] {
        m_serviceRegistered = false;
        m_inhibited = false;
        QDBusConnection::sessionBus().disconnect(s_solidPowerService,
                                                 s_solidPath,
                                                 s_solidPowerService,
                                                 QStringLiteral("InhibitionsChanged"),
                                                 this,
                                                 SLOT(inhibitionsChanged(QList<InhibitionInfo>, QStringList)));
    });
    connect(m_solidPowerServiceWatcher, &QDBusServiceWatcher::serviceRegistered, this, &PowerManagementInhibition::update);

    // check whether the service is registered
    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.DBus"),
                                                          QStringLiteral("/"),
                                                          QStringLiteral("org.freedesktop.DBus"),
                                                          QStringLiteral("ListNames"));
    QDBusPendingReply<QStringList> async = QDBusConnection::sessionBus().asyncCall(message);
    QDBusPendingCallWatcher *callWatcher = new QDBusPendingCallWatcher(async, this);
    connect(callWatcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *self) {
        QDBusPendingReply<QStringList> reply = *self;
        self->deleteLater();
        if (!reply.isValid()) {
            return;
        }
        if (reply.value().contains(s_solidPowerService)) {
            update();
        }
    });
}

PowerManagementInhibition::~PowerManagementInhibition() = default;

void PowerManagementInhibition::update()
{
    m_serviceRegistered = true;
    QDBusConnection::sessionBus().connect(s_solidPowerService,
                                          s_solidPath,
                                          s_solidPowerService,
                                          QStringLiteral("InhibitionsChanged"),
                                          this,
                                          SLOT(inhibitionsChanged(QList<InhibitionInfo>, QStringList)));
    checkInhibition();
}

void PowerManagementInhibition::inhibitionsChanged(const QList<InhibitionInfo> &added, const QStringList &removed)
{
    Q_UNUSED(added)
    Q_UNUSED(removed)
    checkInhibition();
}

void PowerManagementInhibition::checkInhibition()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(s_solidPowerService, s_solidPath, s_solidPowerService, QStringLiteral("HasInhibition"));
    msg << (uint)4; // PowerDevil::PolicyAgent::RequiredPolicy::ChangeScreenSettings
    QDBusPendingReply<bool> pendingReply = QDBusConnection::sessionBus().asyncCall(msg);
    QDBusPendingCallWatcher *callWatcher = new QDBusPendingCallWatcher(pendingReply, this);
    connect(callWatcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *self) {
        QDBusPendingReply<bool> reply = *self;
        self->deleteLater();
        if (!reply.isValid()) {
            return;
        }
        m_inhibited = reply.value();
    });
}
