// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/shell_input.h"

#include <algorithm>

#include <xkbcommon/xkbcommon.h>

#include "base/event_loop.h"
#include "core/main_loop.h"
#include "input/keyboard.h"
#include "input/keyboard_delegate.h"
#include "input/pointer.h"
#include "input/pointer_delegate.h"
#include "input/seat.h"
#include "input/touch.h"
#include "input/touch_delegate.h"
#include "input/xkb_tracker.h"
#include "shell/shell.h"
#include "shell/shell_window.h"
#include "ui/embedded_window.h"

namespace Lingmo::Shell {

namespace {

// Wayland axis units per wheel click, as the protocol delegate sends them.
constexpr double kAxisUnitsPerClick = 10.0;
constexpr int kValue120PerClick = 120;

input::CursorShape seatCursor(ui::CursorShape shape)
{
    switch (shape) {
    case ui::CursorShape::Blank:
        return input::CursorShape::Hidden;
    case ui::CursorShape::Default:
        return input::CursorShape::Default;
    case ui::CursorShape::Pointer:
        return input::CursorShape::Pointer;
    case ui::CursorShape::Text:
        return input::CursorShape::Text;
    case ui::CursorShape::Move:
        return input::CursorShape::Move;
    case ui::CursorShape::Grab:
        return input::CursorShape::Grab;
    case ui::CursorShape::Grabbing:
        return input::CursorShape::Grabbing;
    case ui::CursorShape::NotAllowed:
        return input::CursorShape::NotAllowed;
    case ui::CursorShape::ResizeHorizontal:
        return input::CursorShape::EwResize;
    case ui::CursorShape::ResizeVertical:
        return input::CursorShape::NsResize;
    }
    return input::CursorShape::Default;
}

uint32_t timeMs(input::Timestamp timestamp)
{
    return uint32_t(std::chrono::duration_cast<std::chrono::milliseconds>(
                            timestamp.time_since_epoch())
                            .count());
}

} // namespace

class ShellInput::PointerDelegate final : public input::PointerDelegate
{
public:
    explicit PointerDelegate(ShellInput *input)
        : m_input(input)
    {
    }

    ~PointerDelegate() override { *m_alive = false; }

    void onPointerDestroying(input::Pointer *) override {}

    bool canAcceptPointerEventsForSurface(compositor::Surface *surface) const override
    {
        return m_input->windowForSurface(surface) != nullptr;
    }

    void onPointerEnter(compositor::Surface *surface, const base::PointF &location,
                        input::ButtonFlags) override
    {
        ShellWindow *window = m_input->windowForSurface(surface);
        m_input->m_pointerWindow = window;
        if (!window)
            return;
        window->window()->pointerEnter({location.x(), location.y()});
        // The pointer takes the focus after the enter; the cursor follows once it has.
        m_cursorPending = true;
        core::mainLoop()->post([alive = m_alive, this] {
            if (*alive && m_cursorPending)
                m_input->applyCursor();
        });
    }

    void onPointerLeave(compositor::Surface *surface) override
    {
        ShellWindow *window = m_input->windowForSurface(surface);
        if (window)
            window->window()->pointerLeave();
        if (m_input->m_pointerWindow == window)
            m_input->m_pointerWindow = nullptr;
        m_cursorPending = false;
    }

    void onPointerMotion(input::Timestamp, const base::PointF &location) override
    {
        if (ShellWindow *window = m_input->m_pointerWindow) {
            window->window()->pointerMove({location.x(), location.y()});
            if (m_cursorPending)
                m_input->applyCursor();
        }
    }

    void onPointerButton(input::Timestamp, input::ButtonFlags changed, bool pressed) override
    {
        ShellWindow *window = m_input->m_pointerWindow;
        if (!window)
            return;
        const uint32_t modifiers = m_input->modifiers();
        const std::pair<input::ButtonFlag, ui::MouseButton> buttons[] = {
                {input::LeftButton, ui::MouseButton::Left},
                {input::RightButton, ui::MouseButton::Right},
                {input::MiddleButton, ui::MouseButton::Middle},
        };
        for (const auto &[flag, button] : buttons) {
            // A button may close the window under it; ask again each time.
            if ((changed & flag) && m_input->m_pointerWindow == window)
                window->window()->pointerButton(button, pressed, modifiers);
        }
    }

    void onPointerScroll(input::Timestamp timestamp, const base::PointF &offset,
                         bool discrete) override
    {
        ShellWindow *window = m_input->m_pointerWindow;
        if (!window)
            return;
        ui::ScrollEvent event;
        event.deltaX = offset.x();
        event.deltaY = offset.y();
        if (discrete) {
            event.value120X = int(std::lround(offset.x() / kAxisUnitsPerClick * kValue120PerClick));
            event.value120Y = int(std::lround(offset.y() / kAxisUnitsPerClick * kValue120PerClick));
        }
        event.precise = !discrete;
        event.modifiers = m_input->modifiers();
        event.time = timeMs(timestamp);
        window->window()->scroll(event);
    }

    void onFingerScrollStop(input::Timestamp timestamp) override
    {
        ShellWindow *window = m_input->m_pointerWindow;
        if (!window)
            return;
        ui::ScrollEvent event;
        event.precise = true;
        event.stop = true;
        event.modifiers = m_input->modifiers();
        event.time = timeMs(timestamp);
        window->window()->scroll(event);
    }

    void onPointerFrame() override {}

    void cursorApplied() { m_cursorPending = false; }

private:
    ShellInput *m_input;
    bool m_cursorPending = false;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

class ShellInput::KeyboardDelegate final : public input::KeyboardDelegate
{
public:
    explicit KeyboardDelegate(ShellInput *input)
        : m_input(input)
    {
    }

    bool canAcceptKeyboardEventsForSurface(compositor::Surface *surface) const override
    {
        return m_input->windowForSurface(surface) != nullptr;
    }

    void onKeyboardEnter(compositor::Surface *surface, const PressedKeys &) override
    {
        ShellWindow *window = m_input->windowForSurface(surface);
        m_input->m_keyboardWindow = window;
        if (window)
            window->setKeyboardFocus(true);
    }

    void onKeyboardLeave(compositor::Surface *surface) override
    {
        m_input->stopRepeat();
        ShellWindow *window = m_input->windowForSurface(surface);
        if (window)
            window->setKeyboardFocus(false);
        if (m_input->m_keyboardWindow == window)
            m_input->m_keyboardWindow = nullptr;
    }

    uint32_t onKeyboardKey(input::Timestamp, uint32_t evdevCode, bool pressed) override
    {
        m_input->sendKey(evdevCode, pressed, false);
        return 0;
    }

    void onKeyboardModifiers(const input::KeyboardModifiers &) override {}

    void onKeyRepeatSettingsChanged(bool enabled, std::chrono::milliseconds delay,
                                    std::chrono::milliseconds interval) override
    {
        m_input->m_repeatEnabled = enabled;
        m_input->m_repeatDelay = delay;
        m_input->m_repeatInterval = interval;
        if (!enabled)
            m_input->stopRepeat();
    }

    void onKeyboardLayoutUpdated(const std::string &) override { m_input->stopRepeat(); }

private:
    ShellInput *m_input;
};

class ShellInput::TouchDelegate final : public input::TouchDelegate
{
public:
    explicit TouchDelegate(ShellInput *input)
        : m_input(input)
    {
    }

    void onTouchDestroying(input::Touch *) override {}

    bool canAcceptTouchEventsForSurface(compositor::Surface *surface) const override
    {
        return m_input->windowForSurface(surface) != nullptr;
    }

    void onTouchDown(compositor::Surface *surface, input::Timestamp, int32_t id,
                     const base::PointF &location) override
    {
        ShellWindow *window = m_input->windowForSurface(surface);
        if (!window)
            return;
        m_input->m_touchPoints[id] = window;
        if (window->window())
            window->window()->touchDown(id, {location.x(), location.y()});
    }

    void onTouchUp(input::Timestamp, int32_t id) override
    {
        const auto it = m_input->m_touchPoints.find(id);
        if (it == m_input->m_touchPoints.end())
            return;
        ShellWindow *window = it->second;
        m_input->m_touchPoints.erase(it);
        if (window->window())
            window->window()->touchUp(id);
    }

    void onTouchMotion(input::Timestamp, int32_t id, const base::PointF &location) override
    {
        const auto it = m_input->m_touchPoints.find(id);
        if (it != m_input->m_touchPoints.end() && it->second->window())
            it->second->window()->touchMotion(id, {location.x(), location.y()});
    }

    void onTouchFrame() override {}

    void onTouchCancel() override
    {
        std::vector<ShellWindow *> windows;
        for (const auto &[id, window] : m_input->m_touchPoints) {
            if (std::find(windows.begin(), windows.end(), window) == windows.end())
                windows.push_back(window);
        }
        m_input->m_touchPoints.clear();
        for (ShellWindow *window : windows) {
            if (window->window())
                window->window()->touchCancel();
        }
    }

private:
    ShellInput *m_input;
};

// One of zwp_pointer_gestures_v1's three, for the shell window under the
// pointer, as a client's gesture objects take them.
class ShellInput::GestureDelegate final : public input::PointerGestureDelegate
{
public:
    GestureDelegate(ShellInput *input, input::GestureType type)
        : m_input(input)
        , m_type(type)
    {
    }

    void onPointerDestroying(input::Pointer *) override {}
    input::GestureType gestureType() const override { return m_type; }

    void onPointerGestureBegin(input::Timestamp timestamp, compositor::Surface *surface,
                               uint32_t fingers) override
    {
        ShellWindow *window = m_input->windowForSurface(surface);
        m_input->m_gestureWindow = window;
        m_fingers = fingers;
        send(begin(), timestamp, {});
    }

    void onPointerGestureUpdate(input::Timestamp timestamp,
                                const input::GestureUpdate &update) override
    {
        send(m_type == input::GestureType::Pinch ? ui::GestureEvent::Type::PinchUpdate
                                                 : ui::GestureEvent::Type::SwipeUpdate,
             timestamp, update);
    }

    void onPointerGestureEnd(input::Timestamp timestamp, bool cancelled) override
    {
        input::GestureUpdate none;
        m_cancelled = cancelled;
        send(end(), timestamp, none);
        m_cancelled = false;
        m_input->m_gestureWindow = nullptr;
    }

private:
    ui::GestureEvent::Type begin() const
    {
        switch (m_type) {
        case input::GestureType::Swipe:
            return ui::GestureEvent::Type::SwipeBegin;
        case input::GestureType::Pinch:
            return ui::GestureEvent::Type::PinchBegin;
        case input::GestureType::Hold:
            return ui::GestureEvent::Type::HoldBegin;
        }
        return ui::GestureEvent::Type::SwipeBegin;
    }

    ui::GestureEvent::Type end() const
    {
        switch (m_type) {
        case input::GestureType::Swipe:
            return ui::GestureEvent::Type::SwipeEnd;
        case input::GestureType::Pinch:
            return ui::GestureEvent::Type::PinchEnd;
        case input::GestureType::Hold:
            return ui::GestureEvent::Type::HoldEnd;
        }
        return ui::GestureEvent::Type::SwipeEnd;
    }

    void send(ui::GestureEvent::Type type, input::Timestamp timestamp,
              const input::GestureUpdate &update)
    {
        ShellWindow *window = m_input->m_gestureWindow;
        if (!window || !window->window())
            return;
        ui::GestureEvent event;
        event.type = type;
        event.fingers = m_fingers;
        event.delta = {update.delta.x(), update.delta.y()};
        event.scale = update.scale;
        event.rotation = update.rotation;
        event.cancelled = m_cancelled;
        event.modifiers = m_input->modifiers();
        event.time = timeMs(timestamp);
        window->window()->gesture(event);
    }

    ShellInput *m_input;
    const input::GestureType m_type;
    uint32_t m_fingers = 0;
    bool m_cancelled = false;
};

ShellInput::ShellInput(Shell *shell)
    : m_shell(shell)
    , m_repeatTimer(core::mainLoop())
{
    input::Seat *seat = m_shell->seat();
    m_pointerDelegate = std::make_unique<PointerDelegate>(this);
    m_pointer = std::make_unique<input::Pointer>(m_pointerDelegate.get(), seat);
    m_keyboard = std::make_unique<input::Keyboard>(std::make_unique<KeyboardDelegate>(this), seat);
    m_touchDelegate = std::make_unique<TouchDelegate>(this);
    m_touch = std::make_unique<input::Touch>(m_touchDelegate.get(), seat);
    for (const input::GestureType type :
         {input::GestureType::Swipe, input::GestureType::Pinch, input::GestureType::Hold}) {
        m_gestureDelegates.push_back(std::make_unique<GestureDelegate>(this, type));
        m_pointer->addGestureDelegate(m_gestureDelegates.back().get());
    }

    const input::KeyRepeatSettings &repeat = seat->keyRepeatSettings();
    m_repeatEnabled = repeat.enabled;
    m_repeatDelay = repeat.delay;
    m_repeatInterval = repeat.interval;
    m_repeatTimer.onTimeout = [this] {
        if (!m_keyboardWindow || !m_repeatCode) {
            stopRepeat();
            return;
        }
        m_repeatTimer.start(m_repeatInterval);
        sendKey(m_repeatCode, true, true);
    };
}

ShellInput::~ShellInput()
{
    m_repeatTimer.stop();
    m_touch.reset();
    m_keyboard.reset();
    for (const auto &delegate : m_gestureDelegates)
        m_pointer->removeGestureDelegate(delegate.get());
    m_pointer.reset();
}

ShellWindow *ShellInput::windowForSurface(const compositor::Surface *surface) const
{
    if (!surface)
        return nullptr;
    for (ShellWindow *window : m_shell->windows()) {
        if (window->surface() == surface)
            return window;
    }
    return nullptr;
}

void ShellInput::cursorShapeChanged(ShellWindow *window)
{
    if (window == m_pointerWindow)
        applyCursor();
}

void ShellInput::applyCursor()
{
    m_pointerDelegate->cursorApplied();
    if (!m_pointerWindow || m_pointer->focusedSurface() != m_pointerWindow->surface())
        return;
    if (const auto &cursor = m_pointerWindow->cursor()) {
        if (compositor::Surface *image = m_pointerWindow->cursorSurface(); image && !cursor->image.isNull())
            m_pointer->setCursor(image, cursor->hotspot);
        else
            m_pointer->setCursorShape(cursor->shape);
        return;
    }
    m_pointer->setCursorShape(seatCursor(m_pointerWindow->window()->cursorShape()));
}

void ShellInput::windowDestroying(ShellWindow *window)
{
    // The seat sends leaves when the surface goes; nothing may reach the
    // window between now and then.
    if (m_pointerWindow == window)
        m_pointerWindow = nullptr;
    if (m_keyboardWindow == window) {
        m_keyboardWindow = nullptr;
        stopRepeat();
    }
    if (m_gestureWindow == window)
        m_gestureWindow = nullptr;
    std::erase_if(m_touchPoints, [window](const auto &point) { return point.second == window; });
}

uint32_t ShellInput::modifiers() const
{
    const input::XkbTracker *xkb = m_shell->seat()->clientXkbTracker();
    uint32_t modifiers = ui::NoModifier;
    if (!xkb)
        return modifiers;
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

void ShellInput::sendKey(uint32_t code, bool pressed, bool repeat)
{
    ShellWindow *window = m_keyboardWindow;
    const input::XkbTracker *xkb = m_shell->seat()->clientXkbTracker();
    if (!window || !xkb)
        return;
    if (!pressed && code == m_repeatCode)
        stopRepeat();
    ui::KeyEvent event;
    event.key = xkb->keysym(code);
    event.keycode = code;
    event.modifiers = modifiers();
    event.repeat = repeat;
    const char32_t unicode = xkb_keysym_to_utf32(event.key);
    if (unicode >= 0x20 && unicode != 0x7f) {
        char text[8] = {};
        if (xkb_keysym_to_utf8(event.key, text, sizeof(text)) > 0)
            event.text = text;
    }
    // The host repeats keys for the toolkit, as a Wayland client's toolkit
    // repeats them from wl_keyboard.repeat_info.
    if (pressed && !repeat && m_repeatEnabled && xkb->keyRepeats(code)) {
        m_repeatCode = code;
        m_repeatTimer.start(m_repeatDelay);
    }
    window->window()->key(event, pressed);
}

void ShellInput::stopRepeat()
{
    m_repeatTimer.stop();
    m_repeatCode = 0;
}

} // namespace Lingmo::Shell
