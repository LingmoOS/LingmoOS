// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once

#include <cstdint>
#include <chrono>
#include <map>
#include <string>
#include <vector>

#include "base/geometry.h"
#include "base/signal.h"
#include "base/timer.h"
#include "wm/activation.h"
#include "wm/desks/desks_controller.h"
#include "wm/window_observer.h"

namespace Lingmo::WM {

class WindowManager;

// Session-local identities, never pointers or vector indices. Groups do not
// create or activate Desks. A family belongs to one group on one logical output.
class StageManager final : public WindowObserver, public DesksObserver,
                           public ActivationObserver
{
public:
    using Id = uint64_t;
    struct Group {
        Id id = 0;
        uint32_t desk = 0;
        std::string output;
        std::vector<uint64_t> windows;
        uint64_t lastActive = 0;
        uint64_t lastUsed = 0;
        uint64_t contentRevision = 1;
    };
    struct Snapshot {
        bool enabled = false;
        bool blocked = false;
        uint64_t revision = 0;
        uint32_t desk = 0;
        std::vector<Group> groups;
        std::map<std::string, Id> active;
    };

    explicit StageManager(WindowManager *manager);
    ~StageManager() override;

    bool enabled() const { return m_enabled; }
    bool changingVisibility() const { return m_updating; }
    void setEnabled(bool enabled);
    void setSessionBlocked(bool blocked);
    Snapshot snapshot() const;
    Id groupFor(const Window *window) const;
    // A copy: observers may issue commands or destroy their subscription.
    base::Signal<Snapshot> changed;

    bool switchTo(Id group, bool focus = true);
    // Join one family to an existing group; never restores a minimized window.
    bool moveWindow(uint64_t window, Id group);
    // Makes one family a separate group. Used for drag-out and the shortcut.
    bool separateWindow(uint64_t window);
    bool cycle(bool forward);

    // WM hooks run before public addition/removal notifications and activation.
    void onWindowAdded(Window *window);
    void onWindowRemoving(Window *window);
    void prepareActivation(Window *window);
    void beginOutputChange();
    void outputsChanged();

    void onWindowActivated(Window *gained, Window *lost) override;
    void onWindowBoundsChanged(Window *window, const base::Rect &) override;
    void onWindowDragStateChanged(Window *window, bool dragged) override;
    void onWindowTransientParentChanged(Window *window, Window *) override;
    void onWindowStateTypeChanged(Window *window, WindowStateType) override;
    void onWindowTitleChanged(Window *) override;
    void onWindowContentChanged(Window *window) override;
    void onWindowDeskChanged(Window *) override;
    void onDeskActivationChanged(const Desk *, const Desk *) override;
    void onDeskRemoved(const Desk *) override;

private:
    struct Output {
        std::string id;
        base::Rect bounds;
        // Physical persistent identities survive logical mirror reconstruction.
        std::vector<std::string> physical;
    };
    struct Home {
        std::string output;
        std::vector<std::string> physical;
        std::map<uint64_t, base::Rect> bounds;
    };
    Window *root(Window *window) const;
    bool eligible(const Window *window) const;
    std::vector<Output> outputs() const;
    std::string outputFor(const Window *window) const;
    Group *find(Id id);
    const Group *find(Id id) const;
    Id makeGroup(Window *root, const std::string &output);
    void adopt(Window *window);
    void prune();
    void reconcile(Window *preferred = nullptr, bool focus = false);
    void publish();
    void beginTransition(Id target, Window *preferred, bool focus);
    void tickTransition();
    void finishTransition();
    Window *candidate(const Group &group) const;
    std::map<std::string, Id> &active();

    WindowManager *m_manager;
    bool m_enabled = false;
    bool m_blocked = false;
    bool m_updating = false;
    bool m_outputChanging = false;
    uint64_t m_revision = 0;
    Id m_nextId = 1;
    uint64_t m_clock = 0;
    std::vector<Group> m_groups;
    std::map<uint32_t, std::map<std::string, Id>> m_active;
    std::map<Id, Home> m_homes;
    std::vector<Output> m_outputs;

    struct Motion {
        Window *window = nullptr;
        base::Point fromOffset;
        base::Point toOffset;
        double fromScale = 1;
        double toScale = 1;
        float fromOpacity = 1;
        float toOpacity = 1;
        bool outgoing = false;
    };
    base::Timer m_transitionTimer;
    std::chrono::steady_clock::time_point m_transitionStarted;
    std::vector<Motion> m_transition;
    Window *m_transitionPreferred = nullptr;
    bool m_transitionFocus = true;
    bool m_transitionActive = false;
};

} // namespace Lingmo::WM
