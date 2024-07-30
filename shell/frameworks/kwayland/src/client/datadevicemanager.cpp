/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "datadevicemanager.h"
#include "datadevice.h"
#include "datasource.h"
#include "event_queue.h"
#include "seat.h"
#include "wayland_pointer_p.h"

#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN DataDeviceManager::Private
{
public:
    WaylandPointer<wl_data_device_manager, wl_data_device_manager_destroy> manager;
    EventQueue *queue = nullptr;
};

DataDeviceManager::DataDeviceManager(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

DataDeviceManager::~DataDeviceManager()
{
    release();
}

void DataDeviceManager::release()
{
    d->manager.release();
}

void DataDeviceManager::destroy()
{
    d->manager.destroy();
}

bool DataDeviceManager::isValid() const
{
    return d->manager.isValid();
}

void DataDeviceManager::setup(wl_data_device_manager *manager)
{
    Q_ASSERT(manager);
    Q_ASSERT(!d->manager.isValid());
    d->manager.setup(manager);
}

EventQueue *DataDeviceManager::eventQueue()
{
    return d->queue;
}

void DataDeviceManager::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

DataSource *DataDeviceManager::createDataSource(QObject *parent)
{
    Q_ASSERT(isValid());
    DataSource *s = new DataSource(parent);
    auto w = wl_data_device_manager_create_data_source(d->manager);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

DataDevice *DataDeviceManager::getDataDevice(Seat *seat, QObject *parent)
{
    Q_ASSERT(isValid());
    Q_ASSERT(seat);
    DataDevice *device = new DataDevice(parent);
    auto w = wl_data_device_manager_get_data_device(d->manager, *seat);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    device->setup(w);
    return device;
}

DataDeviceManager::operator wl_data_device_manager *() const
{
    return d->manager;
}

DataDeviceManager::operator wl_data_device_manager *()
{
    return d->manager;
}

}
}

#include "moc_datadevicemanager.cpp"
