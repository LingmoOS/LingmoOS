/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "kwaylandintegration.h"

#include <QExposeEvent>
#include <QGuiApplication>
#include <QWindow>
#include <qpa/qplatformnativeinterface.h>
#include <qtwaylandclientversion.h>

#include "qdbusmenubar_p.h"
#include "qwayland-appmenu.h"
#include "qwayland-server-decoration-palette.h"

#include <KWindowEffects>

static const QByteArray s_schemePropertyName = QByteArrayLiteral("KDE_COLOR_SCHEME_PATH");
static const QByteArray s_blurBehindPropertyName = QByteArrayLiteral("ENABLE_BLUR_BEHIND_HINT");

class AppMenuManager : public QWaylandClientExtensionTemplate<AppMenuManager>, public QtWayland::org_kde_kwin_appmenu_manager
{
    Q_OBJECT
public:
    AppMenuManager()
        : QWaylandClientExtensionTemplate<AppMenuManager>(1)
    {
        // QWaylandClientExtensionTemplate invokes this with a QueuedConnection
        QMetaObject::invokeMethod(this, "addRegistryListener");
    }
};

class AppMenu : public QtWayland::org_kde_kwin_appmenu
{
public:
    using org_kde_kwin_appmenu::org_kde_kwin_appmenu;
    ~AppMenu()
    {
        release();
    }
};

class ServerSideDecorationPaletteManager : public QWaylandClientExtensionTemplate<ServerSideDecorationPaletteManager>,
                                           public QtWayland::org_kde_kwin_server_decoration_palette_manager
{
    Q_OBJECT
public:
    ServerSideDecorationPaletteManager()
        : QWaylandClientExtensionTemplate<ServerSideDecorationPaletteManager>(1)
    {
        QMetaObject::invokeMethod(this, "addRegistryListener");
    }
    ~ServerSideDecorationPaletteManager() override
    {
        if (isActive()) {
            org_kde_kwin_server_decoration_palette_manager_destroy(object());
        }
    }
};

class ServerSideDecorationPalette : public QtWayland::org_kde_kwin_server_decoration_palette
{
public:
    using org_kde_kwin_server_decoration_palette::org_kde_kwin_server_decoration_palette;
    ~ServerSideDecorationPalette() override
    {
        release();
    }
};

Q_DECLARE_METATYPE(AppMenu *);
Q_DECLARE_METATYPE(ServerSideDecorationPalette *);

KWaylandIntegration::KWaylandIntegration(KdePlatformTheme *platformTheme)
    : QObject()
    , m_platformTheme(platformTheme)
{
    QCoreApplication::instance()->installEventFilter(this);
}

KWaylandIntegration::~KWaylandIntegration() = default;

bool KWaylandIntegration::isRelevantTopLevel(QWindow *w)
{
    if (!w || w->parent()) {
        return false;
    }

    // ignore  windows that map to XdgPopup
    if (w->type() == Qt::ToolTip || w->type() == Qt::Popup) {
        return false;
    }
    return true;
}

bool KWaylandIntegration::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Expose) {
        auto ee = static_cast<QExposeEvent *>(event);
        if (ee->region().isNull()) {
            return false;
        }
        QWindow *w = qobject_cast<QWindow *>(watched);
        if (!isRelevantTopLevel(w)) {
            return false;
        }
        if (!w->isVisible()) {
            return false;
        }

        if (w->property("org.kde.plasma.integration.shellSurfaceCreated").isNull()) {
            shellSurfaceCreated(w);
        }
    } else if (event->type() == QEvent::Hide) {
        QWindow *w = qobject_cast<QWindow *>(watched);
        if (!isRelevantTopLevel(w)) {
            return false;
        }
        shellSurfaceDestroyed(w);
    } else if (event->type() == QEvent::ApplicationPaletteChange) {
        if (watched != QGuiApplication::instance()) {
            return false;
        }
        const auto topLevelWindows = QGuiApplication::topLevelWindows();
        for (QWindow *w : topLevelWindows) {
            if (isRelevantTopLevel(w)) {
                installColorScheme(w);
            }
        }
    } else if (event->type() == QEvent::PlatformSurface) {
        QWindow *w = qobject_cast<QWindow *>(watched);
        QPlatformSurfaceEvent *pe = static_cast<QPlatformSurfaceEvent *>(event);
        if (w && !w->flags().testFlag(Qt::ForeignWindow) && pe->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated) {
            m_platformTheme->windowCreated(w);
        }
    }

    return false;
}

void KWaylandIntegration::shellSurfaceCreated(QWindow *w)
{
    // set colorscheme hint
    if (qApp->property(s_schemePropertyName.constData()).isValid()) {
        installColorScheme(w);
    }
    const auto blurBehindProperty = w->property(s_blurBehindPropertyName.constData());
    if (blurBehindProperty.isValid()) {
        KWindowEffects::enableBlurBehind(w, blurBehindProperty.toBool());
    }
    // create deco
    wl_surface *s = surfaceFromWindow(w);
    if (!s) {
        return;
    }

    w->setProperty("org.kde.plasma.integration.shellSurfaceCreated", true);

#ifndef KF6_TODO_DBUS_MENUBAR
    if (!m_appMenuManager) {
        m_appMenuManager.reset(new AppMenuManager());
    }
    if (m_appMenuManager->isActive()) {
        auto menu = new AppMenu(m_appMenuManager->create(s));
        w->setProperty("org.kde.plasma.integration.appmenu", QVariant::fromValue(menu));
        auto menuBar = QDBusMenuBar::menuBarForWindow(w);
        if (!menuBar) {
            menuBar = QDBusMenuBar::globalMenuBar();
        }
        if (menuBar) {
            menu->set_address(QDBusConnection::sessionBus().baseService(), menuBar->objectPath());
        }
    }
#endif
}

void KWaylandIntegration::shellSurfaceDestroyed(QWindow *w)
{
    w->setProperty("org.kde.plasma.integration.shellSurfaceCreated", QVariant());

    auto appMenu = w->property("org.kde.plasma.integration.appmenu").value<AppMenu *>();
    if (appMenu) {
        delete appMenu;
    }
    w->setProperty("org.kde.plasma.integration.appmenu", QVariant());

    auto decoPallete = w->property("org.kde.plasma.integration.palette").value<ServerSideDecorationPalette *>();
    if (decoPallete) {
        delete decoPallete;
    }
    w->setProperty("org.kde.plasma.integration.palette", QVariant());
}

void KWaylandIntegration::installColorScheme(QWindow *w)
{
    if (!m_paletteManager) {
        m_paletteManager.reset(new ServerSideDecorationPaletteManager());
    }
    if (!m_paletteManager->isActive()) {
        return;
    }
    auto palette = w->property("org.kde.plasma.integration.palette").value<ServerSideDecorationPalette *>();
    if (!palette) {
        auto s = surfaceFromWindow(w);
        if (!s) {
            return;
        }
        palette = new ServerSideDecorationPalette(m_paletteManager->create(s));
        w->setProperty("org.kde.plasma.integration.palette", QVariant::fromValue(palette));
    }
    if (palette) {
        palette->set_palette(qApp->property(s_schemePropertyName.constData()).toString());
    }
}

void KWaylandIntegration::setAppMenu(QWindow *window, const QString &serviceName, const QString &objectPath)
{
    auto menu = window->property("org.kde.plasma.integration.appmenu").value<AppMenu *>();
    if (menu) {
        menu->set_address(serviceName, objectPath);
    }
}

wl_surface *KWaylandIntegration::surfaceFromWindow(QWindow *window)
{
    QPlatformNativeInterface *nativeInterface = qGuiApp->platformNativeInterface();
    if (!nativeInterface) {
        return nullptr;
    }
    return static_cast<wl_surface *>(nativeInterface->nativeResourceForWindow("surface", window));
}

#include "kwaylandintegration.moc"
