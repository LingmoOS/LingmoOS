// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "auth/auth_ui.h"
#include "core/main_loop.h"
#include "auth/auth_controller.h"
#include "auth/auth_status.h"
#include "auth/auth_view.h"
#include "auth/greetd_backend.h"
#include "auth/login_sessions.h"
#include "auth/pam_backend.h"
#include "core/logging.h"
#include "input/xkb_tracker.h"
#include "scene/node.h"
#include "session/session_controller.h"
#include "shell/wallpaper/bar_backdrop.h"
#include "toolkit/toolkit_host.h"
#include "ui/embedded_window.h"
#include "ui/theme.h"
#include <algorithm>
#include <cmath>
#include <linux/input-event-codes.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon.h>
namespace scene = Lingmo::Scene;

namespace Lingmo::Auth {
namespace {
class AuthNode final : public scene::Node
{
public:
    AuthNode(AuthUi *ui, const session::SecureContainer *secure)
        : Node("auth-panel")
        , m_ui(ui)
        , m_secure(secure)
    {
    }

    Content content() const override { return Content::Translucent; }

    void releaseRenderResources() override { m_ui->releaseRenderResources(); }

protected:
    void paint(render::Renderer &renderer, const base::Rect &geometry) const override
    {
        // The lock transition draws every frame; the panel is rendered only
        // when it changed.
        render::Texture *texture = m_ui->panelTexture(renderer);
        if (!texture)
            return;
        const session::CoverSample sample = m_secure->sample(renderer);
        if (sample.panelOpacity <= 0)
            return;
        const base::PointF center = base::RectF(geometry).center();
        base::AffineTransform transform;
        transform.translate(center.x(), center.y());
        transform.scale(sample.panelScale, sample.panelScale);
        transform.translate(-center.x(), -center.y());
        // The blur behind the password field fades and scales with the panel.
        if (const base::RectF field = m_ui->fieldBounds(); !field.isEmpty()) {
            render::GlassParams fade;
            fade.opacity = sample.panelOpacity * m_ui->fieldOpacity();
            const base::RectF blurred =
                    transform.mapRect(field.translated(base::PointF(geometry.origin())));
            renderer.drawBackgroundBlur(blurred.toEnclosingRect(),
                                        base::roundToInt(blurred.height() / 2), geometry, fade);
        }
        // Mapped at rest, so its device-density texels land on device pixels.
        if (sample.panelScale == 1) {
            renderer.drawTextureMapped(texture, geometry, sample.panelOpacity, base::AffineTransform());
            return;
        }
        renderer.drawTextureTransformed(texture, geometry, transform, sample.panelOpacity);
    }

private:
    AuthUi *m_ui;
    const session::SecureContainer *m_secure;
};

// The Wi-Fi and battery icons: they fade with the wallpaper and do not scale,
// as they belong to the screen's edge rather than to the panel.
class StatusNode final : public scene::Node
{
public:
    StatusNode(AuthUi *ui, const session::SecureContainer *secure)
        : Node("auth-status")
        , m_ui(ui)
        , m_secure(secure)
    {
    }

    Content content() const override { return Content::Translucent; }

    void releaseRenderResources() override { m_ui->releaseStatusRenderResources(); }

protected:
    void paint(render::Renderer &renderer, const base::Rect &geometry) const override
    {
        render::Texture *texture = m_ui->statusTexture(renderer);
        if (!texture)
            return;
        const float opacity = m_secure->sample(renderer).backdrop;
        if (opacity <= 0)
            return;
        renderer.drawTextureMapped(texture, geometry, opacity, base::AffineTransform());
    }

private:
    AuthUi *m_ui;
    const session::SecureContainer *m_secure;
};

input::CursorShape sessionCursor(ui::CursorShape shape)
{
    switch (shape) {
    case ui::CursorShape::Text:
        return input::CursorShape::Text;
    case ui::CursorShape::Pointer:
        return input::CursorShape::Pointer;
    default:
        return input::CursorShape::Default;
    }
}

uint32_t toolkitModifiers(const input::XkbTracker *xkb)
{
    uint32_t modifiers = ui::NoModifier;
    if (xkb->modifierActive(XKB_MOD_NAME_SHIFT))
        modifiers |= ui::ShiftModifier;
    if (xkb->modifierActive(XKB_MOD_NAME_CTRL))
        modifiers |= ui::ControlModifier;
    if (xkb->modifierActive(XKB_MOD_NAME_ALT))
        modifiers |= ui::AltModifier;
    if (xkb->modifierActive(XKB_MOD_NAME_LOGO))
        modifiers |= ui::SuperModifier;
    return modifiers;
}
} // namespace

AuthUi::AuthUi(session::SessionController *session, const input::XkbTracker *xkb,
               AuthBackend *backend)
    : m_session(session)
    , m_xkb(xkb)
    , m_injectedBackend(backend)
    , m_refresh(core::mainLoop())
    , m_repeatTimer(core::mainLoop())
{
    // Before the observer is added: adding it reports the state, which
    // schedules a refresh.
    m_refresh.setSingleShot(true);
    m_refresh.onTimeout = [this] { refresh(); };
    m_session->addObserver(this);
    m_session->setAuthInput(this);
    scheduleRefresh();
    m_repeatTimer.onTimeout = [this] {
        if (!m_session->isBlocked() || !m_window || !m_repeatFocus
            || m_window->focusView() != m_repeatFocus || !m_repeatFocus->isEnabled()
            || m_session->state() == session::SessionState::Unlocking) {
            m_repeatTimer.stop();
            return;
        }
        m_repeatTimer.start(m_keyRepeat.interval);
        sendKey(m_repeatCode, true, true);
    };
    // logind authenticates the sender and identifies this compositor's session.
    if (base::dbus::Connection *bus = base::dbus::Connection::system(core::mainLoop())) {
        const base::dbus::Proxy manager(bus, "org.freedesktop.login1", "/org/freedesktop/login1",
                                        "org.freedesktop.login1.Manager");
        const std::optional<base::dbus::ObjectPath> path =
                manager.call("GetSessionByPID", uint32_t(getpid())).value<base::dbus::ObjectPath>();
        if (path) {
            const base::dbus::Proxy logindSession(bus, "org.freedesktop.login1", path->path,
                                                  "org.freedesktop.login1.Session");
            m_lockRequested = logindSession.onSignal(
                    "Lock", [this](const base::dbus::Message &) { m_session->requestLock(); });
        }
    }
}

AuthUi::~AuthUi()
{
    m_session->setAuthInput(nullptr);
    m_session->removeObserver(this);
    clear();
    m_judgement.cancel();
    m_wallpaperLoaded.reset();
    if (m_statusNode) {
        m_statusNode->parent()->removeChild(m_statusNode);
        m_statusNode = nullptr;
    }
    m_status = nullptr;
    m_statusPanel.reset();
    toolkit::destroyWindow(m_statusWindow);
}

void AuthUi::onSessionStateChanged(session::SessionState state)
{
    if (state == session::SessionState::Active || state == session::SessionState::Unlocking) {
        m_repeatTimer.stop();
        m_cursorShape = input::CursorShape::Default;
        cursorShapeChanged(m_cursorShape);
    }
    // A lock that cancelled an unlock needs a fresh authentication.
    if (state == session::SessionState::Active
        || (state == session::SessionState::Locking
            && m_state == session::SessionState::Unlocking)) {
        m_authenticated = false;
        m_resetPending = true;
    }
    m_state = state;
    // Authentication signals may still be on the stack when state changes.
    scheduleRefresh();
}

void AuthUi::onSessionOutputsChanged()
{
    scheduleRefresh();
}

void AuthUi::scheduleRefresh()
{
    if (!m_refresh.isActive())
        m_refresh.start(std::chrono::nanoseconds(0));
}

void AuthUi::clear()
{
    m_repeatTimer.stop();
    m_repeatFocus = nullptr;
    m_started = false;
    if (m_node) {
        m_node->parent()->removeChild(m_node);
        m_node = nullptr;
    }
    m_screen = nullptr;
    m_panel.reset();
    toolkit::destroyWindow(m_window);
    m_controllerSubscriptions.clear();
    m_controller.reset();
    m_backend.reset();
    m_sized = false;
    m_pointerInside = false;
    m_viewCursor = input::CursorShape::Default;
    m_attempted = false;
    m_authenticated = false;
}

ui::View *AuthUi::focusView() const
{
    return m_window ? m_window->focusView() : nullptr;
}

void AuthUi::refresh()
{
    if (m_resetPending) {
        clear();
        m_resetPending = false;
    }
    if (m_authenticated && m_session->state() == session::SessionState::Locked)
        m_session->requestUnlock();
    if (m_session->state() == session::SessionState::Unlocking)
        return;
    auto *secure = m_session->secureContainer();
    if (!secure || secure->outputs().empty()) {
        if (m_node)
            m_node->setVisible(false);
        if (m_statusNode)
            m_statusNode->setVisible(false);
        return;
    }
    if (!m_attempted) {
        m_attempted = true;
        const bool login = m_session->state() == session::SessionState::Login;
        if (!m_injectedBackend) {
            if (login)
                m_backend = std::make_unique<GreetdBackend>();
            else
                m_backend = std::make_unique<PamBackend>();
        }
        m_controller = std::make_unique<AuthController>(
                login ? AuthController::Login : AuthController::Lock,
                m_injectedBackend ? m_injectedBackend : m_backend.get());
        m_controllerSubscriptions.push_back(
                m_controller->stateChanged.connect([this] { m_repeatTimer.stop(); }));
        // The login screen shows the wallpaper of the account picked.
        if (login) {
            const auto showBackground = [this, secure](bool crossfade) {
                if (!m_controller->typesUserName() && !m_controller->userName().empty())
                    secure->setSettingsFile(loginBackgroundFile(m_controller->userName()), crossfade);
            };
            showBackground(false);
            m_controllerSubscriptions.push_back(
                    m_controller->userChanged.connect([showBackground] { showBackground(true); }));
        }
        m_controllerSubscriptions.push_back(
                m_controller->authenticationSucceeded.connect([this, login] {
                    // greetd owns login handoff; the greeter stays covered until it exits.
                    if (!login && !m_resetPending) {
                        m_authenticated = true;
                        m_session->requestUnlock();
                    }
                }));
        toolkit::application();
        m_window = std::make_unique<ui::EmbeddedWindow>();
        auto screen = std::make_unique<AuthScreen>(m_window.get(), m_controller.get());
        m_screen = screen.get();
        m_window->setContentView(std::move(screen));
        m_screen->focusPassword();
        m_panel = std::make_unique<toolkit::PanelTexture>(m_window.get());
        // While the session is active the panel only has to be ready for the
        // next lock: drawn once. Later changes wait for the first locked frame.
        m_window->onRenderRequested = [this] {
            m_dirty = true;
            if (m_session->isBlocked() || m_panel->image().isNull())
                scheduleRefresh();
        };
        m_window->onCursorShapeChanged = [this](ui::CursorShape shape) {
            m_viewCursor = sessionCursor(shape);
            if (m_session->isBlocked())
                updateCursor();
        };
        m_focused = false;
        m_node = secure->authContainer()->addChild(std::make_unique<AuthNode>(this, secure));
        // The panel stays first; the icons kept from the last lock come after it.
        secure->authContainer()->lower(m_node);
    }
    if (m_session->isBlocked() && !m_started) {
        m_started = true;
        m_controller->start();
    }
    // A focused field blinks its caret, which would wake the compositor twice
    // a second for a panel no one can see.
    if (m_focused != m_session->isBlocked()) {
        m_focused = m_session->isBlocked();
        m_window->setKeyboardFocus(m_focused);
        m_window->setActive(m_focused);
        m_screen->setKeyboardFocused(m_focused);
    }
    m_screen->setBlocked(m_session->isBlocked());
    const session::SecureOutput &target = panelOutput(secure->outputs());
    m_panelOutput = target.name;
    refreshStatus(target);
    const base::Rect output = target.bounds;
    const double scale = target.scale;
    // The output's full height: the clock at the top, the login column at the bottom.
    const base::Size size(std::min(480, output.width()), output.height());
    // Centred by halving the room left, which a panel as tall as the output
    // cannot overhang.
    const base::Rect bounds(output.origin()
                                    + base::Point((output.width() - size.width()) / 2,
                                                  (output.height() - size.height()) / 2),
                            size);
    m_node->setGeometry(bounds);
    m_node->setVisible(!size.isEmpty());
    if (size.isEmpty())
        return;
    // Drawn at the output's density: an image of the logical size would be
    // stretched over the device pixels and come out blurred.
    if (!(m_window->size() == ui::SizeF {double(size.width()), double(size.height())})
        || m_scale != scale || !m_sized) {
        m_scale = scale;
        m_window->setScale(scale);
        m_window->setSize({double(size.width()), double(size.height())});
        m_sized = true;
        m_dirty = true;
    }
    if (m_dirty) {
        if (m_session->isBlocked())
            m_node->notifyChanged();
        else
            renderImage();
    }
    if (m_session->isBlocked())
        updateCursor();
}

const session::SecureOutput &
AuthUi::panelOutput(const std::vector<session::SecureOutput> &outputs) const
{
    // The screen the user is looking at: the one under the pointer, else the primary one.
    const base::PointF pointer = m_pointerLocator ? m_pointerLocator() : m_pointerPosition;
    for (const session::SecureOutput &output : outputs) {
        // Both edges count as inside.
        const base::RectF bounds(output.bounds);
        if (pointer.x() >= bounds.x() && pointer.x() <= bounds.right() && pointer.y() >= bounds.y()
            && pointer.y() <= bounds.bottom())
            return output;
    }
    for (const session::SecureOutput &output : outputs) {
        if (output.primary)
            return output;
    }
    return outputs.front();
}

base::RectF AuthUi::fieldBounds() const
{
    if (!m_screen)
        return {};
    // Where the next frame puts it, also before the first one.
    m_window->updateLayout();
    return m_screen->fieldBounds();
}

double AuthUi::fieldOpacity() const
{
    return m_screen ? m_screen->fieldOpacity() : 1;
}

ui::Bitmap AuthUi::renderImage()
{
    if (m_resetPending || !m_window || !m_node || !m_node->isVisible())
        return {};
    if (m_dirty || m_panel->image().isNull()) {
        m_dirty = false;
        if (m_panel->renderImage())
            ++m_imageGeneration;
    }
    return m_panel->image();
}

render::Texture *AuthUi::panelTexture(render::Renderer &renderer)
{
    if (m_resetPending || !m_window || !m_node || !m_node->isVisible())
        return nullptr;
    const bool dirty = m_dirty;
    m_dirty = false;
    return m_panel->texture(renderer, dirty);
}

void AuthUi::releaseRenderResources()
{
    if (m_panel)
        m_panel->releaseRenderResources();
}

void AuthUi::refreshStatus(const session::SecureOutput &target)
{
    auto *secure = m_session->secureContainer();
    if (!m_statusWindow) {
        toolkit::application();
        m_statusWindow = std::make_unique<ui::EmbeddedWindow>();
        m_statusWindow->setBackgroundColor(ui::Color {0, 0, 0, 0});
        auto status = std::make_unique<AuthStatus>(m_session->state() == session::SessionState::Login);
        m_status = status.get();
        m_statusWindow->setContentView(std::move(status));
        m_statusPanel = std::make_unique<toolkit::PanelTexture>(m_statusWindow.get());
        // Drawn once while the session is active, as the panel is.
        m_statusWindow->onRenderRequested = [this] {
            m_statusDirty = true;
            if (m_session->isBlocked() || m_statusPanel->image().isNull())
                scheduleRefresh();
        };
        m_statusNode = secure->authContainer()->addChild(std::make_unique<StatusNode>(this, secure));
        // The loaded wallpaper's node may not be in place yet.
        m_wallpaperLoaded = secure->wallpaperLoaded.connect([this] { scheduleRefresh(); });
    }
    judgeBackdrop();

    const base::Rect output = target.bounds;
    const int width = std::min(int(std::ceil(m_status->preferredWidth())), output.width());
    const base::Size size(width, std::min(int(AuthStatus::kHeight), output.height()));
    m_statusNode->setGeometry(base::Rect(output.x() + output.width() - size.width(), output.y(), size.width(),
                                         size.height()));
    m_statusNode->setVisible(!size.isEmpty());
    if (size.isEmpty())
        return;
    if (!(m_statusWindow->size() == ui::SizeF {double(size.width()), double(size.height())})
        || m_statusScale != target.scale) {
        m_statusScale = target.scale;
        m_statusWindow->setScale(target.scale);
        m_statusWindow->setSize({double(size.width()), double(size.height())});
        m_statusDirty = true;
    }
    if (m_statusDirty) {
        if (m_session->isBlocked())
            m_statusNode->notifyChanged();
        else
            renderStatusImage();
    }
}

void AuthUi::judgeBackdrop()
{
    const auto *secure = m_session->secureContainer();
    const ui::Bitmap image = secure->wallpaperImage(m_panelOutput);
    if (image.isNull()) {
        // The system's dark mode while nothing has been judged, as the bar does.
        if (!m_judged)
            m_status->setLightContents(ui::Theme::current().darkMode);
        return;
    }
    if (m_panelOutput == m_judgedOutput && image.constBits() == m_judgedPixels)
        return;
    m_judgedOutput = m_panelOutput;
    m_judgedPixels = image.constBits();
    int outputHeight = 1;
    for (const session::SecureOutput &output : secure->outputs()) {
        if (output.name == m_panelOutput)
            outputHeight = std::max(1, output.bounds.height());
    }
    const int barHeight = int(std::lround(AuthStatus::kHeight * image.height() / outputHeight));
    m_judgement = base::ThreadPool::global().run(
            [image, barHeight] {
                return shell::BarBackdrop::styleForSample(shell::BarBackdrop::sampleImage(image, barHeight));
            },
            [this](shell::BarBackdrop::Style style) {
                m_judged = true;
                m_status->setLightContents(style.useLightForeground);
            });
}

ui::Bitmap AuthUi::renderStatusImage()
{
    if (!m_statusWindow || !m_statusNode->isVisible())
        return {};
    if (m_statusDirty || m_statusPanel->image().isNull()) {
        m_statusDirty = false;
        m_statusPanel->renderImage();
    }
    return m_statusPanel->image();
}

render::Texture *AuthUi::statusTexture(render::Renderer &renderer)
{
    if (!m_statusWindow || !m_statusNode->isVisible())
        return nullptr;
    const bool dirty = m_statusDirty;
    m_statusDirty = false;
    return m_statusPanel->texture(renderer, dirty);
}

void AuthUi::releaseStatusRenderResources()
{
    if (m_statusPanel)
        m_statusPanel->releaseRenderResources();
}

bool AuthUi::onKey(input::Timestamp, uint32_t code, bool pressed)
{
    if (!pressed && code == m_repeatCode)
        m_repeatTimer.stop();
    if (!sendKey(code, pressed, false))
        return true;
    if (pressed && m_keyRepeat.enabled && m_xkb->keyRepeats(code) && !m_controller->busy()
        && !m_controller->succeeded()) {
        m_repeatCode = code;
        m_repeatFocus = m_window->focusView();
        m_repeatTimer.start(m_keyRepeat.delay);
    }
    return true;
}

bool AuthUi::sendKey(uint32_t code, bool pressed, bool repeat)
{
    if (!m_session->isBlocked() || m_resetPending || !m_window || !m_xkb
        || !m_node->isEffectivelyVisible())
        return false;
    ui::KeyEvent event;
    event.key = m_xkb->keysym(code);
    event.keycode = code;
    event.modifiers = toolkitModifiers(m_xkb);
    event.repeat = repeat;
    const char32_t unicode = xkb_keysym_to_utf32(event.key);
    if (unicode >= 0x20 && unicode != 0x7f) {
        char text[8] = {};
        if (xkb_keysym_to_utf8(event.key, text, sizeof(text)) > 0)
            event.text = text;
    }
    m_window->key(event, pressed);
    return true;
}

void AuthUi::updateCursor()
{
    const input::CursorShape shape =
            m_window && m_node && m_node->isEffectivelyVisible() && m_pointerInside
            ? m_viewCursor
            : input::CursorShape::Default;
    if (shape != m_cursorShape) {
        m_cursorShape = shape;
        cursorShapeChanged(shape);
    }
}

bool AuthUi::onPointerMotion(input::Timestamp, const base::PointF &position)
{
    m_pointerPosition = position;
    const auto *secure = m_session->secureContainer();
    if (m_node && secure && !secure->outputs().empty()
        && panelOutput(secure->outputs()).name != m_panelOutput)
        scheduleRefresh();
    if (!m_resetPending && m_window && m_node->isEffectivelyVisible()) {
        const base::PointF local = position - base::PointF(m_node->absolutePosition());
        // Both edges count as inside.
        const base::Size size = m_node->geometry().size();
        m_pointerInside = local.x() >= 0 && local.y() >= 0 && local.x() <= size.width()
                && local.y() <= size.height();
        // The views under the pointer are where the next frame puts them.
        m_window->updateLayout();
        m_window->pointerMove({local.x(), local.y()});
        updateCursor();
    }
    return true;
}

bool AuthUi::onPointerButton(input::Timestamp, const base::PointF &position, uint32_t button,
                             bool pressed)
{
    if (m_resetPending || button != BTN_LEFT || !m_window || !m_node->isEffectivelyVisible())
        return true;
    const base::PointF local = position - base::PointF(m_node->absolutePosition());
    m_window->updateLayout();
    m_window->pointerMove({local.x(), local.y()});
    m_window->pointerButton(ui::MouseButton::Left, pressed, m_xkb ? toolkitModifiers(m_xkb) : 0);
    return true;
}
} // namespace Lingmo::Auth
