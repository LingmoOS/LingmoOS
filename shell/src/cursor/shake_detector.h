// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "base/geometry.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <deque>
#include <optional>

namespace Lingmo::Cursor {
// Logical pixels and monotonic milliseconds, independent of output scale and
// event frequency. Reversals are measured on excursions, not individual jitter.
class ShakeDetector
{
public:
    void reset() { m_last.reset(); m_turns.clear(); m_anchor = {}; m_direction = {}; m_distance = 0; }
    bool move(base::PointF point, int64_t now)
    {
        if (!std::isfinite(point.x()) || !std::isfinite(point.y())) { reset(); return false; }
        if (!m_last) { begin(point, now); return false; }
        const double dx = point.x() - m_last->x(), dy = point.y() - m_last->y();
        const double step = std::hypot(dx, dy);
        if (now < m_time || now - m_time > 250 || step > 300) {
            reset(); begin(point, now); return false;
        }
        if (step < 0.01) return false;
        m_last = point; m_time = now;
        const base::PointF excursion = point - m_anchor;
        const double length = std::hypot(excursion.x(), excursion.y());
        if (length < 24) return false;
        const base::PointF direction(excursion.x() / length, excursion.y() / length);
        while (!m_turns.empty() && now - m_turns.front() > 650) m_turns.pop_front();
        if (direction.x() * m_direction.x() + direction.y() * m_direction.y() < -0.6)
            m_turns.push_back(now);
        else if (m_turns.empty()) m_distance = 0;
        m_distance += length;
        m_anchor = point; m_direction = direction;
        if (m_turns.size() >= 4 && m_distance >= 180) {
            m_turns.clear(); m_distance = 0;
            return true;
        }
        return false;
    }
private:
    void begin(base::PointF point, int64_t now) { m_last = point; m_anchor = point; m_time = now; }
    std::optional<base::PointF> m_last;
    base::PointF m_anchor, m_direction;
    int64_t m_time = 0;
    double m_distance = 0;
    std::deque<int64_t> m_turns;
};

// Repeated shaking extends the hold; stopping or disabling can never leave a
// permanently enlarged cursor. Growth and return are smooth, bounded in time.
class ShakeAnimation
{
public:
    void trigger(int64_t now) { m_start = now; m_from = m_scale; m_until = now + 900; }
    double advance(int64_t now) {
        if (m_until == 0) return m_scale = 1;
        if (now < m_start + 100) {
            const double t = std::clamp(double(now - m_start) / 100, 0.0, 1.0);
            m_scale = m_from + (2.5 - m_from) * t * t * (3 - 2 * t);
        } else if (now <= m_until) m_scale = 2.5;
        else {
            const double t = std::clamp(double(now - m_until) / 300, 0.0, 1.0);
            m_scale = 2.5 - 1.5 * t * t * (3 - 2 * t);
            if (t >= 1) { m_until = 0; m_scale = 1; }
        }
        return m_scale;
    }
    void reset() { m_until = 0; m_scale = 1; }
    bool active() const { return m_until != 0; }
private:
    int64_t m_start = 0, m_until = 0;
    double m_scale = 1, m_from = 1;
};
}
