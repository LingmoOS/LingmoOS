// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/wm/desks/desks_controller.cc and desks_util.cc
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65

#include "wm/desks/desks_controller.h"

#include <algorithm>

#include "base/auto_reset.h"
#include "base/strings.h"
#include "core/logging.h"
#include "scene/slide_out_node.h"
#include "wm/desks/desk.h"
#include "wm/desks/desk_animation.h"
#include "wm/overview/overview_controller.h"
#include "wm/stacking.h"
#include "wm/toplevel_event_handler.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_state.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::WM {

DesksController::DesksController(WindowManager *manager)
    : m_manager(manager)
{
    // There is always an active desk.
    m_desks.push_back(std::make_unique<Desk>(this));
    m_activeDesk = m_desks.front().get();
    m_activeDesk->m_active = true;
    m_manager->activation()->addObserver(this);
}

DesksController::~DesksController()
{
    m_animation.reset();
    m_finishedAnimation.reset();
    m_manager->activation()->removeObserver(this);
    for (const auto &[window, desk] : m_windowDesks)
        const_cast<Window *>(window)->removeObserver(this);
}

void DesksController::addObserver(DesksObserver *observer)
{
    if (std::find(m_observers.begin(), m_observers.end(), observer) == m_observers.end())
        m_observers.push_back(observer);
}

void DesksController::removeObserver(DesksObserver *observer)
{
    std::erase(m_observers, observer);
}

int DesksController::deskIndex(const Desk *desk) const
{
    for (size_t i = 0; i < m_desks.size(); ++i) {
        if (m_desks[i].get() == desk)
            return int(i);
    }
    return -1;
}

Desk *DesksController::deskAt(int index) const
{
    return index >= 0 && index < int(m_desks.size()) ? m_desks[index].get() : nullptr;
}

Desk *DesksController::deskById(uint32_t id) const
{
    for (const auto &desk : m_desks) {
        if (desk->id() == id)
            return desk.get();
    }
    return nullptr;
}

Desk *DesksController::targetActiveDesk() const
{
    if (m_animation)
        return deskAt(m_animation->endingDeskIndex());
    if (m_deskToActivate)
        return m_deskToActivate;
    return m_activeDesk;
}

Desk *DesksController::chosenDesk() const
{
    if (m_animation && m_animation->chosenDeskIndex() >= 0)
        return deskAt(m_animation->chosenDeskIndex());
    return m_activeDesk;
}

Desk *DesksController::nextDesk() const
{
    return deskAt(deskIndex(targetActiveDesk()) + 1);
}

Desk *DesksController::previousDesk() const
{
    return deskAt(deskIndex(targetActiveDesk()) - 1);
}

bool DesksController::areDesksBeingModified() const
{
    return m_modifying || m_animation;
}

bool DesksController::canCreateDesks() const
{
    return int(m_desks.size()) < kMaxDesks;
}

bool DesksController::canRemoveDesks() const
{
    return m_desks.size() > 1;
}

Desk *DesksController::newDesk(DesksCreationRemovalSource, const std::string &name)
{
    if (!canCreateDesks())
        return nullptr;
    const base::AutoReset modifying(m_modifying, true);
    m_desks.push_back(std::make_unique<Desk>(this));
    Desk *desk = m_desks.back().get();
    // Observers learn of the desk before its name.
    notify([desk](DesksObserver *observer) {
        observer->onDeskAdded(desk);
    });
    if (!name.empty())
        desk->setName(name, /*setByUser=*/true);
    return desk;
}

void DesksController::removeDesk(Desk *desk, DesksCreationRemovalSource source)
{
    if (!canRemoveDesks() || deskIndex(desk) < 0)
        return;
    // A running switch knows its desks by index, which the removal shifts:
    // it lands first. A removal it was making may take `desk` with it.
    if (m_animation) {
        m_animation->finishNow();
        if (!canRemoveDesks() || deskIndex(desk) < 0)
            return;
    }

    // In the overview the next desk takes over at once, as upstream: the
    // removal animation's strip is black where the removed desk's frame was.
    const OverviewController *overview = m_manager->overview();
    const bool inOverview = overview && overview->isActive();
    if (desk == m_activeDesk && !inOverview
        && source != DesksCreationRemovalSource::DesksRestore) {
        // Removing the active desk slides to the desk on its left, or on its
        // right when it is the first.
        const int current = deskIndex(desk);
        const int target = current + (current > 0 ? -1 : 1);
        m_animation = std::make_unique<DeskRemovalAnimation>(this, current, target);
        m_animation->launch();
        return;
    }

    removeDeskInternal(desk, /*deskSwitched=*/false);
}

void DesksController::reorderDesk(int from, int to)
{
    const int count = int(m_desks.size());
    if (m_animation || from == to || from < 0 || from >= count || to < 0 || to >= count)
        return;
    std::unique_ptr<Desk> desk = std::move(m_desks[size_t(from)]);
    m_desks.erase(m_desks.begin() + from);
    m_desks.insert(m_desks.begin() + to, std::move(desk));
    notify([from, to](DesksObserver *observer) {
        observer->onDeskReordered(from, to);
    });
}

void DesksController::activateDesk(Desk *desk, DesksSwitchSource source)
{
    if (deskIndex(desk) < 0 || m_animation)
        return;
    if (desk == m_activeDesk)
        return;

    if (source == DesksSwitchSource::DeskRestored) {
        activateDeskInternal(desk, /*updateWindowActivation=*/true);
        return;
    }

    launchActivationAnimation(desk, source);
}

void DesksController::launchActivationAnimation(Desk *desk, DesksSwitchSource source)
{
    // Activation only moves off the window in use when that window is one of
    // the desks' own; the launcher or a system dialog keeps it.
    Window *active = m_manager->activation()->activeWindow();
    const bool updateWindowActivation =
            source != DesksSwitchSource::WindowActivated && (!active || deskOf(active));

    m_animation = std::make_unique<DeskActivationAnimation>(this, activeDeskIndex(),
                                                            deskIndex(desk), source,
                                                            updateWindowActivation);
    m_animation->launch();
}

bool DesksController::activateAdjacentDesk(bool goingLeft, DesksSwitchSource source)
{
    if (!m_manager->allowsActivation())
        return false;

    // Try replacing an ongoing animation of the same source.
    if (m_animation) {
        if (m_animation->replace(goingLeft, source))
            return true;
        // The animation cannot take another step yet: land it where it was
        // heading and go on from there. A removal still removes its desk.
        m_animation->finishNow();
    }

    if (Desk *desk = goingLeft ? previousDesk() : nextDesk())
        activateDesk(desk, source);
    else
        performHitTheWallAnimation(goingLeft);
    return true;
}

bool DesksController::startSwipeAnimation(bool moveLeft)
{
    // A lone desk has nowhere to go, so the swipe does not even pull at it.
    if (!m_manager->allowsActivation() || m_desks.size() < 2)
        return false;

    constexpr auto source = DesksSwitchSource::DeskSwitchTouchpad;
    if (m_animation) {
        if (m_animation->replace(moveLeft, source))
            return true;
        m_animation->finishNow();
    }
    // With no desk that way the strip holds the active desk alone, and the
    // fingers pull it against the edge instead of it nudging on its own.
    Desk *desk = moveLeft ? previousDesk() : nextDesk();
    launchActivationAnimation(desk ? desk : m_activeDesk, source);
    return true;
}

void DesksController::updateSwipeAnimation(double deskDelta, double timeMs)
{
    if (m_animation)
        m_animation->updateSwipeAnimation(deskDelta, timeMs);
}

void DesksController::endSwipeAnimation(double timeMs)
{
    if (m_animation && m_animation->endSwipeAnimation(timeMs))
        notify([](DesksObserver *observer) { observer->onChosenDeskChanged(); });
}

bool DesksController::moveWindowFromActiveDeskTo(Window *window, Desk *target)
{
    return moveWindowFromSourceDeskTo(window, m_activeDesk, target);
}

void DesksController::sendToDeskAtIndex(Window *window, int index)
{
    Desk *target = deskAt(index);
    if (!window || !target)
        return;
    // A window on every desk the user sends somewhere is on that desk alone.
    if (isVisibleOnAllDesks(window))
        setVisibleOnAllDesks(window, false);
    if (target == m_activeDesk)
        return;
    const bool goingLeft = index < activeDeskIndex();
    if (!belongsToActiveDesk(window)) {
        if (Desk *source = deskOf(window))
            moveWindowFromSourceDeskTo(window, source, target);
        return;
    }
    performWindowMoveToDeskAnimation(window, goingLeft);
    moveWindowFromActiveDeskTo(window, target);
}

bool DesksController::moveWindowFromSourceDeskTo(Window *window, Desk *source, Desk *target)
{
    if (!window || !source || !target || source == target)
        return false;
    Window *root = transientRoot(window);
    // An always-on-top window or a system dialog is on no desk to move from.
    if (!source->contains(root))
        return false;

    const base::AutoReset modifying(m_modifying, true);
    source->moveWindowToDesk(root, target);
    return true;
}

Desk *DesksController::deskOf(const Window *window) const
{
    if (!window)
        return nullptr;
    const auto it = m_windowDesks.find(transientRoot(window));
    return it == m_windowDesks.end() ? nullptr : it->second;
}

bool DesksController::belongsToActiveDesk(const Window *window) const
{
    return deskOf(window) == m_activeDesk && m_activeDesk;
}

bool DesksController::isVisibleOnAllDesks(const Window *window) const
{
    return window && m_visibleOnAllDesks.count(transientRoot(window));
}

void DesksController::setVisibleOnAllDesks(Window *window, bool visibleOnAllDesks)
{
    Window *root = transientRoot(window);
    if (!root || isVisibleOnAllDesks(root) == visibleOnAllDesks)
        return;
    if (visibleOnAllDesks) {
        if (!deskOf(root))
            return;
        m_visibleOnAllDesks.insert(root);
        assignWindow(root, nullptr);
    } else {
        m_visibleOnAllDesks.erase(root);
        // Back on a desk of its own: the one in front of the user.
        if (isDeskWindow(root))
            assignWindow(root, m_activeDesk);
    }
}

void DesksController::setDeskName(Desk *desk, const std::string &name)
{
    if (deskIndex(desk) < 0)
        return;
    desk->setName(std::string(base::strings::trimmed(name)), /*setByUser=*/true);
}

bool DesksController::isDeskWindow(const Window *window) const
{
    return window->role() == WindowRole::Normal && !window->isPopup()
            && layerFor(window) == StackingLayer::Normal;
}

Window *DesksController::transientRoot(Window *window) const
{
    while (window && window->transientParent() && m_manager->hasWindow(window->transientParent()))
        window = window->transientParent();
    return window;
}

const Window *DesksController::transientRoot(const Window *window) const
{
    return transientRoot(const_cast<Window *>(window));
}

void DesksController::onWindowAdded(Window *window)
{
    window->addObserver(this);
    updateWindow(window);
}

void DesksController::onWindowRemoving(Window *window)
{
    window->removeObserver(this);
    // Its transient children stay on its desk, as they stay in the desk
    // container upstream, even as they become roots of their own.
    Desk *desk = deskOf(window);
    const bool onAllDesks = isVisibleOnAllDesks(window);
    for (Window *child : std::vector<Window *>(window->transientChildren())) {
        if (!m_manager->hasWindow(child) || !isDeskWindow(child))
            continue;
        if (onAllDesks)
            m_visibleOnAllDesks.insert(child);
        else if (desk)
            assignWindow(child, desk);
    }
    m_visibleOnAllDesks.erase(window);
    const auto it = m_windowDesks.find(window);
    if (it == m_windowDesks.end())
        return;
    it->second->removeWindow(window);
    m_windowDesks.erase(it);
}

void DesksController::updateWindow(Window *window)
{
    if (!m_manager->hasWindow(window))
        return;
    Window *root = transientRoot(window);
    if (window != root) {
        // A transient child goes wherever its root goes.
        if (m_windowDesks.count(window)) {
            m_windowDesks[window]->removeWindow(window);
            m_windowDesks.erase(window);
        }
        m_visibleOnAllDesks.erase(window);
    } else if (!isDeskWindow(window)) {
        m_visibleOnAllDesks.erase(window);
        if (m_windowDesks.count(window))
            assignWindow(window, nullptr);
    } else if (!m_windowDesks.count(window) && !m_visibleOnAllDesks.count(window)) {
        // Upstream puts every new window in the active desk's container.
        assignWindow(window, m_deskToActivate ? m_deskToActivate : m_activeDesk);
        return;
    }
    updateDeskVisibility(window);
}

void DesksController::onWindowTransientParentChanged(Window *window, Window *)
{
    updateWindow(window);
}

void DesksController::assignWindow(Window *window, Desk *desk)
{
    const auto it = m_windowDesks.find(window);
    Desk *previous = it == m_windowDesks.end() ? nullptr : it->second;
    if (previous == desk)
        return;
    if (previous) {
        previous->removeWindow(window);
        m_windowDesks.erase(it);
    }
    if (desk) {
        desk->addWindow(window);
        m_windowDesks[window] = desk;
    }
    updateDeskVisibility(window);
    notify([window](DesksObserver *observer) {
        observer->onWindowDeskChanged(window);
    });
}

void DesksController::updateDeskVisibility()
{
    // Every window once; transient children are in the list themselves.
    for (Window *window : std::vector<Window *>(m_manager->windows())) {
        if (m_manager->hasWindow(window)) {
            const Desk *desk = deskOf(window);
            window->setDeskHidden(desk && !desk->isActive());
        }
    }
}

void DesksController::updateDeskVisibility(Window *window)
{
    const Desk *desk = deskOf(window);
    window->setDeskHidden(desk && !desk->isActive());
    for (Window *child : std::vector<Window *>(window->transientChildren())) {
        if (m_manager->hasWindow(child))
            updateDeskVisibility(child);
    }
}

void DesksController::onWindowActivating(Window *gainingActive, Window *)
{
    // The desks' own activations, as a desk shows its last used window.
    if (m_modifying || !gainingActive)
        return;
    Desk *desk = deskOf(gainingActive);
    if (!desk || desk == m_activeDesk)
        return;
    // Upstream ignores the activation while an animation runs, which here
    // would leave an active window on a desk nobody sees: the running
    // animation lands at once instead.
    if (m_animation)
        m_animation->finishNow();
    activateDesk(desk, DesksSwitchSource::WindowActivated);
}

void DesksController::activateDeskInternal(Desk *desk, bool updateWindowActivation)
{
    if (!desk || desk == m_activeDesk)
        return;

    const base::AutoReset modifying(m_modifying, true);
    const base::AutoReset toActivate(m_deskToActivate, desk);

    // Upstream cancels every menu before a switch.
    m_manager->dismissPopupGrabs();

    Desk *old = m_activeDesk;
    // A window the user is dragging comes along rather than vanishing from
    // under the pointer, which upstream lets happen.
    if (Window *dragged = transientRoot(m_manager->eventHandler()->draggedWindow());
        dragged && m_windowDesks.count(dragged) && m_windowDesks[dragged] == old)
        assignWindow(dragged, desk);

    // The new desk is marked active first, so that the windows losing
    // activation on the old one do not hand it to another window there.
    m_activeDesk = desk;
    // The ending desk shows before the old one hides, so activation moving off
    // a hidden window finds the new desk's windows.
    desk->activate(updateWindowActivation);
    old->deactivate(updateWindowActivation);

    LM_DEBUG(lingmoAurora) << "desk" << deskIndex(desk) << "active";
    notify([desk, old](DesksObserver *observer) {
        observer->onDeskActivationChanged(desk, old);
    });
}

void DesksController::removeDeskInternal(Desk *desk, bool deskSwitched)
{
    const int index = deskIndex(desk);
    if (index < 0 || !canRemoveDesks())
        return;

    const base::AutoReset modifying(m_modifying, true);
    const bool wasActive = desk->isActive() || deskSwitched;
    std::unique_ptr<Desk> removed = std::move(m_desks[index]);
    m_desks.erase(m_desks.begin() + index);

    if (wasActive) {
        Desk *target = m_desks[index > 0 ? index - 1 : 0].get();
        // With the removal animation the target desk is active already.
        // Upstream moves the windows first; activating first spares the
        // removed desk's active window a round through the hidden target.
        if (!deskSwitched)
            activateDeskInternal(target, /*updateWindowActivation=*/true);
        removed->moveWindowsToDesk(target);
    } else {
        removed->moveWindowsToDesk(m_activeDesk);
    }
    // Every window of the removed desk found a new one; nothing refers to it.
    for (auto it = m_windowDesks.begin(); it != m_windowDesks.end();)
        it = it->second == removed.get() ? m_windowDesks.erase(it) : std::next(it);
    updateDeskVisibility();

    notify([&removed](DesksObserver *observer) {
        observer->onDeskRemoved(removed.get());
    });
}

void DesksController::onAnimationFinished(DeskAnimationBase *animation)
{
    if (m_animation.get() != animation)
        return;
    // It is still on the stack; the one retired before it is long done.
    m_finishedAnimation = std::move(m_animation);
}

scene::DeskSwitchNode::Frames
DesksController::takeScreenshots(const std::function<void(render::Renderer &)> &paint) const
{
    if (!m_screenshotter)
        return {};
    return m_screenshotter(paint);
}

scene::DeskSwitchNode::Frames
DesksController::takeWindowScreenshots(const std::function<bool(const Window *)> &filter) const
{
    std::vector<Window *> windows;
    for (Window *window : m_manager->windows()) {
        if (window->isVisible() && filter(window))
            windows.push_back(window);
    }
    return takeScreenshots([&windows](render::Renderer &renderer) {
        renderer.clear({0, 0, 0, 0});
        for (Window *window : windows) {
            if (scene::Node *node = window->node())
                node->render(renderer);
        }
    });
}

int DesksController::referenceWidth() const
{
    return m_manager->outputForNewWindow().width();
}

void DesksController::performWindowMoveToDeskAnimation(Window *window, bool goingLeft)
{
    // The entire transient tree appears to go together, whichever of its
    // windows was sent.
    Window *root = transientRoot(window);
    if (!m_windowMoveNode || !root->node() || !root->isVisible())
        return;
    auto frames = takeWindowScreenshots([this, root](const Window *candidate) {
        return transientRoot(candidate) == root;
    });
    m_windowMoveNode->setGeometry(m_manager->outputBounds());
    m_windowMoveNode->slideOut(std::move(frames), root->bounds(), m_manager->outputBoundsFor(root),
                               goingLeft);
}

void DesksController::performHitTheWallAnimation(bool goingLeft)
{
    if (!m_switchNode || m_switchNode->isActive())
        return;
    auto frames = takeScreenshots([this](render::Renderer &renderer) {
        m_switchNode->paintCoveredContent(renderer);
    });
    if (frames.empty())
        return;
    m_switchNode->setGeometry(m_manager->outputBounds());
    m_switchNode->setFinishedCallback([this] {
        m_manager->runLater(std::chrono::milliseconds(0), [this] {
            // Another animation may have taken the node over meanwhile.
            if (!m_animation && m_switchNode)
                m_switchNode->stop();
        });
    });
    m_switchNode->hitTheWall(goingLeft, std::move(frames), referenceWidth());
}

} // namespace Lingmo::WM
