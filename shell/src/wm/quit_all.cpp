// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wm/quit_all.h"

#include <algorithm>
#include <chrono>
#include <utility>

#include "base/event_loop.h"
#include "core/main_loop.h"
#include "wm/window.h"
#include "wm/window_manager.h"

namespace Lingmo::WM {

namespace {

constexpr int kPollMs = 100;

// A prompt of the application's own, not a menu or tooltip.
bool hasPrompt(const Window *window)
{
    for (const Window *child : window->transientChildren()) {
        if (!child->isPopup() || hasPrompt(child))
            return true;
    }
    return false;
}

} // namespace

QuitAll::QuitAll(WindowManager *manager, std::function<int64_t()> clock)
    : m_manager(manager)
    , m_clock(std::move(clock))
    , m_timer(core::mainLoop())
{
    if (!m_clock) {
        m_clock = [] {
            static const auto start = std::chrono::steady_clock::now();
            return int64_t(std::chrono::duration_cast<std::chrono::milliseconds>(
                                   std::chrono::steady_clock::now() - start)
                                   .count());
        };
    }
    m_timer.setInterval(std::chrono::milliseconds(kPollMs));
    m_timer.onTimeout = [this] {
        update();
    };
    // The last window going is when the session may end; no need to wait a poll.
    m_windowRemoved = m_manager->windowRemoved.connect([this](Window *) {
        if (!isRunning())
            return;
        core::mainLoop()->post([quit = m_weakFactory.getWeakPtr()] {
            if (quit)
                quit->update();
        });
    });
}

bool QuitAll::start(std::function<void(const Result &)> done)
{
    if (isRunning())
        return false;
    m_done = std::move(done);
    m_asked.clear();
    m_timer.start();
    update();
    return true;
}

bool QuitAll::isApplicationWindow(const Window *window) const
{
    return !window->transientParent() && !window->skipTaskbar();
}

void QuitAll::ask(Window *window, int64_t now)
{
    // Quit asks every window of the application at once; asking one of them
    // again would only raise a second prompt.
    for (const Window *candidate : m_manager->windows()) {
        if (isApplicationWindow(candidate) && m_manager->belongsToApplication(candidate, window))
            m_asked[candidate->id()] = Asked {now, std::nullopt};
    }
    m_manager->quitApplication(window);
}

void QuitAll::update()
{
    if (!isRunning())
        return;
    const int64_t now = m_clock();
    std::vector<Window *> windows;
    for (Window *window : m_manager->windows()) {
        if (isApplicationWindow(window))
            windows.push_back(window);
    }
    if (windows.empty()) {
        finish({Outcome::Quit, {}});
        return;
    }
    // Windows that opened since are asked too, on their own if their
    // application was already asked, so as not to raise its prompt again.
    for (Window *window : windows) {
        if (m_asked.contains(window->id()))
            continue;
        const bool askedBefore = std::any_of(windows.begin(), windows.end(), [&](Window *other) {
            return other != window && m_asked.contains(other->id())
                    && m_manager->belongsToApplication(window, other);
        });
        if (!askedBefore) {
            ask(window, now);
            continue;
        }
        m_asked[window->id()] = Asked {now, std::nullopt};
        if (window->canClose())
            window->requestClose();
    }
    for (Window *window : windows) {
        Asked &asked = m_asked[window->id()];
        if (hasPrompt(window)) {
            asked.promptSeen = now;
            continue;
        }
        if (asked.promptSeen ? now - *asked.promptSeen > kCancelledAfterMs
                             : now - asked.at > kUnresponsiveAfterMs) {
            finish({asked.promptSeen ? Outcome::Cancelled : Outcome::Unresponsive,
                    window->applicationId()});
            return;
        }
    }
}

void QuitAll::finish(const Result &result)
{
    m_timer.stop();
    m_asked.clear();
    // The callback may start another request.
    const auto done = std::exchange(m_done, nullptr);
    done(result);
}

} // namespace Lingmo::WM
