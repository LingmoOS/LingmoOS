#pragma once

#include <algorithm>
#include <cmath>
#include <optional>
#include <utility>
#include <vector>

#include "cadence/easing.h"
#include "cadence/interpolator.h"

namespace Cadence {

template<typename T>
struct Keyframe {
    double offset = 0;
    T value {};
    // Easing from this keyframe to the next one.
    Easing easing = Easing::Linear;
};

// A validated, normalized value track. Timing remains owned by the caller or Timeline.
template<typename T, typename ValueInterpolator = Interpolator<T>>
class KeyframeTrack {
public:
    bool setKeyframes(std::vector<Keyframe<T>> frames)
    {
        if (frames.empty())
            return false;
        for (size_t i = 0; i < frames.size(); ++i) {
            if (!std::isfinite(frames[i].offset) || frames[i].offset < 0 || frames[i].offset > 1)
                return false;
            if (i > 0 && frames[i - 1].offset >= frames[i].offset)
                return false;
        }
        m_frames = std::move(frames);
        return true;
    }

    std::optional<T> sample(double progress) const
    {
        if (m_frames.empty() || !std::isfinite(progress))
            return std::nullopt;
        if (progress <= m_frames.front().offset)
            return m_frames.front().value;
        if (progress >= m_frames.back().offset)
            return m_frames.back().value;

        const auto next = std::upper_bound(m_frames.begin(), m_frames.end(), progress,
                                           [](double value, const Keyframe<T> &frame) {
                                               return value < frame.offset;
                                           });
        const Keyframe<T> &to = *next;
        const Keyframe<T> &from = *(next - 1);
        const double local = (progress - from.offset) / (to.offset - from.offset);
        return ValueInterpolator::mix(from.value, to.value, ease(from.easing, local));
    }

    const std::vector<Keyframe<T>> &keyframes() const { return m_frames; }
    bool empty() const { return m_frames.empty(); }

private:
    std::vector<Keyframe<T>> m_frames;
};

} // namespace Cadence
