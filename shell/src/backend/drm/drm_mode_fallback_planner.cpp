// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/gpu/drm_gpu_display_manager.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: a deterministic breadth-first search bounds driver
// tests and keeps internal panels fixed while MST branches shed bandwidth.

#include "backend/drm/drm_mode_fallback_planner.h"

#include <algorithm>
#include <deque>
#include <set>

namespace Lingmo::Backend {
namespace {

std::vector<DrmModeCandidate> sortedModes(const DrmFallbackHead &head)
{
    std::vector<DrmModeCandidate> modes = head.modes;
    std::stable_sort(modes.begin(), modes.end(), [](const auto &left, const auto &right) {
        if (left.preferred != right.preferred)
            return left.preferred;
        const int64_t leftArea = int64_t(left.width) * left.height;
        const int64_t rightArea = int64_t(right.width) * right.height;
        if (leftArea != rightArea)
            return leftArea > rightArea;
        if (left.refreshMilliHz != right.refreshMilliHz)
            return left.refreshMilliHz > right.refreshMilliHz;
        return left.key < right.key;
    });
    return modes;
}

} // namespace

std::optional<DrmFallbackPlan>
DrmModeFallbackPlanner::choose(const std::vector<DrmFallbackHead> &heads, Tester tester,
                               int maximumAttempts)
{
    if (heads.empty() || !tester || maximumAttempts <= 0)
        return std::nullopt;
    std::vector<std::vector<DrmModeCandidate>> modes;
    for (const auto &head : heads) {
        modes.push_back(sortedModes(head));
        if (modes.back().empty())
            return std::nullopt;
    }

    std::deque<std::vector<int>> queue;
    queue.push_back(std::vector<int>(heads.size(), 0));
    std::set<std::vector<int>> visited;
    int attempts = 0;
    while (!queue.empty() && attempts < maximumAttempts) {
        std::vector<int> indexes = queue.front();
        queue.pop_front();
        if (!visited.insert(indexes).second)
            continue;
        std::vector<DrmModeCandidate> selected;
        for (size_t i = 0; i < heads.size(); ++i)
            selected.push_back(modes[i][indexes[i]]);
        const DrmFallbackTestResult tested = tester(selected);
        ++attempts;
        if (tested.succeeded) {
            DrmFallbackPlan result;
            result.modes = selected;
            result.attempts = attempts;
            result.diagnostic = tested.message;
            for (size_t i = 0; i < heads.size(); ++i)
                if (indexes[i] > 0)
                    result.downgradedConnectors.push_back(heads[i].connectorName);
            return result;
        }

        if (!tested.retryable)
            return std::nullopt;

        for (size_t i = 0; i < heads.size(); ++i) {
            const auto &head = heads[i];
            const auto &failed = tested.failedBaseConnectorKeys;
            const bool affected = failed.empty()
                    || std::find(failed.begin(), failed.end(), head.baseConnectorKey)
                            != failed.end();
            if (!head.internal && affected && size_t(indexes[i]) + 1 < modes[i].size()) {
                std::vector<int> next = indexes;
                ++next[i];
                queue.push_back(std::move(next));
            }
        }
    }
    return std::nullopt;
}

} // namespace Lingmo::Backend
