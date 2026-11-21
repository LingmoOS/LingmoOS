// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/gpu/page_flip_request.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// What is taken: matched completion, kernel time, fixed interval and hardware
// flags.
#pragma once

#include <map>

#include "render/presentation.h"

namespace Lingmo::Backend {

class PresentationTracker
{
public:
    void submitted(uint64_t token, render::PresentationSubmission submission)
    {
        m_pending.emplace(token, Pending {std::move(submission), m_sequence});
    }

    void discontinuity()
    {
        auto next = std::make_shared<SequenceEpoch>();
        next->previous = m_sequence;
        m_sequence = std::move(next);
    }

    bool complete(uint64_t token, uint32_t seconds, uint32_t microseconds, uint32_t sequence,
                  bool monotonic, double refreshHz,
                  const std::shared_ptr<render::PresentationOutput>& output)
    {
        auto entry = m_pending.extract(token);
        if (!entry)
            return false;
        render::PresentationResult result;
        result.presented = monotonic && microseconds < 1000000;
        result.seconds = seconds;
        result.nanoseconds = microseconds < 1000000 ? microseconds * 1000u : 0;
        result.sequence = entry.mapped().sequence->extend(sequence).value_or(sequence);
        result.refresh = render::refreshInterval(refreshHz);
        result.flags = render::PresentationResult::Vsync | render::PresentationResult::HwClock |
            render::PresentationResult::HwCompletion;
        result.output = output;
        entry.mapped().submission.complete(result);
        return true;
    }

    void abandon()
    {
        auto pending = std::exchange(m_pending, {});
        for (auto& [token, submission] : pending)
            submission.submission.complete({});
    }

    // Resolves one accepted submission as never presented.
    void abandon(uint64_t token)
    {
        if (auto entry = m_pending.extract(token))
            entry.mapped().submission.complete({});
    }

    ~PresentationTracker() { abandon(); }

private:
    struct SequenceEpoch {
        render::PresentationSequence counter;
        std::shared_ptr<SequenceEpoch> previous;

        std::optional<uint64_t> extend(uint32_t value)
        {
            if (previous) {
                counter = previous->counter;
                counter.reset();
                previous.reset();
            }
            return counter.extend(value);
        }
    };

    struct Pending {
        render::PresentationSubmission submission;
        std::shared_ptr<SequenceEpoch> sequence;
    };

    std::shared_ptr<SequenceEpoch> m_sequence = std::make_shared<SequenceEpoch>();
    std::map<uint64_t, Pending> m_pending;
};

} // namespace Lingmo::Backend
