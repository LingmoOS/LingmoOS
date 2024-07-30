/*
    SPDX-FileCopyrightText: 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
    SPDX-FileCopyrightText: 2023 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "windowshadow.h"
#include "logging.h"
#include "shm.h"
#include "surfacehelper.h"
#include "waylandintegration.h"

#include <qwayland-shadow.h>

#include <QDebug>
#include <QExposeEvent>
#include <QWaylandClientExtension>

#include <private/qwaylandwindow_p.h>

class ShadowManager : public QWaylandClientExtensionTemplate<ShadowManager>, public QtWayland::org_kde_kwin_shadow_manager
{
    Q_OBJECT
    static constexpr int version = 2;
    explicit ShadowManager(QObject *parent = nullptr)
        : QWaylandClientExtensionTemplate(version)
    {
        setParent(parent);
        QMetaObject::invokeMethod(this, "addRegistryListener");

        connect(this, &QWaylandClientExtension::activeChanged, this, [this] {
            if (!isActive()) {
                destroy();
            }
        });
    }

public:
    ~ShadowManager()
    {
        if (isActive()) {
            destroy();
        }
    }
    static ShadowManager *instance()
    {
        static ShadowManager *instance = new ShadowManager(qGuiApp);
        return instance;
    }
};

class Shadow : public QtWayland::org_kde_kwin_shadow
{
public:
    using QtWayland::org_kde_kwin_shadow::org_kde_kwin_shadow;
    ~Shadow()
    {
        destroy();
    }
};

WindowShadowTile::WindowShadowTile()
{
    connect(Shm::instance(), &Shm::activeChanged, this, [this] {
        if (Shm::instance()->isActive()) {
            buffer.reset();
        }
    });
}
WindowShadowTile::~WindowShadowTile()
{
}

bool WindowShadowTile::create()
{
    if (!Shm::instance()->isActive()) {
        return false;
    }

    buffer = Shm::instance()->createBuffer(image);
    return true;
}

void WindowShadowTile::destroy()
{
    buffer.reset();
}

WindowShadowTile *WindowShadowTile::get(const KWindowShadowTile *tile)
{
    KWindowShadowTilePrivate *d = KWindowShadowTilePrivate::get(tile);
    return static_cast<WindowShadowTile *>(d);
}

static wl_buffer *bufferForTile(const KWindowShadowTile::Ptr &tile)
{
    if (!tile) {
        return nullptr;
    }
    WindowShadowTile *d = WindowShadowTile::get(tile.data());
    // Our buffer has been deleted in the meantime, try to create it again
    if (!d->buffer && d->isCreated) {
        d->buffer = Shm::instance()->createBuffer(d->image);
    }
    return d->buffer ? d->buffer.get()->object() : nullptr;
}

WindowShadow::WindowShadow()
{
}
WindowShadow::~WindowShadow()
{
}

bool WindowShadow::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)
    if (event->type() == QEvent::Expose) {
        QExposeEvent *exposeEvent = static_cast<QExposeEvent *>(event);
        if (!exposeEvent->region().isNull()) {
            if (!internalCreate()) {
                qCWarning(KWAYLAND_KWS) << "Failed to recreate shadow for" << window;
            }
        }
    }
    return false;
}

bool WindowShadow::internalCreate()
{
    if (shadow) {
        return true;
    }
    if (!ShadowManager::instance()->isActive()) {
        return false;
    }
    auto surface = surfaceForWindow(window);
    if (!surface) {
        return false;
    }

    shadow = std::make_unique<Shadow>(ShadowManager::instance()->create(surface));
    auto waylandWindow = dynamic_cast<QtWaylandClient::QWaylandWindow *>(window->handle());
    if (waylandWindow) {
        connect(waylandWindow, &QtWaylandClient::QWaylandWindow::wlSurfaceDestroyed, this, &WindowShadow::internalDestroy, Qt::UniqueConnection);
    }

    auto attach = [](const std::unique_ptr<Shadow> &shadow, auto attach_func, const KWindowShadowTile::Ptr &tile) {
        if (auto buffer = bufferForTile(tile)) {
            (*shadow.*attach_func)(buffer);
        }
    };
    attach(shadow, &Shadow::attach_left, leftTile);
    attach(shadow, &Shadow::attach_top_left, topLeftTile);
    attach(shadow, &Shadow::attach_top, topTile);
    attach(shadow, &Shadow::attach_top_right, topRightTile);
    attach(shadow, &Shadow::attach_right, rightTile);
    attach(shadow, &Shadow::attach_bottom_right, bottomRightTile);
    attach(shadow, &Shadow::attach_bottom, bottomTile);
    attach(shadow, &Shadow::attach_bottom_left, bottomLeftTile);

    shadow->set_left_offset(wl_fixed_from_double(padding.left()));
    shadow->set_top_offset(wl_fixed_from_double(padding.top()));
    shadow->set_right_offset(wl_fixed_from_double(padding.right()));
    shadow->set_bottom_offset(wl_fixed_from_double(padding.bottom()));

    shadow->commit();

    // Commit wl_surface at the next available time.
    window->requestUpdate();

    return true;
}

bool WindowShadow::create()
{
    if (!ShadowManager::instance()->isActive()) {
        return false;
    }

    internalCreate();
    window->installEventFilter(this);
    return true;
}

void WindowShadow::internalDestroy()
{
    if (!shadow) {
        return;
    }

    if (ShadowManager::instance()->isActive()) {
        if (auto surface = surfaceForWindow(window)) {
            ShadowManager::instance()->unset(surface);
        }
    }

    shadow.reset();

    if (window) {
        window->requestUpdate();
    }
}

void WindowShadow::destroy()
{
    if (window) {
        window->removeEventFilter(this);
    }
    internalDestroy();
}

#include "windowshadow.moc"
