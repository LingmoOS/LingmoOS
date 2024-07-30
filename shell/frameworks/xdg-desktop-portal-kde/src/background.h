/*
 * SPDX-FileCopyrightText: 2020 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2020 Jan Grulich <jgrulich@redhat.com>
 */

#ifndef XDG_DESKTOP_PORTAL_KDE_BACKGROUND_H
#define XDG_DESKTOP_PORTAL_KDE_BACKGROUND_H

#include <QDBusAbstractAdaptor>
#include <QDBusContext>
#include <QDBusObjectPath>
#include <QSet>

namespace KWayland
{
namespace Client
{
class PlasmaWindow;
}
}

class KNotification;

class BackgroundPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.Background")
public:
    explicit BackgroundPortal(QObject *parent, QDBusContext *context);
    ~BackgroundPortal() override;

    enum ApplicationState {
        Background = 0,
        Running = 1,
        Active = 2,
    };

    enum AutostartFlag {
        None = 0x0,
        Activatable = 0x1,
    };
    Q_DECLARE_FLAGS(AutostartFlags, AutostartFlag)

    enum NotifyResult {
        Forbid = 0,
        Allow = 1,
        AllowOnce = 2,
    };

public Q_SLOTS:
    QVariantMap GetAppState();

    uint NotifyBackground(const QDBusObjectPath &handle, const QString &app_id, const QString &name, QVariantMap &results);

    bool EnableAutostart(const QString &app_id, bool enable, const QStringList &commandline, uint flags);
Q_SIGNALS:
    void RunningApplicationsChanged();

private:
    void addWindow(KWayland::Client::PlasmaWindow *window);
    void setActiveWindow(const QString &appId, bool active);

    uint m_notificationCounter = 0;
    QList<KWayland::Client::PlasmaWindow *> m_windows;
    QVariantMap m_appStates;
    QSet<QString> m_backgroundAppWarned;
    QDBusContext *const m_context;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(BackgroundPortal::AutostartFlags)

#endif // XDG_DESKTOP_PORTAL_KDE_BACKGROUND_H
