// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/computer_control_environment.h"


#include <linux/sockios.h>
#include <sys/ioctl.h>

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "clipboard/stored_source.h"
#include "compositor/surface.h"
#include "computer_control/permission_consent.h"
#include "computer_control/remembered_apps.h"
#include "input/seat.h"
#include "session/session_controller.h"
#include "wayland/protocols/input_method.h"
#include "wayland/server_util.h"
#include "wm/mru_window_tracker.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_state.h"
#include "wm/wm_event.h"

namespace Lingmo::Core {

computer_control::Environment computerControlEnvironment(ComputerControlHosts hosts)
{
    wm::WindowManager *const wm = hosts.windowManager;
    input::Seat *const seat = hosts.seat;
    const session::SessionController *const session = hosts.session;
    const auto showsSystemDialogs = hosts.showsSystemDialogs;

    computer_control::Environment environment;
    // The agent's application is the nearest process above its server that
    // has a window: the agent app itself, or the terminal it runs in.
    const auto agentApplication = [wm, windowApplication = hosts.windowApplication](int64_t pid) {
        if (!windowApplication)
            return std::string();
        const std::vector<wm::Window *> windows = wm->mruWindowTracker()->buildMruWindowList();
        for (int depth = 0; depth < 16; ++depth) {
            const std::optional<int64_t> parent = computer_control::parentProcess(pid);
            if (!parent || *parent <= 1)
                break;
            pid = *parent;
            for (wm::Window *window : windows) {
                if (window->processId() != pid)
                    continue;
                if (std::string appId = windowApplication(window->id()); !appId.empty())
                    return appId;
            }
        }
        return std::string();
    };
    environment.askConsent = [consent = hosts.consent, agentApplication](
                                     const std::string &agentName, int64_t pid,
                                     std::function<void(computer_control::ConsentResult)> answer) {
        return consent->ask(agentName, pid, agentApplication(pid), std::move(answer));
    };
    // The permission prompt, the authentication dialogs and the shell's secret
    // prompts are the user's to answer, so the agent neither clicks nor types
    // into them or their popups. Shell windows have no process.
    const auto protects = [showsSystemDialogs](const wm::Window *window) {
        for (; window; window = window->transientParent()) {
            if (window->isProtectedFromControl() || showsSystemDialogs(window->processId()))
                return true;
        }
        return false;
    };
    environment.protectedAt = [wm, protects](const base::PointF &point) {
        wm::Window *window = wm->windowAt(point);
        if (!window)
            window = wm->frameAt(point);
        return protects(window);
    };
    environment.protectedFocus = [wm, seat, protects] {
        return protects(wm->windowForSurface(seat->focusedSurface()));
    };
    const auto inputMethod = hosts.inputMethod;
    environment.textInputFocused = [inputMethod] {
        return inputMethod() && inputMethod()->hasFocusedTextInput();
    };
    environment.inputMethodGrabsKeyboard = [inputMethod] {
        return inputMethod() && inputMethod()->grabsKeyboard();
    };
    environment.commitText = [inputMethod](const std::string &text) {
        return inputMethod() && inputMethod()->commitText(text);
    };
    environment.sessionBlocked = [session] {
        return session->isBlocked();
    };
    // Every desk's windows, as Ash's autotest API lists them
    // (ash/autotest_private_api_utils.cc, GetAppWindowList).
    environment.windows = [wm] {
        std::vector<computer_control::WindowInfo> windows;
        for (wm::Window *window : wm->mruWindowTracker()->buildMruWindowList()) {
            computer_control::WindowInfo info;
            info.id = window->id();
            info.applicationId = window->applicationId();
            info.title = window->title();
            info.bounds = window->bounds();
            if (window->isActive())
                info.flags |= computer_control::WindowInfo::Active;
            if (window->state()->isMinimized())
                info.flags |= computer_control::WindowInfo::Minimized;
            if (window->isHidden())
                info.flags |= computer_control::WindowInfo::Hidden;
            if (window->isDeskHidden())
                info.flags |= computer_control::WindowInfo::OtherDesk;
            windows.push_back(info);
        }
        return windows;
    };
    // What the dock's click on a task does; each step can end the window.
    environment.activateWindow = [wm](uint64_t id) {
        wm::Window *window = wm->windowById(id);
        if (!window || !wm::canIncludeWindowInMruList(window))
            return false;
        wm->unhideApplication(window);
        window->setHidden(false);
        window = wm->windowById(id);
        if (window && window->state()->isMinimized())
            window->restore();
        window = wm->windowById(id);
        if (!window)
            return false;
        wm->activation()->activate(window);
        return true;
    };
    environment.protectedWindow = [wm, protects](uint64_t id) {
        return protects(wm->windowById(id));
    };
    // What the close button does (commands::closeWindow).
    environment.closeWindow = [wm](uint64_t id) {
        wm::Window *window = wm->windowById(id);
        if (!window || !wm::canIncludeWindowInMruList(window) || !window->canClose())
            return false;
        window->requestClose();
        return true;
    };
    // A window out of view is brought back with activateWindow first, so the
    // agent sees what it moves.
    environment.setWindowFrame = [wm](uint64_t id, const base::Rect &requested) {
        wm::Window *window = wm->windowById(id);
        if (!window || !wm::canIncludeWindowInMruList(window) || window->isHidden()
            || window->isDeskHidden() || window->state()->isMinimized())
            return false;
        base::Rect bounds = requested;
        if (!window->isResizable()) {
            bounds.setSize(window->bounds().size());
        } else {
            // No larger than the work area, as a resize by hand (WindowResizer).
            base::Size maximum = wm->workAreaFor(window).size();
            if (const base::Size limit = window->maximumOuterSize(); !limit.isEmpty())
                maximum = maximum.isEmpty() ? limit : maximum.boundedTo(limit);
            if (!maximum.isEmpty()) {
                bounds.setWidth(std::min(bounds.width(), maximum.width()));
                bounds.setHeight(std::min(bounds.height(), maximum.height()));
            }
        }
        wm::WindowState *state = window->state();
        // As Ash's tiling does: a tiled window leaves its state for the bounds.
        if (!state->isNormal()) {
            state->setRestoreBounds(bounds);
            state->onEvent(wm::WMEvent(wm::WMEventType::Normal));
        } else {
            window->requestBounds(bounds);
        }
        // Placed like a window moved by hand: nothing left to restore to.
        state->clearRestoreBounds();
        return true;
    };
    // The text as wl-copy offers it, so Xwayland clients get it too.
    environment.setClipboard = [seat](const std::string &text) {
        auto source = std::make_shared<clipboard::StoredSource>();
        for (const char *mime : {"text/plain;charset=utf-8", "text/plain", "UTF8_STRING",
                                 "STRING", "TEXT"}) {
            source->offer(mime);
            source->payloads[mime] = text;
        }
        seat->selection()->setSource(std::move(source));
    };
    // SIOCOUTQ on a Unix socket counts what the peer has not read yet.
    environment.inputRead = [seat] {
        for (compositor::Surface *surface : {seat->focusedSurface(), seat->pointerFocus()}) {
            wl_resource *resource = surface ? wayland::getSurfaceResource(surface) : nullptr;
            if (!resource)
                continue;
            int unread = 0;
            if (ioctl(wl_client_get_fd(wl_resource_get_client(resource)), SIOCOUTQ, &unread) == 0
                && unread > 0)
                return false;
        }
        return true;
    };
    return environment;
}

} // namespace Lingmo::Core
