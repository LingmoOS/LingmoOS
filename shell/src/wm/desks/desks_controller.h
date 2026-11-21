// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/wm/desks/desks_controller.h and desks_util.cc
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65
//
// Adapted for Lingmo: the desks, their activation and removal, moving windows
// between them and the switch animations. Overview, split view, float, saved
// desks, coral, the undo toast, per-user desks and metrics are not carried over.
// A desk's windows are the application windows of the normal stacking layer;
// always-on-top windows, system dialogs and the shell's own windows are on
// every desk, as upstream keeps them out of the desk containers.

#ifndef LINGMO_WM_DESKS_DESKS_CONTROLLER_H_
#define LINGMO_WM_DESKS_DESKS_CONTROLLER_H_

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "scene/desk_switch_node.h"
#include "wm/activation.h"
#include "wm/window_observer.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Scene {
class SlideOutNode;
}

namespace Lingmo::WM {

class Desk;
class DeskAnimationBase;
class Window;
class WindowManager;

enum class DesksCreationRemovalSource {
    Keyboard,
    // A programmatic request.
    Api,
    DesksRestore,
    // The overview's new-desk button, or a window dropped on it.
    Button,
};

enum class DesksSwitchSource {
    NewDeskShortcut,
    DeskSwitchShortcut,
    WindowActivated,
    DeskSwitchTouchpad,
    IndexedDeskSwitchShortcut,
    ApiSwitch,
    DeskRestored,
    // A desk clicked in the overview's desk bar.
    MiniViewButton,
};

class DesksObserver
{
public:
    virtual ~DesksObserver() = default;
    virtual void onDeskAdded(const Desk *desk) { }
    // After the desk left the list; it is destroyed right after.
    virtual void onDeskRemoved(const Desk *desk) { }
    // The desk at `from` moved to `to`, the ones between moving over by one.
    virtual void onDeskReordered(int from, int to) { }
    virtual void onDeskActivationChanged(const Desk *activated, const Desk *deactivated) { }
    virtual void onDeskNameChanged(const Desk *desk) { }
    virtual void onDeskSwitchAnimationLaunching() { }
    virtual void onDeskSwitchAnimationFinished() { }
    // See DesksController::chosenDesk.
    virtual void onChosenDeskChanged() { }
    // The window moved to another desk, or onto or off every desk.
    virtual void onWindowDeskChanged(Window *window) { }
};

class DesksController : public ActivationObserver, public WindowObserver
{
public:
    // desks_util's kDesksDefaultLimit.
    static constexpr int kMaxDesks = 8;

    // Takes each output's frame of what `paint` draws; see DeskSwitchNode::start.
    using Screenshotter = std::function<scene::DeskSwitchNode::Frames(
            const std::function<void(render::Renderer &)> &paint)>;

    explicit DesksController(WindowManager *manager);
    DesksController(const DesksController &) = delete;
    DesksController &operator=(const DesksController &) = delete;
    ~DesksController() override;

    WindowManager *windowManager() const { return m_manager; }

    const std::vector<std::unique_ptr<Desk>> &desks() const { return m_desks; }
    Desk *activeDesk() const { return m_activeDesk; }
    int activeDeskIndex() const { return deskIndex(m_activeDesk); }
    int deskIndex(const Desk *desk) const;
    Desk *deskAt(int index) const;
    Desk *deskById(uint32_t id) const;

    // The desk an animation is heading for, or the active one.
    Desk *targetActiveDesk() const;
    // The desk a switch has been decided for: a swipe decides as the fingers
    // lift, though desks it pulls in are active on the way to take frames.
    Desk *chosenDesk() const;
    Desk *nextDesk() const;
    Desk *previousDesk() const;

    bool areDesksBeingModified() const;
    bool canCreateDesks() const;
    bool canRemoveDesks() const;

    // Appends a desk; it is not activated.
    Desk *newDesk(DesksCreationRemovalSource source, const std::string &name = {});

    // Combines `desk` into the desk on its left, or on its right when it is
    // the first. Removing the active desk slides to that desk first.
    void removeDesk(Desk *desk, DesksCreationRemovalSource source);

    // Upstream ReorderDesk: the desk at `from` goes to `to`. Not while a
    // desk animation runs, whose strip is laid out by index.
    void reorderDesk(int from, int to);

    void activateDesk(Desk *desk, DesksSwitchSource source);

    // Steps to the adjacent desk; with none there the desk content nudges
    // against the edge. False only while the session is blocked.
    bool activateAdjacentDesk(bool goingLeft, DesksSwitchSource source);

    // The four-finger swipe of upstream, three fingers here. Deltas are in
    // desks, times in milliseconds on the steady clock.
    bool startSwipeAnimation(bool moveLeft);
    void updateSwipeAnimation(double deskDelta, double timeMs);
    void endSwipeAnimation(double timeMs);

    bool moveWindowFromActiveDeskTo(Window *window, Desk *target);
    // Upstream SendToDeskAtIndex: with the move animation, from the active desk.
    void sendToDeskAtIndex(Window *window, int index);

    // The window's desk, or null for a window on every desk.
    Desk *deskOf(const Window *window) const;
    bool belongsToActiveDesk(const Window *window) const;

    bool isVisibleOnAllDesks(const Window *window) const;
    void setVisibleOnAllDesks(Window *window, bool visibleOnAllDesks);

    void setDeskName(Desk *desk, const std::string &name);

    // The desk content nudges against the edge it cannot go past.
    void performHitTheWallAnimation(bool goingLeft);

    void addObserver(DesksObserver *observer);
    void removeObserver(DesksObserver *observer);

    // Called by WindowManager.
    void onWindowAdded(Window *window);
    void onWindowRemoving(Window *window);
    // The window's stacking layer or transient parent may have changed.
    void updateWindow(Window *window);

    // Where the switch animations draw, and how the starting frames are taken.
    // Without both, desks switch at once.
    void setSwitchNode(scene::DeskSwitchNode *node) { m_switchNode = node; }
    scene::DeskSwitchNode *switchNode() const { return m_switchNode; }
    void setWindowMoveNode(scene::SlideOutNode *node) { m_windowMoveNode = node; }
    void setScreenshotter(Screenshotter screenshotter)
    {
        m_screenshotter = std::move(screenshotter);
    }

    DeskAnimationBase *animation() const { return m_animation.get(); }

    // --- ActivationObserver ---
    void onWindowActivating(Window *gainingActive, Window *losingActive) override;
    void onWindowActivated(Window *, Window *) override { }

    // --- WindowObserver ---
    void onWindowTransientParentChanged(Window *window, Window *oldParent) override;

private:
    friend class Desk;
    friend class DeskAnimationBase;
    friend class DeskActivationAnimation;
    friend class DeskRemovalAnimation;

    // The frames the switch animations take of each output's desk content.
    scene::DeskSwitchNode::Frames takeScreenshots(
            const std::function<void(render::Renderer &)> &paint) const;
    // The visible windows `filter` picks, alone on transparency, bottom to top.
    scene::DeskSwitchNode::Frames
    takeWindowScreenshots(const std::function<bool(const Window *)> &filter) const;
    int referenceWidth() const;

    void onAnimationFinished(DeskAnimationBase *animation);
    void launchActivationAnimation(Desk *desk, DesksSwitchSource source);
    void activateDeskInternal(Desk *desk, bool updateWindowActivation);
    void removeDeskInternal(Desk *desk, bool deskSwitched);
    bool moveWindowFromSourceDeskTo(Window *window, Desk *source, Desk *target);
    void performWindowMoveToDeskAnimation(Window *window, bool goingLeft);

    // Puts `window`'s tree on `desk`, or on every desk when null.
    void assignWindow(Window *window, Desk *desk);
    void updateDeskVisibility();
    void updateDeskVisibility(Window *window);
    bool isDeskWindow(const Window *window) const;
    Window *transientRoot(Window *window) const;
    const Window *transientRoot(const Window *window) const;

    template <typename F>
    void notify(F &&fn)
    {
        for (DesksObserver *observer : std::vector<DesksObserver *>(m_observers)) {
            if (std::find(m_observers.begin(), m_observers.end(), observer) != m_observers.end())
                fn(observer);
        }
    }

    WindowManager *m_manager = nullptr;
    std::vector<std::unique_ptr<Desk>> m_desks;
    Desk *m_activeDesk = nullptr;
    Desk *m_deskToActivate = nullptr;
    // Transient roots on a desk.
    std::map<const Window *, Desk *> m_windowDesks;
    std::set<const Window *> m_visibleOnAllDesks;
    bool m_modifying = false;
    std::unique_ptr<DeskAnimationBase> m_animation;
    std::unique_ptr<DeskAnimationBase> m_finishedAnimation;
    scene::DeskSwitchNode *m_switchNode = nullptr;
    scene::SlideOutNode *m_windowMoveNode = nullptr;
    Screenshotter m_screenshotter;
    std::vector<DesksObserver *> m_observers;
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_DESKS_DESKS_CONTROLLER_H_
