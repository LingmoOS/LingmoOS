/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWAYLANDINTEGRATION_H
#define KWAYLANDINTEGRATION_H

#include "kdeplatformtheme.h"
#include <QHash>
#include <QObject>
#include <QtWaylandClient/QWaylandClientExtensionTemplate>

class QWindow;

class AppMenuManager;
class ServerSideDecorationPaletteManager;

class KWaylandIntegration : public QObject
{
    Q_OBJECT
public:
    explicit KWaylandIntegration(KdePlatformTheme *platformTheme);
    ~KWaylandIntegration() override;

    void setAppMenu(QWindow *window, const QString &serviceName, const QString &objectPath);
    void setPalette(QWindow *window, const QString &paletteName);

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    static bool isRelevantTopLevel(QWindow *w);
    static struct wl_surface *surfaceFromWindow(QWindow *w);
    void shellSurfaceCreated(QWindow *w);
    void shellSurfaceDestroyed(QWindow *w);

    void installColorScheme(QWindow *w);
    QScopedPointer<AppMenuManager> m_appMenuManager;
    QScopedPointer<ServerSideDecorationPaletteManager> m_paletteManager;
    struct DBusMenuInfo {
        QString serviceName;
        QString objectPath;
    };
    QHash<QWindow *, DBusMenuInfo> m_dbusMenuInfos;

    KdePlatformTheme *m_platformTheme;
};

#endif
