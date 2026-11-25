// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "wm/stage/stage_manager.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <limits>

#include "base/auto_reset.h"
#include "core/main_loop.h"
#include "core/preferences.h"
#include "display/display_manager.h"
#include "wm/desks/desk.h"
#include "wm/mru_window_tracker.h"
#include "wm/overview/overview_controller.h"
#include "wm/show_desktop_controller.h"
#include "wm/stacking.h"
#include "wm/toplevel_event_handler.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_state.h"

namespace Lingmo::WM {

StageManager::StageManager(WindowManager *manager)
    : m_manager(manager)
    , m_settings(preferencesFile("shell"))
    , m_transitionTimer(core::mainLoop())
{
    std::error_code error;
    std::filesystem::create_directories(
            std::filesystem::path(m_settings.path()).parent_path(), error);
    m_settingsWatch = base::FileWatch::file(m_settings.path(), [this] {
        reloadPreferences();
    }, core::mainLoop());
    reloadPreferences();
    m_transitionTimer.setInterval(std::chrono::milliseconds(16));
    m_transitionTimer.onTimeout = [this] { tickTransition(); };
    m_manager->desks()->addObserver(this);
    m_manager->activation()->addObserver(this);
}

StageManager::~StageManager()
{
    finishTransition();
    m_transitionTimer.stop();
    m_manager->desks()->removeObserver(this);
    m_manager->activation()->removeObserver(this);
    for (Window *window : m_manager->windows())
        window->removeObserver(this);
}

Window *StageManager::root(Window *window) const
{
    while (window && window->transientParent())
        window = window->transientParent();
    return window;
}

bool StageManager::eligible(const Window *window) const
{
    return window && !window->isPopup() && !window->isShellWindow()
            && window->role() == WindowRole::Normal && !window->skipTaskbar()
            && layerFor(window) == StackingLayer::Normal
            && m_manager->desks()->deskOf(window)
            && !m_manager->desks()->isVisibleOnAllDesks(window);
}

std::vector<StageManager::Output> StageManager::outputs() const
{
    std::vector<Output> result;
    if (const auto *manager = m_manager->displayManager()) {
        if (!manager->applied())
            return result;
        for (const auto &logical : manager->applied()->logicalDisplays) {
            Output output {logical.id.value(), logical.bounds, {}};
            for (const auto &physical : manager->applied()->physicalDisplays) {
                if (logical.hasDestination(physical.sessionId) && !physical.persistentKey.empty())
                    output.physical.push_back(physical.persistentKey);
            }
            result.push_back(std::move(output));
        }
    } else if (!m_manager->outputBounds().isEmpty()) {
        result.push_back({"default", m_manager->outputBounds(), {}});
    }
    return result;
}

std::string StageManager::outputFor(const Window *window) const
{
    std::string id;
    int64_t best = -1;
    // Keep the previous owner on equal overlap while crossing an edge.
    const Group *previous = find(groupFor(window));
    for (const auto &output : m_outputs) {
        const auto overlap = output.bounds.intersected(window->bounds());
        const int64_t area = int64_t(overlap.width()) * overlap.height();
        if (area > best || (area == best && previous && previous->output == output.id)) {
            best = area;
            id = output.id;
        }
    }
    return id;
}

StageManager::Group *StageManager::find(Id id)
{
    const auto it = std::find_if(m_groups.begin(), m_groups.end(), [id](const Group &g) { return g.id == id; });
    return it == m_groups.end() ? nullptr : &*it;
}

const StageManager::Group *StageManager::find(Id id) const
{
    return const_cast<StageManager *>(this)->find(id);
}

StageManager::Id StageManager::groupFor(const Window *window) const
{
    const Window *family = root(const_cast<Window *>(window));
    if (family) {
        for (const Group &group : m_groups) {
            if (std::find(group.windows.begin(), group.windows.end(), family->id()) != group.windows.end())
                return group.id;
        }
    }
    return 0;
}

std::map<std::string, StageManager::Id> &StageManager::active()
{
    return m_active[m_manager->desks()->activeDesk()->id()];
}

StageManager::Id StageManager::makeGroup(Window *window, const std::string &output)
{
    const uint32_t desk = m_manager->desks()->deskOf(window)->id();
    const Id id = m_nextId++;
    m_groups.push_back({id, desk, output, {window->id()}, window->id(), ++m_clock});
    auto &choice = m_active[desk][output];
    if (!choice)
        choice = id;
    return id;
}

void StageManager::adopt(Window *window)
{
    Window *family = root(window);
    if (!eligible(family) || groupFor(family) || m_outputs.empty())
        return;
    // Initial enrollment keeps each root separate. A newly mapped window joins
    // its transient parent's group, otherwise forms a new current task.
    makeGroup(family, outputFor(family));
}

StageManager::Snapshot StageManager::snapshot() const
{
    Snapshot result;
    result.enabled = m_enabled;
    result.blocked = m_blocked || m_outputs.empty();
    result.revision = m_revision;
    result.desk = m_manager->desks()->activeDesk()->id();
    for (const Group &group : m_groups) {
        if (group.desk == result.desk)
            result.groups.push_back(group);
    }
    if (const auto it = m_active.find(result.desk); it != m_active.end())
        result.active = it->second;
    return result;
}

void StageManager::publish()
{
    ++m_revision;
    changed(snapshot());
}

void StageManager::reloadPreferences()
{
    m_settings.reload();
    if (m_settings.contains("StageManager/Enabled"))
        setEnabled(preferenceEnabled());
}

void StageManager::beginTransition(Id target, Window *preferred, bool focus)
{
    finishTransition();
    m_transitionPreferred = preferred;
    m_transitionFocus = focus;
    const Group *destination = find(target);
    if (!destination)
        return;

    m_transition.clear();
    m_transitionActive = true;
    m_manager->activation()->beginVisibilityUpdate();
    for (Window *window : m_manager->windows()) {
        const Group *group = find(groupFor(window));
        if (!group || group->desk != destination->desk || group->output != destination->output)
            continue;
        const bool incoming = group->id == target
                && (window->isStageHidden() || window == preferred);
        const bool outgoing = group->id != target && !window->isStageHidden();
        if (!incoming && !outgoing)
            continue;

        const base::Rect bounds = window->bounds();
        const base::Rect workArea = m_manager->workAreaFor(window);
        const int leftOffset = workArea.left() - bounds.right() - 32;
        const int entering = -std::min(220, std::max(80, bounds.width() / 3));
        Motion motion;
        motion.window = window;
        motion.outgoing = outgoing;
        motion.fromOffset = outgoing ? base::Point() : base::Point(entering, 0);
        motion.toOffset = outgoing ? base::Point(leftOffset, 0) : base::Point();
        motion.fromScale = outgoing ? 1.0 : .92;
        motion.toScale = outgoing ? .90 : 1.0;
        motion.fromOpacity = outgoing ? window->opacity() : 0;
        motion.toOpacity = outgoing ? 0 : window->opacity();
        m_transition.push_back(motion);

        window->setStageTransitioning(true);
        if (incoming) {
            window->setStageHidden(false);
        } else if (outgoing) {
            window->setStageHidden(true);
        }
        const base::PointF anchor(bounds.width() / 2.0, bounds.height() / 2.0);
        window->setPresentationOffset(motion.fromOffset);
        window->setPresentationScale(motion.fromScale, anchor, false);
        window->setOpacity(motion.fromOpacity);
    }
    m_manager->activation()->endVisibilityUpdate(focus, preferred);
    if (m_transition.empty()) {
        m_transitionActive = false;
        reconcile(preferred, focus);
        return;
    }
    m_transitionStarted = std::chrono::steady_clock::now();
    m_transitionTimer.start();
    tickTransition();
}

void StageManager::tickTransition()
{
    if (!m_transitionActive)
        return;
    constexpr double duration = 280.0;
    const double elapsed = std::chrono::duration<double, std::milli>(
                                   std::chrono::steady_clock::now() - m_transitionStarted)
                                   .count();
    const double linear = std::clamp(elapsed / duration, 0.0, 1.0);
    const double progress = 1 - std::pow(1 - linear, 3);
    for (const Motion &motion : m_transition) {
        Window *window = motion.window;
        if (!m_manager->hasWindow(window))
            continue;
        const auto interpolate = [progress](int from, int to) {
            return int(std::lround(from + (to - from) * progress));
        };
        const base::Point offset(interpolate(motion.fromOffset.x(), motion.toOffset.x()),
                                 interpolate(motion.fromOffset.y(), motion.toOffset.y()));
        const double scale = motion.fromScale + (motion.toScale - motion.fromScale) * progress;
        const float opacity = float(motion.fromOpacity
                                    + (motion.toOpacity - motion.fromOpacity) * progress);
        window->setPresentationOffset(offset);
        window->setPresentationScale(scale,
                                     {window->bounds().width() / 2.0, window->bounds().height() / 2.0},
                                     false);
        window->setOpacity(opacity);
    }
    if (linear >= 1)
        finishTransition();
}

void StageManager::finishTransition()
{
    if (!m_transitionActive)
        return;
    m_transitionTimer.stop();
    const auto motions = std::move(m_transition);
    Window *preferred = m_transitionPreferred;
    const bool focus = m_transitionFocus;
    m_transitionPreferred = nullptr;
    m_transitionActive = false;
    for (const Motion &motion : motions) {
        Window *window = motion.window;
        if (!m_manager->hasWindow(window))
            continue;
        window->setPresentationOffset({});
        window->setPresentationScale(1,
                                     {window->bounds().width() / 2.0, window->bounds().height() / 2.0},
                                     false);
        window->setOpacity(motion.outgoing ? motion.fromOpacity : motion.toOpacity);
    }
    reconcile(preferred, focus);
    for (const Motion &motion : motions) {
        if (m_manager->hasWindow(motion.window))
            motion.window->setStageTransitioning(false);
    }
}

void StageManager::setEnabled(bool enabled)
{
    if (m_enabled == enabled || m_updating || (!m_manager->allowsActivation() && enabled))
        return;
    if (m_transitionActive)
        finishTransition();
    m_enabled = enabled;
    if (enabled) {
        m_outputs = outputs();
        for (Window *window : m_manager->windows()) {
            window->addObserver(this);
            adopt(window);
        }
        for (const auto &output : m_outputs) {
            auto &choice = active()[output.id];
            const bool valid = find(choice) && find(choice)->desk == m_manager->desks()->activeDesk()->id();
            if (!valid) {
                const auto it = std::find_if(m_groups.begin(), m_groups.end(), [&](const Group &group) {
                    return group.desk == m_manager->desks()->activeDesk()->id()
                            && group.output == output.id;
                });
                if (it != m_groups.end())
                    choice = it->id;
            }
        }
        if (Window *current = m_manager->activation()->activeWindow()) {
            if (const auto *group = find(groupFor(current)))
                m_active[group->desk][group->output] = group->id;
        }
    } else {
        for (Window *window : m_manager->windows()) {
            window->setStageHidden(false);
            window->removeObserver(this);
        }
    }
    reconcile();
    publish();
}

void StageManager::setSessionBlocked(bool blocked)
{
    if (m_blocked == blocked)
        return;
    m_blocked = blocked;
    publish();
}

Window *StageManager::candidate(const Group &group) const
{
    Window *last = m_manager->windowById(group.lastActive);
    if (last && groupFor(last) == group.id && last->targetVisibility() && !last->state()->isMinimized())
        return last;
    for (Window *window : m_manager->mruWindowTracker()->buildMruWindowList()) {
        if (groupFor(window) == group.id && window->targetVisibility() && !window->state()->isMinimized())
            return window;
    }
    return nullptr;
}

void StageManager::prune()
{
    for (auto &group : m_groups) {
        std::erase_if(group.windows, [this, &group](uint64_t id) {
            Window *window = m_manager->windowById(id);
            return !eligible(window) || root(window) != window
                    || m_manager->desks()->deskOf(window)->id() != group.desk;
        });
    }
    std::erase_if(m_groups, [this](const Group &g) {
        if (!g.windows.empty())
            return false;
        m_homes.erase(g.id);
        return true;
    });
    for (auto &[desk, choices] : m_active) {
        std::erase_if(choices, [this, desk](const auto &entry) {
            const Group *group = find(entry.second);
            return !group || group->desk != desk || group->output != entry.first;
        });
    }
    for (const auto &group : m_groups) {
        auto &choice = m_active[group.desk][group.output];
        if (!choice || !find(choice))
            choice = group.id;
    }
}

void StageManager::reconcile(Window *preferred, bool focus)
{
    if (m_updating || m_transitionActive)
        return;
    const base::AutoReset updating(m_updating, true);
    prune();
    m_manager->activation()->beginVisibilityUpdate();
    for (Window *window : m_manager->windows()) {
        const Group *group = find(groupFor(window));
        const bool hidden = m_enabled && group && eligible(root(window))
                && m_active[group->desk][group->output] != group->id;
        window->setStageHidden(hidden);
    }
    m_manager->activation()->endVisibilityUpdate(focus, preferred);
    m_manager->pointerTargetsChanged();
}

bool StageManager::switchTo(Id id, bool focus)
{
    if (!m_enabled || m_updating || m_blocked || !m_manager->allowsActivation()
        || m_outputChanging || m_manager->overview()->isActive())
        return false;
    if (m_transitionActive)
        finishTransition();
    Group *group = find(id);
    if (!group || group->desk != m_manager->desks()->activeDesk()->id())
        return false;
    if (m_manager->eventHandler()->draggedWindow())
        return false;
    // Return from Show Desktop with its existing frame-driven animation. The
    // transition marks its own windows so the two presentation transforms do
    // not race during the overlap.
    m_manager->showDesktop()->exit();
    m_manager->dismissPopupGrabs();
    group->lastUsed = ++m_clock;
    active()[group->output] = id;
    Window *target = candidate(*group);
    beginTransition(id, target, focus);
    publish();
    return true;
}

bool StageManager::moveWindow(uint64_t id, Id target)
{
    Window *window = root(m_manager->windowById(id));
    const Group *destination = find(target);
    if (!m_enabled || m_updating || m_blocked || !m_manager->allowsActivation()
        || !eligible(window) || !destination || m_outputChanging
        || destination->desk != m_manager->desks()->deskOf(window)->id())
        return false;
    const Id previous = groupFor(window);
    if (previous == target)
        return true;
    const std::string currentOutput = outputFor(window);
    const std::string output = destination->output;
    if (Group *old = find(previous))
        std::erase(old->windows, window->id());
    // Removing the last family from its previous group must be visible in the
    // same transaction as the join; otherwise observers briefly receive an
    // empty group and may render a phantom thumbnail.
    prune();
    Group *group = find(target);
    group->windows.push_back(window->id());
    group->lastActive = window->id();
    group->lastUsed = ++m_clock;
    m_homes.erase(previous);
    m_homes.erase(target);
    m_active[group->desk][output] = target;
    // A cross-output drop translates the family only once. Same-output joins
    // do not request any geometry from the client.
    if (currentOutput != output) {
        const auto from = std::find_if(m_outputs.begin(), m_outputs.end(), [&](const Output &o) { return o.id == currentOutput; });
        const auto to = std::find_if(m_outputs.begin(), m_outputs.end(), [&](const Output &o) { return o.id == output; });
        if (from != m_outputs.end() && to != m_outputs.end()) {
            const base::Point delta = to->bounds.topLeft() - from->bounds.topLeft();
            const base::AutoReset updating(m_updating, true);
            window->requestBounds(window->bounds().translated(delta));
        }
    }
    // The destination transition treats the joined window as an incoming
    // member and hides its client surface until the presentation settles.
    beginTransition(target, window,
                    window->targetVisibility() && !window->state()->isMinimized());
    publish();
    return true;
}

bool StageManager::separateWindow(uint64_t id)
{
    Window *window = root(m_manager->windowById(id));
    if (!m_enabled || m_updating || m_blocked || !m_manager->allowsActivation() || !eligible(window))
        return false;
    Group *old = find(groupFor(window));
    if (!old || old->windows.size() < 2)
        return false;
    const std::string output = old->output;
    const Id oldId = old->id;
    std::erase(old->windows, window->id());
    m_homes.erase(oldId);
    makeGroup(window, output);
    // Leave the remaining task in front; the detached window goes to the strip.
    m_active[m_manager->desks()->deskOf(window)->id()][output] = oldId;
    Window *target = candidate(*find(oldId));
    beginTransition(oldId, target, true);
    publish();
    return true;
}

bool StageManager::cycle(bool forward)
{
    const auto *current = find(groupFor(m_manager->activation()->activeWindow()));
    if (!current)
        return false;
    std::vector<Id> choices;
    for (const auto &group : m_groups) {
        if (group.desk == current->desk && group.output == current->output)
            choices.push_back(group.id);
    }
    const auto it = std::find(choices.begin(), choices.end(), current->id);
    if (it == choices.end() || choices.size() < 2)
        return false;
    const size_t index = size_t(it - choices.begin());
    return switchTo(choices[(index + (forward ? 1 : choices.size() - 1)) % choices.size()]);
}

void StageManager::onWindowAdded(Window *window)
{
    if (!m_enabled)
        return;
    if (m_transitionActive)
        finishTransition();
    window->addObserver(this);
    adopt(window);
    reconcile();
    publish();
}

void StageManager::onWindowRemoving(Window *window)
{
    if (m_transitionActive)
        finishTransition();
    window->removeObserver(this);
    const Id removedGroup = groupFor(window);
    if (Group *group = find(removedGroup))
        std::erase(group->windows, window->id());
    for (auto &[id, home] : m_homes)
        home.bounds.erase(window->id());
    if (m_enabled) {
        // The window has left WM's registry, but activation still knows it.
        // Expose a replacement before Activation chooses its fallback.
        reconcile(window, true);
        publish();
    }
}

void StageManager::prepareActivation(Window *window)
{
    if (!m_enabled || m_updating || m_blocked || m_transitionActive)
        return;
    adopt(window);
    if (Group *group = find(groupFor(window))) {
        m_active[group->desk][group->output] = group->id;
        group->lastActive = window->id();
        group->lastUsed = ++m_clock;
        reconcile();
    }
}

void StageManager::onWindowActivated(Window *gained, Window *)
{
    if (!m_enabled || m_updating)
        return;
    if (Group *group = find(groupFor(gained))) {
        group->lastActive = gained->id();
        group->lastUsed = ++m_clock;
    }
    publish();
}

void StageManager::onWindowBoundsChanged(Window *window, const base::Rect &)
{
    if (m_enabled && !m_updating && !m_outputChanging && !window->isDragged()) {
        m_homes.erase(groupFor(window));
        onWindowDragStateChanged(window, false);
    }
}

void StageManager::onWindowDragStateChanged(Window *window, bool dragged)
{
    if (!m_enabled || dragged || m_updating || m_outputChanging || root(window) != window)
        return;
    m_homes.erase(groupFor(window));
    Group *group = find(groupFor(window));
    if (!group)
        return;
    const std::string output = outputFor(window);
    if (output.empty() || output == group->output)
        return;
    const Id oldId = group->id;
    const uint32_t desk = group->desk;
    const auto destination = m_active[desk].find(output);
    if (destination != m_active[desk].end()) {
        moveWindow(window->id(), destination->second);
    } else {
        std::erase(group->windows, window->id());
        makeGroup(window, output);
        m_homes.erase(oldId);
        reconcile(window, window->isActive());
        publish();
    }
}

void StageManager::onWindowTransientParentChanged(Window *, Window *)
{
    if (!m_enabled || m_updating)
        return;
    prune();
    for (Window *window : m_manager->windows())
        adopt(window);
    reconcile();
    publish();
}

void StageManager::onWindowStateTypeChanged(Window *, WindowStateType)
{
    if (m_enabled)
        publish();
}

void StageManager::onWindowTitleChanged(Window *)
{
    if (m_enabled)
        publish();
}

void StageManager::onWindowContentChanged(Window *window)
{
    if (!m_enabled || !window)
        return;
    if (Group *group = find(groupFor(window))) {
        ++group->contentRevision;
        publish();
    }
}

void StageManager::onWindowDeskChanged(Window *)
{
    onWindowTransientParentChanged(nullptr, nullptr);
}

void StageManager::onDeskActivationChanged(const Desk *, const Desk *)
{
    if (m_enabled) {
        reconcile();
        publish();
    }
}

void StageManager::onDeskRemoved(const Desk *desk)
{
    m_active.erase(desk->id());
    onWindowTransientParentChanged(nullptr, nullptr);
}

void StageManager::beginOutputChange()
{
    if (!m_enabled || m_outputChanging)
        return;
    m_outputChanging = true;
    for (const auto &group : m_groups) {
        if (m_homes.contains(group.id))
            continue;
        Home home;
        home.output = group.output;
        for (const auto &output : m_outputs) {
            if (output.id == group.output)
                home.physical = output.physical;
        }
        for (uint64_t id : group.windows) {
            if (Window *window = m_manager->windowById(id))
                home.bounds[id] = window->bounds();
        }
        m_homes[group.id] = std::move(home);
    }
}

void StageManager::outputsChanged()
{
    if (!m_enabled)
        return;
    const auto next = outputs();
    if (next.empty()) {
        // Reprobe's empty baseline is not a disconnect. Keep every record.
        m_outputChanging = false;
        publish();
        return;
    }
    {
    const base::AutoReset updating(m_updating, true);
    m_outputs = next;
    const Id focusedGroup = groupFor(m_manager->activation()->activeWindow());
    for (auto &group : m_groups) {
        const auto home = m_homes.find(group.id);
        const Output *preferred = nullptr;
        if (home != m_homes.end()) {
            for (const auto &output : next) {
                bool matches = output.id == home->second.output;
                for (const auto &key : home->second.physical)
                    matches |= std::find(output.physical.begin(), output.physical.end(), key) != output.physical.end();
                if (matches) {
                    if (preferred) { preferred = nullptr; break; }
                    preferred = &output;
                }
            }
        }
        const bool ownerExists = std::any_of(next.begin(), next.end(), [&](const Output &o) { return o.id == group.output; });
        if (preferred && !ownerExists) {
            group.output = preferred->id;
        } else if (!ownerExists) {
            group.output = next.front().id;
        } else if (preferred && group.output != preferred->id) {
            group.output = preferred->id;
            for (const auto &[id, bounds] : home->second.bounds) {
                if (Window *window = m_manager->windowById(id); window && window->state()->isNormal())
                    window->requestBounds(bounds);
            }
        }
        if (group.id == focusedGroup)
            m_active[group.desk][group.output] = group.id;
    }
    m_outputChanging = false;
    }
    reconcile();
    publish();
}

} // namespace Lingmo::WM
