// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_WM_QUIT_ALL_H_
#define LINGMO_WM_QUIT_ALL_H_

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>

#include "base/signal.h"
#include "base/timer.h"
#include "base/weak_ptr.h"

namespace Lingmo::WM {

class Window;
class WindowManager;

// Before the session ends, every application is asked to quit as Quit asks one,
// and the session waits for them. An application asking about unsaved work is
// waited for however long the user takes; one that dismissed its prompt and kept
// its window cancelled, and one that neither closes nor asks stopped responding.
class QuitAll
{
public:
    enum class Outcome {
        Quit,
        Cancelled,
        Unresponsive,
    };

    struct Result {
        Outcome outcome = Outcome::Quit;
        // The application that kept the session from ending.
        std::string applicationId;
    };

    // How long a window may stay after its prompt closed, time enough to save.
    static constexpr int kCancelledAfterMs = 2000;
    static constexpr int kUnresponsiveAfterMs = 10000;

    explicit QuitAll(WindowManager *manager, std::function<int64_t()> clock = {});

    // False while a request is already running.
    bool start(std::function<void(const Result &)> done);

    bool isRunning() const { return static_cast<bool>(m_done); }

    // Looks at the windows again. Runs on a timer and on every window removed.
    void update();

private:
    struct Asked {
        int64_t at = 0;
        std::optional<int64_t> promptSeen;
    };

    bool isApplicationWindow(const Window *window) const;
    void ask(Window *window, int64_t now);
    void finish(const Result &result);

    WindowManager *m_manager;
    std::function<int64_t()> m_clock;
    std::function<void(const Result &)> m_done;
    // By window id, which is never reused.
    std::map<uint64_t, Asked> m_asked;
    base::Timer m_timer;
    base::Subscription m_windowRemoved;
    base::WeakPtrFactory<QuitAll> m_weakFactory {this};
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_QUIT_ALL_H_
