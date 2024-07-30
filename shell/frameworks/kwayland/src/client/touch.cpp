/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "touch.h"
#include "surface.h"
#include "wayland_pointer_p.h"
// Qt
#include <QList>
#include <QPointF>
#include <QPointer>
// wayland
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN Touch::Private
{
public:
    Private(Touch *q);
    void setup(wl_touch *t);
    WaylandPointer<wl_touch, wl_touch_release> touch;
    bool active = false;
    QList<TouchPoint *> sequence;
    TouchPoint *getActivePoint(qint32 id) const;

private:
    static void downCallback(void *data, wl_touch *touch, uint32_t serial, uint32_t time, wl_surface *surface, int32_t id, wl_fixed_t x, wl_fixed_t y);
    static void upCallback(void *data, wl_touch *touch, uint32_t serial, uint32_t time, int32_t id);
    static void motionCallback(void *data, wl_touch *touch, uint32_t time, int32_t id, wl_fixed_t x, wl_fixed_t y);
    static void frameCallback(void *data, wl_touch *touch);
    static void cancelCallback(void *data, wl_touch *touch);
    void down(quint32 serial, quint32 time, qint32 id, const QPointF &position, const QPointer<Surface> &surface);
    void up(quint32 serial, quint32 time, qint32 id);
    void motion(quint32 time, qint32 id, const QPointF &position);

    Touch *q;
    static const wl_touch_listener s_listener;
};

class TouchPoint::Private
{
public:
    qint32 id = 0;
    quint32 downSerial = 0;
    quint32 upSerial = 0;
    QPointer<Surface> surface;
    QList<QPointF> positions;
    QList<quint32> timestamps;
    bool down = true;
};

TouchPoint::TouchPoint()
    : d(new Private)
{
}

TouchPoint::~TouchPoint() = default;

QPointF TouchPoint::position() const
{
    if (d->positions.isEmpty()) {
        return QPointF();
    }
    return d->positions.last();
}

QList<QPointF> TouchPoint::positions() const
{
    return d->positions;
}

quint32 TouchPoint::downSerial() const
{
    return d->downSerial;
}

quint32 TouchPoint::upSerial() const
{
    return d->upSerial;
}

QPointer<Surface> TouchPoint::surface() const
{
    return d->surface;
}

quint32 TouchPoint::time() const
{
    if (d->timestamps.isEmpty()) {
        return 0;
    }
    return d->timestamps.last();
}

QList<quint32> TouchPoint::timestamps() const
{
    return d->timestamps;
}

bool TouchPoint::isDown() const
{
    return d->down;
}

qint32 TouchPoint::id() const
{
    return d->id;
}

Touch::Private::Private(Touch *q)
    : q(q)
{
}

void Touch::Private::setup(wl_touch *t)
{
    Q_ASSERT(t);
    Q_ASSERT(!touch);
    touch.setup(t);
    wl_touch_add_listener(touch, &s_listener, this);
}

const wl_touch_listener Touch::Private::s_listener = {downCallback, upCallback, motionCallback, frameCallback, cancelCallback};

void Touch::Private::downCallback(void *data, wl_touch *touch, uint32_t serial, uint32_t time, wl_surface *surface, int32_t id, wl_fixed_t x, wl_fixed_t y)
{
    auto t = reinterpret_cast<Touch::Private *>(data);
    Q_ASSERT(t->touch == touch);
    t->down(serial, time, id, QPointF(wl_fixed_to_double(x), wl_fixed_to_double(y)), QPointer<Surface>(Surface::get(surface)));
}

void Touch::Private::down(quint32 serial, quint32 time, qint32 id, const QPointF &position, const QPointer<Surface> &surface)
{
    TouchPoint *p = new TouchPoint;
    p->d->downSerial = serial;
    p->d->surface = surface;
    p->d->id = id;
    p->d->positions << position;
    p->d->timestamps << time;
    if (active) {
        sequence << p;
        Q_EMIT q->pointAdded(p);
    } else {
        qDeleteAll(sequence);
        sequence.clear();
        sequence << p;
        active = true;
        Q_EMIT q->sequenceStarted(p);
    }
}

TouchPoint *Touch::Private::getActivePoint(qint32 id) const
{
    auto it = std::find_if(sequence.constBegin(), sequence.constEnd(), [id](TouchPoint *p) {
        return p->id() == id && p->isDown();
    });
    if (it == sequence.constEnd()) {
        return nullptr;
    }
    return *it;
}

void Touch::Private::upCallback(void *data, wl_touch *touch, uint32_t serial, uint32_t time, int32_t id)
{
    auto t = reinterpret_cast<Touch::Private *>(data);
    Q_ASSERT(t->touch == touch);
    t->up(serial, time, id);
}

void Touch::Private::up(quint32 serial, quint32 time, qint32 id)
{
    TouchPoint *p = getActivePoint(id);
    if (!p) {
        return;
    }
    p->d->timestamps << time;
    p->d->upSerial = serial;
    p->d->down = false;
    Q_EMIT q->pointRemoved(p);
    // check whether the sequence ended
    for (auto it = sequence.constBegin(); it != sequence.constEnd(); ++it) {
        if ((*it)->isDown()) {
            return;
        }
    }
    // no touch point is down
    active = false;
    Q_EMIT q->sequenceEnded();
}

void Touch::Private::motionCallback(void *data, wl_touch *touch, uint32_t time, int32_t id, wl_fixed_t x, wl_fixed_t y)
{
    auto t = reinterpret_cast<Touch::Private *>(data);
    Q_ASSERT(t->touch == touch);
    t->motion(time, id, QPointF(wl_fixed_to_double(x), wl_fixed_to_double(y)));
}

void Touch::Private::motion(quint32 time, qint32 id, const QPointF &position)
{
    TouchPoint *p = getActivePoint(id);
    if (!p) {
        return;
    }
    p->d->positions << position;
    p->d->timestamps << time;
    Q_EMIT q->pointMoved(p);
}

void Touch::Private::frameCallback(void *data, wl_touch *touch)
{
    auto t = reinterpret_cast<Touch::Private *>(data);
    Q_ASSERT(t->touch == touch);
    Q_EMIT t->q->frameEnded();
}

void Touch::Private::cancelCallback(void *data, wl_touch *touch)
{
    auto t = reinterpret_cast<Touch::Private *>(data);
    Q_ASSERT(t->touch == touch);
    t->active = false;
    Q_EMIT t->q->sequenceCanceled();
}

Touch::Touch(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

Touch::~Touch()
{
    release();
}

void Touch::destroy()
{
    d->touch.destroy();
}

void Touch::release()
{
    d->touch.release();
}

void Touch::setup(wl_touch *touch)
{
    d->setup(touch);
}

bool Touch::isValid() const
{
    return d->touch.isValid();
}

Touch::operator wl_touch *() const
{
    return d->touch;
}

Touch::operator wl_touch *()
{
    return d->touch;
}

QList<TouchPoint *> Touch::sequence() const
{
    return d->sequence;
}

}
}

#include "moc_touch.cpp"
