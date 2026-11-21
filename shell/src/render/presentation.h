// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/gfx/presentation_feedback.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: callback ownership follows Exo.
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace Lingmo::Render {

struct PresentationOutput {
};

struct PresentationResult {
    enum Flag : uint32_t {
        Vsync = 1,
        HwClock = 2,
        HwCompletion = 4,
        ZeroCopy = 8
    };

    bool presented = false;
    uint64_t seconds = 0;
    uint32_t nanoseconds = 0;
    uint64_t sequence = 0;
    uint32_t refresh = 0;
    uint32_t flags = 0;
    std::shared_ptr<PresentationOutput> output;

    bool valid() const { return presented && nanoseconds < 1000000000; }

    uint32_t frameTimeMs() const { return uint32_t(seconds) * 1000u + nanoseconds / 1000000u; }
};

inline uint32_t refreshInterval(double hz)
{
    if (!std::isfinite(hz) || hz <= 0)
        return 0;
    const double ns = std::round(1e9 / hz);
    return ns >= 1 && ns <= UINT32_MAX ? uint32_t(ns) : 0;
}

// Extend only forward deltas smaller than half the counter range. A known
// modeset starts a new counter epoch; do not manufacture a wrap on reset.
class PresentationSequence
{
public:
    std::optional<uint64_t> extend(uint32_t value)
    {
        if (!m_last) {
            m_last = value;
            m_extended = value;
            return m_extended;
        }
        const uint32_t delta = value - *m_last;
        if (m_discontinuity) {
            m_discontinuity = false;
            m_extended += delta < 0x80000000u ? delta : 1;
            m_last = value;
            return m_extended;
        }
        if (delta >= 0x80000000u) {
            const uint32_t backward = *m_last - value;
            return backward <= m_extended ? std::optional<uint64_t>(m_extended - backward)
                                          : std::nullopt;
        }
        m_extended += delta;
        m_last = value;
        return m_extended;
    }

    void reset() { m_discontinuity = m_last.has_value(); }

private:
    std::optional<uint32_t> m_last;
    uint64_t m_extended = 0;
    bool m_discontinuity = false;
};

// One content commit. Applied state and accepted output submissions share it;
// the last abandoned owner discards it. Completion never releases storage.
class PresentationContent
{
public:
    using Callback = std::function<void(const PresentationResult&)>;

    ~PresentationContent() { complete({}); }

    void add(Callback callback) { m_callbacks.push_back(std::move(callback)); }

    // Someone still waits to hear how this content reached the screen.
    bool isPending() const { return !m_callbacks.empty(); }

    void complete(const PresentationResult& result)
    {
        auto callbacks = std::exchange(m_callbacks, {});
        for (auto& callback : callbacks)
            callback(result);
    }

private:
    std::vector<Callback> m_callbacks;
};

struct PresentationParticipant {
    std::shared_ptr<PresentationContent> content;
    bool zeroCopy = false;
};

class PresentationSubmission
{
public:
    void add(const std::shared_ptr<PresentationContent>& content, bool zeroCopy)
    {
        if (!content)
            return;
        for (auto& participant : m_participants) {
            if (participant.content == content) {
                participant.zeroCopy &= zeroCopy;
                return;
            }
        }
        m_participants.push_back({content, zeroCopy});
    }

    void complete(PresentationResult result)
    {
        auto participants = std::exchange(m_participants, {});
        for (auto& participant : participants) {
            result.flags &= ~PresentationResult::ZeroCopy;
            if (participant.zeroCopy && result.valid())
                result.flags |= PresentationResult::ZeroCopy;
            participant.content->complete(result);
        }
    }

    void remove(const std::shared_ptr<PresentationContent>& content)
    {
        std::erase_if(m_participants, [&](const auto& p) {
            return p.content == content;
        });
    }

    bool contains(const std::shared_ptr<PresentationContent>& content) const
    {
        return std::any_of(m_participants.begin(), m_participants.end(), [&](const auto& p) {
            return p.content == content;
        });
    }

private:
    std::vector<PresentationParticipant> m_participants;
};

} // namespace Lingmo::Render
