// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_CORE_MAIN_LOOP_H_
#define LINGMO_CORE_MAIN_LOOP_H_

#include <memory>

namespace Lingmo::base {
class EventLoop;
} // namespace Lingmo::Base

namespace Lingmo::Core {

// The main thread's event loop, for base::Timer, fd watches, posted tasks and
// sd-bus; main() runs it. Made before anything that uses it and destroyed
// after. A second one stands in for the first while it exists.
class MainLoop
{
public:
    MainLoop();
    ~MainLoop();

    MainLoop(const MainLoop &) = delete;
    MainLoop &operator=(const MainLoop &) = delete;

    base::EventLoop &loop() const { return *m_loop; }

private:
    std::unique_ptr<base::EventLoop> m_loop;
    MainLoop *m_previous = nullptr;
};

// The loop of the MainLoop that exists, or null.
base::EventLoop *mainLoop();

} // namespace Lingmo::Core

#endif // LINGMO_CORE_MAIN_LOOP_H_
