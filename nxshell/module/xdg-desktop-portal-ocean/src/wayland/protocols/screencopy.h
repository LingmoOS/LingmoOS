// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <private/qwaylandclientextension_p.h>
#include <qwayland-wlr-screencopy-unstable-v1.h>
#include <QList>
#include <QPointer>
#include <private/qwaylandshmbackingstore_p.h>

class ScreenCopyFrame : public QObject, public QtWayland::zwlr_screencopy_frame_v1
{
    Q_OBJECT
public:
    ScreenCopyFrame(struct ::zwlr_screencopy_frame_v1 *object);
    QtWayland::zwlr_screencopy_frame_v1::flags flags();

Q_SIGNALS:
    void ready(QImage image);
    void failed();

protected:
    void zwlr_screencopy_frame_v1_buffer(uint32_t format, uint32_t width, uint32_t height, uint32_t stride) override;
    void zwlr_screencopy_frame_v1_flags(uint32_t flags) override;
    void zwlr_screencopy_frame_v1_ready(uint32_t tv_sec_hi, uint32_t tv_sec_lo, uint32_t tv_nsec) override;
    void zwlr_screencopy_frame_v1_failed() override;

private:
    QtWaylandClient::QWaylandShmBuffer *m_shmBuffer;
    QtWaylandClient::QWaylandShmBuffer *m_pendingShmBuffer;
    QtWayland::zwlr_screencopy_frame_v1::flags m_flags;
};

class ScreenCopyManager;
void destruct_screen_copy_manager(ScreenCopyManager *screenCopyManager);
class ScreenCopyManager : public QWaylandClientExtensionTemplate<ScreenCopyManager, destruct_screen_copy_manager>, public QtWayland::zwlr_screencopy_manager_v1
{
    Q_OBJECT
public:
    ScreenCopyManager(QObject *parent = nullptr);

    QPointer<ScreenCopyFrame> captureOutput(int32_t overlay_cursor, struct ::wl_output *output);
    QPointer<ScreenCopyFrame> captureOutputRegion(int32_t overlay_cursor, struct ::wl_output *output, int32_t x, int32_t y, int32_t width, int32_t height);

private:
    QList<ScreenCopyFrame *> m_screenCopyFrames;
    friend void destruct_screen_copy_manager(ScreenCopyManager *screenCopyManager);
};
