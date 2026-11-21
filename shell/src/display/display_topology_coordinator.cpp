// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "display/display_topology_coordinator.h"
#include "display/recommended_scale.h"

#include <algorithm>
#include <set>

#include "base/event_loop.h"
#include "core/main_loop.h"

namespace Lingmo::Display {

DisplayTopologyCoordinator::DisplayTopologyCoordinator(DisplayManager *manager,
                                                       CallbackNativeDisplayDelegate *delegate,
                                                       DisplayConfigurationController *controller,
                                                       Probe probe, DisplayPreferences preferences)
    : m_manager(manager)
    , m_delegate(delegate)
    , m_controller(controller)
    , m_probe(std::move(probe))
    , m_preferences(std::move(preferences))
    , m_retry(core::mainLoop())
{
    m_retry.setSingleShot(true);
    m_retry.onTimeout = [this] { run(); };
    // Handled from the loop: the operation's own caller finishes first.
    m_operationFinished = controller->operationFinished.connect(
            [this](const std::string &token, const ConfigurationResult &result) {
                core::mainLoop()->post([this, alive = std::weak_ptr<bool>(m_alive), token,
                                        succeeded = result.succeeded] {
                    if (alive.expired())
                        return;
                    if (m_running && token == m_operationToken) {
                        m_running = false;
                        if (!m_pending)
                            m_controller->setTopologyPending(false);
                        reconciled(succeeded);
                    }
                    if (m_pending)
                        m_retry.start(std::chrono::milliseconds(0));
                });
            });
}

void DisplayTopologyCoordinator::invalidate()
{
    if (!m_pending && !m_running && m_manager->applied()
        && !m_manager->applied()->logicalDisplays.empty())
        m_previous = m_controller->recoveryState();
    schedule();
}

void DisplayTopologyCoordinator::start()
{
    m_previous.reset();
    schedule();
}

void DisplayTopologyCoordinator::schedule()
{
    m_pending = true;
    m_attempts = 0;
    m_controller->setTopologyPending(true);
    m_delegate->invalidateSnapshots();
    if (!m_retry.isActive())
        m_retry.start(std::chrono::milliseconds(100));
}

DisplayConfiguration DisplayTopologyCoordinator::candidate(const HardwareSnapshot &hardware,
                                                           uint64_t revision) const
{
    if (auto saved = m_preferences.restore(hardware, revision)) {
        if (DisplayConfigurationValidator::validateExtended(hardware, revision, *saved).succeeded)
            return *saved;
    }
    DisplayConfiguration result;
    result.base = {hardware.compositorInstanceId, hardware.topologyGeneration, revision};
    std::set<DisplayId> active;
    for (const auto &physical : hardware.displays) {
        const LogicalDisplay *logical = nullptr;
        if (m_previous) {
            for (const auto &old : m_previous->logicalDisplays) {
                if (old.hasDestination(physical.sessionId)) {
                    logical = &old;
                    break;
                }
            }
        }
        auto mode = physical.nativeMode.value_or(
                physical.currentMode.value_or(physical.modes.front()));
        bool previouslyKnown = false;
        if (m_previous) {
            for (const auto &old : m_previous->physicalDisplays) {
                if (old.sessionId != physical.sessionId)
                    continue;
                previouslyKnown = true;
                if (old.currentMode) {
                    for (const auto &available : physical.modes)
                        if (available.stableKey() == old.currentMode->stableKey())
                            mode = available;
                }
            }
        }
        const bool enabled = logical || !previouslyKnown;
        result.displays.push_back({physical.sessionId, enabled, mode,
                                   logical ? logical->scale : recommendedScale(physical),
                                   logical ? logical->rotation : DisplayRotation::Normal});
        if (enabled)
            active.insert(physical.sessionId);
        if (logical && logical->primary && result.primary.value().empty())
            result.primary = physical.sessionId;
    }
    if (active.empty() && !result.displays.empty()) {
        result.displays.front().enabled = true;
        active.insert(result.displays.front().display);
    }
    if (!active.contains(result.primary)) {
        for (const auto &request : result.displays) {
            if (request.enabled) {
                result.primary = request.display;
                break;
            }
        }
    }
    std::set<DisplayId> mirrorSinks;
    if (m_previous) {
        for (const auto &logical : m_previous->logicalDisplays) {
            std::vector<DisplayId> group;
            for (const auto &id : logical.destinations)
                if (active.contains(id))
                    group.push_back(id);
            if (group.size() > 1) {
                result.mirrorGroups.push_back(group);
                mirrorSinks.insert(group.begin() + 1, group.end());
                if (std::ranges::find(group, result.primary) != group.end())
                    result.primary = group.front();
            }
        }
        for (const auto &placement : m_previous->placements) {
            if (active.contains(placement.display) && active.contains(placement.parent)
                && !mirrorSinks.contains(placement.display)
                && !mirrorSinks.contains(placement.parent))
                result.placements.push_back(placement);
        }
    }
    if (DisplayConfigurationValidator::validateExtended(hardware, revision, result).succeeded)
        return result;
    result.placements.clear();
    DisplayId parent = result.primary;
    for (const auto &request : result.displays) {
        if (!request.enabled || request.display == result.primary
            || mirrorSinks.contains(request.display))
            continue;
        result.placements.push_back({request.display, parent, LayoutEdge::Right, 0});
        parent = request.display;
    }
    return result;
}

void DisplayTopologyCoordinator::run()
{
    if (!m_pending || m_running || m_controller->busy())
        return;
    if (m_manager->applied())
        m_revision = std::max(m_revision, m_manager->applied()->revision);
    m_manager->clearApplied(m_manager->hardware()->topologyGeneration);
    auto hardware = m_probe();
    if (!hardware || !hardware->isValid()) {
        if (++m_attempts < 3)
            m_retry.start(std::chrono::milliseconds(250));
        else {
            m_pending = false;
            m_controller->setTopologyPending(false);
            reconciled(false);
        }
        return;
    }
    const uint64_t revision = ++m_revision;
    const auto request = candidate(*hardware, revision);
    if (!m_manager->installHardware(*hardware)) {
        reconciled(false);
        return;
    }
    m_delegate->setSnapshot(*hardware);
    AppliedConfiguration empty;
    empty.compositorInstanceId = hardware->compositorInstanceId;
    empty.topologyGeneration = hardware->topologyGeneration;
    empty.revision = revision;
    empty.physicalDisplays = hardware->displays;
    for (auto &physical : empty.physicalDisplays)
        physical.currentMode.reset();
    if (!m_manager->installApplied(empty)) {
        reconciled(false);
        return;
    }
    m_pending = false;
    if (hardware->displays.empty()) {
        m_controller->setTopologyPending(false);
        reconciled(true);
        return;
    }
    m_running = true;
    m_operationToken = m_controller->apply(request, "topology", -1);
}

} // namespace Lingmo::Display
