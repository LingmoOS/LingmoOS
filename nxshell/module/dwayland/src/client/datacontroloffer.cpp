/*
    SPDX-FileCopyrightText: 2021 TangHaixiang <tanghaixiang@unionteh.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only
*/

#include "datacontroloffer.h"
#include "datacontroldevice.h"

#include "wayland_pointer_p.h"
// Qt
#include <QMimeType>
#include <QMimeDatabase>
// Wayland
#include <wayland-wlr-data-control-unstable-v1-client-protocol.h>

namespace KWayland
{

namespace Client
{

class Q_DECL_HIDDEN DataControlOfferV1::Private
{
public:
    Private(zwlr_data_control_offer_v1 *offer, DataControlOfferV1 *q);
    WaylandPointer<zwlr_data_control_offer_v1, zwlr_data_control_offer_v1_destroy> dataOffer;

    QList<QString> mimeTypes;
private:
    void offer(const QString &mimeType);
    static void offerCallback(void *data, zwlr_data_control_offer_v1 *dataOffer, const char *mimeType);

    DataControlOfferV1 *q;
    static const struct zwlr_data_control_offer_v1_listener s_listener;
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zwlr_data_control_offer_v1_listener DataControlOfferV1::Private::s_listener = {
    offerCallback
};
#endif

DataControlOfferV1::Private::Private(zwlr_data_control_offer_v1 *offer, DataControlOfferV1 *q)
    : q(q)
{
    dataOffer.setup(offer);
    zwlr_data_control_offer_v1_add_listener(offer, &s_listener, this);
}

void DataControlOfferV1::Private::offerCallback(void *data, zwlr_data_control_offer_v1 *dataOffer, const char *mimeType)
{
    auto d = reinterpret_cast<Private*>(data);
    Q_ASSERT(d->dataOffer == dataOffer);
    d->offer(QString::fromUtf8(mimeType));
}

void DataControlOfferV1::Private::offer(const QString &mimeType)
{
    mimeTypes <<mimeType;
    emit q->mimeTypeOffered(mimeType);
}

DataControlOfferV1::DataControlOfferV1(DataControlDeviceV1 *parent, zwlr_data_control_offer_v1 *dataOffer)
    : QObject(parent)
    , d(new Private(dataOffer, this))
{
}

DataControlOfferV1::~DataControlOfferV1()
{
    release();
}

void DataControlOfferV1::release()
{
    d->dataOffer.release();
}

void DataControlOfferV1::destroy()
{
    d->dataOffer.destroy();
}

bool DataControlOfferV1::isValid() const
{
    return d->dataOffer.isValid();
}

QList<QString> DataControlOfferV1::offeredMimeTypes() const
{
    return d->mimeTypes;
}

void DataControlOfferV1::receive(const QMimeType &mimeType, qint32 fd)
{
    receive(mimeType.name(), fd);
}

void DataControlOfferV1::receive(const QString &mimeType, qint32 fd)
{
    Q_ASSERT(isValid());
    zwlr_data_control_offer_v1_receive(d->dataOffer, mimeType.toUtf8().constData(), fd);
}

DataControlOfferV1::operator zwlr_data_control_offer_v1*()
{
    return d->dataOffer;
}

DataControlOfferV1::operator zwlr_data_control_offer_v1*() const
{
    return d->dataOffer;
}

}
}
