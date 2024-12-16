// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "primaryoutput_v1.h"
#include "event_queue.h"
#include "wayland_pointer_p.h"

#include <wayland-kde-primary-output-v1-client-protocol.h>
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{

class Q_DECL_HIDDEN PrimaryOutputV1::Private : public QObject
{
    Q_OBJECT
public:
    Private(PrimaryOutputV1 *q);

    void setup(kde_primary_output_v1 *primaryOutput);

    WaylandPointer<kde_primary_output_v1, kde_primary_output_v1_destroy> primaryOutput;
    static struct kde_primary_output_v1_listener s_primaryOutputListener;
    EventQueue *queue = nullptr;

    PrimaryOutputV1 *q;

private:
    static void primaryOutputCallback(void *data, kde_primary_output_v1 *primaryOutput, const char *name);
};

kde_primary_output_v1_listener PrimaryOutputV1::Private::s_primaryOutputListener = {primaryOutputCallback};

void PrimaryOutputV1::Private::setup(kde_primary_output_v1 *primaryOutput)
{
    kde_primary_output_v1_add_listener(primaryOutput, &s_primaryOutputListener, this);
}

void PrimaryOutputV1::Private::primaryOutputCallback(void *data, kde_primary_output_v1 *primaryOutput, const char *outputName)
{
    Q_UNUSED(primaryOutput);
    auto o = reinterpret_cast<PrimaryOutputV1::Private *>(data);
    Q_EMIT o->q->primaryOutputChanged(QString::fromUtf8(outputName));
}

PrimaryOutputV1::PrimaryOutputV1(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

PrimaryOutputV1::~PrimaryOutputV1()
{
    release();
}

PrimaryOutputV1::Private::Private(PrimaryOutputV1 *q)
    : QObject(q), q(q)
{
}

void PrimaryOutputV1::setup(kde_primary_output_v1 *primaryOutput)
{
    Q_ASSERT(primaryOutput);
    Q_ASSERT(!d->primaryOutput);
    d->primaryOutput.setup(primaryOutput);
    d->setup(primaryOutput);
}

void PrimaryOutputV1::release()
{
    d->primaryOutput.release();
}

void PrimaryOutputV1::destroy()
{
    d->primaryOutput.destroy();
}

void PrimaryOutputV1::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *PrimaryOutputV1::eventQueue()
{
    return d->queue;
}

PrimaryOutputV1::operator kde_primary_output_v1 *()
{
    return d->primaryOutput;
}

PrimaryOutputV1::operator kde_primary_output_v1 *() const
{
    return d->primaryOutput;
}

bool PrimaryOutputV1::isValid() const
{
    return d->primaryOutput.isValid();
}

}
}

#include "primaryoutput_v1.moc"