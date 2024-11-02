/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "timewheel.h"
#include <QTimer>
#include <QDebug>
namespace timewheel {

TimeWheel::TimeWheel(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
    , m_currentSlot(0)
    , m_currentTimerCount(0)
{
    connect(m_timer, &QTimer::timeout, this, &TimeWheel::tick);
}

unsigned long long TimeWheel::addTimer(
    int timeout, const TimeoutCallback &callback)
{
    int ticks = 0;
    if (timeout <= kSlostInterval) {
        ticks = kSlostInterval;
    } else {
        ticks = timeout / kSlostInterval;
    }

    int rotation = timeout / kSlotNumber;
    int tickSlot = (m_currentSlot + (ticks % kSlotNumber)) % kSlotNumber;

    if (m_currentTimerCount == UINT64_MAX) {
        m_currentTimerCount = 1;
    } else {
        ++m_currentTimerCount;
    }

    std::unique_ptr<Timer> timer(new Timer {
        m_currentTimerCount, rotation, tickSlot, callback
    });

    m_slotTimers[tickSlot].emplace_back(std::move(timer));
    if (!m_timer->isActive()) {
        m_timer->start(kSlostInterval * 1000);
    }

    return m_currentTimerCount;
}

void TimeWheel::tick()
{
    if (m_slotTimers.find(m_currentSlot) == m_slotTimers.end()) {
        m_currentSlot = ++m_currentSlot % kSlotNumber;
        return;
    }

    auto &timers = m_slotTimers[m_currentSlot];
    for (auto it = timers.begin(); it != timers.end();) {
        if (it->get()->rotation > 0) {
            -- it->get()->rotation;
            ++ it;
        } else {
            it->get()->callback();
            timers.erase(it ++);
        }
    }

    m_currentSlot = ++m_currentSlot % kSlotNumber;
}

void TimeWheel::deleteTimer(unsigned long long timerId)
{
    for (auto &timers : m_slotTimers) {
        timers.second.remove_if([timerId](const std::unique_ptr<Timer> &timer) { return timer->timerId == timerId; });
    }
}

} // namspace timewheel


