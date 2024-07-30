/*
    SPDX-FileCopyrightText: 2017 David Edmundson <kde@davidedmundson.co.uk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "appmenu.h"
#include "event_queue.h"
#include "surface.h"
#include "wayland_pointer_p.h"

#include <wayland-appmenu-client-protocol.h>

namespace KWayland
{
namespace Client
{
class AppMenuManager::Private
{
public:
    Private() = default;

    void setup(org_kde_kwin_appmenu_manager *arg);

    WaylandPointer<org_kde_kwin_appmenu_manager, org_kde_kwin_appmenu_manager_destroy> appmenumanager;
    EventQueue *queue = nullptr;
};

AppMenuManager::AppMenuManager(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

void AppMenuManager::Private::setup(org_kde_kwin_appmenu_manager *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!appmenumanager);
    appmenumanager.setup(arg);
}

AppMenuManager::~AppMenuManager()
{
    release();
}

void AppMenuManager::setup(org_kde_kwin_appmenu_manager *appmenumanager)
{
    d->setup(appmenumanager);
}

void AppMenuManager::release()
{
    d->appmenumanager.release();
}

void AppMenuManager::destroy()
{
    d->appmenumanager.destroy();
}

AppMenuManager::operator org_kde_kwin_appmenu_manager *()
{
    return d->appmenumanager;
}

AppMenuManager::operator org_kde_kwin_appmenu_manager *() const
{
    return d->appmenumanager;
}

bool AppMenuManager::isValid() const
{
    return d->appmenumanager.isValid();
}

void AppMenuManager::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *AppMenuManager::eventQueue()
{
    return d->queue;
}

AppMenu *AppMenuManager::create(Surface *surface, QObject *parent)
{
    Q_ASSERT(isValid());
    auto p = new AppMenu(parent);
    auto w = org_kde_kwin_appmenu_manager_create(d->appmenumanager, *surface);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    p->setup(w);
    return p;
}

class AppMenu::Private
{
public:
    void setup(org_kde_kwin_appmenu *arg);

    WaylandPointer<org_kde_kwin_appmenu, org_kde_kwin_appmenu_release> appmenu;
};

AppMenu::AppMenu(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

void AppMenu::Private::setup(org_kde_kwin_appmenu *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!appmenu);
    appmenu.setup(arg);
}

AppMenu::~AppMenu()
{
    release();
}

void AppMenu::setup(org_kde_kwin_appmenu *appmenu)
{
    d->setup(appmenu);
}

void AppMenu::release()
{
    d->appmenu.release();
}

void AppMenu::destroy()
{
    d->appmenu.destroy();
}

AppMenu::operator org_kde_kwin_appmenu *()
{
    return d->appmenu;
}

AppMenu::operator org_kde_kwin_appmenu *() const
{
    return d->appmenu;
}

bool AppMenu::isValid() const
{
    return d->appmenu.isValid();
}

void AppMenu::setAddress(const QString &serviceName, const QString &objectPath)
{
    Q_ASSERT(isValid());
    org_kde_kwin_appmenu_set_address(d->appmenu, serviceName.toLatin1(), objectPath.toLatin1());
}

}
}

#include "moc_appmenu.cpp"
