// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "globalproperty.h"
#include "event_queue.h"
#include "surface.h"
#include "wayland_pointer_p.h"
// Qt

// wayland
#include "wayland-dde-globalproperty-client-protocol.h"
#include "wayland-client-protocol.h"

namespace KWayland
{

namespace Client
{

class Q_DECL_HIDDEN GlobalProperty::Private
{
public:
    Private(GlobalProperty *q);
    void setup(dde_globalproperty *o);

    WaylandPointer<dde_globalproperty, dde_globalproperty_destroy> ddeglobalproperty;
    EventQueue *queue = nullptr;

private:
    GlobalProperty *q;
};

GlobalProperty::Private::Private(GlobalProperty *q)
    : q(q)
{

}

void GlobalProperty::Private::setup(dde_globalproperty *o)
{
    Q_ASSERT(o);
    Q_ASSERT(!ddeglobalproperty);
    ddeglobalproperty.setup(o);
}

GlobalProperty::GlobalProperty(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{

}

GlobalProperty::~GlobalProperty()
{

}

void GlobalProperty::setup(dde_globalproperty *ddeglobalproperty)
{
    d->setup(ddeglobalproperty);
}

bool GlobalProperty::isValid() const
{
    return d->ddeglobalproperty.isValid();
}

GlobalProperty::operator dde_globalproperty*() {
    return d->ddeglobalproperty;
}

GlobalProperty::operator dde_globalproperty*() const {
    return d->ddeglobalproperty;
}

EventQueue *GlobalProperty::eventQueue() const
{
    return d->queue;
}

void GlobalProperty::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

void GlobalProperty::setProperty(const QString &module, const QString &function, wl_surface *surface, const int32_t &type, const QString &data)
{
    dde_globalproperty_set_property(d->ddeglobalproperty, module.toStdString().c_str(), function.toStdString().c_str(), surface, type, data.toStdString().c_str());
}

void GlobalProperty::setProperty(const QString &module, const QString &function, Surface *surface, const int32_t &type, const QString &data)
{
    setProperty(module, function, *surface, type, data);
}

QString GlobalProperty::getProperty(const QString &module, const QString &function)
{
    return QString();
}

void GlobalProperty::destroy()
{
    if (!d->ddeglobalproperty) {
        return;
    }
    emit interfaceAboutToBeDestroyed();
    d->ddeglobalproperty.destroy();
}

}

}
