/*
    This file is part of libkdbusaddons

    SPDX-FileCopyrightText: 2011 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2011 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kdbusservice.h"

#include <QCoreApplication>
#include <QDebug>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>

#include "FreeDesktopApplpicationIface.h"
#include "KDBusServiceIface.h"

#include "config-kdbusaddons.h"

#if HAVE_X11
#include <private/qtx11extras_p.h>
#endif

#include "kdbusaddons_debug.h"
#include "kdbusservice_adaptor.h"
#include "kdbusserviceextensions_adaptor.h"

class KDBusServicePrivate
{
public:
    KDBusServicePrivate()
        : registered(false)
        , exitValue(0)
    {
    }

    QString generateServiceName()
    {
        const QCoreApplication *app = QCoreApplication::instance();
        const QString domain = app->organizationDomain();
        const QStringList parts = domain.split(QLatin1Char('.'), Qt::SkipEmptyParts);

        QString reversedDomain;
        if (parts.isEmpty()) {
            reversedDomain = QStringLiteral("local.");
        } else {
            for (const QString &part : parts) {
                reversedDomain.prepend(QLatin1Char('.'));
                reversedDomain.prepend(part);
            }
        }

        return reversedDomain + app->applicationName();
    }

    static void handlePlatformData(const QVariantMap &platformData)
    {
        #if HAVE_X11
        if (QX11Info::isPlatformX11()) {
            QByteArray desktopStartupId = platformData.value(QStringLiteral("desktop-startup-id")).toByteArray();
            if (!desktopStartupId.isEmpty()) {
                QX11Info::setNextStartupId(desktopStartupId);
            }
        }
        #endif

        const auto xdgActivationToken = platformData.value(QLatin1String("activation-token")).toByteArray();
        if (!xdgActivationToken.isEmpty()) {
            qputenv("XDG_ACTIVATION_TOKEN", xdgActivationToken);
        }
    }

    bool registered;
    QString serviceName;
    QString errorMessage;
    int exitValue;
};

// Wraps a serviceName registration.
class Registration : public QObject
{
    Q_OBJECT
public:
    enum class Register {
        RegisterWitoutQueue,
        RegisterWithQueue,
    };

    Registration(KDBusService *s_, KDBusServicePrivate *d_, KDBusService::StartupOptions options_)
        : s(s_)
        , d(d_)
        , options(options_)
    {
        if (!QDBusConnection::sessionBus().isConnected() || !(bus = QDBusConnection::sessionBus().interface())) {
            d->errorMessage = QLatin1String(
                "DBus session bus not found. To circumvent this problem try the following command (with bash):\n"
                "    export $(dbus-launch)");
        } else {
            generateServiceName();
        }
    }

    void run()
    {
        if (bus) {
            registerOnBus();
        }

        if (!d->registered && ((options & KDBusService::NoExitOnFailure) == 0)) {
            qCCritical(KDBUSADDONS_LOG) << qPrintable(d->errorMessage);
            exit(1);
        }
    }

private:
    void generateServiceName()
    {
        d->serviceName = d->generateServiceName();
        objectPath = QLatin1Char('/') + d->serviceName;
        objectPath.replace(QLatin1Char('.'), QLatin1Char('/'));
        objectPath.replace(QLatin1Char('-'), QLatin1Char('_')); // see spec change at https://bugs.freedesktop.org/show_bug.cgi?id=95129

        if (options & KDBusService::Multiple) {
            const bool inSandbox = QFileInfo::exists(QStringLiteral("/.flatpak-info"));
            if (inSandbox) {
                d->serviceName += QStringLiteral(".kdbus-")
                    + QDBusConnection::sessionBus().baseService().replace(QRegularExpression(QStringLiteral("[\\.:]")), QStringLiteral("_"));
            } else {
                d->serviceName += QLatin1Char('-') + QString::number(QCoreApplication::applicationPid());
            }
        }
    }

    void registerOnBus()
    {
        auto bus = QDBusConnection::sessionBus();
        bool objectRegistered = false;
        objectRegistered = bus.registerObject(QStringLiteral("/MainApplication"),
                                              QCoreApplication::instance(),
                                              QDBusConnection::ExportAllSlots //
                                                  | QDBusConnection::ExportScriptableProperties //
                                                  | QDBusConnection::ExportAdaptors);
        if (!objectRegistered) {
            qCWarning(KDBUSADDONS_LOG) << "Failed to register /MainApplication on DBus";
            return;
        }

        objectRegistered = bus.registerObject(objectPath, s, QDBusConnection::ExportAdaptors);
        if (!objectRegistered) {
            qCWarning(KDBUSADDONS_LOG) << "Failed to register" << objectPath << "on DBus";
            return;
        }

        attemptRegistration();

        if (d->registered) {
            if (QCoreApplication *app = QCoreApplication::instance()) {
                connect(app, &QCoreApplication::aboutToQuit, s, &KDBusService::unregister);
            }
        }
    }

    void attemptRegistration()
    {
        Q_ASSERT(!d->registered);

        auto queueOption = QDBusConnectionInterface::DontQueueService;

        if (options & KDBusService::Unique) {
            // When a process crashes and gets auto-restarted by KCrash we may
            // be in this code path "too early". There is a bit of a delay
            // between the restart and the previous process dropping off of the
            // bus and thus releasing its registered names. As a result there
            // is a good chance that if we wait a bit the name will shortly
            // become registered.

            queueOption = QDBusConnectionInterface::QueueService;

            connect(bus, &QDBusConnectionInterface::serviceRegistered, this, [this](const QString &service) {
                if (service != d->serviceName) {
                    return;
                }

                d->registered = true;
                registrationLoop.quit();
            });
        }

        d->registered = (bus->registerService(d->serviceName, queueOption) == QDBusConnectionInterface::ServiceRegistered);

        if (d->registered) {
            return;
        }

        if (options & KDBusService::Replace) {
            auto message = QDBusMessage::createMethodCall(d->serviceName,
                                                          QStringLiteral("/MainApplication"),
                                                          QStringLiteral("org.qtproject.Qt.QCoreApplication"),
                                                          QStringLiteral("quit"));
            QDBusConnection::sessionBus().asyncCall(message);
            waitForRegistration();
        } else if (options & KDBusService::Unique) {
            // Already running so it's ok!
            QVariantMap platform_data;
#if HAVE_X11
            if (QX11Info::isPlatformX11()) {
                QString startupId = QString::fromUtf8(qgetenv("DESKTOP_STARTUP_ID"));
                if (startupId.isEmpty()) {
                    startupId = QString::fromUtf8(QX11Info::nextStartupId());
                }
                if (!startupId.isEmpty()) {
                    platform_data.insert(QStringLiteral("desktop-startup-id"), startupId);
                }
            }
#endif

            if (qEnvironmentVariableIsSet("XDG_ACTIVATION_TOKEN")) {
                platform_data.insert(QStringLiteral("activation-token"), qgetenv("XDG_ACTIVATION_TOKEN"));
            }

            if (QCoreApplication::arguments().count() > 1) {
                OrgKdeKDBusServiceInterface iface(d->serviceName, objectPath, QDBusConnection::sessionBus());
                iface.setTimeout(5 * 60 * 1000); // Application can take time to answer
                QDBusReply<int> reply = iface.CommandLine(QCoreApplication::arguments(), QDir::currentPath(), platform_data);
                if (reply.isValid()) {
                    exit(reply.value());
                } else {
                    d->errorMessage = reply.error().message();
                }
            } else {
                OrgFreedesktopApplicationInterface iface(d->serviceName, objectPath, QDBusConnection::sessionBus());
                iface.setTimeout(5 * 60 * 1000); // Application can take time to answer
                QDBusReply<void> reply = iface.Activate(platform_data);
                if (reply.isValid()) {
                    exit(0);
                } else {
                    d->errorMessage = reply.error().message();
                }
            }

            // service did not respond in a valid way....
            // let's wait to see if our queued registration finishes perhaps.
            waitForRegistration();
        }

        if (!d->registered) { // either multi service or failed to reclaim name
            d->errorMessage = QLatin1String("Failed to register name '") + d->serviceName + QLatin1String("' with DBUS - does this process have permission to use the name, and do no other processes own it already?");
        }
    }

    void waitForRegistration()
    {
        QTimer quitTimer;
        // Wait a bit longer when we know this instance was restarted. There's
        // a very good chance we'll eventually get the name once the defunct
        // process closes its sockets.
        quitTimer.start(qEnvironmentVariableIsSet("KCRASH_AUTO_RESTARTED") ? 8000 : 2000);
        connect(&quitTimer, &QTimer::timeout, &registrationLoop, &QEventLoop::quit);
        registrationLoop.exec();
    }

    QDBusConnectionInterface *bus = nullptr;
    KDBusService *s = nullptr;
    KDBusServicePrivate *d = nullptr;
    KDBusService::StartupOptions options;
    QEventLoop registrationLoop;
    QString objectPath;
};

KDBusService::KDBusService(StartupOptions options, QObject *parent)
    : QObject(parent)
    , d(new KDBusServicePrivate)
{
    new KDBusServiceAdaptor(this);
    new KDBusServiceExtensionsAdaptor(this);

    Registration registration(this, d.get(), options);
    registration.run();
}

KDBusService::~KDBusService() = default;

bool KDBusService::isRegistered() const
{
    return d->registered;
}

QString KDBusService::errorMessage() const
{
    return d->errorMessage;
}

void KDBusService::setExitValue(int value)
{
    d->exitValue = value;
}

QString KDBusService::serviceName() const
{
    return d->serviceName;
}

void KDBusService::unregister()
{
    QDBusConnectionInterface *bus = nullptr;
    if (!d->registered || !QDBusConnection::sessionBus().isConnected() || !(bus = QDBusConnection::sessionBus().interface())) {
        return;
    }
    bus->unregisterService(d->serviceName);
}

void KDBusService::Activate(const QVariantMap &platform_data)
{
    d->handlePlatformData(platform_data);
    Q_EMIT activateRequested(QStringList(QCoreApplication::arguments()[0]), QDir::currentPath());
    qunsetenv("XDG_ACTIVATION_TOKEN");
}

void KDBusService::Open(const QStringList &uris, const QVariantMap &platform_data)
{
    d->handlePlatformData(platform_data);
    Q_EMIT openRequested(QUrl::fromStringList(uris));
    qunsetenv("XDG_ACTIVATION_TOKEN");
}

void KDBusService::ActivateAction(const QString &action_name, const QVariantList &maybeParameter, const QVariantMap &platform_data)
{
    d->handlePlatformData(platform_data);

    // This is a workaround for D-Bus not supporting null variants.
    const QVariant param = maybeParameter.count() == 1 ? maybeParameter.first() : QVariant();

    Q_EMIT activateActionRequested(action_name, param);
    qunsetenv("XDG_ACTIVATION_TOKEN");
}

int KDBusService::CommandLine(const QStringList &arguments, const QString &workingDirectory, const QVariantMap &platform_data)
{
    d->exitValue = 0;
    d->handlePlatformData(platform_data);
    // The TODOs here only make sense if this method can be called from the GUI.
    // If it's for pure "usage in the terminal" then no startup notification got started.
    // But maybe one day the workspace wants to call this for the Exec key of a .desktop file?
    Q_EMIT activateRequested(arguments, workingDirectory);
    qunsetenv("XDG_ACTIVATION_TOKEN");
    return d->exitValue;
}

#include "kdbusservice.moc"
#include "moc_kdbusservice.cpp"
