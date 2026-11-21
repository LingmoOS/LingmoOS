#include "waylandcapture.h"
#include "requestobject.h"

#include "base/event_loop.h"
#include "base/logging.h"
#include "base/strings.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <sys/epoll.h>
#include <utility>
#include <ext-foreign-toplevel-list-v1-client-protocol.h>
#include <ext-image-capture-source-v1-client-protocol.h>
#include <ext-image-copy-capture-v1-client-protocol.h>
#include <linux-dmabuf-v1-client-protocol.h>
#include <wayland-client.h>
#include <xdg-output-unstable-v1-client-protocol.h>

using namespace Lingmo;

struct WaylandCapture::Toplevel {
    ext_foreign_toplevel_handle_v1 *handle = nullptr;
    CaptureToplevel pending;
    CaptureToplevel current;
    bool done = false;
    bool closed = false;
};

struct WaylandCapture::Output {
    wl_output *proxy = nullptr;
    zxdg_output_v1 *xdgOutput = nullptr;
    uint32_t global = 0;
    CaptureOutput pending;
    CaptureOutput current;
    bool done = false;
};

static const wl_output_listener outputListener = {
        [](void *data, wl_output *, int32_t x, int32_t y, int32_t, int32_t, int32_t, const char *,
           const char *, int32_t) {
            auto *output = static_cast<WaylandCapture::Output *>(data);
            if (!output->xdgOutput)
                output->pending.logicalPosition = base::Point(x, y);
        },
        [](void *data, wl_output *, uint32_t flags, int32_t width, int32_t height,
           int32_t refresh) {
            auto *output = static_cast<WaylandCapture::Output *>(data);
            if (!(flags & WL_OUTPUT_MODE_CURRENT))
                return;
            output->pending.refreshMilliHz = refresh;
            if (!output->xdgOutput)
                output->pending.logicalSize = base::Size(width, height);
        },
        [](void *data, wl_output *) {
            auto *output = static_cast<WaylandCapture::Output *>(data);
            output->current = output->pending;
            output->done = true;
        },
        [](void *, wl_output *, int32_t) {},
        [](void *data, wl_output *, const char *name) {
            static_cast<WaylandCapture::Output *>(data)->pending.name = base::strings::sanitizedUtf8(name);
        },
        [](void *data, wl_output *, const char *description) {
            static_cast<WaylandCapture::Output *>(data)->pending.description =
                    base::strings::sanitizedUtf8(description);
        }};

static const zxdg_output_v1_listener xdgOutputListener = {
        [](void *data, zxdg_output_v1 *, int32_t x, int32_t y) {
            static_cast<WaylandCapture::Output *>(data)->pending.logicalPosition = base::Point(x, y);
        },
        [](void *data, zxdg_output_v1 *, int32_t width, int32_t height) {
            static_cast<WaylandCapture::Output *>(data)->pending.logicalSize = base::Size(width, height);
        },
        // Version 3 folds this into wl_output.done.
        [](void *, zxdg_output_v1 *) {},
        [](void *, zxdg_output_v1 *, const char *) {},
        [](void *, zxdg_output_v1 *, const char *) {}};

// wl_output_destroy sends nothing, which leaves the binding in the compositor.
static void releaseOutput(wl_output *output)
{
    if (wl_output_get_version(output) >= WL_OUTPUT_RELEASE_SINCE_VERSION)
        wl_output_release(output);
    else
        wl_output_destroy(output);
}

struct WaylandCaptureEvents {
    static void global(void *data, wl_registry *, uint32_t name, const char *interface,
                       uint32_t version)
    {
        static_cast<WaylandCapture *>(data)->handleGlobal(name, interface, version);
    }

    static void globalRemove(void *data, wl_registry *, uint32_t name)
    {
        static_cast<WaylandCapture *>(data)->handleGlobalRemove(name);
    }

    static void toplevel(void *data, ext_foreign_toplevel_list_v1 *,
                         ext_foreign_toplevel_handle_v1 *handle)
    {
        static_cast<WaylandCapture *>(data)->addToplevel(handle);
    }

    static void finished(void *, ext_foreign_toplevel_list_v1 *) {}

    static void closed(void *data, ext_foreign_toplevel_handle_v1 *)
    {
        auto *toplevel = static_cast<WaylandCapture::Toplevel *>(data);
        toplevel->closed = true;
    }

    static void done(void *data, ext_foreign_toplevel_handle_v1 *)
    {
        auto *toplevel = static_cast<WaylandCapture::Toplevel *>(data);
        toplevel->current = toplevel->pending;
        toplevel->done = true;
    }

    static void title(void *data, ext_foreign_toplevel_handle_v1 *, const char *title)
    {
        static_cast<WaylandCapture::Toplevel *>(data)->pending.title = base::strings::sanitizedUtf8(title);
    }

    static void appId(void *data, ext_foreign_toplevel_handle_v1 *, const char *appId)
    {
        static_cast<WaylandCapture::Toplevel *>(data)->pending.appId = base::strings::sanitizedUtf8(appId);
    }

    static void identifier(void *data, ext_foreign_toplevel_handle_v1 *, const char *identifier)
    {
        static_cast<WaylandCapture::Toplevel *>(data)->pending.identifier =
                base::strings::sanitizedUtf8(identifier);
    }
};

struct CaptureSessionEvents {
    static void bufferSize(void *data, ext_image_copy_capture_session_v1 *, uint32_t width,
                           uint32_t height)
    {
        auto *session = static_cast<CaptureSession *>(data);
        session->m_pendingSize = width > 0 && height > 0 && width <= 16384 && height <= 16384
                ? base::Size(int(width), int(height))
                : base::Size();
        session->m_pendingArgb = false;
    }

    static void shmFormat(void *data, ext_image_copy_capture_session_v1 *, uint32_t format)
    {
        if (format == WL_SHM_FORMAT_ARGB8888)
            static_cast<CaptureSession *>(data)->m_pendingArgb = true;
    }

    static void dmabufDevice(void *data, ext_image_copy_capture_session_v1 *, wl_array *device)
    {
        auto *session = static_cast<CaptureSession *>(data);
        session->m_pendingDmabufDevice = 0;
        if (device->size == sizeof(dev_t))
            std::memcpy(&session->m_pendingDmabufDevice, device->data, sizeof(dev_t));
        session->m_pendingDmabufFormats.clear();
    }

    static void dmabufFormat(void *data, ext_image_copy_capture_session_v1 *, uint32_t format,
                             wl_array *modifiers)
    {
        auto *session = static_cast<CaptureSession *>(data);
        const auto *begin = static_cast<const uint64_t *>(modifiers->data);
        session->m_pendingDmabufFormats[format].assign(
                begin, begin + modifiers->size / sizeof(uint64_t));
    }

    static void done(void *data, ext_image_copy_capture_session_v1 *)
    {
        auto *session = static_cast<CaptureSession *>(data);
        session->m_size = session->m_pendingSize;
        session->m_argb = session->m_pendingArgb;
        session->m_dmabufDevice = session->m_pendingDmabufDevice;
        session->m_dmabufFormats = session->m_pendingDmabufFormats;
        if (auto callback = session->onConstraintsChanged)
            callback();
    }

    static void stopped(void *data, ext_image_copy_capture_session_v1 *)
    {
        auto *session = static_cast<CaptureSession *>(data);
        if (session->m_stopped)
            return;
        session->m_stopped = true;
        if (auto callback = session->onStopped)
            callback();
    }

    static void transform(void *data, ext_image_copy_capture_frame_v1 *, uint32_t transform)
    {
        static_cast<CaptureSession *>(data)->m_transform = transform;
    }

    static void damage(void *data, ext_image_copy_capture_frame_v1 *, int32_t x, int32_t y,
                       int32_t width, int32_t height)
    {
        static_cast<CaptureSession *>(data)->m_damage.emplace_back(x, y, width, height);
    }

    static void presentationTime(void *data, ext_image_copy_capture_frame_v1 *, uint32_t hi,
                                 uint32_t lo, uint32_t nsec)
    {
        static_cast<CaptureSession *>(data)->m_presentationNs =
                ((uint64_t(hi) << 32) | lo) * 1000000000ull + nsec;
    }

    static void ready(void *data, ext_image_copy_capture_frame_v1 *)
    {
        auto *session = static_cast<CaptureSession *>(data);
        ext_image_copy_capture_frame_v1_destroy(session->m_frame);
        session->m_frame = nullptr;
        const std::vector<base::Rect> damage = std::exchange(session->m_damage, {});
        if (auto callback = session->onFrameReady)
            callback(session->m_presentationNs, session->m_transform, damage);
    }

    static void failed(void *data, ext_image_copy_capture_frame_v1 *, uint32_t reason)
    {
        auto *session = static_cast<CaptureSession *>(data);
        ext_image_copy_capture_frame_v1_destroy(session->m_frame);
        session->m_frame = nullptr;
        session->m_damage.clear();
        if (auto callback = session->onFrameFailed)
            callback(reason);
    }
};

static const wl_registry_listener registryListener = {WaylandCaptureEvents::global,
                                                      WaylandCaptureEvents::globalRemove};

static const ext_foreign_toplevel_list_v1_listener toplevelListListener = {
        WaylandCaptureEvents::toplevel, WaylandCaptureEvents::finished};

static const ext_foreign_toplevel_handle_v1_listener toplevelListener = {
        WaylandCaptureEvents::closed, WaylandCaptureEvents::done, WaylandCaptureEvents::title,
        WaylandCaptureEvents::appId, WaylandCaptureEvents::identifier};

static const ext_image_copy_capture_session_v1_listener sessionListener = {
        CaptureSessionEvents::bufferSize,   CaptureSessionEvents::shmFormat,
        CaptureSessionEvents::dmabufDevice, CaptureSessionEvents::dmabufFormat,
        CaptureSessionEvents::done,         CaptureSessionEvents::stopped};

static const ext_image_copy_capture_frame_v1_listener frameListener = {
        CaptureSessionEvents::transform, CaptureSessionEvents::damage,
        CaptureSessionEvents::presentationTime, CaptureSessionEvents::ready,
        CaptureSessionEvents::failed};

WaylandCapture::WaylandCapture()
    : m_eventLoop(base::EventLoop::current())
{
}

WaylandCapture::~WaylandCapture()
{
    disconnect();
}

bool WaylandCapture::connectToDisplay()
{
    if (m_display)
        return true;
    m_display = wl_display_connect(nullptr);
    if (!m_display) {
        LM_WARNING(base::LogCategory::defaultCategory())
                << "Cannot connect to the Wayland display for screen capture";
        return false;
    }
    m_registry = wl_display_get_registry(m_display);
    wl_registry_add_listener(m_registry, &registryListener, this);
    // Globals, then the output events the bindings trigger.
    if (wl_display_roundtrip(m_display) < 0 || wl_display_roundtrip(m_display) < 0) {
        disconnect();
        return false;
    }
    m_watch = m_eventLoop->watch(wl_display_get_fd(m_display), EPOLLIN, [this](uint32_t) { dispatch(); });
    return true;
}

bool WaylandCapture::captureAvailable() const
{
    return m_display && m_shm && m_sources && m_copies;
}

void WaylandCapture::handleGlobal(uint32_t name, const char *interface, uint32_t version)
{
    if (!strcmp(interface, wl_output_interface.name)) {
        auto output = std::make_unique<Output>();
        output->global = name;
        output->proxy = static_cast<wl_output *>(
                wl_registry_bind(m_registry, name, &wl_output_interface, std::min(version, 4u)));
        wl_output_add_listener(output->proxy, &outputListener, output.get());
        m_outputs.push_back(std::move(output));
        bindXdgOutputs();
    } else if (!strcmp(interface, wl_shm_interface.name)) {
        m_shm = static_cast<wl_shm *>(wl_registry_bind(m_registry, name, &wl_shm_interface, 1));
    } else if (!strcmp(interface, zxdg_output_manager_v1_interface.name)) {
        m_xdgOutputs = static_cast<zxdg_output_manager_v1 *>(wl_registry_bind(
                m_registry, name, &zxdg_output_manager_v1_interface, std::min(version, 3u)));
        bindXdgOutputs();
    } else if (!strcmp(interface, ext_output_image_capture_source_manager_v1_interface.name)) {
        m_sources = static_cast<ext_output_image_capture_source_manager_v1 *>(wl_registry_bind(
                m_registry, name, &ext_output_image_capture_source_manager_v1_interface, 1));
    } else if (!strcmp(interface, ext_image_copy_capture_manager_v1_interface.name)) {
        m_copies = static_cast<ext_image_copy_capture_manager_v1 *>(
                wl_registry_bind(m_registry, name, &ext_image_copy_capture_manager_v1_interface, 1));
    } else if (!strcmp(interface,
                       ext_foreign_toplevel_image_capture_source_manager_v1_interface.name)) {
        m_toplevelSources =
                static_cast<ext_foreign_toplevel_image_capture_source_manager_v1 *>(wl_registry_bind(
                        m_registry, name,
                        &ext_foreign_toplevel_image_capture_source_manager_v1_interface, 1));
    } else if (!strcmp(interface, ext_foreign_toplevel_list_v1_interface.name)) {
        m_toplevelList = static_cast<ext_foreign_toplevel_list_v1 *>(
                wl_registry_bind(m_registry, name, &ext_foreign_toplevel_list_v1_interface, 1));
        ext_foreign_toplevel_list_v1_add_listener(m_toplevelList, &toplevelListListener, this);
    } else if (!strcmp(interface, zwp_linux_dmabuf_v1_interface.name)) {
        // Only create_params is used; the compositor's own capture constraints
        // say which formats to allocate, so no feedback object is needed.
        m_dmabuf = static_cast<zwp_linux_dmabuf_v1 *>(wl_registry_bind(
                m_registry, name, &zwp_linux_dmabuf_v1_interface, std::min(version, 4u)));
    }
}

void WaylandCapture::addToplevel(ext_foreign_toplevel_handle_v1 *handle)
{
    // A closed window's handle is dropped once a new one arrives.
    for (auto it = m_toplevels.begin(); it != m_toplevels.end();) {
        if ((*it)->closed) {
            ext_foreign_toplevel_handle_v1_destroy((*it)->handle);
            it = m_toplevels.erase(it);
        } else {
            ++it;
        }
    }
    auto toplevel = std::make_unique<Toplevel>();
    toplevel->handle = handle;
    ext_foreign_toplevel_handle_v1_add_listener(handle, &toplevelListener, toplevel.get());
    m_toplevels.push_back(std::move(toplevel));
}

std::vector<CaptureToplevel> WaylandCapture::toplevels() const
{
    std::vector<CaptureToplevel> result;
    for (const auto &toplevel : m_toplevels) {
        if (toplevel->done && !toplevel->closed && !toplevel->current.identifier.empty())
            result.push_back(toplevel->current);
    }
    return result;
}

void WaylandCapture::handleGlobalRemove(uint32_t name)
{
    const auto it = std::find_if(m_outputs.begin(), m_outputs.end(), [name](const auto &output) {
        return output->global == name;
    });
    if (it == m_outputs.end())
        return;
    if ((*it)->xdgOutput)
        zxdg_output_v1_destroy((*it)->xdgOutput);
    releaseOutput((*it)->proxy);
    m_outputs.erase(it);
}

void WaylandCapture::bindXdgOutputs()
{
    if (!m_xdgOutputs)
        return;
    for (auto &output : m_outputs) {
        if (output->xdgOutput)
            continue;
        output->xdgOutput = zxdg_output_manager_v1_get_xdg_output(m_xdgOutputs, output->proxy);
        zxdg_output_v1_add_listener(output->xdgOutput, &xdgOutputListener, output.get());
    }
}

std::vector<CaptureOutput> WaylandCapture::outputs() const
{
    std::vector<CaptureOutput> result;
    for (const auto &output : m_outputs) {
        if (output->done && !output->current.name.empty())
            result.push_back(output->current);
    }
    return result;
}

std::unique_ptr<CaptureSession> WaylandCapture::createSession(const std::string &name, bool cursor)
{
    if (!captureAvailable())
        return nullptr;
    const auto it = std::find_if(m_outputs.begin(), m_outputs.end(), [&](const auto &output) {
        return output->done && output->current.name == name;
    });
    if (it == m_outputs.end())
        return nullptr;
    return makeSession(
            ext_output_image_capture_source_manager_v1_create_source(m_sources, (*it)->proxy),
            cursor);
}

std::unique_ptr<CaptureSession> WaylandCapture::createToplevelSession(const std::string &identifier,
                                                                      bool cursor)
{
    if (!captureAvailable() || !m_toplevelSources)
        return nullptr;
    const auto it = std::find_if(m_toplevels.begin(), m_toplevels.end(), [&](const auto &toplevel) {
        return toplevel->done && !toplevel->closed && toplevel->current.identifier == identifier;
    });
    if (it == m_toplevels.end())
        return nullptr;
    return makeSession(ext_foreign_toplevel_image_capture_source_manager_v1_create_source(
                               m_toplevelSources, (*it)->handle),
                       cursor);
}

std::unique_ptr<CaptureSession> WaylandCapture::makeSession(ext_image_capture_source_v1 *source,
                                                            bool cursor)
{
    auto *session = ext_image_copy_capture_manager_v1_create_session(
            m_copies, source, cursor ? EXT_IMAGE_COPY_CAPTURE_MANAGER_V1_OPTIONS_PAINT_CURSORS : 0);
    // The session keeps its own reference to the source.
    ext_image_capture_source_v1_destroy(source);
    std::unique_ptr<CaptureSession> result(new CaptureSession(this, session));
    ext_image_copy_capture_session_v1_add_listener(session, &sessionListener, result.get());
    flush();
    return result;
}

namespace {

struct PendingDmaBuf {
    std::function<void(wl_buffer *)> done;
    std::vector<zwp_linux_buffer_params_v1 *> *pendingParams;
};

} // namespace

// Takes the import out of the pending list and frees its proxy and record,
// handing back the callback to run.
static std::function<void(wl_buffer *)> finishPending(void *data, zwp_linux_buffer_params_v1 *params)
{
    auto *pending = static_cast<PendingDmaBuf *>(data);
    auto &list = *pending->pendingParams;
    list.erase(std::remove(list.begin(), list.end(), params), list.end());
    zwp_linux_buffer_params_v1_destroy(params);
    auto done = std::move(pending->done);
    delete pending;
    return done;
}

static const zwp_linux_buffer_params_v1_listener paramsListener = {
        [](void *data, zwp_linux_buffer_params_v1 *params, wl_buffer *buffer) {
            finishPending(data, params)(buffer);
        },
        [](void *data, zwp_linux_buffer_params_v1 *params) {
            finishPending(data, params)(nullptr);
        }};

bool WaylandCapture::createDmaBufBuffer(const base::Size &size, uint32_t format, uint64_t modifier,
                                        const std::vector<DmaBufPlane> &planes,
                                        std::function<void(wl_buffer *)> done)
{
    if (!m_dmabuf || planes.empty())
        return false;
    auto *params = zwp_linux_dmabuf_v1_create_params(m_dmabuf);
    for (size_t index = 0; index < planes.size(); ++index) {
        zwp_linux_buffer_params_v1_add(params, planes[index].fd, uint32_t(index),
                                       planes[index].offset, planes[index].stride,
                                       uint32_t(modifier >> 32), uint32_t(modifier & 0xffffffff));
    }
    zwp_linux_buffer_params_v1_add_listener(params, &paramsListener,
                                            new PendingDmaBuf {std::move(done), &m_pendingParams});
    m_pendingParams.push_back(params);
    zwp_linux_buffer_params_v1_create(params, size.width(), size.height(), format, 0);
    flush();
    return true;
}

wl_buffer *WaylandCapture::createShmBuffer(int fd, const base::Size &size, int stride)
{
    if (!m_shm)
        return nullptr;
    auto *pool = wl_shm_create_pool(m_shm, fd, stride * size.height());
    auto *buffer = wl_shm_pool_create_buffer(pool, 0, size.width(), size.height(), stride,
                                             WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);
    return buffer;
}

void WaylandCapture::flush()
{
    if (m_display && wl_display_flush(m_display) < 0 && errno != EAGAIN)
        scheduleDisconnect();
}

void WaylandCapture::scheduleDisconnect()
{
    if (m_disconnectPending || !m_display)
        return;
    m_disconnectPending = true;
    later(m_alive, [this] { disconnect(); });
}

void WaylandCapture::dispatch()
{
    if (!m_display || m_dispatching)
        return;
    m_dispatching = true;
    bool failed = false;
    while (wl_display_prepare_read(m_display) != 0) {
        if (wl_display_dispatch_pending(m_display) < 0) {
            failed = true;
            break;
        }
    }
    if (!failed) {
        wl_display_flush(m_display);
        failed = wl_display_read_events(m_display) < 0
                || wl_display_dispatch_pending(m_display) < 0;
    }
    m_dispatching = false;
    if (failed)
        disconnect();
    else
        flush();
}

void WaylandCapture::disconnect()
{
    m_disconnectPending = false;
    if (!m_display)
        return;
    LM_WARNING(base::LogCategory::defaultCategory()) << "Lost the Wayland display used for screen capture";
    // An import the compositor will never answer fails now, while its
    // session is still whole.
    const auto pending = std::exchange(m_pendingParams, {});
    for (zwp_linux_buffer_params_v1 *params : pending) {
        auto *record = static_cast<PendingDmaBuf *>(
                wl_proxy_get_user_data(reinterpret_cast<wl_proxy *>(params)));
        finishPending(record, params)(nullptr);
    }
    // Sessions release their proxies while the display still exists.
    disconnected.emit();
    if (m_watch) {
        m_eventLoop->unwatch(m_watch);
        m_watch = 0;
    }
    for (auto &output : m_outputs) {
        if (output->xdgOutput)
            zxdg_output_v1_destroy(output->xdgOutput);
        releaseOutput(output->proxy);
    }
    m_outputs.clear();
    for (auto &toplevel : m_toplevels)
        ext_foreign_toplevel_handle_v1_destroy(toplevel->handle);
    m_toplevels.clear();
    if (m_toplevelList)
        ext_foreign_toplevel_list_v1_destroy(m_toplevelList);
    if (m_toplevelSources)
        ext_foreign_toplevel_image_capture_source_manager_v1_destroy(m_toplevelSources);
    if (m_dmabuf)
        zwp_linux_dmabuf_v1_destroy(m_dmabuf);
    if (m_copies)
        ext_image_copy_capture_manager_v1_destroy(m_copies);
    if (m_sources)
        ext_output_image_capture_source_manager_v1_destroy(m_sources);
    if (m_xdgOutputs)
        zxdg_output_manager_v1_destroy(m_xdgOutputs);
    if (m_shm)
        wl_shm_destroy(m_shm);
    wl_registry_destroy(m_registry);
    wl_display_disconnect(m_display);
    m_copies = nullptr;
    m_sources = nullptr;
    m_toplevelSources = nullptr;
    m_toplevelList = nullptr;
    m_dmabuf = nullptr;
    m_xdgOutputs = nullptr;
    m_shm = nullptr;
    m_registry = nullptr;
    m_display = nullptr;
}

CaptureSession::CaptureSession(WaylandCapture *wayland, ext_image_copy_capture_session_v1 *session)
    : m_wayland(wayland)
    , m_session(session)
{
    m_disconnected = wayland->disconnected.connect([this] {
        if (m_frame)
            ext_image_copy_capture_frame_v1_destroy(m_frame);
        m_frame = nullptr;
        ext_image_copy_capture_session_v1_destroy(m_session);
        m_session = nullptr;
        if (!m_stopped) {
            m_stopped = true;
            if (auto callback = onStopped)
                callback();
        }
    });
}

CaptureSession::~CaptureSession()
{
    cancel();
    if (m_session)
        ext_image_copy_capture_session_v1_destroy(m_session);
    m_wayland->flush();
}

bool CaptureSession::capture(wl_buffer *buffer, const std::vector<base::Rect> &damage)
{
    if (!m_session || m_stopped || m_frame || !buffer)
        return false;
    m_frame = ext_image_copy_capture_session_v1_create_frame(m_session);
    ext_image_copy_capture_frame_v1_add_listener(m_frame, &frameListener, this);
    ext_image_copy_capture_frame_v1_attach_buffer(m_frame, buffer);
    if (damage.empty()) {
        ext_image_copy_capture_frame_v1_damage_buffer(m_frame, 0, 0, m_size.width(),
                                                      m_size.height());
    } else {
        for (const base::Rect &rect : damage) {
            const base::Rect clipped = rect.intersected(base::Rect(m_size));
            if (!clipped.isEmpty())
                ext_image_copy_capture_frame_v1_damage_buffer(m_frame, clipped.x(), clipped.y(),
                                                              clipped.width(), clipped.height());
        }
    }
    ext_image_copy_capture_frame_v1_capture(m_frame);
    m_wayland->flush();
    return true;
}

void CaptureSession::cancel()
{
    if (!m_frame)
        return;
    ext_image_copy_capture_frame_v1_destroy(m_frame);
    m_frame = nullptr;
    m_wayland->flush();
}
