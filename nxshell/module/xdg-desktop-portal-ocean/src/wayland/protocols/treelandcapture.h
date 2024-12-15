// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "qwayland-treeland-capture-unstable-v1.h"

#include <private/qwaylandclientextension_p.h>
#include <private/qwaylandshmbackingstore_p.h>

class TreeLandCaptureFrame : public QObject, public QtWayland::treeland_capture_frame_v1
{
    Q_OBJECT
public:
    explicit TreeLandCaptureFrame(struct ::treeland_capture_frame_v1 *object)
        : QObject()
        , QtWayland::treeland_capture_frame_v1(object)
        , m_shmBuffer(nullptr)
        , m_pendingShmBuffer(nullptr)
        , m_flags(0)
    { }

    ~TreeLandCaptureFrame() override
    {
        delete m_shmBuffer;
        delete m_pendingShmBuffer;
        destroy();
    }

    inline uint flags() const { return m_flags; }

Q_SIGNALS:
    void ready(QImage image);
    void failed();

protected:
    void treeland_capture_frame_v1_buffer(uint32_t format, uint32_t width, uint32_t height, uint32_t stride) override;
    void treeland_capture_frame_v1_flags(uint32_t flags) override;
    void treeland_capture_frame_v1_ready() override;
    void treeland_capture_frame_v1_failed() override;

private:
    QtWaylandClient::QWaylandShmBuffer *m_shmBuffer;
    QtWaylandClient::QWaylandShmBuffer *m_pendingShmBuffer;
    uint m_flags;
};

class TreeLandCaptureContext : public QObject, public QtWayland::treeland_capture_context_v1
{
    Q_OBJECT
public:
    explicit TreeLandCaptureContext(struct ::treeland_capture_context_v1 *object);
    ~TreeLandCaptureContext() override
    {
        releaseCaptureFrame();
        destroy();
    }

    inline QRect captureRegion() const { return m_captureRegion; }
    inline QtWayland::treeland_capture_context_v1::source_type sourceType() const { return m_sourceType; }

    QPointer<TreeLandCaptureFrame> frame();
    void selectSource(uint32_t sourceHint, bool freeze, bool withCursor, ::wl_surface *mask);
    void releaseCaptureFrame();

Q_SIGNALS:
    void sourceReady(QRect region, uint32_t sourceType);
    void sourceFailed(uint32_t reason);

protected:
    void treeland_capture_context_v1_source_ready(int32_t region_x, int32_t region_y, uint32_t region_width, uint32_t region_height, uint32_t source_type) override;
    void treeland_capture_context_v1_source_failed(uint32_t reason) override;

private:
    QRect m_captureRegion;
    TreeLandCaptureFrame *m_captureFrame;
    QtWayland::treeland_capture_context_v1::source_type m_sourceType;
};

class TreeLandCaptureManager;
void destruct_treeland_capture_manager(TreeLandCaptureManager *manager);

class TreeLandCaptureManager : public QWaylandClientExtensionTemplate<TreeLandCaptureManager, destruct_treeland_capture_manager>,
                               public QtWayland::treeland_capture_manager_v1
{
    Q_OBJECT
public:
    explicit TreeLandCaptureManager(QObject *parent = nullptr)
        : QWaylandClientExtensionTemplate<TreeLandCaptureManager, destruct_treeland_capture_manager>(1)
        , QtWayland::treeland_capture_manager_v1()
    { }

    ~TreeLandCaptureManager() override
    {
        destroy();
    }

    QPointer<TreeLandCaptureContext> getContext();
    void releaseCaptureContext(QPointer<TreeLandCaptureContext> context);

private:
    QList<TreeLandCaptureContext *> captureContexts;
    friend void destruct_treeland_capture_manager(TreeLandCaptureManager *manager);
};
