// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/base/accelerators/accelerator_manager.cc
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "accelerators/accelerator_manager.h"

#include <algorithm>
#include <cassert>

#include "core/logging.h"

namespace Lingmo::Accelerators {

AcceleratorManager::AcceleratorManager() = default;

AcceleratorManager::~AcceleratorManager() = default;

void AcceleratorManager::registerAccelerators(const std::vector<Accelerator> &accelerators,
                                              HandlerPriority priority, AcceleratorTarget *target)
{
    assert(target);
    for (const Accelerator &accelerator : accelerators) {
        TargetInfo &info = m_accelerators.getOrInsertDefault(accelerator);
        if (info.contains(target)) {
            // Registering the same target twice would let one unregister
            // leave a stale entry behind.
            continue;
        }
        info.registerWithPriority(target, priority);
    }
}

void AcceleratorManager::unregisterAccelerator(const Accelerator &accelerator,
                                               AcceleratorTarget *target)
{
    TargetInfo *info = m_accelerators.find(accelerator);
    if (!info) {
        LM_WARNING(lingmoInput) << "Unregistering an unknown accelerator" << accelerator.shortcutText();
        return;
    }
    if (info->unregister(target) && !info->hasTargets())
        m_accelerators.erase(accelerator);
}

void AcceleratorManager::unregisterAll(AcceleratorTarget *target)
{
    for (auto it = m_accelerators.begin(); it != m_accelerators.end();) {
        TargetInfo &info = it->second;
        info.unregister(target);
        if (!info.hasTargets()) {
            const Accelerator accelerator = it->first;
            ++it;
            m_accelerators.erase(accelerator);
        } else {
            ++it;
        }
    }
}

bool AcceleratorManager::isRegistered(const Accelerator &accelerator) const
{
    const TargetInfo *info = m_accelerators.find(accelerator);
    return info && info->hasTargets();
}

bool AcceleratorManager::process(const Accelerator &accelerator)
{
    TargetInfo *info = m_accelerators.find(accelerator);
    return info && info->tryProcess(accelerator);
}

bool AcceleratorManager::hasPriorityHandler(const Accelerator &accelerator) const
{
    const TargetInfo *info = m_accelerators.find(accelerator);
    return info && info->hasPriorityHandler();
}

void AcceleratorManager::TargetInfo::registerWithPriority(AcceleratorTarget *target,
                                                          HandlerPriority priority)
{
    if (priority == HighPriority) {
        // Only one priority handler per accelerator, and it goes first.
        assert(!m_hasPriorityHandler);
        m_hasPriorityHandler = true;
        m_targets.push_front(target);
    } else {
        // Normal-priority targets go after the priority handler, most recent first.
        m_targets.insert(m_hasPriorityHandler ? std::next(m_targets.begin()) : m_targets.begin(),
                         target);
    }
}

bool AcceleratorManager::TargetInfo::unregister(AcceleratorTarget *target)
{
    const auto it = std::find(m_targets.begin(), m_targets.end(), target);
    if (it == m_targets.end())
        return false;
    if (it == m_targets.begin() && m_hasPriorityHandler)
        m_hasPriorityHandler = false;
    m_targets.erase(it);
    return true;
}

bool AcceleratorManager::TargetInfo::tryProcess(const Accelerator &accelerator)
{
    // A handler may unregister others while it runs; iterate over a copy.
    for (AcceleratorTarget *target : std::vector<AcceleratorTarget *>(m_targets.begin(),
                                                                       m_targets.end())) {
        if (std::find(m_targets.begin(), m_targets.end(), target) == m_targets.end())
            continue;
        if (target->canHandleAccelerators() && target->acceleratorPressed(accelerator))
            return true;
    }
    return false;
}

bool AcceleratorManager::TargetInfo::hasPriorityHandler() const
{
    return m_hasPriorityHandler && !m_targets.empty()
            && m_targets.front()->canHandleAccelerators();
}

bool AcceleratorManager::TargetInfo::contains(AcceleratorTarget *target) const
{
    return std::find(m_targets.begin(), m_targets.end(), target) != m_targets.end();
}

} // namespace Lingmo::Accelerators
