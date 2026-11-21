#pragma once

#include <functional>
#include <utility>

#include "cadence/easing.h"
#include "cadence/interpolator.h"
#include "cadence/timeline.h"

namespace Cadence {

template<typename T, typename ValueInterpolator = Interpolator<T>>
class Tween {
public:
    using Duration = Timeline::Duration;

    void setDuration(Duration duration) { m_timeline.setDuration(duration); }
    void setEasing(Easing easing) { m_timeline.setEasing(easing); }
    void setLoops(int loops) { m_timeline.setLoops(loops); }
    void setAlternating(bool alternating) { m_timeline.setAlternating(alternating); }

    void setValue(T value)
    {
        m_timeline.stop();
        m_from = m_target = m_value = std::move(value);
    }

    void animateTo(T target, TimePoint now)
    {
        animateTo(std::move(target), m_value, now);
    }

    void animateTo(T target, const Clock &clock) { animateTo(std::move(target), clock.now()); }

    void animateTo(T target, T current, TimePoint now)
    {
        m_from = std::move(current);
        m_target = target;
        m_timeline.onUpdate = [this](double progress) {
            m_value = ValueInterpolator::mix(m_from, m_target, progress);
            if (onValueChanged)
                onValueChanged(m_value);
        };
        m_timeline.onFinished = [this] {
            m_value = m_target;
            if (onFinished)
                onFinished();
        };
        m_timeline.start(now);
    }

    void animateTo(T target, T current, const Clock &clock)
    {
        animateTo(std::move(target), std::move(current), clock.now());
    }

    bool advance(TimePoint now) { return m_timeline.advance(now); }
    bool advance(const Clock &clock) { return advance(clock.now()); }
    void stop() { m_timeline.stop(); }
    bool isRunning() const { return m_timeline.isRunning(); }
    const T &value() const { return m_value; }
    const T &target() const { return m_target; }

    std::function<void(const T &)> onValueChanged;
    std::function<void()> onFinished;

private:
    T m_from {};
    T m_target {};
    T m_value {};
    Timeline m_timeline;
};

} // namespace Cadence
