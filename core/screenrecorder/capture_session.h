// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LINGMO_RECORDER_CAPTURE_SESSION_H
#define LINGMO_RECORDER_CAPTURE_SESSION_H

#include "image.h"

#include "base/geometry.h"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <sys/types.h>

class CaptureClient;
struct gbm_bo;
struct gbm_device;
struct ext_image_capture_source_v1;
struct ext_image_copy_capture_session_v1;
struct ext_image_copy_capture_frame_v1;
struct wl_buffer;

// One ext_image_copy_capture_v1 session into a pool of shared memory or DMA-BUF buffers.
// A buffer handed out stays the consumer's until releaseBuffer().
class CaptureSession
{
public:
    //! Where frames land: shared memory the CPU reads, or GPU memory.
    enum class Memory {
        Shm,
        DmaBuf,
    };

    struct Frame {
        int buffer = -1;
        //! The shared buffer itself, valid until releaseBuffer(). Null for
        //! a DMA-BUF frame.
        Image image;
        DmaBuf dmabuf;
        //! The wl_output.transform the pixels are in.
        uint32_t transform = 0;
        //! When the compositor presented this frame, on CLOCK_MONOTONIC.
        int64_t presentedNs = 0;
    };

    //! Takes @p source over; it is destroyed once the session exists. Asked
    //! for DMA-BUFs, it falls back to shared memory where the compositor or
    //! the device offers none.
    CaptureSession(CaptureClient *client, ext_image_capture_source_v1 *source, bool paintCursors,
                   int bufferCount = 1, Memory memory = Memory::Shm);
    ~CaptureSession();

    CaptureSession(const CaptureSession &) = delete;
    CaptureSession &operator=(const CaptureSession &) = delete;

    bool hasConstraints() const { return !m_size.isEmpty(); }
    bool usesDmaBuf() const { return m_gbm && m_memory == Memory::DmaBuf; }
    //! The render node the DMA-BUFs belong to.
    const std::string &renderNode() const { return m_renderNode; }
    Lingmo::Base::Size bufferSize() const { return m_size; }
    bool isCapturing() const { return m_frame != nullptr; }

    //! Asks for the next frame. False while one is pending, before the
    //! compositor has said what buffers it takes, or while every buffer is held.
    bool capture();
    void releaseBuffer(int buffer);

    //! The compositor told the session what to capture into, for the first
    //! time or anew after the source changed size.
    std::function<void()> onConstraintsChanged;
    std::function<void(const Frame &frame)> onFrameReady;
    //! This frame is lost; the next capture() can try again. Called from
    //! the Wayland event handler.
    std::function<void()> onFrameFailed;
    //! The source is gone, or capture is not possible at all.
    std::function<void()> onStopped;

private:
    friend struct CaptureSessionListeners;

    struct Buffer {
        wl_buffer *buffer = nullptr;
        uint8_t *pixels = nullptr;
        size_t length = 0;
        gbm_bo *bo = nullptr;
        DmaBuf dmabuf;
        Lingmo::Base::Size size;
        bool held = false;
        //! From before a size change; freed once nobody uses it.
        bool retired = false;
    };

    void applyConstraints();
    bool openDevice();
    //! A device that cannot allocate what it was offered.
    bool fallBackToShm();
    bool allocate(int count);
    bool allocateShm(Buffer *buffer);
    bool allocateDmaBuf(Buffer *buffer);
    void freeBuffer(int id);
    void finishFrame();
    void stop();

    CaptureClient *m_client;
    ext_image_copy_capture_session_v1 *m_session = nullptr;
    ext_image_copy_capture_frame_v1 *m_frame = nullptr;
    int m_bufferCount;
    std::map<int, std::unique_ptr<Buffer>> m_buffers;
    int m_nextBuffer = 0;
    int m_frameBuffer = -1;
    Lingmo::Base::Size m_size;
    Lingmo::Base::Size m_pendingSize;
    bool m_pendingArgb = false;
    Memory m_memory;
    dev_t m_pendingDevice = 0;
    std::vector<std::pair<uint32_t, std::vector<uint64_t>>> m_pendingFormats;
    uint32_t m_drmFormat = 0;
    uint64_t m_modifier = 0;
    int m_deviceFd = -1;
    gbm_device *m_gbm = nullptr;
    std::string m_renderNode;
    bool m_stopped = false;
    uint32_t m_frameTransform = 0;
    int64_t m_framePresentedNs = 0;
};

#endif // LINGMO_RECORDER_CAPTURE_SESSION_H
