#pragma once

#include <chrono>

namespace Cadence {

using TimePoint = std::chrono::nanoseconds;

class Clock {
public:
    virtual ~Clock() = default;
    virtual TimePoint now() const = 0;
};

class ManualClock final : public Clock {
public:
    explicit ManualClock(TimePoint initial = TimePoint::zero()) : m_now(initial) {}

    TimePoint now() const override { return m_now; }
    void set(TimePoint now) { m_now = now; }
    void advance(TimePoint delta) { m_now += delta; }

private:
    TimePoint m_now;
};

} // namespace Cadence
