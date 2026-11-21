// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "videoview.h"
#include "mainwindow.h"
#include "mpvhandle.h"
#include "player.h"
#include "softwarevideo.h"

#include "base/logging.h"
#include "ui/application.h"
#include "ui/painter.h"
#include "ui/subsurface.h"
#include "ui/window.h"

#include <mpv/render_gl.h>

#include <chrono>
#include <cmath>

using namespace Lingmo;
using namespace Lingmo::ui;
using namespace std::chrono_literals;

static const base::LogCategory cfVideo("lingmo.videoplayer.video", base::LogLevel::Warning);

static void* procAddress(void*, const char* name)
{
    return Subsurface::getProcAddress(name);
}

VideoSurface::VideoSurface(Player& player, Window* window)
    : m_player(player)
    , m_playerWeak(player.weakPtr())
    , m_handle(player.handle())
    , m_surface(std::make_unique<Subsurface>(window, Subsurface::Stacking::Below))
{
    if (!m_surface->isValid())
        return;
    m_surface->setOpaque(true);
    m_surface->onResize = [this](base::Size) {
        {
            std::lock_guard lock(m_mutex);
            m_dirty = true;
        }
        m_wake.notify_all();
        m_surface->wakeUp();
    };
    m_selfWeak = m_weakFactory.getWeakPtr();
    m_thread = std::thread([this] { run(); });
}

VideoSurface::~VideoSurface()
{
    stopThread();
    m_surface.reset();
    if (m_contextReady && m_playerWeak)
        m_playerWeak->setRenderLost();
}

bool VideoSurface::isValid() const
{
    return m_surface && m_surface->isValid();
}

void VideoSurface::stopThread()
{
    if (!m_thread.joinable())
        return;
    {
        std::lock_guard lock(m_mutex);
        m_stopping = true;
    }
    m_wake.notify_all();
    m_surface->wakeUp();
    m_thread.join();
}

void VideoSurface::setGeometry(const RectF& rect)
{
    if (m_surface)
        m_surface->setGeometry(rect);
}

// From the thread; the surface may be gone by the time the loop runs it.
void VideoSurface::fail()
{
    m_handle->post([weak = m_selfWeak] {
        if (!weak || !weak->onFailed)
            return;
        // A copy: the handler may destroy this surface.
        const std::function<void()> failed = weak->onFailed;
        failed();
    });
}

void VideoSurface::run()
{
    if (!m_surface->makeCurrent() || !m_handle->mpv) {
        LM_WARNING(cfVideo) << "No OpenGL context for the video";
        fail();
        return;
    }
    mpv_opengl_init_params glParams{procAddress, nullptr};
    mpv_render_param display{MPV_RENDER_PARAM_INVALID, nullptr};
    // Lets hardware decoders hand frames to GL without a copy.
    if (wl_display* wayland = Subsurface::waylandDisplay())
        display = {MPV_RENDER_PARAM_WL_DISPLAY, wayland};
    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char*>(MPV_RENDER_API_TYPE_OPENGL)},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &glParams},
        display,
        {MPV_RENDER_PARAM_INVALID, nullptr},
    };
    mpv_render_context* context = nullptr;
    if (mpv_render_context_create(&context, m_handle->mpv, params) < 0) {
        LM_WARNING(cfVideo) << "mpv: could not create the OpenGL render context";
        m_surface->doneCurrent();
        fail();
        return;
    }
    mpv_render_context_set_update_callback(
            context,
            [](void* data) {
                auto* self = static_cast<VideoSurface*>(data);
                {
                    std::lock_guard lock(self->m_mutex);
                    self->m_dirty = true;
                }
                self->m_wake.notify_all();
            },
            this);
    m_contextReady = true;
    // Files load once the output can draw.
    m_handle->post([player = m_playerWeak] {
        if (player)
            player->setRenderReady();
    });

    for (;;) {
        {
            std::unique_lock lock(m_mutex);
            m_wake.wait(lock, [this] { return m_stopping || m_dirty; });
            if (m_stopping)
                break;
            m_dirty = false;
        }
        mpv_render_context_update(context);
        const base::Size size = m_surface->beginFrame();
        if (size.width() > 0 && size.height() > 0) {
            mpv_opengl_fbo fbo{0, size.width(), size.height(), 0};
            int flipY = 1;
            int block = 0;
            mpv_render_param render[] = {
                {MPV_RENDER_PARAM_OPENGL_FBO, &fbo},
                {MPV_RENDER_PARAM_FLIP_Y, &flipY},
                {MPV_RENDER_PARAM_BLOCK_FOR_TARGET_TIME, &block},
                {MPV_RENDER_PARAM_INVALID, nullptr},
            };
            mpv_render_context_render(context, render);
            m_surface->endFrame();
            mpv_render_context_report_swap(context);
            // Paced by the compositor's frame callbacks.
            m_surface->waitForFrame(100);
        }
    }
    releaseOutput(context);
    mpv_render_context_free(context);
    m_surface->doneCurrent();
}

// Freeing the context takes mpv's video output down, but an output in the
// middle of a file waits up to 200 ms for the frame it handed over last. An
// unloaded file takes it down while this thread still takes its frames.
void VideoSurface::releaseOutput(mpv_render_context* context)
{
    const char* stop[] = {"stop", nullptr};
    if (mpv_command(m_handle->mpv, stop) < 0)
        return;
    const auto deadline = std::chrono::steady_clock::now() + 1s;
    while (std::chrono::steady_clock::now() < deadline) {
        int configured = 0;
        if (mpv_get_property(m_handle->mpv, "vo-configured", MPV_FORMAT_FLAG, &configured) < 0 || !configured)
            return;
        {
            std::unique_lock lock(m_mutex);
            if (!m_wake.wait_for(lock, 5ms, [this] { return m_dirty; }))
                continue;
            m_dirty = false;
        }
        if (mpv_render_context_update(context) & MPV_RENDER_UPDATE_FRAME) {
            int skip = 1;
            mpv_render_param render[] = {
                {MPV_RENDER_PARAM_SKIP_RENDERING, &skip},
                {MPV_RENDER_PARAM_INVALID, nullptr},
            };
            mpv_render_context_render(context, render);
            mpv_render_context_report_swap(context);
        }
    }
}

VideoView::VideoView(MainWindow& owner)
    : m_owner(owner)
{
    setName("player");
}

VideoView::~VideoView() = default;

void VideoView::attach()
{
    if (m_surface || m_software)
        return;
    const ui::Application* application = ui::Application::instance();
    if (application && application->platform() == ui::Application::Platform::Wayland && window()) {
        auto surface = std::make_unique<VideoSurface>(m_owner.player(), window());
        if (surface->isValid()) {
            m_surface = std::move(surface);
            m_surface->onFailed = [this] {
                m_surface.reset();
                attachSoftware();
            };
            m_surface->setGeometry(windowRect());
            return;
        }
    }
    attachSoftware();
}

void VideoView::attachSoftware()
{
    m_software = std::make_unique<SoftwareVideo>(m_owner.player());
    m_frames = m_software->frameDue.connect([this] { requestPaint(); });
    m_software->ensureContext();
    requestPaint();
}

std::unique_ptr<VideoSurface> VideoView::takeSurface()
{
    if (m_surface)
        m_surface->onFailed = nullptr;
    return std::move(m_surface);
}

RectF VideoView::windowRect() const
{
    const PointF origin = mapToWindow({0, 0});
    return {origin.x, origin.y, geometry().width, geometry().height};
}

void VideoView::layout()
{
    if (m_surface)
        m_surface->setGeometry(windowRect());
}

void VideoView::paint(Painter& painter)
{
    if (!m_software || !m_software->hasContext())
        return;
    const RectF rect = localRect();
    const int width = std::max(1, int(std::lround(rect.width * painter.scale())));
    const int height = std::max(1, int(std::lround(rect.height * painter.scale())));
    painter.drawImage(m_software->render(width, height), rect);
}
