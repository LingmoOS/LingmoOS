/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "datadevice.h"
#include "datasource.h"
#include "surface.h"
#include "wayland_pointer_p.h"
// Qt
#include <QPointer>
// Wayland
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN DataDevice::Private
{
public:
    explicit Private(DataDevice *q);
    void setup(wl_data_device *d);

    WaylandPointer<wl_data_device, wl_data_device_release> device;
    QScopedPointer<DataOffer> selectionOffer;
    struct Drag {
        QPointer<DataOffer> offer;
        QPointer<Surface> surface;
    };
    Drag drag;

private:
    void dataOffer(wl_data_offer *id);
    void selection(wl_data_offer *id);
    void dragEnter(quint32 serial, const QPointer<Surface> &surface, const QPointF &relativeToSurface, wl_data_offer *dataOffer);
    void dragLeft();
    static void dataOfferCallback(void *data, wl_data_device *dataDevice, wl_data_offer *id);
    static void enterCallback(void *data, wl_data_device *dataDevice, uint32_t serial, wl_surface *surface, wl_fixed_t x, wl_fixed_t y, wl_data_offer *id);
    static void leaveCallback(void *data, wl_data_device *dataDevice);
    static void motionCallback(void *data, wl_data_device *dataDevice, uint32_t time, wl_fixed_t x, wl_fixed_t y);
    static void dropCallback(void *data, wl_data_device *dataDevice);
    static void selectionCallback(void *data, wl_data_device *dataDevice, wl_data_offer *id);

    static const struct wl_data_device_listener s_listener;

    DataDevice *q;
    DataOffer *lastOffer = nullptr;
};

const wl_data_device_listener DataDevice::Private::s_listener =
    {dataOfferCallback, enterCallback, leaveCallback, motionCallback, dropCallback, selectionCallback};

void DataDevice::Private::dataOfferCallback(void *data, wl_data_device *dataDevice, wl_data_offer *id)
{
    auto d = reinterpret_cast<Private *>(data);
    Q_ASSERT(d->device == dataDevice);
    d->dataOffer(id);
}

void DataDevice::Private::dataOffer(wl_data_offer *id)
{
    Q_ASSERT(!lastOffer);
    lastOffer = new DataOffer(q, id);
    Q_ASSERT(lastOffer->isValid());
}

void DataDevice::Private::enterCallback(void *data,
                                        wl_data_device *dataDevice,
                                        uint32_t serial,
                                        wl_surface *surface,
                                        wl_fixed_t x,
                                        wl_fixed_t y,
                                        wl_data_offer *id)
{
    auto d = reinterpret_cast<Private *>(data);
    Q_ASSERT(d->device == dataDevice);
    d->dragEnter(serial, QPointer<Surface>(Surface::get(surface)), QPointF(wl_fixed_to_double(x), wl_fixed_to_double(y)), id);
}

void DataDevice::Private::dragEnter(quint32 serial, const QPointer<Surface> &surface, const QPointF &relativeToSurface, wl_data_offer *dataOffer)
{
    drag.surface = surface;
    Q_ASSERT(*lastOffer == dataOffer);
    drag.offer = lastOffer;
    lastOffer = nullptr;
    Q_EMIT q->dragEntered(serial, relativeToSurface);
}

void DataDevice::Private::leaveCallback(void *data, wl_data_device *dataDevice)
{
    auto d = reinterpret_cast<Private *>(data);
    Q_ASSERT(d->device == dataDevice);
    d->dragLeft();
}

void DataDevice::Private::dragLeft()
{
    if (drag.offer) {
        delete drag.offer;
    }
    drag = Drag();
    Q_EMIT q->dragLeft();
}

void DataDevice::Private::motionCallback(void *data, wl_data_device *dataDevice, uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
    auto d = reinterpret_cast<Private *>(data);
    Q_ASSERT(d->device == dataDevice);
    Q_EMIT d->q->dragMotion(QPointF(wl_fixed_to_double(x), wl_fixed_to_double(y)), time);
}

void DataDevice::Private::dropCallback(void *data, wl_data_device *dataDevice)
{
    auto d = reinterpret_cast<Private *>(data);
    Q_ASSERT(d->device == dataDevice);
    Q_EMIT d->q->dropped();
}

void DataDevice::Private::selectionCallback(void *data, wl_data_device *dataDevice, wl_data_offer *id)
{
    auto d = reinterpret_cast<Private *>(data);
    Q_ASSERT(d->device == dataDevice);
    d->selection(id);
}

void DataDevice::Private::selection(wl_data_offer *id)
{
    if (!id) {
        selectionOffer.reset();
        Q_EMIT q->selectionCleared();
        return;
    }
    Q_ASSERT(*lastOffer == id);
    selectionOffer.reset(lastOffer);
    lastOffer = nullptr;
    Q_EMIT q->selectionOffered(selectionOffer.data());
}

DataDevice::Private::Private(DataDevice *q)
    : q(q)
{
}

void DataDevice::Private::setup(wl_data_device *d)
{
    Q_ASSERT(d);
    Q_ASSERT(!device.isValid());
    device.setup(d);
    wl_data_device_add_listener(device, &s_listener, this);
}

DataDevice::DataDevice(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

DataDevice::~DataDevice()
{
    if (d->drag.offer) {
        delete d->drag.offer;
    }
    release();
}

void DataDevice::destroy()
{
    d->device.destroy();
}

void DataDevice::release()
{
    d->device.release();
}

bool DataDevice::isValid() const
{
    return d->device.isValid();
}

void DataDevice::setup(wl_data_device *dataDevice)
{
    d->setup(dataDevice);
}

void DataDevice::startDragInternally(quint32 serial, Surface *origin, Surface *icon)
{
    startDrag(serial, nullptr, origin, icon);
}

namespace
{
static wl_data_source *dataSource(const DataSource *source)
{
    if (!source) {
        return nullptr;
    }
    return *source;
}
}

void DataDevice::startDrag(quint32 serial, DataSource *source, Surface *origin, Surface *icon)
{
    wl_data_device_start_drag(d->device, dataSource(source), *origin, icon ? (wl_surface *)*icon : nullptr, serial);
}

void DataDevice::setSelection(quint32 serial, DataSource *source)
{
    wl_data_device_set_selection(d->device, dataSource(source), serial);
}

void DataDevice::clearSelection(quint32 serial)
{
    setSelection(serial);
}

DataOffer *DataDevice::offeredSelection() const
{
    return d->selectionOffer.data();
}

QPointer<Surface> DataDevice::dragSurface() const
{
    return d->drag.surface;
}

DataOffer *DataDevice::dragOffer() const
{
    return d->drag.offer;
}

DataDevice::operator wl_data_device *()
{
    return d->device;
}

DataDevice::operator wl_data_device *() const
{
    return d->device;
}

}
}

#include "moc_datadevice.cpp"
