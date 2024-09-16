/*
    SPDX-FileCopyrightText: 2015 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "outputmanagement.h"
#include "event_queue.h"
#include "outputconfiguration.h"
#include "wayland-output-management-client-protocol.h"
#include "wayland_pointer_p.h"

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN OutputManagement::Private
{
public:
    Private() = default;

    WaylandPointer<org_kde_kwin_outputmanagement, org_kde_kwin_outputmanagement_destroy> outputmanagement;
    EventQueue *queue = nullptr;
};

OutputManagement::OutputManagement(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

OutputManagement::~OutputManagement()
{
    d->outputmanagement.release();
}

void OutputManagement::setup(org_kde_kwin_outputmanagement *outputmanagement)
{
    Q_ASSERT(outputmanagement);
    Q_ASSERT(!d->outputmanagement);
    d->outputmanagement.setup(outputmanagement);
}

void OutputManagement::release()
{
    d->outputmanagement.release();
}

void OutputManagement::destroy()
{
    d->outputmanagement.destroy();
}

void OutputManagement::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *OutputManagement::eventQueue()
{
    return d->queue;
}

OutputManagement::operator org_kde_kwin_outputmanagement *()
{
    return d->outputmanagement;
}

OutputManagement::operator org_kde_kwin_outputmanagement *() const
{
    return d->outputmanagement;
}

bool OutputManagement::isValid() const
{
    return d->outputmanagement.isValid();
}

OutputConfiguration *OutputManagement::createConfiguration(QObject *parent)
{
    Q_UNUSED(parent);
    OutputConfiguration *config = new OutputConfiguration(this);
    auto w = org_kde_kwin_outputmanagement_create_configuration(d->outputmanagement);

    if (d->queue) {
        d->queue->addProxy(w);
    }

    config->setup(w);
    return config;
}

}
}
