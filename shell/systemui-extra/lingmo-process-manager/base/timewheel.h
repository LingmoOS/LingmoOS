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

#ifndef TIMEWHEEL_H
#define TIMEWHEEL_H

#include <QObject>
#include <functional>
#include <variant>
#include <map>
#include <memory>

class QTimer;
namespace timewheel {

using TimeoutCallback = std::function<void()>;

struct Timer
{
    quint64 timerId;
    int rotation;
    int tickSlot;
    TimeoutCallback callback;
};

class TimeWheel : public QObject
{
public:
    explicit TimeWheel(QObject *parent = nullptr);
    unsigned long long addTimer(int timeout,
                                const TimeoutCallback &callback);

    void deleteTimer(unsigned long long timerId);

private Q_SLOTS:
    void tick();

private:
    // <slotnumber, list<timer>>
    using SlotTimerMap = std::map<int, std::list<std::unique_ptr<Timer>>>;
    QTimer *m_timer;
    int m_currentSlot;
    unsigned long long m_currentTimerCount;

    SlotTimerMap m_slotTimers;
    const int kSlostInterval = 1; // 1s
    const int kSlotNumber = 60;
};

} // namsapce timewheel;


#endif // TIMEWHEEL_H
