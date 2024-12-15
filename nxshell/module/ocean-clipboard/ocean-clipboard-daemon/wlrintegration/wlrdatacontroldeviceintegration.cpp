// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wlrdatacontroldeviceintegration.h"
#include "wlrdatacontrolofferintegration.h"
#include <QThread>
#include <QMutexLocker>

WlrDataControlDeviceIntegration::WlrDataControlDeviceIntegration(::zwlr_data_control_device_v1 *object)
    : QObject()
    , QtWayland::zwlr_data_control_device_v1(object)
    , m_offerObjectBuffer(nullptr)
{

}

WlrDataControlDeviceIntegration::~WlrDataControlDeviceIntegration()
{
    if (object() && zwlr_data_control_device_v1_get_version(object()) >= ZWLR_DATA_CONTROL_DEVICE_V1_DESTROY_SINCE_VERSION) {
        zwlr_data_control_device_v1_destroy(object());
    }
}

void WlrDataControlDeviceIntegration::zwlr_data_control_device_v1_data_offer(zwlr_data_control_offer_v1 *id)
{
    // Stage 1.1: We will receive "data_offer" events before "selection" event.
    // Since the offer object delivered here and in "selection" event are the same, we buffer the wrapper
    // object in a member. "selection" event handler will eventually move the wrapper object out.

    // "data_offer" event comes only once for each clipboard content change (it is the offer object that receives
    // multiple "offer" events, advertising multiple MIME types), so we're safe to flush the MIME type buffer here,
    // destroy the existing offer object (if any) to abort the unfinished clipboard read.
    QMutexLocker locker(&m_mimeTypeBufferLock);
    m_mimeTypeBuffer.clear();
    if (!id) {
        // Nothing...
        return;
    }

    // Collect the advertised MIME types
    if (m_offerObjectBuffer) {
        m_offerObjectBuffer->deleteLater();
    }
    m_offerObjectBuffer = new WlrDataControlOfferIntegration(id);
    // Must use direct connection here, because if we use queued connection we must return to event loop before
    // we can receive the MIME type offers, by that time it would be too late and "selection" event will already
    // be dispatched. However we will have a data race on the m_mimeTypeBuffer object and it has caused crashes
    // in testing. We opt to use a mutex to protect the buffer object.
    connect(m_offerObjectBuffer, &WlrDataControlOfferIntegration::advertiseMimeType, [=](QString mimeType){
        QMutexLocker locker(&m_mimeTypeBufferLock);
        m_mimeTypeBuffer.append(mimeType);
    });
}

void WlrDataControlDeviceIntegration::zwlr_data_control_device_v1_selection(zwlr_data_control_offer_v1 *id)
{
    // Stage 1.2: By the time the current function was called we should have already have buffered
    // the advertised MIME types.
    // QMutexLocker locker(&m_mimeTypeBufferLock);
    if (!id) {
        // No data. Flush MIME buffer and fail
        m_mimeTypeBuffer.clear();
        return;
    }

    // Accept the buffered MIME types.
    m_offerObjectBuffer->setAvailableMimeTypes(m_mimeTypeBuffer);
    m_mimeTypeBuffer.clear();

    // Pass it to the clipboard interface. Ownership also follows along.
    Q_EMIT newSelection(m_offerObjectBuffer);
    m_offerObjectBuffer = nullptr;
}

void WlrDataControlDeviceIntegration::zwlr_data_control_device_v1_finished()
{
    Q_EMIT finished();
}

void WlrDataControlDeviceIntegration::zwlr_data_control_device_v1_primary_selection(zwlr_data_control_offer_v1 *id)
{

}
