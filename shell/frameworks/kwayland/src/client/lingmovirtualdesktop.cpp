/*
    SPDX-FileCopyrightText: 2018 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "lingmovirtualdesktop.h"
#include "event_queue.h"
#include "wayland_pointer_p.h"

#include <QDebug>
#include <QMap>

#include <wayland-lingmo-virtual-desktop-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN LingmoVirtualDesktopManagement::Private
{
public:
    Private(LingmoVirtualDesktopManagement *q);

    void setup(org_kde_lingmo_virtual_desktop_management *arg);

    WaylandPointer<org_kde_lingmo_virtual_desktop_management, org_kde_lingmo_virtual_desktop_management_destroy> lingmovirtualdesktopmanagement;
    EventQueue *queue = nullptr;

    quint32 rows = 1;
    QList<LingmoVirtualDesktop *> desktops;

    inline QList<LingmoVirtualDesktop *>::const_iterator constFindDesktop(const QString &id);
    inline QList<LingmoVirtualDesktop *>::iterator findDesktop(const QString &id);

private:
    static void
    createdCallback(void *data, org_kde_lingmo_virtual_desktop_management *org_kde_lingmo_virtual_desktop_management, const char *id, uint32_t position);
    static void removedCallback(void *data, org_kde_lingmo_virtual_desktop_management *org_kde_lingmo_virtual_desktop_management, const char *id);
    static void rowsCallback(void *data, org_kde_lingmo_virtual_desktop_management *org_kde_lingmo_virtual_desktop_management, uint32_t rows);
    static void doneCallback(void *data, org_kde_lingmo_virtual_desktop_management *org_kde_lingmo_virtual_desktop_management);

    LingmoVirtualDesktopManagement *q;

    static const org_kde_lingmo_virtual_desktop_management_listener s_listener;
};

class Q_DECL_HIDDEN LingmoVirtualDesktop::Private
{
public:
    Private(LingmoVirtualDesktop *q);

    void setup(org_kde_lingmo_virtual_desktop *arg);

    WaylandPointer<org_kde_lingmo_virtual_desktop, org_kde_lingmo_virtual_desktop_destroy> lingmovirtualdesktop;

    QString id;
    QString name;
    bool active = false;

private:
    LingmoVirtualDesktop *q;

private:
    static void idCallback(void *data, org_kde_lingmo_virtual_desktop *org_kde_lingmo_virtual_desktop, const char *id);
    static void nameCallback(void *data, org_kde_lingmo_virtual_desktop *org_kde_lingmo_virtual_desktop, const char *name);

    static void activatedCallback(void *data, org_kde_lingmo_virtual_desktop *org_kde_lingmo_virtual_desktop);
    static void deactivatedCallback(void *data, org_kde_lingmo_virtual_desktop *org_kde_lingmo_virtual_desktop);
    static void doneCallback(void *data, org_kde_lingmo_virtual_desktop *org_kde_lingmo_virtual_desktop);
    static void removedCallback(void *data, org_kde_lingmo_virtual_desktop *org_kde_lingmo_virtual_desktop);

    static const org_kde_lingmo_virtual_desktop_listener s_listener;
};

inline QList<LingmoVirtualDesktop *>::const_iterator LingmoVirtualDesktopManagement::Private::constFindDesktop(const QString &id)
{
    return std::find_if(desktops.constBegin(), desktops.constEnd(), [id](const LingmoVirtualDesktop *desk) {
        return desk->id() == id;
    });
}

inline QList<LingmoVirtualDesktop *>::iterator LingmoVirtualDesktopManagement::Private::findDesktop(const QString &id)
{
    return std::find_if(desktops.begin(), desktops.end(), [id](const LingmoVirtualDesktop *desk) {
        return desk->id() == id;
    });
}

const org_kde_lingmo_virtual_desktop_management_listener LingmoVirtualDesktopManagement::Private::s_listener = {createdCallback,
                                                                                                                removedCallback,
                                                                                                                doneCallback,
                                                                                                                rowsCallback};

void LingmoVirtualDesktopManagement::Private::createdCallback(void *data,
                                                              org_kde_lingmo_virtual_desktop_management *org_kde_lingmo_virtual_desktop_management,
                                                              const char *id,
                                                              uint32_t position)
{
    auto p = reinterpret_cast<LingmoVirtualDesktopManagement::Private *>(data);
    Q_ASSERT(p->lingmovirtualdesktopmanagement == org_kde_lingmo_virtual_desktop_management);
    const QString stringId = QString::fromUtf8(id);
    LingmoVirtualDesktop *vd = p->q->getVirtualDesktop(stringId);
    Q_ASSERT(vd);

    p->desktops.insert(position, vd);
    // TODO: emit a lot of desktopMoved?

    Q_EMIT p->q->desktopCreated(stringId, position);
}

void LingmoVirtualDesktopManagement::Private::removedCallback(void *data,
                                                              org_kde_lingmo_virtual_desktop_management *org_kde_lingmo_virtual_desktop_management,
                                                              const char *id)
{
    auto p = reinterpret_cast<LingmoVirtualDesktopManagement::Private *>(data);
    Q_ASSERT(p->lingmovirtualdesktopmanagement == org_kde_lingmo_virtual_desktop_management);
    const QString stringId = QString::fromUtf8(id);
    LingmoVirtualDesktop *vd = p->q->getVirtualDesktop(stringId);
    // TODO: emit a lot of desktopMoved?
    Q_ASSERT(vd);
    auto i = p->findDesktop(stringId);
    p->desktops.erase(i);
    vd->release();
    vd->destroy();
    vd->deleteLater();
    Q_EMIT p->q->desktopRemoved(stringId);
}

void LingmoVirtualDesktopManagement::Private::rowsCallback(void *data,
                                                           org_kde_lingmo_virtual_desktop_management *org_kde_lingmo_virtual_desktop_management,
                                                           uint32_t rows)
{
    auto p = reinterpret_cast<LingmoVirtualDesktopManagement::Private *>(data);
    Q_ASSERT(p->lingmovirtualdesktopmanagement == org_kde_lingmo_virtual_desktop_management);
    if (rows == 0) {
        return;
    }
    p->rows = rows;
    Q_EMIT p->q->rowsChanged(rows);
}

void LingmoVirtualDesktopManagement::Private::doneCallback(void *data, org_kde_lingmo_virtual_desktop_management *org_kde_lingmo_virtual_desktop_management)
{
    auto p = reinterpret_cast<LingmoVirtualDesktopManagement::Private *>(data);
    Q_ASSERT(p->lingmovirtualdesktopmanagement == org_kde_lingmo_virtual_desktop_management);
    Q_EMIT p->q->done();
}

LingmoVirtualDesktopManagement::LingmoVirtualDesktopManagement(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

LingmoVirtualDesktopManagement::Private::Private(LingmoVirtualDesktopManagement *q)
    : q(q)
{
}

void LingmoVirtualDesktopManagement::Private::setup(org_kde_lingmo_virtual_desktop_management *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!lingmovirtualdesktopmanagement);
    lingmovirtualdesktopmanagement.setup(arg);
    org_kde_lingmo_virtual_desktop_management_add_listener(lingmovirtualdesktopmanagement, &s_listener, this);
}

LingmoVirtualDesktopManagement::~LingmoVirtualDesktopManagement()
{
    release();
}

void LingmoVirtualDesktopManagement::setup(org_kde_lingmo_virtual_desktop_management *lingmovirtualdesktopmanagement)
{
    d->setup(lingmovirtualdesktopmanagement);
}

void LingmoVirtualDesktopManagement::release()
{
    d->lingmovirtualdesktopmanagement.release();
}

void LingmoVirtualDesktopManagement::destroy()
{
    d->lingmovirtualdesktopmanagement.destroy();
}

LingmoVirtualDesktopManagement::operator org_kde_lingmo_virtual_desktop_management *()
{
    return d->lingmovirtualdesktopmanagement;
}

LingmoVirtualDesktopManagement::operator org_kde_lingmo_virtual_desktop_management *() const
{
    return d->lingmovirtualdesktopmanagement;
}

bool LingmoVirtualDesktopManagement::isValid() const
{
    return d->lingmovirtualdesktopmanagement.isValid();
}

void LingmoVirtualDesktopManagement::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *LingmoVirtualDesktopManagement::eventQueue()
{
    return d->queue;
}

LingmoVirtualDesktop *LingmoVirtualDesktopManagement::getVirtualDesktop(const QString &id)
{
    Q_ASSERT(isValid());

    if (id.isEmpty()) {
        return nullptr;
    }

    auto i = d->constFindDesktop(id);
    if (i != d->desktops.constEnd()) {
        return *i;
    }

    auto w = org_kde_lingmo_virtual_desktop_management_get_virtual_desktop(d->lingmovirtualdesktopmanagement, id.toUtf8());

    if (!w) {
        return nullptr;
    }

    if (d->queue) {
        d->queue->addProxy(w);
    }

    auto desktop = new LingmoVirtualDesktop(this);
    desktop->setup(w);
    desktop->d->id = id;

    return desktop;
}

void LingmoVirtualDesktopManagement::requestRemoveVirtualDesktop(const QString &id)
{
    Q_ASSERT(isValid());

    org_kde_lingmo_virtual_desktop_management_request_remove_virtual_desktop(d->lingmovirtualdesktopmanagement, id.toUtf8());
}

void LingmoVirtualDesktopManagement::requestCreateVirtualDesktop(const QString &name, quint32 position)
{
    Q_ASSERT(isValid());

    org_kde_lingmo_virtual_desktop_management_request_create_virtual_desktop(d->lingmovirtualdesktopmanagement, name.toUtf8(), position);
}

QList<LingmoVirtualDesktop *> LingmoVirtualDesktopManagement::desktops() const
{
    return d->desktops;
}

quint32 LingmoVirtualDesktopManagement::rows() const
{
    return d->rows;
}

const org_kde_lingmo_virtual_desktop_listener LingmoVirtualDesktop::Private::s_listener =
    {idCallback, nameCallback, activatedCallback, deactivatedCallback, doneCallback, removedCallback};

void LingmoVirtualDesktop::Private::idCallback(void *data, org_kde_lingmo_virtual_desktop *org_kde_lingmo_virtual_desktop, const char *id)
{
    auto p = reinterpret_cast<LingmoVirtualDesktop::Private *>(data);
    Q_ASSERT(p->lingmovirtualdesktop == org_kde_lingmo_virtual_desktop);
    p->id = QString::fromUtf8(id);
}

void LingmoVirtualDesktop::Private::nameCallback(void *data, org_kde_lingmo_virtual_desktop *org_kde_lingmo_virtual_desktop, const char *name)
{
    auto p = reinterpret_cast<LingmoVirtualDesktop::Private *>(data);
    Q_ASSERT(p->lingmovirtualdesktop == org_kde_lingmo_virtual_desktop);
    p->name = QString::fromUtf8(name);
}

void LingmoVirtualDesktop::Private::activatedCallback(void *data, org_kde_lingmo_virtual_desktop *org_kde_lingmo_virtual_desktop)
{
    auto p = reinterpret_cast<LingmoVirtualDesktop::Private *>(data);
    Q_ASSERT(p->lingmovirtualdesktop == org_kde_lingmo_virtual_desktop);
    p->active = true;
    Q_EMIT p->q->activated();
}

void LingmoVirtualDesktop::Private::deactivatedCallback(void *data, org_kde_lingmo_virtual_desktop *org_kde_lingmo_virtual_desktop)
{
    auto p = reinterpret_cast<LingmoVirtualDesktop::Private *>(data);
    Q_ASSERT(p->lingmovirtualdesktop == org_kde_lingmo_virtual_desktop);
    p->active = false;
    Q_EMIT p->q->deactivated();
}

void LingmoVirtualDesktop::Private::doneCallback(void *data, org_kde_lingmo_virtual_desktop *org_kde_lingmo_virtual_desktop)
{
    auto p = reinterpret_cast<LingmoVirtualDesktop::Private *>(data);
    Q_ASSERT(p->lingmovirtualdesktop == org_kde_lingmo_virtual_desktop);
    Q_EMIT p->q->done();
}

void LingmoVirtualDesktop::Private::removedCallback(void *data, org_kde_lingmo_virtual_desktop *org_kde_lingmo_virtual_desktop)
{
    auto p = reinterpret_cast<LingmoVirtualDesktop::Private *>(data);
    Q_ASSERT(p->lingmovirtualdesktop == org_kde_lingmo_virtual_desktop);
    Q_EMIT p->q->removed();
}

LingmoVirtualDesktop::Private::Private(LingmoVirtualDesktop *q)
    : q(q)
{
}

LingmoVirtualDesktop::LingmoVirtualDesktop(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

void LingmoVirtualDesktop::Private::setup(org_kde_lingmo_virtual_desktop *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!lingmovirtualdesktop);
    lingmovirtualdesktop.setup(arg);
    org_kde_lingmo_virtual_desktop_add_listener(lingmovirtualdesktop, &s_listener, this);
}

LingmoVirtualDesktop::~LingmoVirtualDesktop()
{
    release();
}

void LingmoVirtualDesktop::setup(org_kde_lingmo_virtual_desktop *lingmovirtualdesktop)
{
    d->setup(lingmovirtualdesktop);
}

void LingmoVirtualDesktop::release()
{
    d->lingmovirtualdesktop.release();
}

void LingmoVirtualDesktop::destroy()
{
    d->lingmovirtualdesktop.destroy();
}

LingmoVirtualDesktop::operator org_kde_lingmo_virtual_desktop *()
{
    return d->lingmovirtualdesktop;
}

LingmoVirtualDesktop::operator org_kde_lingmo_virtual_desktop *() const
{
    return d->lingmovirtualdesktop;
}

bool LingmoVirtualDesktop::isValid() const
{
    return d->lingmovirtualdesktop.isValid();
}

void LingmoVirtualDesktop::requestActivate()
{
    Q_ASSERT(isValid());
    org_kde_lingmo_virtual_desktop_request_activate(d->lingmovirtualdesktop);
}

QString LingmoVirtualDesktop::id() const
{
    return d->id;
}

QString LingmoVirtualDesktop::name() const
{
    return d->name;
}

bool LingmoVirtualDesktop::isActive() const
{
    return d->active;
}

}
}

#include "moc_lingmovirtualdesktop.cpp"
