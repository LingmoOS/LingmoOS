// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once

#include "base/dbus.h"
#include "base/geometry.h"
#include "base/signal.h"
#include "base/thread_pool.h"
#include "base/timer.h"
#include "input/cursor_shape.h"
#include "input/event_sink.h"
#include "session/secure_container.h"
#include "session/session_state.h"
#include "ui/bitmap.h"
#include <ctime>
#include <optional>
#include <functional>
#include <memory>
#include <vector>
namespace scene = Lingmo::Scene;

namespace Lingmo::Input {
class XkbTracker;
}

namespace Lingmo::Render {
class Renderer;
class Texture;
} // namespace Lingmo::Render

namespace Lingmo::Toolkit {
class PanelTexture;
}

namespace Lingmo::Scene {
class Node;
}

namespace Lingmo::ui {
class EmbeddedWindow;
class View;
} // namespace Lingmo::UI

namespace Lingmo::Session {
class SessionController;
}

namespace Lingmo::Auth {
class AuthBackend;
class AuthController;
class AuthScreen;
class AuthStatus;

class AuthUi : public input::EventSink, public session::SessionObserver
{
public:
    AuthUi(session::SessionController *session, const input::XkbTracker *xkb,
           AuthBackend *backend = nullptr);
    ~AuthUi() override;
    void onSessionStateChanged(session::SessionState state) override;
    void onSessionOutputsChanged() override;
    bool onKey(input::Timestamp, uint32_t code, bool pressed) override;
    bool onPointerMotion(input::Timestamp, const base::PointF &position) override;
    bool onPointerButton(input::Timestamp, const base::PointF &position, uint32_t button,
                         bool pressed) override;

    // The panel's toolkit window, made with the controller on the first
    // refresh with an output.
    ui::EmbeddedWindow *window() const { return m_window.get(); }
    // The view that has the keyboard, for tests.
    ui::View *focusView() const;

    AuthController *controller() const { return m_controller.get(); }

    // Whether a refresh() is queued. Nothing queues one while the panel is idle.
    bool isRefreshPending() const { return m_refresh.isActive(); }

    // The panel has its size for the output, so the next frame can show it.
    bool isPanelDrawn() const { return m_window && m_sized; }

    // Changes whenever renderImage() draws the panel anew.
    uint64_t imageGeneration() const { return m_imageGeneration; }

    // Where the pointer is, for the panel to open on the screen it is on.
    void setPointerLocator(std::function<base::PointF()> locator)
    {
        m_pointerLocator = std::move(locator);
    }

    void setKeyRepeatSettings(const input::KeyRepeatSettings &settings)
    {
        m_keyRepeat = settings;
    }

    // The password field in panel coordinates, for the backdrop blur behind it.
    base::RectF fieldBounds() const;
    double fieldOpacity() const;

    // The panel rendered on the CPU, when it changed.
    ui::Bitmap renderImage();
    // In the panel node's paint: the panel as a texture of the renderer,
    // drawn on the GPU where the renderer allows.
    render::Texture *panelTexture(render::Renderer &renderer);
    void releaseRenderResources();

    // The Wi-Fi and battery icons in the top-right corner of the panel's
    // output, in a window of their own: the panel's scales as it comes and
    // goes, they only fade with the wallpaper.
    ui::EmbeddedWindow *statusWindow() const { return m_statusWindow.get(); }
    AuthStatus *status() const { return m_status; }
    // The icons rendered on the CPU, when they changed.
    ui::Bitmap renderStatusImage();
    render::Texture *statusTexture(render::Renderer &renderer);
    void releaseStatusRenderResources();
    void refresh();
    // The cursor the session shows while it is blocked.
    base::Signal<input::CursorShape> cursorShapeChanged;

private:
    void clear();
    void scheduleRefresh();
    void refreshStatus(const session::SecureOutput &target);
    // Light or dark icons from the wallpaper under them, as the status bar
    // judges its own.
    void judgeBackdrop();
    const session::SecureOutput &
    panelOutput(const std::vector<session::SecureOutput> &outputs) const;
    bool sendKey(uint32_t code, bool pressed, bool repeat);
    void updateCursor();
    session::SessionController *m_session;
    const input::XkbTracker *m_xkb;
    AuthBackend *m_injectedBackend;
    base::Timer m_refresh;
    base::Timer m_repeatTimer;
    // logind's Lock for this session.
    base::dbus::Match m_lockRequested;
    input::KeyRepeatSettings m_keyRepeat;
    uint32_t m_repeatCode = 0;
    ui::View *m_repeatFocus = nullptr;
    base::PointF m_pointerPosition;
    std::function<base::PointF()> m_pointerLocator;
    std::string m_panelOutput;
    input::CursorShape m_cursorShape = input::CursorShape::Default;
    bool m_started = false;
    std::unique_ptr<AuthBackend> m_backend;
    std::unique_ptr<AuthController> m_controller;
    std::vector<base::Subscription> m_controllerSubscriptions;
    std::unique_ptr<ui::EmbeddedWindow> m_window;
    std::unique_ptr<toolkit::PanelTexture> m_panel;
    AuthScreen *m_screen = nullptr;
    // The cursor the panel's views ask for, shown while the session is blocked.
    input::CursorShape m_viewCursor = input::CursorShape::Default;
    double m_scale = 1;
    bool m_sized = false;
    scene::Node *m_node = nullptr;
    bool m_pointerInside = false;
    bool m_attempted = false;
    bool m_authenticated = false;
    bool m_focused = false;
    bool m_resetPending = false;
    session::SessionState m_state = session::SessionState::Active;
    bool m_dirty = true;
    uint64_t m_imageGeneration = 0;
    // Kept from lock to lock, unlike the panel.
    std::unique_ptr<ui::EmbeddedWindow> m_statusWindow;
    std::unique_ptr<toolkit::PanelTexture> m_statusPanel;
    AuthStatus *m_status = nullptr;
    scene::Node *m_statusNode = nullptr;
    double m_statusScale = 1;
    bool m_statusDirty = true;
    base::Subscription m_wallpaperLoaded;
    base::Job m_judgement;
    // What the icons were last judged against.
    std::string m_judgedOutput;
    const void *m_judgedPixels = nullptr;
    bool m_judged = false;
};
} // namespace Lingmo::Auth

