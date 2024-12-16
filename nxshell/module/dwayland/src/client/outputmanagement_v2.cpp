/*
    SPDX-FileCopyrightText: 2022 JccKevin <luochaojiang@uniontech.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "outputmanagement_v2.h"
#include "event_queue.h"
#include "outputconfiguration_v2.h"
#include "wayland-kde-output-management-v2-client-protocol.h"
#include "wayland_pointer_p.h"

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN OutputManagementV2::Private
{
public:
    Private() = default;

    WaylandPointer<kde_output_management_v2, kde_output_management_v2_destroy> outputmanagement;
    EventQueue *queue = nullptr;
};

OutputManagementV2::OutputManagementV2(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

OutputManagementV2::~OutputManagementV2()
{
    d->outputmanagement.release();
}

void OutputManagementV2::setup(kde_output_management_v2 *outputmanagement)
{
    Q_ASSERT(outputmanagement);
    Q_ASSERT(!d->outputmanagement);
    d->outputmanagement.setup(outputmanagement);
}

void OutputManagementV2::release()
{
    d->outputmanagement.release();
}

void OutputManagementV2::destroy()
{
    d->outputmanagement.destroy();
}

void OutputManagementV2::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *OutputManagementV2::eventQueue()
{
    return d->queue;
}

OutputManagementV2::operator kde_output_management_v2 *()
{
    return d->outputmanagement;
}

OutputManagementV2::operator kde_output_management_v2 *() const
{
    return d->outputmanagement;
}

bool OutputManagementV2::isValid() const
{
    return d->outputmanagement.isValid();
}

OutputConfigurationV2 *OutputManagementV2::createConfiguration(QObject *parent)
{
    Q_UNUSED(parent);
    OutputConfigurationV2 *config = new OutputConfigurationV2(this);
    auto w = kde_output_management_v2_create_configuration(d->outputmanagement);

    if (d->queue) {
        d->queue->addProxy(w);
    }

    config->setup(w);
    return config;
}

}
}
