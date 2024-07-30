/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "kwaylandintegration.h"

#include <QExposeEvent>
#include <QGuiApplication>
#include <QWindow>
#include <qpa/qplatformwindow_p.h>

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
        initialize();
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
        initialize();
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
    if (event->type() == QEvent::ApplicationPaletteChange) {
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
            if (auto waylandWindow = w->nativeInterface<QNativeInterface::Private::QWaylandWindow>()) {
                connect(waylandWindow, &QNativeInterface::Private::QWaylandWindow::surfaceCreated, this, [this, w] {
                    shellSurfaceCreated(w);
                });
                connect(waylandWindow, &QNativeInterface::Private::QWaylandWindow::surfaceDestroyed, this, [this, w] {
                    shellSurfaceDestroyed(w);
                });
                if (waylandWindow->surface()) {
                    shellSurfaceCreated(w);
                }
            }
        }
    }
    return false;
}

void KWaylandIntegration::shellSurfaceCreated(QWindow *w)
{
    if (!isRelevantTopLevel(w)) {
        return;
    }
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

    if (!m_appMenuManager) {
        m_appMenuManager.reset(new AppMenuManager());
    }
    if (m_appMenuManager->isActive()) {
        auto menu = new AppMenu(m_appMenuManager->create(s));
        w->setProperty("org.kde.plasma.integration.appmenu", QVariant::fromValue(menu));
        if (auto it = m_dbusMenuInfos.constFind(w); it != m_dbusMenuInfos.cend()) {
            menu->set_address(it->serviceName, it->objectPath);
        }
    }
}

void KWaylandIntegration::shellSurfaceDestroyed(QWindow *w)
{
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
    m_dbusMenuInfos.insert(window, {serviceName, objectPath});
    connect(window, &QWindow::destroyed, this, [this, window] {
        m_dbusMenuInfos.remove(window);
    });
}

wl_surface *KWaylandIntegration::surfaceFromWindow(QWindow *window)
{
    auto waylandWindow = window->nativeInterface<QNativeInterface::Private::QWaylandWindow>();
    if (!waylandWindow) {
        return nullptr;
    }
    return waylandWindow->surface();
}

#include "kwaylandintegration.moc"

#include "moc_kwaylandintegration.cpp"
