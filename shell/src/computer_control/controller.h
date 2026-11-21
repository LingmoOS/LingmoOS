// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once

// Lets an agent drive the primary seat once the user allows it: consent, the
// single current control, pausing on real input and protected targets. The
// pause follows remoting/host/remote_input_filter.cc at Chromium revision
// 8abbac30b9cf2fe976422c58c000c94685648b65.


#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "base/geometry.h"
#include "base/signal.h"
#include "base/timer.h"
#include "input/types.h"
#include "input/xkb_tracker.h"

namespace Lingmo::Input {
class InputManager;
}

namespace Lingmo::ComputerControl {

// The values are lingmo_computer_control_v1's.
enum class RevokeReason : uint32_t {
    Denied = 0,
    Stopped = 1,
    Locked = 2,
    Busy = 3,
    Unavailable = 4,
};

enum class Rejection : uint32_t {
    NotGranted = 0,
    Paused = 1,
    Protected = 2,
    Untypable = 3,
    NoWindow = 4,
};

// An application window as the agent sees it.
struct WindowInfo {
    // The value is lingmo_computer_control_v1's window_state.
    enum Flag : uint32_t {
        Active = 1,
        Minimized = 2,
        Hidden = 4,
        OtherDesk = 8,
    };

    uint64_t id = 0;
    std::string applicationId;
    std::string title;
    base::Rect bounds;
    uint32_t flags = 0;
};

enum class ConsentResult {
    Allowed,
    Denied,
    Unavailable,
};

// The protocol side of one control.
class ControlClient
{
public:
    virtual ~ControlClient() = default;

    virtual void granted() = 0;
    virtual void revoked(RevokeReason reason) = 0;
    virtual void rejected(Rejection rejection) = 0;
    virtual void window(uint32_t handle, const WindowInfo &window) = 0;
    virtual void windowsDone() = 0;
    virtual void pointer(const base::PointF &position) = 0;
};

// What the controller asks of the rest of the compositor.
struct Environment {
    // Asks the user whether `agentName`, whose server is process `pid` (0 when
    // unknown), may take control. `answer` runs once, at once or later; the
    // returned function withdraws the question before then.
    std::function<std::function<void()>(const std::string &agentName, int64_t pid,
                                        std::function<void(ConsentResult)> answer)>
            askConsent;
    // A window the user answers in person (a system dialog or a shell prompt,
    // with its popups) is at `position`, or has the keyboard focus.
    std::function<bool(const base::PointF &position)> protectedAt;
    std::function<bool()> protectedFocus;
    // A text field of the focused client is enabled; an input method composes
    // the keys typed into it; commit sends text to it.
    std::function<bool()> textInputFocused;
    std::function<bool()> inputMethodGrabsKeyboard;
    std::function<bool(const std::string &text)> commitText;
    std::function<bool()> sessionBlocked;
    // Application windows, most recently used first, and bringing one forward
    // as the dock does; false when the window is gone.
    std::function<std::vector<WindowInfo>()> windows;
    std::function<bool(uint64_t id)> activateWindow;
    // Such a window, by id. Closing and setting the frame return false when
    // the window is gone or cannot take it.
    std::function<bool(uint64_t id)> protectedWindow;
    std::function<bool(uint64_t id)> closeWindow;
    std::function<bool(uint64_t id, const base::Rect &frame)> setWindowFrame;
    // Replaces the clipboard with UTF-8 text.
    std::function<void(const std::string &text)> setClipboard;
    // The clients with the keyboard and pointer focus have read every event
    // sent to them.
    std::function<bool()> inputRead;
    std::function<input::Timestamp()> now;
};

class Control;

class Controller
{
public:
    // Real input blocks injection this long, as RemoteInputFilter's
    // kRemoteBlockTimeout.
    static constexpr std::chrono::milliseconds kPauseAfterInput {2000};
    // How long the screen and the focused clients stay quiet before a settle
    // is answered, and the longest a settle waits.
    static constexpr std::chrono::milliseconds kSettleQuiet {150};
    static constexpr std::chrono::milliseconds kMaxSettleTimeout {10000};

    Controller(input::InputManager *input, Environment environment);
    Controller(const Controller &) = delete;
    Controller &operator=(const Controller &) = delete;
    ~Controller();

    // A new control for `client`, which `owner` (its wl_client) holds. Only
    // one control is pending or granted at a time; a second one is revoked at
    // once with Busy. The control ends when the returned handle is destroyed.
    std::unique_ptr<Control> request(ControlClient *client, const void *owner,
                                     const std::string &agentName, int64_t pid = 0);

    // A control is granted.
    bool isActive() const;

    // `owner` holds the granted control, so it may read the screen.
    bool allowsCapture(const void *owner) const;

    // Ends the pending or granted control with Stopped.
    void stop();

    // Ends it with Locked; call when the session becomes blocked.
    void sessionBlocked();

    // An output drew a frame.
    void drawn();
    // Answers the settles that are due; a timer calls it every frame while
    // one waits.
    void checkSettles();

    base::Signal<bool> activeChanged;

private:
    friend class Control;

    input::Timestamp now() const;
    bool paused(const Control *control) const;

    // Whether `control` is granted, telling its client otherwise.
    bool granted(Control *control);
    // Whether `control` may inject now, telling its client why not otherwise.
    // `protectedTarget` is asked last, once nothing else refuses.
    bool admits(Control *control, const std::function<bool()> &protectedTarget = {});
    void answer(Control *control, uint64_t serial, ConsentResult result);
    void end(Control *control, std::optional<RevokeReason> reason);
    void releaseAll(Control *control);
    void realInput();
    void dropSettles(const Control *control);

    // Injects through the InputManager with real input detection off.
    void inject(const std::function<void()> &event);

    input::InputManager *m_input;
    base::Subscription m_realInput;
    Environment m_environment;
    std::set<Control *> m_controls;
    Control *m_current = nullptr;
    uint64_t m_serial = 0;
    bool m_injecting = false;
    std::optional<input::Timestamp> m_lastRealInput;

    struct Settle {
        const Control *control;
        input::Timestamp deadline;
        // Since when the focused clients have read their input.
        input::Timestamp readSince;
        std::function<void(bool settled)> done;
    };
    std::vector<Settle> m_settles;
    std::optional<input::Timestamp> m_lastDrawn;
    base::Timer m_settleTimer;
};

// One control, as lingmo_computer_control_v1 describes it. Input requests
// answer a refusal through ControlClient::rejected.
class Control
{
public:
    ~Control();

    Control(const Control &) = delete;
    Control &operator=(const Control &) = delete;

    void motion(const base::PointF &position);
    void button(uint32_t button, bool pressed);
    void axis(int horizontal, int vertical);
    void key(uint32_t key, bool pressed);
    // The key that types `keysym` on the current layout, as `key`.
    void keysym(uint32_t keysym, bool pressed);
    void typeText(const std::string &text);
    void listWindows();
    void activateWindow(uint32_t handle);
    void closeWindow(uint32_t handle);
    void setWindowFrame(uint32_t handle, const base::Rect &frame);
    void setClipboard(const std::string &text);
    void queryPointer();
    // `done` runs once the screen and the focused clients are quiet, with
    // false when `timeout` passes first. Modelled on the NotifyWhenDone calls
    // of chromium/src/ui/base/test/ui_controls.h at the revision above.
    void settle(std::chrono::milliseconds timeout, std::function<void(bool settled)> done);

private:
    friend class Controller;

    enum class State {
        Pending,
        Granted,
        Revoked,
    };

    Control(Controller *controller, ControlClient *client, const void *owner);

    void stroke(const input::KeyStroke &stroke);
    void changeWindow(uint32_t handle, bool guarded,
                      const std::function<bool(uint64_t id)> &change);

    Controller *m_controller;
    ControlClient *const m_client;
    const void *const m_owner;
    State m_state = State::Pending;
    uint64_t m_serial = 0;
    std::function<void()> m_withdrawConsent;
    std::set<uint32_t> m_buttons;
    std::set<uint32_t> m_keys;
    // The key each keysym press pressed, for its release after a layout change.
    std::map<uint32_t, uint32_t> m_keysymKeys;
    // The handle of each window listed so far, and back.
    std::map<uint64_t, uint32_t> m_windowHandles;
    std::map<uint32_t, uint64_t> m_handleWindows;
};

} // namespace Lingmo::ComputerControl

