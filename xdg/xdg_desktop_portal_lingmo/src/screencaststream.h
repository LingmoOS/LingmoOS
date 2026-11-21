#pragma once

#include "base/elapsed_timer.h"
#include "base/geometry.h"
#include "base/signal.h"
#include "base/timer.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

struct gbm_device;
struct pw_buffer;
struct pw_stream;
struct spa_hook;
struct spa_pod;
struct spa_pod_builder;

class CaptureSession;
class PipeWireCore;
class WaylandCapture;

// What a stream captures: one monitor by output name, or one window by the
// identifier the compositor's toplevel list gave it.
struct CaptureTarget {
    enum class Kind {
        Monitor,
        Window,
    };
    Kind kind = Kind::Monitor;
    std::string name;
};

// One capture target exported as a PipeWire video source. Buffers are the
// consumer's choice: DMA-BUFs the compositor renders into on the GPU when it
// offers that and the consumer takes them, memfd-backed wl_shm buffers
// otherwise. A frame is only requested while a buffer is free.
class ScreenCastStream
{
public:
    ScreenCastStream(WaylandCapture *wayland, PipeWireCore *pipewire, const CaptureTarget &target,
                     bool cursor, int maxFramerate);
    ~ScreenCastStream();

    ScreenCastStream(const ScreenCastStream &) = delete;
    ScreenCastStream &operator=(const ScreenCastStream &) = delete;

    bool start();
    uint32_t nodeId() const { return m_nodeId; }
    Lingmo::base::Size size() const { return m_size; }
    const CaptureTarget &target() const { return m_target; }

    // Exactly one of onReady and onFailed runs before the node id is known.
    // onReady runs inside a PipeWire callback and must leave the stream be;
    // the others run from the event loop and may destroy it.
    std::function<void()> onReady;
    std::function<void()> onFailed;
    // After onReady: the compositor or PipeWire ended the stream.
    std::function<void()> onStopped;

private:
    struct Buffer;
    friend struct ScreenCastStreamEvents;

    bool connectStream();
    void updateParams(bool fixate = false);
    void buildFormats(spa_pod_builder *builder, std::vector<const spa_pod *> &params,
                      bool fixate) const;
    bool openGbm();
    bool chooseModifier(uint32_t format, const std::vector<uint64_t> &modifiers);
    void handleStateChanged(int previous, int state, const char *error);
    void handleFormat(const spa_pod *param);
    void addBuffer(pw_buffer *buffer);
    void removeBuffer(pw_buffer *buffer);
    void captureNext();
    void beginCapture();
    void frameReady(uint64_t presentationNs, uint32_t transform, const std::vector<Lingmo::base::Rect> &damage);
    void frameFailed(uint32_t reason);
    void returnCurrent();
    void destroyStream();
    void finish();

    WaylandCapture *m_wayland;
    PipeWireCore *m_pipewire;
    CaptureTarget m_target;
    bool m_cursor;
    int m_maxFramerate;
    std::unique_ptr<CaptureSession> m_session;
    pw_stream *m_stream = nullptr;
    spa_hook *m_listener = nullptr;
    Lingmo::base::Size m_size;
    // The negotiated format's size; buffers follow it, never a newer m_size.
    Lingmo::base::Size m_formatSize;
    uint32_t m_nodeId = 0;
    bool m_announced = false;
    bool m_streaming = false;
    bool m_finished = false;
    int m_framerate = 0;
    uint64_t m_sequence = 0;
    pw_buffer *m_current = nullptr;
    std::vector<Buffer *> m_buffers;
    Lingmo::base::ElapsedTimer m_lastCapture;
    Lingmo::base::Timer m_throttle;
    // DMA-BUF negotiation: the device the compositor named, and what the
    // consumer settled on. A failed allocation withdraws the offer.
    int m_gbmFd = -1;
    gbm_device *m_gbm = nullptr;
    bool m_avoidDmaBuf = false;
    bool m_dmabuf = false;
    // Captures the compositor failed in a row; enough of them on DMA-BUFs
    // withdraws the offer, since the probe only proved a 16x16 render.
    int m_failures = 0;
    uint32_t m_drmFormat = 0;
    uint64_t m_modifier = 0;
    // What chooseModifier() settled a DONT_FIXATE choice on.
    uint32_t m_fixateFormat = 0;
    uint64_t m_fixateModifier = 0;
    Lingmo::base::Subscription m_pipewireDisconnected;
    Lingmo::base::Subscription m_waylandDisconnected;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};
