// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2020 Lucas Biaggi <lbjanuario@gmail.com>
/*
 * Firewalld backend for plasma firewall
 */

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDebug>
#include <QRegularExpression>

#include <KLocalizedString>

#include "firewalldjob.h"

Q_LOGGING_CATEGORY(FirewallDJobDebug, "firewalld.job")

namespace FIREWALLD
{
const QString BUS = QStringLiteral("org.fedoraproject.FirewallD1");
const QString PATH = QStringLiteral("/org/fedoraproject/FirewallD1");
}

namespace DIRECT
{
const QString KCM_FIREWALLD_DIR = QStringLiteral("/etc/kcm/firewalld");
const QString LOG_FILE = QStringLiteral("/var/log/firewalld.log");
const QString INTERFACE = QStringLiteral("org.fedoraproject.FirewallD1.direct");
}

namespace SAVE
{
const QString METHOD = QStringLiteral("runtimeToPermanent");
}

namespace SERVICES
{
const QString INTERFACE = QStringLiteral("org.fedoraproject.FirewallD1");
const QString METHOD = QStringLiteral("listServices");
}

namespace SIMPLE
{
const QString INTERFACE = QStringLiteral("org.fedoraproject.FirewallD1.zone");
}

namespace AUTH
{
const QString METHOD = QStringLiteral("authorizeAll");
}

enum {
    DBUSFIREWALLDDERROR = KJob::UserDefinedError,
};

FirewalldJob::FirewalldJob(){};

FirewalldJob::FirewalldJob(const QByteArray &call, const QVariantList &args, const FirewalldJob::JobType &type)
    : KJob()
    , m_type(type)
    , m_call(call)
    , m_args(args){};

FirewalldJob::FirewalldJob(const FirewalldJob::JobType &type)
    : KJob()
    , m_type(type){};

template<typename T>
T FirewalldJob::connectCall(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<T> reply = *watcher;
    if (reply.isError()) {
        setErrorText(reply.error().message());
        setError(DBUSFIREWALLDDERROR);
        qCDebug(FirewallDJobDebug) << "job error message: " << errorString();
        emitResult();
    }
    return reply.value();
}

void FirewalldJob::connectCall(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<> reply = *watcher;
    if (reply.isError()) {
        setErrorText(reply.error().message());
        setError(DBUSFIREWALLDDERROR);
        qCDebug(FirewallDJobDebug) << "job error message: " << errorString();
        emitResult();
    }
}

void FirewalldJob::firewalldAction(const QString &bus, const QString &path, const QString &interface, const QString &method, const QVariantList &args)
{
    QDBusMessage call = QDBusMessage::createMethodCall(bus, path, interface, method);
    call.setArguments(args);
    QDBusPendingCall message = QDBusConnection::systemBus().asyncCall(call);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(message, this);
    if (args.isEmpty()) {
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, interface, method](QDBusPendingCallWatcher *watcher) {
            watcher->deleteLater();
            if (interface == DIRECT::INTERFACE) { // iptables rules like
                QList<firewalld_reply> reply = connectCall<QList<firewalld_reply>>(watcher);
                if (!reply.isEmpty()) {
                    m_firewalldreply = reply;
                }
            } else if (interface == SERVICES::INTERFACE && method != SAVE::METHOD
                       && method != AUTH::METHOD) { // list services available or enabled services AND don't execute runtimeToPermanent HERE
                QStringList reply = connectCall<QStringList>(watcher);
                if (!reply.isEmpty()) {
                    m_services = reply;
                }

            } else {
                connectCall(watcher); // save executed here
            }
            emitResult();
            return;
        });

    } else {
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, interface, method](QDBusPendingCallWatcher *watcher) {
            watcher->deleteLater();
            if (interface == SIMPLE::INTERFACE) { // believe or not to get the data from active zone you need to send ""
                if (method.contains(QRegularExpression("^(add|remove)"))) {
                    QString reply = connectCall<QString>(watcher);
                    if (!reply.isEmpty())
                        qCDebug(FirewallDJobDebug) << "manipulated zone: " << reply;

                } else if (method == "getZoneSettings2") {
                    QMap<QString, QVariant> settings;
                    settings = connectCall<QMap<QString, QVariant>>(watcher);
                    m_target = settings["target"].toString();
                } else {
                    QStringList reply = connectCall<QStringList>(watcher);
                    if (!reply.isEmpty()) {
                        m_services = reply;
                    }
                }
            } else {
                connectCall(watcher);
            }
            emitResult();
            return;
        });
    }
}

QList<firewalld_reply> FirewalldJob::getFirewalldreply() const
{
    return m_firewalldreply;
}

FirewalldJob::~FirewalldJob() = default;

void FirewalldJob::start()
{
    switch (m_type) {
    case FirewalldJob::SIMPLIFIEDRULE:
    case FirewalldJob::SIMPLELIST: {
        qCDebug(FirewallDJobDebug) << "firewalld zone interface: " << m_call << m_args;
        firewalldAction(FIREWALLD::BUS, FIREWALLD::PATH, SIMPLE::INTERFACE, m_call, m_args);
        break;
    }
    case FirewalldJob::FIREWALLD: {
        qCDebug(FirewallDJobDebug) << "firewalld direct: " << m_call << m_args;
        /* firewalldAction(m_call, m_args); */
        firewalldAction(FIREWALLD::BUS, FIREWALLD::PATH, DIRECT::INTERFACE, m_call, m_args);
        break;
    }
    case FirewalldJob::SAVEFIREWALLD: {
        qCDebug(FirewallDJobDebug) << i18n("firewalld saving (runtime to permanent)");
        firewalldAction(FIREWALLD::BUS, FIREWALLD::PATH, SERVICES::INTERFACE, SAVE::METHOD);
        /* saveFirewalld(); */
        break;
    }
    case FirewalldJob::LISTSERVICES: {
        /* listServices(); */
        firewalldAction(FIREWALLD::BUS, FIREWALLD::PATH, SERVICES::INTERFACE, SERVICES::METHOD);
        break;
    }
    case FirewalldJob::ALL: {
        firewalldAction(FIREWALLD::BUS, FIREWALLD::PATH, SERVICES::INTERFACE, AUTH::METHOD);
        break;
    }

    default:
        emitResult();
        return;
    }
    return;
};

QString FirewalldJob::name() const
{
    return m_type == FirewalldJob::SAVEFIREWALLD ? i18n("firewalld saving") : i18n("firewalld %1").arg(QString(m_call));
}

QStringList FirewalldJob::getServices() const
{
    return m_services;
}

QString FirewalldJob::getDefaultIncomingPolicy() const
{
    return m_target;
}
