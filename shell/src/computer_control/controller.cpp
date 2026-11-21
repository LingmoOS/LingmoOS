// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "computer_control/controller.h"
#include "core/main_loop.h"

#include <algorithm>
#include <utility>
#include <vector>

#include <linux/input-event-codes.h>
#include <xkbcommon/xkbcommon.h>

#include "base/auto_reset.h"
#include "base/strings.h"
#include "input/input_manager.h"
#include "input/seat.h"

namespace Lingmo::ComputerControl {

namespace {

bool validButton(uint32_t button)
{
    return button >= BTN_MOUSE && button <= BTN_TASK;
}

// Keyboard keys only: the button ranges in between belong to other devices.
bool validKey(uint32_t key)
{
    return (key > KEY_RESERVED && key < BTN_MISC) || (key >= KEY_OK && key <= KEY_MAX);
}

} // namespace

Controller::Controller(input::InputManager *input, Environment environment)
    : m_input(input)
    , m_environment(std::move(environment))
    , m_settleTimer(core::mainLoop())
{
    // Direct, so that what the agent holds is released before the real event
    // that follows the signal is dispatched.
    m_realInput = m_input->userActivity.connect([this] { realInput(); });

    m_settleTimer.setInterval(std::chrono::milliseconds(16));
    m_settleTimer.onTimeout = [this] { checkSettles(); };
}

Controller::~Controller()
{
    // The seat goes away with the compositor, so nothing is released here.
    for (Control *control : m_controls) {
        if (auto withdraw = std::exchange(control->m_withdrawConsent, {}))
            withdraw();
        control->m_state = Control::State::Revoked;
        control->m_controller = nullptr;
    }
}

std::unique_ptr<Control> Controller::request(ControlClient *client, const void *owner,
                                             const std::string &agentName, int64_t pid)
{
    std::unique_ptr<Control> control(new Control(this, client, owner));
    m_controls.insert(control.get());

    const auto refuse = [&](RevokeReason reason) {
        control->m_state = Control::State::Revoked;
        client->revoked(reason);
        return std::move(control);
    };
    if (m_current)
        return refuse(RevokeReason::Busy);
    if (m_environment.sessionBlocked && m_environment.sessionBlocked())
        return refuse(RevokeReason::Locked);
    if (!m_environment.askConsent)
        return refuse(RevokeReason::Unavailable);

    m_current = control.get();
    control->m_serial = ++m_serial;
    auto withdraw = m_environment.askConsent(
            agentName, pid, [this, raw = control.get(), serial = control->m_serial](ConsentResult result) {
                answer(raw, serial, result);
            });
    // The answer can come at once, when the user cannot be asked.
    if (control->m_state == Control::State::Pending)
        control->m_withdrawConsent = std::move(withdraw);
    return control;
}

bool Controller::isActive() const
{
    return m_current && m_current->m_state == Control::State::Granted;
}

bool Controller::allowsCapture(const void *owner) const
{
    return owner && isActive() && m_current->m_owner == owner;
}

void Controller::stop()
{
    if (m_current)
        end(m_current, RevokeReason::Stopped);
}

void Controller::sessionBlocked()
{
    if (m_current)
        end(m_current, RevokeReason::Locked);
}

void Controller::drawn()
{
    m_lastDrawn = now();
}

void Controller::checkSettles()
{
    const input::Timestamp time = now();
    const bool read = !m_environment.inputRead || m_environment.inputRead();
    std::vector<std::pair<std::function<void(bool)>, bool>> answers;
    for (auto it = m_settles.begin(); it != m_settles.end();) {
        if (!read)
            it->readSince = time;
        // Once a client reads its input, it gets the quiet time to draw.
        const input::Timestamp quietSince =
                m_lastDrawn ? std::max(it->readSince, *m_lastDrawn) : it->readSince;
        const bool settled = time - quietSince >= kSettleQuiet;
        if (settled || time >= it->deadline) {
            answers.emplace_back(std::move(it->done), settled);
            it = m_settles.erase(it);
        } else {
            ++it;
        }
    }
    if (m_settles.empty())
        m_settleTimer.stop();
    for (auto &[done, settled] : answers)
        done(settled);
}

input::Timestamp Controller::now() const
{
    return m_environment.now ? m_environment.now() : std::chrono::steady_clock::now();
}

bool Controller::paused(const Control *control) const
{
    if (m_lastRealInput && now() - *m_lastRealInput < kPauseAfterInput)
        return true;
    // Whatever the user still holds is shared seat state: the agent's own
    // release of it would let go of the user's.
    const input::Seat *seat = m_input->seat();
    for (const auto &[key, state] : seat->pressedKeys()) {
        if (!control->m_keys.contains(key))
            return true;
    }
    input::ButtonFlags agentButtons = input::NoButton;
    for (uint32_t button : control->m_buttons)
        agentButtons |= input::buttonFlagFromEvdev(button);
    return (seat->pressedButtons() & ~agentButtons) != input::NoButton;
}

bool Controller::granted(Control *control)
{
    // A block that has not reached sessionBlocked() yet still ends the control.
    if (control->m_state == Control::State::Granted && m_environment.sessionBlocked
        && m_environment.sessionBlocked())
        end(control, RevokeReason::Locked);

    if (control->m_state == Control::State::Granted)
        return true;
    control->m_client->rejected(Rejection::NotGranted);
    return false;
}

bool Controller::admits(Control *control, const std::function<bool()> &protectedTarget)
{
    if (!granted(control))
        return false;

    std::optional<Rejection> rejection;
    if (paused(control))
        rejection = Rejection::Paused;
    else if (protectedTarget && protectedTarget())
        rejection = Rejection::Protected;
    if (rejection)
        control->m_client->rejected(*rejection);
    return !rejection;
}

void Controller::answer(Control *control, uint64_t serial, ConsentResult result)
{
    if (control != m_current || control->m_serial != serial
        || control->m_state != Control::State::Pending)
        return;
    control->m_withdrawConsent = {};
    if (result != ConsentResult::Allowed) {
        end(control,
            result == ConsentResult::Denied ? RevokeReason::Denied : RevokeReason::Unavailable);
        return;
    }
    control->m_state = Control::State::Granted;
    control->m_client->granted();
    activeChanged(true);
}

void Controller::end(Control *control, std::optional<RevokeReason> reason)
{
    if (control->m_state == Control::State::Revoked)
        return;
    const bool wasActive = control->m_state == Control::State::Granted;
    auto withdraw = std::exchange(control->m_withdrawConsent, {});
    control->m_state = Control::State::Revoked;
    if (m_current == control)
        m_current = nullptr;
    releaseAll(control);

    if (withdraw)
        withdraw();
    if (reason)
        control->m_client->revoked(*reason);
    if (wasActive)
        activeChanged(false);
}

void Controller::releaseAll(Control *control)
{
    const auto buttons = std::exchange(control->m_buttons, {});
    const auto keys = std::exchange(control->m_keys, {});
    inject([&] {
        for (uint32_t button : buttons)
            m_input->onPointerButton(now(), button, false);
        for (uint32_t key : keys)
            m_input->onKey(now(), key, false, input::KeyboardModifierLayout::Apple);
    });
}

void Controller::realInput()
{
    if (m_injecting)
        return;
    // RemoteInputFilter::LocalInputDetected: block, and let go of everything.
    m_lastRealInput = now();
    if (m_current && m_current->m_state == Control::State::Granted)
        releaseAll(m_current);
}

void Controller::dropSettles(const Control *control)
{
    std::erase_if(m_settles, [control](const Settle &settle) {
        return settle.control == control;
    });
    if (m_settles.empty())
        m_settleTimer.stop();
}

void Controller::inject(const std::function<void()> &event)
{
    const base::AutoReset injecting(m_injecting, true);
    event();
}

Control::Control(Controller *controller, ControlClient *client, const void *owner)
    : m_controller(controller)
    , m_client(client)
    , m_owner(owner)
{
}

Control::~Control()
{
    if (!m_controller)
        return;
    m_controller->end(this, std::nullopt);
    m_controller->dropSettles(this);
    m_controller->m_controls.erase(this);
}

void Control::motion(const base::PointF &position)
{
    if (!m_controller) {
        m_client->rejected(Rejection::NotGranted);
        return;
    }
    Controller *controller = m_controller;
    if (!controller->admits(this))
        return;
    controller->inject([&] {
        controller->m_input->onPointerMotionLogical(controller->now(), position);
    });
}

void Control::button(uint32_t button, bool pressed)
{
    Controller *controller = m_controller;
    if (!pressed) {
        if (controller && m_buttons.erase(button)) {
            controller->inject([&] {
                controller->m_input->onPointerButton(controller->now(), button, false);
            });
        }
        return;
    }
    if (!controller) {
        m_client->rejected(Rejection::NotGranted);
        return;
    }
    if (!validButton(button) || m_buttons.contains(button))
        return;
    const auto &environment = controller->m_environment;
    if (!controller->admits(this, [&] {
            return environment.protectedAt
                    && environment.protectedAt(
                            controller->m_input->pointerPosition());
        }))
        return;
    m_buttons.insert(button);
    controller->inject([&] {
        controller->m_input->onPointerButton(controller->now(), button, true);
    });
}

void Control::axis(int horizontal, int vertical)
{
    Controller *controller = m_controller;
    if (!controller) {
        m_client->rejected(Rejection::NotGranted);
        return;
    }
    if (horizontal == 0 && vertical == 0)
        return;
    const auto &environment = controller->m_environment;
    if (!controller->admits(this, [&] {
            return environment.protectedAt
                    && environment.protectedAt(
                            controller->m_input->pointerPosition());
        }))
        return;
    controller->inject([&] {
        controller->m_input->onPointerAxis(
                controller->now(),
                base::PointF(horizontal, vertical) * input::kAxisUnitsPerClick, true);
    });
}

void Control::key(uint32_t key, bool pressed)
{
    Controller *controller = m_controller;
    if (!pressed) {
        if (controller && m_keys.erase(key)) {
            controller->inject([&] {
                controller->m_input->onKey(controller->now(), key, false,
                                           input::KeyboardModifierLayout::Apple);
            });
        }
        return;
    }
    if (!controller) {
        m_client->rejected(Rejection::NotGranted);
        return;
    }
    if (!validKey(key) || m_keys.contains(key))
        return;
    const auto &environment = controller->m_environment;
    if (!controller->admits(this, [&] {
            return environment.protectedFocus && environment.protectedFocus();
        }))
        return;
    m_keys.insert(key);
    controller->inject([&] {
        controller->m_input->onKey(controller->now(), key, true,
                                   input::KeyboardModifierLayout::Apple);
    });
}

void Control::keysym(uint32_t keysym, bool pressed)
{
    const auto held = m_keysymKeys.find(keysym);
    if (!pressed) {
        if (held != m_keysymKeys.end()) {
            const uint32_t code = held->second;
            m_keysymKeys.erase(held);
            key(code, false);
        }
        return;
    }
    Controller *controller = m_controller;
    if (!controller) {
        m_client->rejected(Rejection::NotGranted);
        return;
    }
    if (held != m_keysymKeys.end() && m_keys.contains(held->second))
        return;
    const std::optional<input::KeyStroke> stroke =
            controller->m_input->seat()->xkbTracker()->keyStroke(keysym);
    if (!stroke) {
        m_client->rejected(Rejection::Untypable);
        return;
    }
    key(stroke->key, true);
    if (m_keys.contains(stroke->key))
        m_keysymKeys.insert_or_assign(keysym, stroke->key);
}

void Control::typeText(const std::string &text)
{
    Controller *controller = m_controller;
    if (!controller) {
        m_client->rejected(Rejection::NotGranted);
        return;
    }
    const auto &environment = controller->m_environment;
    if (!controller->admits(this, [&] {
            return environment.protectedFocus && environment.protectedFocus();
        }))
        return;

    // Key strokes reach the client on wl_keyboard and commits on text-input,
    // which toolkits apply out of order (a commit overtakes queued keys), so a
    // string goes one way only. While an input method composes, keys would be
    // its input rather than text, so it is committed.
    const bool composing =
            environment.inputMethodGrabsKeyboard && environment.inputMethodGrabsKeyboard();
    const input::XkbTracker *keymap = controller->m_input->seat()->xkbTracker();
    const std::u32string characters = base::strings::utf8ToUtf32(text);
    std::vector<input::KeyStroke> strokes;
    bool controlCharacters = false;
    bool commit = false;
    for (size_t i = 0; i < characters.size(); ++i) {
        const char32_t character = characters[i];
        std::optional<input::KeyStroke> stroke;
        if (character == '\r' && i + 1 < characters.size() && characters[i + 1] == '\n')
            continue;
        if (character == '\n' || character == '\r')
            stroke = input::KeyStroke {KEY_ENTER, {}};
        else if (character == '\t')
            stroke = input::KeyStroke {KEY_TAB, {}};
        else if (character == '\b')
            stroke = input::KeyStroke {KEY_BACKSPACE, {}};
        else if (character < 0x20 || character == 0x7f) {
            m_client->rejected(Rejection::Untypable);
            return;
        } else if (!composing)
            stroke = keymap->keyStroke(xkb_utf32_to_keysym(character));

        if (!stroke) {
            commit = true;
            continue;
        }
        controlCharacters = controlCharacters || character < 0x20;
        strokes.push_back(*stroke);
    }
    if (commit
        && (controlCharacters || !environment.commitText
            || !(environment.textInputFocused && environment.textInputFocused()))) {
        m_client->rejected(Rejection::Untypable);
        return;
    }

    // A commit is refused when the text is not valid UTF-8.
    bool typed = true;
    controller->inject([&] {
        if (commit) {
            typed = environment.commitText(text);
            return;
        }
        for (const input::KeyStroke &keyStroke : strokes)
            stroke(keyStroke);
    });
    if (!typed)
        m_client->rejected(Rejection::Untypable);
}

void Control::listWindows()
{
    Controller *controller = m_controller;
    if (!controller) {
        m_client->rejected(Rejection::NotGranted);
        return;
    }
    if (!controller->granted(this))
        return;
    const auto &environment = controller->m_environment;
    const std::vector<WindowInfo> windows =
            environment.windows ? environment.windows() : std::vector<WindowInfo> {};
    for (const WindowInfo &window : windows) {
        auto [it, added] = m_windowHandles.try_emplace(
                window.id, static_cast<uint32_t>(m_windowHandles.size() + 1));
        if (added)
            m_handleWindows.emplace(it->second, window.id);
        m_client->window(it->second, window);
    }
    m_client->windowsDone();
}

void Control::activateWindow(uint32_t handle)
{
    changeWindow(handle, false, m_controller ? m_controller->m_environment.activateWindow
                                             : std::function<bool(uint64_t)> {});
}

void Control::closeWindow(uint32_t handle)
{
    changeWindow(handle, true, m_controller ? m_controller->m_environment.closeWindow
                                            : std::function<bool(uint64_t)> {});
}

void Control::setWindowFrame(uint32_t handle, const base::Rect &frame)
{
    std::function<bool(uint64_t)> change;
    if (m_controller && m_controller->m_environment.setWindowFrame) {
        change = [this, frame](uint64_t id) {
            return m_controller->m_environment.setWindowFrame(id, frame);
        };
    }
    changeWindow(handle, true, change);
}

void Control::setClipboard(const std::string &text)
{
    Controller *controller = m_controller;
    if (!controller) {
        m_client->rejected(Rejection::NotGranted);
        return;
    }
    if (!controller->admits(this))
        return;
    if (controller->m_environment.setClipboard)
        controller->m_environment.setClipboard(text);
}

void Control::queryPointer()
{
    Controller *controller = m_controller;
    if (!controller) {
        m_client->rejected(Rejection::NotGranted);
        return;
    }
    if (controller->granted(this))
        m_client->pointer(controller->m_input->pointerPosition());
}

void Control::settle(std::chrono::milliseconds timeout, std::function<void(bool settled)> done)
{
    Controller *controller = m_controller;
    if (!controller) {
        done(false);
        return;
    }
    const input::Timestamp time = controller->now();
    controller->m_settles.push_back({this, time + std::min(timeout, Controller::kMaxSettleTimeout),
                                     time, std::move(done)});
    if (!controller->m_settleTimer.isActive())
        controller->m_settleTimer.start();
}

void Control::changeWindow(uint32_t handle, bool guarded,
                           const std::function<bool(uint64_t id)> &change)
{
    Controller *controller = m_controller;
    if (!controller) {
        m_client->rejected(Rejection::NotGranted);
        return;
    }
    const auto &environment = controller->m_environment;
    const auto window = m_handleWindows.find(handle);
    const bool known = window != m_handleWindows.end();
    if (!controller->admits(this, [&] {
            return guarded && known && environment.protectedWindow
                    && environment.protectedWindow(window->second);
        }))
        return;
    bool changed = false;
    if (known && change) {
        controller->inject([&] {
            changed = change(window->second);
        });
    }
    if (!changed)
        m_client->rejected(Rejection::NoWindow);
}

void Control::stroke(const input::KeyStroke &stroke)
{
    input::InputManager *input = m_controller->m_input;
    const auto send = [&](uint32_t key, bool pressed) {
        input->onKey(m_controller->now(), key, pressed, input::KeyboardModifierLayout::Apple);
    };
    // A modifier the agent already holds stays held.
    std::vector<uint32_t> modifiers;
    for (uint32_t modifier : stroke.modifiers) {
        if (!m_keys.contains(modifier))
            modifiers.push_back(modifier);
    }
    for (uint32_t modifier : modifiers)
        send(modifier, true);
    send(stroke.key, true);
    send(stroke.key, false);
    for (auto it = modifiers.rbegin(); it != modifiers.rend(); ++it)
        send(*it, false);
}

} // namespace Lingmo::ComputerControl
