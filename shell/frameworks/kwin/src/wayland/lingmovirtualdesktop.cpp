/*
    SPDX-FileCopyrightText: 2018 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "lingmovirtualdesktop.h"
#include "display.h"
#include "wayland/quirks.h"

#include <QDebug>
#include <QTimer>

#include <qwayland-server-org-kde-lingmo-virtual-desktop.h>
#include <wayland-server.h>

namespace KWin
{
static const quint32 s_version = 2;

class LingmoVirtualDesktopInterfacePrivate : public QtWaylandServer::org_kde_lingmo_virtual_desktop
{
public:
    LingmoVirtualDesktopInterfacePrivate(LingmoVirtualDesktopInterface *q);
    ~LingmoVirtualDesktopInterfacePrivate();

    LingmoVirtualDesktopInterface *q;
    QString id;
    QString name;
    bool active = false;

protected:
    void org_kde_lingmo_virtual_desktop_bind_resource(Resource *resource) override;
    void org_kde_lingmo_virtual_desktop_request_activate(Resource *resource) override;
};

class LingmoVirtualDesktopManagementInterfacePrivate : public QtWaylandServer::org_kde_lingmo_virtual_desktop_management
{
public:
    LingmoVirtualDesktopManagementInterfacePrivate(LingmoVirtualDesktopManagementInterface *_q, Display *display);

    QList<LingmoVirtualDesktopInterface *> desktops;
    quint32 rows = 0;
    quint32 columns = 0;
    LingmoVirtualDesktopManagementInterface *q;

    inline QList<LingmoVirtualDesktopInterface *>::const_iterator constFindDesktop(const QString &id);
    inline QList<LingmoVirtualDesktopInterface *>::iterator findDesktop(const QString &id);

protected:
    void org_kde_lingmo_virtual_desktop_management_get_virtual_desktop(Resource *resource, uint32_t id, const QString &desktop_id) override;
    void org_kde_lingmo_virtual_desktop_management_request_create_virtual_desktop(Resource *resource, const QString &name, uint32_t position) override;
    void org_kde_lingmo_virtual_desktop_management_request_remove_virtual_desktop(Resource *resource, const QString &desktop_id) override;
    void org_kde_lingmo_virtual_desktop_management_bind_resource(Resource *resource) override;
};

inline QList<LingmoVirtualDesktopInterface *>::const_iterator LingmoVirtualDesktopManagementInterfacePrivate::constFindDesktop(const QString &id)
{
    return std::find_if(desktops.constBegin(), desktops.constEnd(), [id](const LingmoVirtualDesktopInterface *desk) {
        return desk->id() == id;
    });
}

inline QList<LingmoVirtualDesktopInterface *>::iterator LingmoVirtualDesktopManagementInterfacePrivate::findDesktop(const QString &id)
{
    return std::find_if(desktops.begin(), desktops.end(), [id](const LingmoVirtualDesktopInterface *desk) {
        return desk->id() == id;
    });
}

void LingmoVirtualDesktopManagementInterfacePrivate::org_kde_lingmo_virtual_desktop_management_get_virtual_desktop(Resource *resource,
                                                                                                                   uint32_t id,
                                                                                                                   const QString &desktop_id)
{
    auto i = constFindDesktop(desktop_id);
    if (i == desktops.constEnd()) {
        return;
    }

    (*i)->d->add(resource->client(), id, resource->version());
}

void LingmoVirtualDesktopManagementInterfacePrivate::org_kde_lingmo_virtual_desktop_management_request_create_virtual_desktop(Resource *resource,
                                                                                                                              const QString &name,
                                                                                                                              uint32_t position)
{
    Q_EMIT q->desktopCreateRequested(name, std::clamp<quint32>(position, 0, desktops.size()));
}

void LingmoVirtualDesktopManagementInterfacePrivate::org_kde_lingmo_virtual_desktop_management_request_remove_virtual_desktop(Resource *resource,
                                                                                                                              const QString &desktop_id)
{
    Q_EMIT q->desktopRemoveRequested(desktop_id);
}

LingmoVirtualDesktopManagementInterfacePrivate::LingmoVirtualDesktopManagementInterfacePrivate(LingmoVirtualDesktopManagementInterface *_q, Display *display)
    : QtWaylandServer::org_kde_lingmo_virtual_desktop_management(*display, s_version)
    , q(_q)
{
}

void LingmoVirtualDesktopManagementInterfacePrivate::org_kde_lingmo_virtual_desktop_management_bind_resource(Resource *resource)
{
    quint32 i = 0;
    for (auto it = desktops.constBegin(); it != desktops.constEnd(); ++it) {
        send_desktop_created(resource->handle, (*it)->id(), i++);
    }

    if (resource->version() >= ORG_KDE_LINGMO_VIRTUAL_DESKTOP_MANAGEMENT_ROWS_SINCE_VERSION) {
        send_rows(resource->handle, rows);
    }

    send_done(resource->handle);
}

LingmoVirtualDesktopManagementInterface::LingmoVirtualDesktopManagementInterface(Display *display, QObject *parent)
    : QObject(parent)
    , d(new LingmoVirtualDesktopManagementInterfacePrivate(this, display))
{
}

LingmoVirtualDesktopManagementInterface::~LingmoVirtualDesktopManagementInterface()
{
    while (!d->desktops.isEmpty()) {
        const QString id = d->desktops[0]->id();
        removeDesktop(id);
    }
}

void LingmoVirtualDesktopManagementInterface::setRows(quint32 rows)
{
    if (rows == 0 || d->rows == rows) {
        return;
    }

    d->rows = rows;

    const auto clientResources = d->resourceMap();
    for (auto resource : clientResources) {
        if (resource->version() < ORG_KDE_LINGMO_VIRTUAL_DESKTOP_MANAGEMENT_ROWS_SINCE_VERSION) {
            continue;
        }
        d->send_rows(resource->handle, rows);
    }
}

LingmoVirtualDesktopInterface *LingmoVirtualDesktopManagementInterface::desktop(const QString &id)
{
    auto i = d->constFindDesktop(id);
    if (i != d->desktops.constEnd()) {
        return *i;
    }
    return nullptr;
}

LingmoVirtualDesktopInterface *LingmoVirtualDesktopManagementInterface::createDesktop(const QString &id, quint32 position)
{
    auto i = d->constFindDesktop(id);
    if (i != d->desktops.constEnd()) {
        return *i;
    }

    const quint32 actualPosition = std::min(position, (quint32)d->desktops.count());

    auto desktop = new LingmoVirtualDesktopInterface();
    desktop->d->id = id;

    const auto desktopClientResources = desktop->d->resourceMap();
    for (auto resource : desktopClientResources) {
        desktop->d->send_desktop_id(resource->handle, id);
    }

    // activate the first desktop TODO: to be done here?
    if (d->desktops.isEmpty()) {
        desktop->d->active = true;
    }

    d->desktops.insert(actualPosition, desktop);

    const auto clientResources = d->resourceMap();
    for (auto resource : clientResources) {
        d->send_desktop_created(resource->handle, id, actualPosition);
    }

    return desktop;
}

void LingmoVirtualDesktopManagementInterface::removeDesktop(const QString &id)
{
    auto deskIt = d->findDesktop(id);
    if (deskIt == d->desktops.end()) {
        return;
    }

    LingmoVirtualDesktopInterface *desktop = *deskIt;
    d->desktops.erase(deskIt);
    delete desktop;

    const auto clientResources = d->resourceMap();
    for (auto resource : clientResources) {
        d->send_desktop_removed(resource->handle, id);
    }
}

QList<LingmoVirtualDesktopInterface *> LingmoVirtualDesktopManagementInterface::desktops() const
{
    return d->desktops;
}

void LingmoVirtualDesktopManagementInterface::sendDone()
{
    const auto clientResources = d->resourceMap();
    for (auto resource : clientResources) {
        d->send_done(resource->handle);
    }
}

//// LingmoVirtualDesktopInterface

void LingmoVirtualDesktopInterfacePrivate::org_kde_lingmo_virtual_desktop_request_activate(Resource *resource)
{
    Q_EMIT q->activateRequested();
}

LingmoVirtualDesktopInterfacePrivate::LingmoVirtualDesktopInterfacePrivate(LingmoVirtualDesktopInterface *q)
    : QtWaylandServer::org_kde_lingmo_virtual_desktop()
    , q(q)
{
}

LingmoVirtualDesktopInterfacePrivate::~LingmoVirtualDesktopInterfacePrivate()
{
    const auto clientResources = resourceMap();
    for (Resource *resource : clientResources) {
        send_removed(resource->handle);
        wl_resource_destroy(resource->handle);
    }
}

void LingmoVirtualDesktopInterfacePrivate::org_kde_lingmo_virtual_desktop_bind_resource(Resource *resource)
{
    send_desktop_id(resource->handle, id);

    if (!name.isEmpty()) {
        send_name(resource->handle, truncate(name));
    }

    if (active) {
        send_activated(resource->handle);
    }
}

LingmoVirtualDesktopInterface::LingmoVirtualDesktopInterface()
    : d(new LingmoVirtualDesktopInterfacePrivate(this))
{
}

LingmoVirtualDesktopInterface::~LingmoVirtualDesktopInterface()
{
}

QString LingmoVirtualDesktopInterface::id() const
{
    return d->id;
}

void LingmoVirtualDesktopInterface::setName(const QString &name)
{
    if (d->name == name) {
        return;
    }

    d->name = name;

    const auto clientResources = d->resourceMap();
    for (auto resource : clientResources) {
        d->send_name(resource->handle, truncate(name));
    }
}

QString LingmoVirtualDesktopInterface::name() const
{
    return d->name;
}

void LingmoVirtualDesktopInterface::setActive(bool active)
{
    if (d->active == active) {
        return;
    }

    d->active = active;
    const auto clientResources = d->resourceMap();

    if (active) {
        for (auto resource : clientResources) {
            d->send_activated(resource->handle);
        }
    } else {
        for (auto resource : clientResources) {
            d->send_deactivated(resource->handle);
        }
    }
}

bool LingmoVirtualDesktopInterface::isActive() const
{
    return d->active;
}

void LingmoVirtualDesktopInterface::sendDone()
{
    const auto clientResources = d->resourceMap();
    for (auto resource : clientResources) {
        d->send_done(resource->handle);
    }
}

}

#include "moc_lingmovirtualdesktop.cpp"
