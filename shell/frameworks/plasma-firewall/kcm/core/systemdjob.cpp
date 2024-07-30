// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2020 Lucas Biaggi <lbjanuario@gmail.com>
/*
 * Firewalld backend for plasma firewall
 */

#include <KLocalizedString>
#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDebug>
#include <QTimer>

#include "systemdjob.h"
namespace SYSTEMD
{
const QString BUS = QStringLiteral("org.freedesktop.systemd1");
const QString PATH = QStringLiteral("/org/freedesktop/systemd1");
const QString INTERFACE = QStringLiteral("org.freedesktop.systemd1.Manager");
}

Q_LOGGING_CATEGORY(SystemDJobDebug, "systemd.job")

enum {
    DBUSSYSTEMDERROR = KJob::UserDefinedError,
};

SystemdJob::SystemdJob(const SYSTEMD::actions &action, const QString &service, bool serviceOnly)
    : KJob()
    , m_action(action)
    , m_service(service)
    , m_serviceOnly(serviceOnly){};

void SystemdJob::systemdAction(const SYSTEMD::actions value)
{
    QDBusMessage call;
    QVariantList unitData;
    switch (value) {
    case SYSTEMD::START:
        call = QDBusMessage::createMethodCall(SYSTEMD::BUS, SYSTEMD::PATH, SYSTEMD::INTERFACE, QStringLiteral("StartUnit"));
        call.setArguments({m_service, "fail"});
        unitData << QStringList(m_service) << false << true;
        break;
    case SYSTEMD::STOP:
        call = QDBusMessage::createMethodCall(SYSTEMD::BUS, SYSTEMD::PATH, SYSTEMD::INTERFACE, QStringLiteral("StopUnit"));
        call.setArguments({m_service, "fail"});
        unitData << QStringList(m_service) << false;
        break;

    default:
        setErrorText(i18n("Invalid Call"));
        setError(DBUSSYSTEMDERROR);
        emitResult();
    }

    /* waiting for start/stop of firewalld */
    if (!m_serviceOnly) {
        QDBusPendingCall message = QDBusConnection::systemBus().asyncCall(call);
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(message, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, unitData, value](QDBusPendingCallWatcher *watcher) {
            QDBusPendingReply<> reply = *watcher;
            watcher->deleteLater();
            if (reply.isError()) {
                setErrorText(reply.reply().errorMessage());
                setError(DBUSSYSTEMDERROR);
                emitResult();
            }

            systemdUnit(unitData, value);
        });
        return;
    }

    systemdUnit(unitData, value);
}

void SystemdJob::systemdUnit(const QVariantList values, SYSTEMD::actions action)
{
    QDBusMessage call;
    const QString actionType = action == SYSTEMD::START ? "EnableUnitFiles" : "DisableUnitFiles";
    call = QDBusMessage::createMethodCall(SYSTEMD::BUS, SYSTEMD::PATH, SYSTEMD::INTERFACE, actionType);
    call.setArguments(values);
    call.setInteractiveAuthorizationAllowed(true);
    QDBusPendingCall message = QDBusConnection::systemBus().asyncCall(call);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(message, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<> reply = *watcher;
        watcher->deleteLater();
        if (reply.isError()) {
            setErrorText(reply.reply().errorMessage());
            setError(DBUSSYSTEMDERROR);
            emitResult();
        }
        reloadSystemd();
    });
}

void SystemdJob::reloadSystemd()
{
    QDBusMessage call;
    call = QDBusMessage::createMethodCall(SYSTEMD::BUS, SYSTEMD::PATH, SYSTEMD::INTERFACE, QStringLiteral("Reload"));
    QDBusPendingCall message = QDBusConnection::systemBus().asyncCall(call);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(message, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<> reply = *watcher;
        watcher->deleteLater();
        if (reply.isError()) {
            setErrorText(reply.reply().errorMessage());
            setError(DBUSSYSTEMDERROR);
        }
        emitResult();
    });
}
SystemdJob::~SystemdJob() = default;

void SystemdJob::start()
{
    systemdAction(m_action);
};
