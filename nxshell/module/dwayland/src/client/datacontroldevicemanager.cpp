/*
    SPDX-FileCopyrightText: 2021 TangHaixiang <tanghaixiang@unionteh.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only
*/


#include "datacontroldevicemanager.h"
#include "datacontroldevice.h"
#include "datacontrolsource.h"
#include "event_queue.h"
#include "seat.h"
#include "wayland_pointer_p.h"

#include <wayland-wlr-data-control-unstable-v1-client-protocol.h>

namespace KWayland
{
namespace Client
{

class Q_DECL_HIDDEN DataControlDeviceManager::Private
{
public:
    WaylandPointer<zwlr_data_control_manager_v1, zwlr_data_control_manager_v1_destroy> manager;
    EventQueue *queue = nullptr;
};

DataControlDeviceManager::DataControlDeviceManager(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

DataControlDeviceManager::~DataControlDeviceManager()
{
    release();
}

void DataControlDeviceManager::release()
{
    d->manager.release();
}

void DataControlDeviceManager::destroy()
{
    d->manager.destroy();
}

bool DataControlDeviceManager::isValid() const
{
    return d->manager.isValid();
}

void DataControlDeviceManager::setup(zwlr_data_control_manager_v1 *manager)
{
    Q_ASSERT(manager);
    Q_ASSERT(!d->manager.isValid());
    d->manager.setup(manager);
}

EventQueue *DataControlDeviceManager::eventQueue()
{
    return d->queue;
}

void DataControlDeviceManager::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

DataControlSourceV1 *DataControlDeviceManager::createDataSource(QObject *parent)
{
    Q_ASSERT(isValid());
    DataControlSourceV1 *s = new DataControlSourceV1(parent);
    auto w = zwlr_data_control_manager_v1_create_data_source(d->manager);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

DataControlDeviceV1 *DataControlDeviceManager::getDataDevice(Seat *seat, QObject *parent)
{
    Q_ASSERT(isValid());
    Q_ASSERT(seat);
    DataControlDeviceV1 *device = new DataControlDeviceV1(parent);
    auto w = zwlr_data_control_manager_v1_get_data_device(d->manager, *seat);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    device->setup(w);
    return device;
}

DataControlDeviceManager::operator zwlr_data_control_manager_v1*() const
{
    return d->manager;
}

DataControlDeviceManager::operator zwlr_data_control_manager_v1*()
{
    return d->manager;
}

}
}
