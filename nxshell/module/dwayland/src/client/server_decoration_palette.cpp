/*
    SPDX-FileCopyrightText: 2017 David Edmundson <kde@davidedmundson.co.uk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "server_decoration_palette.h"
#include "event_queue.h"
#include "surface.h"
#include "wayland_pointer_p.h"

#include <wayland-server-decoration-palette-client-protocol.h>

#include <QDebug>

namespace KWayland
{
namespace Client
{
class ServerSideDecorationPaletteManager::Private
{
public:
    Private() = default;

    void setup(org_kde_kwin_server_decoration_palette_manager *arg);

    WaylandPointer<org_kde_kwin_server_decoration_palette_manager, org_kde_kwin_server_decoration_palette_manager_destroy> serverdecomanager;
    EventQueue *queue = nullptr;
};

ServerSideDecorationPaletteManager::ServerSideDecorationPaletteManager(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

void ServerSideDecorationPaletteManager::Private::setup(org_kde_kwin_server_decoration_palette_manager *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!serverdecomanager);
    serverdecomanager.setup(arg);
}

ServerSideDecorationPaletteManager::~ServerSideDecorationPaletteManager()
{
    release();
}

void ServerSideDecorationPaletteManager::setup(org_kde_kwin_server_decoration_palette_manager *serverdecomanager)
{
    d->setup(serverdecomanager);
}

void ServerSideDecorationPaletteManager::release()
{
    d->serverdecomanager.release();
}

void ServerSideDecorationPaletteManager::destroy()
{
    d->serverdecomanager.destroy();
}

ServerSideDecorationPaletteManager::operator org_kde_kwin_server_decoration_palette_manager *()
{
    return d->serverdecomanager;
}

ServerSideDecorationPaletteManager::operator org_kde_kwin_server_decoration_palette_manager *() const
{
    return d->serverdecomanager;
}

bool ServerSideDecorationPaletteManager::isValid() const
{
    return d->serverdecomanager.isValid();
}

void ServerSideDecorationPaletteManager::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *ServerSideDecorationPaletteManager::eventQueue()
{
    return d->queue;
}

ServerSideDecorationPalette *ServerSideDecorationPaletteManager::create(Surface *surface, QObject *parent)
{
    Q_ASSERT(isValid());
    auto p = new ServerSideDecorationPalette(parent);
    auto w = org_kde_kwin_server_decoration_palette_manager_create(d->serverdecomanager, *surface);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    p->setup(w);

    return p;
}

class ServerSideDecorationPalette::Private
{
public:
    void setup(org_kde_kwin_server_decoration_palette *arg);

    WaylandPointer<org_kde_kwin_server_decoration_palette, org_kde_kwin_server_decoration_palette_release> decoration_palette;
};

ServerSideDecorationPalette::ServerSideDecorationPalette(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

void ServerSideDecorationPalette::Private::setup(org_kde_kwin_server_decoration_palette *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!decoration_palette);
    decoration_palette.setup(arg);
}

ServerSideDecorationPalette::~ServerSideDecorationPalette()
{
    release();
}

void ServerSideDecorationPalette::setup(org_kde_kwin_server_decoration_palette *decoration_palette)
{
    d->setup(decoration_palette);
}

void ServerSideDecorationPalette::release()
{
    d->decoration_palette.release();
}

void ServerSideDecorationPalette::destroy()
{
    d->decoration_palette.destroy();
}

ServerSideDecorationPalette::operator org_kde_kwin_server_decoration_palette *()
{
    return d->decoration_palette;
}

ServerSideDecorationPalette::operator org_kde_kwin_server_decoration_palette *() const
{
    return d->decoration_palette;
}

bool ServerSideDecorationPalette::isValid() const
{
    return d->decoration_palette.isValid();
}

void ServerSideDecorationPalette::setPalette(const QString &palette)
{
    Q_ASSERT(isValid());
    org_kde_kwin_server_decoration_palette_set_palette(*this, palette.toUtf8());
}
}
}
