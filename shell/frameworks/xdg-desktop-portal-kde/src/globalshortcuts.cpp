/*
 * SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "globalshortcuts.h"
#include "notificationinhibition.h"
#include "quickdialog.h"
#include "session.h"
#include "utils.h"
#include "waylandintegration.h"

#include <KGlobalAccel>
#include <QAction>
#include <QDBusMetaType>
#include <QDataStream>
#include <QDesktopServices>
#include <QLoggingCategory>
#include <QUrl>

Q_LOGGING_CATEGORY(XdgDesktopPortalKdeGlobalShortcuts, "xdp-kde-GlobalShortcuts")

GlobalShortcutsPortal::GlobalShortcutsPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    qDBusRegisterMetaType<Shortcut>();
    qDBusRegisterMetaType<Shortcuts>();

    Q_ASSERT(QLatin1String(QDBusMetaType::typeToSignature(QMetaType(qMetaTypeId<Shortcuts>()))) == QLatin1String("a(sa{sv})"));
}

GlobalShortcutsPortal::~GlobalShortcutsPortal() = default;

uint GlobalShortcutsPortal::version() const
{
    return 1;
}

uint GlobalShortcutsPortal::CreateSession(const QDBusObjectPath &handle,
                                          const QDBusObjectPath &session_handle,
                                          const QString &app_id,
                                          const QVariantMap &options,
                                          QVariantMap &results)
{
    qCDebug(XdgDesktopPortalKdeGlobalShortcuts) << "CreateSession called with parameters:";
    qCDebug(XdgDesktopPortalKdeGlobalShortcuts) << "    handle: " << handle.path();
    qCDebug(XdgDesktopPortalKdeGlobalShortcuts) << "    session_handle: " << session_handle.path();
    qCDebug(XdgDesktopPortalKdeGlobalShortcuts) << "    app_id: " << app_id;
    qCDebug(XdgDesktopPortalKdeGlobalShortcuts) << "    options: " << options;

    auto session = qobject_cast<GlobalShortcutsSession *>(Session::createSession(this, Session::GlobalShortcuts, app_id, session_handle.path()));
    if (!session) {
        return 2;
    }

    session->loadActions();

    connect(session, &GlobalShortcutsSession::shortcutsChanged, this, [this, session, session_handle] {
        Q_EMIT ShortcutsChanged(session_handle, session->shortcutDescriptions());
    });

    connect(session, &GlobalShortcutsSession::shortcutActivated, this, [this, session](const QString &shortcutName, qlonglong timestamp) {
        Q_EMIT Activated(QDBusObjectPath(session->handle()), shortcutName, timestamp);
    });
    connect(session, &GlobalShortcutsSession::shortcutDeactivated, this, [this, session](const QString &shortcutName, qlonglong timestamp) {
        Q_EMIT Deactivated(QDBusObjectPath(session->handle()), shortcutName, timestamp);
    });

    Q_UNUSED(results);
    return 0;
}

uint GlobalShortcutsPortal::ListShortcuts(const QDBusObjectPath &handle, const QDBusObjectPath &session_handle, QVariantMap &results)
{
    qCDebug(XdgDesktopPortalKdeGlobalShortcuts) << "ListShortcuts called with parameters:";
    qCDebug(XdgDesktopPortalKdeGlobalShortcuts) << "    handle: " << handle.path();
    qCDebug(XdgDesktopPortalKdeGlobalShortcuts) << "    session_handle: " << session_handle.path();

    auto session = qobject_cast<GlobalShortcutsSession *>(Session::getSession(session_handle.path()));
    if (!session) {
        return 2;
    }
    results = {
        {"shortcuts", session->shortcutDescriptionsVariant()},
    };
    return 0;
}

uint GlobalShortcutsPortal::BindShortcuts(const QDBusObjectPath &handle,
                                          const QDBusObjectPath &session_handle,
                                          const Shortcuts &shortcuts,
                                          const QString &parent_window,
                                          const QVariantMap &options,
                                          QVariantMap &results)
{
    qCDebug(XdgDesktopPortalKdeGlobalShortcuts) << "BindShortcuts called with parameters:";
    qCDebug(XdgDesktopPortalKdeGlobalShortcuts) << "    handle: " << handle.path();
    qCDebug(XdgDesktopPortalKdeGlobalShortcuts) << "    session_handle: " << session_handle.path();
    qCDebug(XdgDesktopPortalKdeGlobalShortcuts) << "    shortcuts: " << shortcuts;
    qCDebug(XdgDesktopPortalKdeGlobalShortcuts) << "    parent_window: " << parent_window;
    qCDebug(XdgDesktopPortalKdeGlobalShortcuts) << "    options: " << options;

    auto session = qobject_cast<GlobalShortcutsSession *>(Session::getSession(session_handle.path()));
    if (!session) {
        return 2;
    }
    session->setActions(shortcuts);
    QDesktopServices::openUrl(QUrl(QStringLiteral("systemsettings://kcm_keys/") + session->componentName()));

    results = {
        {"shortcuts", session->shortcutDescriptionsVariant()},
    };
    return 0;
}
