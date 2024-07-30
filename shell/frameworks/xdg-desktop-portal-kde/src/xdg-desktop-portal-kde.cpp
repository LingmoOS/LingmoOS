/*
 * SPDX-FileCopyrightText: 2016 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>
 */

#include <QApplication>
#include <QDBusConnection>

#include <KAboutData>
#include <KLocalizedString>

#include "../version.h"
#include "debug.h"
#include "desktopportal.h"

int main(int argc, char *argv[])
{
    // Do not try to reconnect to the compositor as KWayland can't handle that
    // Reconsider enabling when we do not use KWayland anymore
    qunsetenv("QT_WAYLAND_RECONNECT");
    QCoreApplication::setAttribute(Qt::AA_DisableSessionManager);
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);
    a.setQuitLockEnabled(false);

    KAboutData about(QStringLiteral("xdg-desktop-portal-kde"), QString(), QStringLiteral(XDPK_VERSION_STRING));
    about.setDesktopFileName(QStringLiteral("org.freedesktop.impl.portal.desktop.kde"));
    KAboutData::setApplicationData(about);

    QDBusConnection sessionBus = QDBusConnection::sessionBus();

    if (sessionBus.registerService(QStringLiteral("org.freedesktop.impl.portal.desktop.kde"))) {
        DesktopPortal *desktopPortal = new DesktopPortal(&a);
        if (sessionBus.registerObject(QStringLiteral("/org/freedesktop/portal/desktop"), desktopPortal, QDBusConnection::ExportAdaptors)) {
            qCDebug(XdgDesktopPortalKde) << "Desktop portal registered successfully";
        } else {
            qCDebug(XdgDesktopPortalKde) << "Failed to register desktop portal";
        }
    } else {
        qCDebug(XdgDesktopPortalKde) << "Failed to register org.freedesktop.impl.portal.desktop.kde service";
        return 1;
    }

    return a.exec();
}
