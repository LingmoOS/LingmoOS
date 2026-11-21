// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/input/availability.h"

#include "base/event_loop.h"

namespace settings {

namespace {

class OnceWatch : public AvailabilityWatch
{
public:
    OnceWatch(bool available, AvailabilityCallback callback)
    {
        // Not from inside the factory: the window is still adding entries.
        Lingmo::base::EventLoop::current()->post(
                [available, callback = std::move(callback), alive = std::weak_ptr<int>(m_alive)] {
                    if (alive.lock())
                        callback(available);
                });
    }

private:
    std::shared_ptr<int> m_alive = std::make_shared<int>(0);
};

} // namespace

std::unique_ptr<AvailabilityWatch> answerOnce(bool available, AvailabilityCallback callback)
{
    return std::make_unique<OnceWatch>(available, std::move(callback));
}

} // namespace settings
