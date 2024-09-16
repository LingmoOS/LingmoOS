/*
    SPDX-FileCopyrightText: 2021 TangHaixiang <tanghaixiang@unionteh.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only
*/
#include "datacontrolsource.h"
#include "wayland_pointer_p.h"
// Qt
#include <QMimeType>
// Wayland
#include <wayland-wlr-data-control-unstable-v1-client-protocol.h>

namespace KWayland
{
namespace Client
{

class Q_DECL_HIDDEN DataControlSourceV1::Private
{
public:
    explicit Private(DataControlSourceV1 *q);
    void setup(zwlr_data_control_source_v1 *s);

    WaylandPointer<zwlr_data_control_source_v1, zwlr_data_control_source_v1_destroy> source;

private:

    static void sendCallback(void *data,
                             struct zwlr_data_control_source_v1 *zwlr_data_control_source_v1,
                             const char *mime_type,
                             int32_t fd);
    static void cancelledCallback(void *data,
                                  struct zwlr_data_control_source_v1 *zwlr_data_control_source_v1);

    static const struct zwlr_data_control_source_v1_listener s_listener;

    DataControlSourceV1 *q;
};

const zwlr_data_control_source_v1_listener DataControlSourceV1::Private::s_listener = {
    sendCallback,
    cancelledCallback
};

DataControlSourceV1::Private::Private(DataControlSourceV1 *q)
    : q(q)
{
}

void DataControlSourceV1::Private::sendCallback(void *data, zwlr_data_control_source_v1 *dataSource, const char *mimeType, int32_t fd)
{
    auto d = reinterpret_cast<DataControlSourceV1::Private*>(data);
    Q_ASSERT(d->source == dataSource);
    emit d->q->sendDataRequested(QString::fromUtf8(mimeType), fd);
}

void DataControlSourceV1::Private::cancelledCallback(void *data, zwlr_data_control_source_v1 *dataSource)
{
    auto d = reinterpret_cast<DataControlSourceV1::Private*>(data);
    Q_ASSERT(d->source == dataSource);
    emit d->q->cancelled();
}



void DataControlSourceV1::Private::setup(zwlr_data_control_source_v1 *s)
{
    Q_ASSERT(!source.isValid());
    Q_ASSERT(s);
    source.setup(s);
    zwlr_data_control_source_v1_add_listener(s, &s_listener, this);
}

DataControlSourceV1::DataControlSourceV1(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

DataControlSourceV1::~DataControlSourceV1()
{
    release();
}

void DataControlSourceV1::release()
{
    d->source.release();
}

void DataControlSourceV1::destroy()
{
    d->source.destroy();
}

bool DataControlSourceV1::isValid() const
{
    return d->source.isValid();
}

void DataControlSourceV1::setup(zwlr_data_control_source_v1 *dataSource)
{
    d->setup(dataSource);
}

void DataControlSourceV1::offer(const QString &mimeType)
{
    zwlr_data_control_source_v1_offer(d->source, mimeType.toUtf8().constData());
}

void DataControlSourceV1::offer(const QMimeType &mimeType)
{
    if (!mimeType.isValid()) {
        return;
    }
    offer(mimeType.name());
}

DataControlSourceV1::operator zwlr_data_control_source_v1*() const
{
    return d->source;
}

DataControlSourceV1::operator zwlr_data_control_source_v1*()
{
    return d->source;
}
}
}
