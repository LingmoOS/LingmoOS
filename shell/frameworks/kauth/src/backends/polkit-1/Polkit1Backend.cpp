/*
    SPDX-FileCopyrightText: 2008 Nicola Gigante <nicola.gigante@gmail.com>
    SPDX-FileCopyrightText: 2009 Radek Novacek <rnovacek@redhat.com>
    SPDX-FileCopyrightText: 2009-2010 Dario Freddi <drf@kde.org>
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2023 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "Polkit1Backend.h"
#include "kauthdebug.h"

#include <KWaylandExtras>
#include <KWindowSystem>

#include <QCoreApplication>
#include <QTimer>
#include <qplugin.h>

#include <QGuiApplication>
#include <QWindow>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

#include <PolkitQt1/Subject>
#include <polkitqt1-version.h>

constexpr QLatin1String c_kdeAgentService{"org.kde.polkit-kde-authentication-agent-1"};
constexpr QLatin1String c_kdeAgentPath{"/org/kde/Polkit1AuthAgent"};
constexpr QLatin1String c_kdeAgentInterface{"org.kde.Polkit1AuthAgent"};

namespace KAuth
{

Polkit1Backend::Polkit1Backend()
    : AuthBackend()
{
    setCapabilities(AuthorizeFromHelperCapability | PreAuthActionCapability);

    // Setup useful signals
    connect(PolkitQt1::Authority::instance(), &PolkitQt1::Authority::configChanged, this, &KAuth::Polkit1Backend::checkForResultChanged);
    connect(PolkitQt1::Authority::instance(), &PolkitQt1::Authority::consoleKitDBChanged, this, &KAuth::Polkit1Backend::checkForResultChanged);
}

Polkit1Backend::~Polkit1Backend()
{
}

void Polkit1Backend::preAuthAction(const QString &action, QWindow *parentWindow)
{
    // If a parent was not specified, skip this
    if (!parentWindow) {
        qCDebug(KAUTH) << "Parent widget does not exist, skipping";
        return;
    }

    // Check if we actually are entitled to use GUI capabilities
    if (!qGuiApp) {
        qCDebug(KAUTH) << "Not streaming parent as we are on a TTY application";
        return;
    }

    // Are we running our KDE auth agent?
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(QLatin1String("org.kde.polkit-kde-authentication-agent-1"))) {
        if (KWindowSystem::isPlatformWayland()) {
            KWaylandExtras::exportWindow(parentWindow);
            connect(
                KWaylandExtras::self(),
                &KWaylandExtras::windowExported,
                this,
                [this, action, parentWindow](QWindow *window, const QString &handle) {
                    if (window == parentWindow) {
                        sendWindowHandle(action, handle);
                    }
                },
                Qt::SingleShotConnection);

            // Generate and send an XDG Activation token.
            sendActivationToken(action, parentWindow);
        } else {
            // Retrieve the dialog root window Id
            const qulonglong wId = parentWindow->winId();

            sendWindowHandle(action, QString::number(wId));

            // Call the old method for compatibility.
            QDBusMessage methodCall = QDBusMessage::createMethodCall(c_kdeAgentService, c_kdeAgentPath, c_kdeAgentInterface, QLatin1String("setWIdForAction"));
            methodCall << action;
            methodCall << wId;

            // Legacy call has to be blocking, old agent doesn't handle it coming in delayed.
            const auto reply = QDBusConnection::sessionBus().call(methodCall);
            if (reply.type() != QDBusMessage::ReplyMessage) {
                qWarning() << "Failed to set window id" << wId << "for" << action << reply.errorMessage();
            }
        }
    } else {
        qCDebug(KAUTH) << "KDE polkit agent appears too old or not registered on the bus";
    }
}

void Polkit1Backend::sendWindowHandle(const QString &action, const QString &handle)
{
    // Send it over the bus to our agent
    QDBusMessage methodCall = QDBusMessage::createMethodCall(c_kdeAgentService, c_kdeAgentPath, c_kdeAgentInterface, QLatin1String("setWindowHandleForAction"));
    methodCall << action;
    methodCall << handle;

    const auto reply = QDBusConnection::sessionBus().asyncCall(methodCall);
    auto *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, handle, action] {
        watcher->deleteLater();

        QDBusPendingReply<> reply = *watcher;
        if (reply.isError()) {
            qCWarning(KAUTH) << "Failed to set window handle" << handle << "for" << action << reply.error().message();
        }
    });
}

void Polkit1Backend::sendActivationToken(const QString &action, QWindow *window)
{
    const auto requestedSerial = KWaylandExtras::lastInputSerial(window);
    connect(
        KWaylandExtras::self(),
        &KWaylandExtras::xdgActivationTokenArrived,
        this,
        [this, requestedSerial, action](quint32 serial, const QString &token) {
            if (serial != requestedSerial || token.isEmpty()) {
                return;
            }
            QDBusMessage methodCall =
                QDBusMessage::createMethodCall(c_kdeAgentService, c_kdeAgentPath, c_kdeAgentInterface, QLatin1String("setActivationTokenForAction"));
            methodCall << action;
            methodCall << token;

            const auto reply = QDBusConnection::sessionBus().asyncCall(methodCall);
            auto *watcher = new QDBusPendingCallWatcher(reply, this);
            connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, token, action] {
                watcher->deleteLater();

                QDBusPendingReply<> reply = *watcher;
                if (reply.isError()) {
                    qCWarning(KAUTH) << "Failed to set activation token" << token << "for" << action << reply.error().message();
                }
            });
        },
        Qt::SingleShotConnection);
    KWaylandExtras::requestXdgActivationToken(window, requestedSerial, {});
}

Action::AuthStatus Polkit1Backend::authorizeAction(const QString &action)
{
    Q_UNUSED(action)
    // Always return Yes here, we'll authorize inside isCallerAuthorized
    return Action::AuthorizedStatus;
}

void Polkit1Backend::setupAction(const QString &action)
{
    m_cachedResults[action] = actionStatus(action);
}

Action::AuthStatus Polkit1Backend::actionStatus(const QString &action)
{
    PolkitQt1::SystemBusNameSubject subject(QString::fromUtf8(callerID()));
    auto authority = PolkitQt1::Authority::instance();
    PolkitQt1::Authority::Result r = authority->checkAuthorizationSync(action, subject, PolkitQt1::Authority::None);

    if (authority->hasError()) {
        qCDebug(KAUTH) << "Encountered error while checking action status, error code:" << authority->lastError() << authority->errorDetails();
        authority->clearError();
        return Action::InvalidStatus;
    }

    switch (r) {
    case PolkitQt1::Authority::Yes:
        return Action::AuthorizedStatus;
    case PolkitQt1::Authority::No:
    case PolkitQt1::Authority::Unknown:
        return Action::DeniedStatus;
    default:
        return Action::AuthRequiredStatus;
    }
}

QByteArray Polkit1Backend::callerID() const
{
    return QDBusConnection::systemBus().baseService().toUtf8();
}

bool Polkit1Backend::isCallerAuthorized(const QString &action, const QByteArray &callerID, const QVariantMap &details)
{
    PolkitQt1::SystemBusNameSubject subject(QString::fromUtf8(callerID));
    PolkitQt1::Authority *authority = PolkitQt1::Authority::instance();
    QMap<QString, QString> polkit1Details;
    for (auto it = details.cbegin(); it != details.cend(); ++it) {
        polkit1Details.insert(it.key(), it.value().toString());
    }

    PolkitQt1::Authority::Result result;
    QEventLoop e;
    connect(authority, &PolkitQt1::Authority::checkAuthorizationFinished, &e, [&result, &e](PolkitQt1::Authority::Result _result) {
        result = _result;
        e.quit();
    });

#if POLKITQT1_IS_VERSION(0, 113, 0)
    authority->checkAuthorizationWithDetails(action, subject, PolkitQt1::Authority::AllowUserInteraction, polkit1Details);
#else
    authority->checkAuthorization(action, subject, PolkitQt1::Authority::AllowUserInteraction);
#endif
    e.exec();

    if (authority->hasError()) {
        qCDebug(KAUTH) << "Encountered error while checking authorization, error code:" << authority->lastError() << authority->errorDetails();
        authority->clearError();
    }

    switch (result) {
    case PolkitQt1::Authority::Yes:
        return true;
    default:
        return false;
    }
}

void Polkit1Backend::checkForResultChanged()
{
    for (auto it = m_cachedResults.begin(); it != m_cachedResults.end(); ++it) {
        const QString action = it.key();
        if (it.value() != actionStatus(action)) {
            *it = actionStatus(action);
            Q_EMIT actionStatusChanged(action, *it);
        }
    }
}

QVariantMap Polkit1Backend::backendDetails(const DetailsMap &details)
{
    QVariantMap backendDetails;
    for (auto it = details.cbegin(); it != details.cend(); ++it) {
        switch (it.key()) {
        case Action::AuthDetail::DetailMessage:
            backendDetails.insert(QStringLiteral("polkit.message"), it.value());
            break;
        case Action::AuthDetail::DetailOther:
        default:
            backendDetails.insert(QStringLiteral("other_details"), it.value());
            break;
        }
    }
    return backendDetails;
}

} // namespace Auth

#include "Polkit1Backend.moc"
