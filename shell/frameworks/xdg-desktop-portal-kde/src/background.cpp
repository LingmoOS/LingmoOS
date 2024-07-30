/*
 * SPDX-FileCopyrightText: 2020 Red Hat Inc
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2020 Jan Grulich <jgrulich@redhat.com>
 */

#include "background.h"
#include "background_debug.h"
#include "ksharedconfig.h"
#include "waylandintegration.h"

#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusMessage>
#include <QDBusMetaType>

#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>

#include <KConfigGroup>
#include <KDesktopFile>
#include <KLocalizedString>
#include <KNotification>
#include <KService>
#include <KShell>

#include <KWayland/Client/plasmawindowmanagement.h>

BackgroundPortal::BackgroundPortal(QObject *parent, QDBusContext *context)
    : QDBusAbstractAdaptor(parent)
    , m_context(context)
{
    connect(WaylandIntegration::waylandIntegration(), &WaylandIntegration::WaylandIntegration::plasmaWindowManagementInitialized, this, [this]() {
        connect(WaylandIntegration::plasmaWindowManagement(),
                &KWayland::Client::PlasmaWindowManagement::windowCreated,
                this,
                [this](KWayland::Client::PlasmaWindow *window) {
                    addWindow(window);
                });

        m_windows = WaylandIntegration::plasmaWindowManagement()->windows();
        for (KWayland::Client::PlasmaWindow *window : std::as_const(m_windows)) {
            addWindow(window);
        }
    });
}

BackgroundPortal::~BackgroundPortal()
{
}

QVariantMap BackgroundPortal::GetAppState()
{
    qCDebug(XdgDesktopPortalKdeBackground) << "GetAppState called: no parameters";
    return m_appStates;
}

uint BackgroundPortal::NotifyBackground(const QDBusObjectPath &handle, const QString &app_id, const QString &name, QVariantMap &results)
{
    Q_UNUSED(results);

    qCDebug(XdgDesktopPortalKdeBackground) << "NotifyBackground called with parameters:";
    qCDebug(XdgDesktopPortalKdeBackground) << "    handle: " << handle.path();
    qCDebug(XdgDesktopPortalKdeBackground) << "    app_id: " << app_id;
    qCDebug(XdgDesktopPortalKdeBackground) << "    name: " << name;

    // If KWayland::Client::PlasmaWindowManagement hasn't been created, we would be notified about every
    // application, which is not what we want. This will be mostly happening on X11 session.
    if (!WaylandIntegration::plasmaWindowManagement()) {
        results.insert(QStringLiteral("result"), static_cast<uint>(BackgroundPortal::AllowOnce));
        return 0;
    }

    QDBusMessage message = m_context->message();
    auto allow = [message]() {
        const QVariantMap map = {{QStringLiteral("result"), static_cast<uint>(BackgroundPortal::Allow)}};
        QDBusMessage reply = message.createReply({static_cast<uint>(0), map});
        if (!QDBusConnection::sessionBus().send(reply)) {
            qCWarning(XdgDesktopPortalKdeBackground) << "Failed to send response";
        }
    };

    auto allowOnce = [message]() {
        const QVariantMap map = {{QStringLiteral("result"), static_cast<uint>(BackgroundPortal::AllowOnce)}};
        QDBusMessage reply = message.createReply({static_cast<uint>(0), map});
        if (!QDBusConnection::sessionBus().send(reply)) {
            qCWarning(XdgDesktopPortalKdeBackground) << "Failed to send response";
        }
    };

    auto forbid = [message]() {
        const QVariantMap map = {{QStringLiteral("result"), static_cast<uint>(BackgroundPortal::Forbid)}};
        QDBusMessage reply = message.createReply({static_cast<uint>(0), map});
        if (!QDBusConnection::sessionBus().send(reply)) {
            qCWarning(XdgDesktopPortalKdeBackground) << "Failed to send response";
        }
    };

    if (KSharedConfig::openConfig()->group("Background").readEntry("NotifyBackgroundApps", true)) {
        allowOnce();
        return 0;
    }

    const KService::Ptr app = KService::serviceByDesktopName(app_id);

    QObject *obj = QObject::parent();

    if (!obj) {
        qCWarning(XdgDesktopPortalKdeBackground) << "Failed to get dbus context";
        return 2;
    }

    const QString appName = app ? app->name() : app_id;
    if (m_backgroundAppWarned.contains(app_id)) {
        const QVariantMap map = {
            {QStringLiteral("result"), static_cast<uint>(BackgroundPortal::AllowOnce)},
        };
        QDBusMessage reply = message.createReply({uint(0), map});
        if (!QDBusConnection::sessionBus().send(reply)) {
            qCWarning(XdgDesktopPortalKdeBackground) << "Failed to send response";
        }

        return 0;
    }

    KNotification *notify = new KNotification(QStringLiteral("notification"), KNotification::Persistent | KNotification::DefaultEvent, this);
    notify->setTitle(i18n("Background Activity"));
    notify->setText(
        i18nc("@info %1 is the name of an application",
              "%1 wants to remain running when it has no visible windows. If you forbid this, the application will quit when its last window is closed.",
              appName));
    notify->setProperty("activated", false);

    message.setDelayedReply(true);

    auto allowAction = notify->addAction(i18nc("@action:button Allow the app to keep running with no open windows", "Allow"));

    connect(allowAction, &KNotificationAction::activated, this, [allow, notify] {
        allow();
        notify->setProperty("activated", true);
    });

    auto forbidAction = notify->addAction(i18nc("@action:button Don't allow the app to keep running without any open windows", "Forbid"));

    connect(forbidAction, &KNotificationAction::activated, this, [this, appName, allow, forbid, notify] {
        const QString title =
            i18nc("@title title of a dialog to confirm whether to allow an app to remain running with no visible windows", "Background App Usage");
        const QString text = i18nc("%1 is the name of an application",
                                   "Note that this will force %1 to quit when its last window is closed. This could cause data loss if the application has "
                                   "any unsaved changes when it happens.",
                                   appName);
        auto messageBox = new QMessageBox(QMessageBox::Question, title, text);
        messageBox->addButton(i18nc("@action:button Allow the app to keep running with no open windows", "Allow"), QMessageBox::AcceptRole);
        messageBox->addButton(i18nc("@action:button Don't allow the app to keep running without any open windows", "Forbid Anyway"), QMessageBox::RejectRole);
        messageBox->show();

        connect(messageBox, &QMessageBox::accepted, this, [messageBox, allow]() {
            allow();
            messageBox->deleteLater();
        });
        connect(messageBox, &QMessageBox::rejected, this, [messageBox, forbid]() {
            forbid();
            messageBox->deleteLater();
        });

        notify->setProperty("activated", true);
    });

    connect(notify, &KNotification::closed, this, [notify, allowOnce]() {
        if (notify->property("activated").toBool()) {
            return;
        }

        allowOnce();
    });

    notify->sendEvent();

    Q_ASSERT(!m_backgroundAppWarned.contains(app_id));
    connect(notify, &KNotification::closed, this, [this, app_id] {
        m_backgroundAppWarned.remove(app_id);
    });
    m_backgroundAppWarned.insert(app_id);

    return 0;
}

bool BackgroundPortal::EnableAutostart(const QString &app_id, bool enable, const QStringList &commandline, uint flags)
{
    qCDebug(XdgDesktopPortalKdeBackground) << "EnableAutostart called with parameters:";
    qCDebug(XdgDesktopPortalKdeBackground) << "    app_id: " << app_id;
    qCDebug(XdgDesktopPortalKdeBackground) << "    enable: " << enable;
    qCDebug(XdgDesktopPortalKdeBackground) << "    commandline: " << commandline;
    qCDebug(XdgDesktopPortalKdeBackground) << "    flags: " << flags;

    const QString fileName = app_id + QStringLiteral(".desktop");
    const QString directory = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/autostart/");
    const QString fullPath = directory + fileName;
    const AutostartFlags autostartFlags = static_cast<AutostartFlags>(flags);

    if (!enable) {
        QFile file(fullPath);
        if (!file.remove()) {
            qCDebug(XdgDesktopPortalKdeBackground) << "Failed to remove " << fileName << " to disable autostart.";
        }
        return false;
    }

    QDir dir(directory);
    if (!dir.mkpath(dir.absolutePath())) {
        qCDebug(XdgDesktopPortalKdeBackground) << "Failed to create autostart directory.";
        return false;
    }

    KDesktopFile desktopFile(fullPath);
    KConfigGroup desktopEntryConfigGroup = desktopFile.desktopGroup();
    desktopEntryConfigGroup.writeEntry(QStringLiteral("Type"), QStringLiteral("Application"));
    desktopEntryConfigGroup.writeEntry(QStringLiteral("Name"), app_id);
    desktopEntryConfigGroup.writeEntry(QStringLiteral("Exec"), KShell::joinArgs(commandline));
    if (autostartFlags.testFlag(AutostartFlag::Activatable)) {
        desktopEntryConfigGroup.writeEntry(QStringLiteral("DBusActivatable"), true);
    }
    desktopEntryConfigGroup.writeEntry(QStringLiteral("X-Flatpak"), app_id);

    return true;
}

void BackgroundPortal::addWindow(KWayland::Client::PlasmaWindow *window)
{
    const QString appId = window->appId();
    const bool isActive = window->isActive();
    m_appStates[appId] = QVariant::fromValue<uint>(isActive ? Active : Running);

    connect(window, &KWayland::Client::PlasmaWindow::activeChanged, this, [this, window]() {
        setActiveWindow(window->appId(), window->isActive());
    });
    connect(window, &KWayland::Client::PlasmaWindow::unmapped, this, [this, window]() {
        uint windows = 0;
        const QString appId = window->appId();
        const auto plasmaWindows = WaylandIntegration::plasmaWindowManagement()->windows();
        for (KWayland::Client::PlasmaWindow *otherWindow : plasmaWindows) {
            if (otherWindow->appId() == appId && otherWindow->uuid() != window->uuid()) {
                windows++;
            }
        }

        if (!windows) {
            m_appStates.remove(appId);
            Q_EMIT RunningApplicationsChanged();
        }
    });

    Q_EMIT RunningApplicationsChanged();
}

void BackgroundPortal::setActiveWindow(const QString &appId, bool active)
{
    m_appStates[appId] = QVariant::fromValue<uint>(active ? Active : Running);

    Q_EMIT RunningApplicationsChanged();
}
