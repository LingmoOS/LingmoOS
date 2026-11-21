#pragma once

#include "base/geometry.h"
#include "base/signal.h"

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <sys/types.h>
#include <vector>

struct wl_buffer;
struct wl_display;
struct wl_output;
struct wl_registry;
struct wl_shm;
struct zxdg_output_manager_v1;
struct ext_image_copy_capture_manager_v1;
struct ext_image_copy_capture_session_v1;
struct ext_image_copy_capture_frame_v1;
struct ext_output_image_capture_source_manager_v1;
struct ext_image_capture_source_v1;
struct ext_foreign_toplevel_image_capture_source_manager_v1;
struct ext_foreign_toplevel_list_v1;
struct ext_foreign_toplevel_handle_v1;
struct zwp_linux_dmabuf_v1;
struct zwp_linux_buffer_params_v1;

namespace Lingmo::base {
class EventLoop;
}

struct CaptureOutput {
    std::string name;
    std::string description;
    Lingmo::base::Point logicalPosition;
    Lingmo::base::Size logicalSize;
    int refreshMilliHz = 0;
};

// An application window the compositor lists for capture. The identifier is
// stable for the window's lifetime and is what a selection names.
struct CaptureToplevel {
    std::string identifier;
    std::string title;
    std::string appId;
};

// One plane of a DMA-BUF the portal allocated; the fd stays the caller's.
struct DmaBufPlane {
    int fd = -1;
    uint32_t offset = 0;
    uint32_t stride = 0;
};

class CaptureSession;

// One connection to the compositor for the whole portal, driven from the
// portal's event loop. It carries output enumeration and the capture globals, which the
// compositor only announces to allowlisted executables.
class WaylandCapture
{
public:
    struct Output;
    struct Toplevel;

    WaylandCapture();
    ~WaylandCapture();

    WaylandCapture(const WaylandCapture &) = delete;
    WaylandCapture &operator=(const WaylandCapture &) = delete;

    // Connects on first use; a broken connection is dropped and retried here.
    bool connectToDisplay();
    bool captureAvailable() const;

    std::vector<CaptureOutput> outputs() const;
    // Empty unless the compositor lists windows to this client.
    std::vector<CaptureToplevel> toplevels() const;
    bool dmabufAvailable() const { return m_dmabuf != nullptr; }

    std::unique_ptr<CaptureSession> createSession(const std::string &output, bool cursor);
    std::unique_ptr<CaptureSession> createToplevelSession(const std::string &identifier, bool cursor);
    wl_buffer *createShmBuffer(int fd, const Lingmo::base::Size &size, int stride);
    // A buffer over planes the caller allocated, delivered to `done` once the
    // compositor has imported it, or as null when it refused. False without
    // linux-dmabuf. A refusal is an event, not a protocol error, which is why
    // this is asynchronous.
    bool createDmaBufBuffer(const Lingmo::base::Size &size, uint32_t format, uint64_t modifier,
                            const std::vector<DmaBufPlane> &planes,
                            std::function<void(wl_buffer *)> done);
    void flush();

    Lingmo::base::Signal<> disconnected;

private:
    friend class CaptureSession;
    friend struct WaylandCaptureEvents;

    void handleGlobal(uint32_t name, const char *interface, uint32_t version);
    void handleGlobalRemove(uint32_t name);
    void bindXdgOutputs();
    void addToplevel(ext_foreign_toplevel_handle_v1 *handle);
    std::unique_ptr<CaptureSession> makeSession(ext_image_capture_source_v1 *source, bool cursor);
    void dispatch();
    void disconnect();
    // From inside a dispatch or a PipeWire callback the display cannot go
    // yet; the disconnect runs from the event loop instead.
    void scheduleDisconnect();

    wl_display *m_display = nullptr;
    wl_registry *m_registry = nullptr;
    wl_shm *m_shm = nullptr;
    zxdg_output_manager_v1 *m_xdgOutputs = nullptr;
    ext_output_image_capture_source_manager_v1 *m_sources = nullptr;
    ext_foreign_toplevel_image_capture_source_manager_v1 *m_toplevelSources = nullptr;
    ext_foreign_toplevel_list_v1 *m_toplevelList = nullptr;
    ext_image_copy_capture_manager_v1 *m_copies = nullptr;
    zwp_linux_dmabuf_v1 *m_dmabuf = nullptr;
    Lingmo::base::EventLoop *m_eventLoop;
    int m_watch = 0;
    std::vector<std::unique_ptr<Output>> m_outputs;
    std::vector<std::unique_ptr<Toplevel>> m_toplevels;
    bool m_dispatching = false;
    bool m_disconnectPending = false;
    // DMA-BUF imports still waiting for the compositor's created or failed.
    std::vector<zwp_linux_buffer_params_v1 *> m_pendingParams;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

// One ext-image-copy-capture session on one source. The compositor allows a
// single frame at a time, so a capture is refused while one is in flight.
class CaptureSession
{
public:
    using DmaBufFormats = std::map<uint32_t, std::vector<uint64_t>>;

    ~CaptureSession();

    CaptureSession(const CaptureSession &) = delete;
    CaptureSession &operator=(const CaptureSession &) = delete;

    Lingmo::base::Size bufferSize() const { return m_size; }
    bool argbSupported() const { return m_argb; }
    // DRM fourcc to modifiers the compositor renders into, with the device
    // (st_rdev of a DRM node) to allocate on; empty when SHM is all there is.
    const DmaBufFormats &dmabufFormats() const { return m_dmabufFormats; }
    dev_t dmabufDevice() const { return m_dmabufDevice; }
    bool capturing() const { return m_frame != nullptr; }

    // `damage` is what changed in the buffer since it was last filled; empty
    // means all of it.
    bool capture(wl_buffer *buffer, const std::vector<Lingmo::base::Rect> &damage = {});
    void cancel();

    // Each may destroy the session.
    std::function<void()> onConstraintsChanged;
    std::function<void()> onStopped;
    // `damage` is in buffer coordinates, relative to this session's previous frame.
    std::function<void(uint64_t presentationNs, uint32_t transform, const std::vector<Lingmo::base::Rect> &damage)>
            onFrameReady;
    std::function<void(uint32_t reason)> onFrameFailed;

private:
    friend class WaylandCapture;
    friend struct CaptureSessionEvents;
    CaptureSession(WaylandCapture *wayland, ext_image_copy_capture_session_v1 *session);

    WaylandCapture *m_wayland;
    Lingmo::base::Subscription m_disconnected;
    ext_image_copy_capture_session_v1 *m_session;
    ext_image_copy_capture_frame_v1 *m_frame = nullptr;
    Lingmo::base::Size m_pendingSize;
    Lingmo::base::Size m_size;
    bool m_pendingArgb = false;
    bool m_argb = false;
    DmaBufFormats m_pendingDmabufFormats;
    DmaBufFormats m_dmabufFormats;
    dev_t m_pendingDmabufDevice = 0;
    dev_t m_dmabufDevice = 0;
    bool m_stopped = false;
    uint32_t m_transform = 0;
    uint64_t m_presentationNs = 0;
    std::vector<Lingmo::base::Rect> m_damage;
};
