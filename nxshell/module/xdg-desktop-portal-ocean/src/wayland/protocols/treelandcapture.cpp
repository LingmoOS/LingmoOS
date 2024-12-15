// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "treelandcapture.h"
#include "common.h"

Q_DECLARE_LOGGING_CATEGORY(portalWaylandProtocol);
void destruct_treeland_capture_manager(TreeLandCaptureManager *manager)
{
    qDeleteAll(manager->captureContexts);
    manager->captureContexts.clear();
}


QPointer<TreeLandCaptureContext> TreeLandCaptureManager::getContext()
{
    auto context = get_context();
    auto captureContext = new TreeLandCaptureContext(context);
    captureContexts.append(captureContext);
    return captureContext;
}

TreeLandCaptureContext::TreeLandCaptureContext(struct ::treeland_capture_context_v1 *object)
    : QObject()
    , QtWayland::treeland_capture_context_v1(object)
    , m_captureFrame(nullptr)
{}

void TreeLandCaptureContext::treeland_capture_context_v1_source_ready(int32_t region_x, int32_t region_y, uint32_t region_width, uint32_t region_height, uint32_t source_type)
{
    Q_EMIT sourceReady(QRect(region_x, region_y, region_width, region_height), source_type);
}

void TreeLandCaptureContext::treeland_capture_context_v1_source_failed(uint32_t reason)
{
    Q_EMIT sourceFailed(reason);
}

QPointer<TreeLandCaptureFrame> TreeLandCaptureContext::frame()
{
    if (m_captureFrame)
        return m_captureFrame;
    auto capture_frame = capture();
    m_captureFrame = new TreeLandCaptureFrame(capture_frame);
    return m_captureFrame;
}

void TreeLandCaptureContext::selectSource(uint32_t sourceHint, bool freeze, bool withCursor, ::wl_surface *mask)
{
    select_source(sourceHint, freeze, withCursor, mask);
}
void TreeLandCaptureContext::releaseCaptureFrame() {
    if (m_captureFrame) {
        delete m_captureFrame;
        m_captureFrame = nullptr;
    }
}

void TreeLandCaptureFrame::treeland_capture_frame_v1_buffer(uint32_t format, uint32_t width, uint32_t height, uint32_t stride)
{
    if (stride != width * 4) {
        qCDebug(portalWaylandProtocol)
                << "Receive a buffer format which is not compatible with QWaylandShmBuffer."
                << "format:" << format << "width:" << width << "height:" << height
                << "stride:" << stride;
        return;
    }
    if (m_pendingShmBuffer)
        return; // We only need one supported format
    m_pendingShmBuffer = new QtWaylandClient::QWaylandShmBuffer(waylandDisplay(), QSize(width, height), QtWaylandClient::QWaylandShm::formatFrom(static_cast<::wl_shm_format>(format)));
    copy(m_pendingShmBuffer->buffer());
}

void TreeLandCaptureFrame::treeland_capture_frame_v1_flags(uint32_t flags)
{
    m_flags = flags;
}

void TreeLandCaptureFrame::treeland_capture_frame_v1_ready()
{
    if (m_shmBuffer)
        delete m_shmBuffer;
    m_shmBuffer = m_pendingShmBuffer;
    m_pendingShmBuffer = nullptr;
    Q_EMIT ready(*m_shmBuffer->image());
}

void TreeLandCaptureFrame::treeland_capture_frame_v1_failed()
{
    Q_EMIT failed();
}

void TreeLandCaptureManager::releaseCaptureContext(QPointer<TreeLandCaptureContext> context)
{
    for (const auto &entry : captureContexts) {
        if (entry == context.data()) {
            entry->deleteLater();
            captureContexts.removeOne(entry);
        }
    }
}
