// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/accelerators/accelerator_commands.cc and
//           chromium/src/ash/display/display_move_window_util.cc
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "accelerators/accelerator_commands.h"

#include <chrono>
#include <cstdint>

#include "accelerators/accelerator_delegate.h"
#include "display/display_manager.h"
#include "session/screen_saver_service.h"
#include "session/session_controller.h"
#include "wm/activation.h"
#include "wm/desks/desk.h"
#include "wm/desks/desks_controller.h"
#include "wm/overview/overview_controller.h"
#include "wm/show_desktop_controller.h"
#include "wm/toplevel_event_handler.h"
#include "wm/window.h"
#include "wm/window_cycle_controller.h"
#include "wm/window_manager.h"
#include "wm/window_snap.h"
#include "wm/window_state.h"
#include "wm/wm_event.h"

namespace Lingmo::Accelerators::commands {

wm::Window *targetWindow(const CommandContext &context)
{
    wm::Window *window = context.windowManager->activation()->activeWindow();
    // Upstream picks from the application windows of the cycle list; the
    // desktop, which takes the keyboard once no application does, is never
    // one to minimize, snap or close.
    return window && window->isVisible() && !window->isShellWindow() ? window : nullptr;
}

bool canCycleMru(const CommandContext &context)
{
    return context.windowCycle && context.windowCycle->canCycle();
}

bool canLock(const CommandContext &context)
{
    return context.session && !context.session->isBlocked();
}

bool canWindowSnap(const CommandContext &context)
{
    wm::Window *window = targetWindow(context);
    return window && window->isResizable();
}

bool canMoveActiveWindowBetweenDisplays(const CommandContext &context)
{
    display::DisplayManager *displays = context.windowManager->displayManager();
    return targetWindow(context) && displays && displays->applied()
            && displays->applied()->logicalDisplays.size() > 1;
}

bool canCloseWindow(const CommandContext &context)
{
    wm::Window *window = targetWindow(context);
    return window && window->canClose();
}

bool canToggleOverview(const CommandContext &context)
{
    return context.windowManager && context.windowManager->overview()
            && context.windowManager->overview()->node();
}

bool canToggleShowDesktop(const CommandContext &context)
{
    return context.windowManager && context.windowManager->showDesktop();
}

void cycleForwardMru(const CommandContext &context)
{
    context.windowCycle->handleCycleWindow(wm::WindowCycleController::Direction::Forward);
}

void cycleBackwardMru(const CommandContext &context)
{
    context.windowCycle->handleCycleWindow(wm::WindowCycleController::Direction::Backward);
}

void toggleMaximized(const CommandContext &context)
{
    if (wm::Window *window = targetWindow(context)) {
        const wm::WMEvent event(wm::WMEventType::ToggleMaximize);
        window->state()->onEvent(event);
    }
}

void windowMinimize(const CommandContext &context)
{
    if (wm::Window *window = targetWindow(context))
        window->minimize();
}

void quitApplication(const CommandContext &context)
{
    // Command+Q acts a moment after the keys, on the window that was active
    // when they went down.
    constexpr std::chrono::milliseconds kQuitDelay {100};
    const wm::Window *window = targetWindow(context);
    if (!window)
        return;
    wm::WindowManager *manager = context.windowManager;
    const uint64_t id = window->id();
    manager->runLater(kQuitDelay, [manager, id] {
        manager->quitApplication(manager->windowById(id));
    });
}

void hideApplication(const CommandContext &context)
{
    context.windowManager->hideApplication(targetWindow(context));
}

void hideOtherApplications(const CommandContext &context)
{
    context.windowManager->hideOtherApplications(targetWindow(context));
}

void toggleFullscreen(const CommandContext &context)
{
    if (wm::Window *window = targetWindow(context)) {
        const wm::WMEvent event(wm::WMEventType::ToggleFullscreen);
        window->state()->onEvent(event);
    }
}

void windowSnap(const CommandContext &context, AcceleratorAction action)
{
    wm::Window *window = targetWindow(context);
    if (!window)
        return;
    // ash::BaseState::CycleSnap: snap unless already snapped to that side, in
    // which case restore. Rotated displays are not distinguished; Lingmo
    // snaps in logical coordinates.
    const wm::WindowStateType desired = action == AcceleratorAction::WindowCycleSnapLeft
            ? wm::WindowStateType::PrimarySnapped
            : wm::WindowStateType::SecondarySnapped;
    if (window->isResizable() && window->stateType() != desired) {
        if (desired == wm::WindowStateType::PrimarySnapped)
            window->snapPrimary();
        else
            window->snapSecondary();
        return;
    }
    if (window->state()->isSnapped())
        window->restore();
}

void closeWindow(const CommandContext &context)
{
    if (wm::Window *window = targetWindow(context))
        window->requestClose();
}

void toggleOverview(const CommandContext &context)
{
    context.windowManager->overview()->toggle();
}

void toggleShowDesktop(const CommandContext &context)
{
    context.windowManager->showDesktop()->toggle();
}

void moveActiveWindowBetweenDisplays(const CommandContext &context)
{
    wm::Window *window = targetWindow(context);
    display::DisplayManager *displays = context.windowManager->displayManager();
    if (!window || !displays || !displays->applied())
        return;
    const std::vector<display::LogicalDisplay> &logical = displays->applied()->logicalDisplays;
    if (logical.size() < 2)
        return;

    // ash::display_move_window_util: the window goes to the next display in
    // the list, keeping its position relative to the display's origin, then
    // is fitted into that display's work area.
    const base::Rect current = context.windowManager->outputBoundsFor(window);
    int index = 0;
    for (int i = 0; i < logical.size(); ++i) {
        if (logical[i].bounds == current) {
            index = i;
            break;
        }
    }
    const display::LogicalDisplay &target = logical[(index + 1) % logical.size()];

    const base::Rect targetBounds = target.bounds;
    const base::Rect workArea = context.windowManager->workAreaForOutput(targetBounds);
    base::Rect bounds = window->bounds();
    bounds.moveTopLeft(targetBounds.topLeft() + (bounds.topLeft() - current.topLeft()));
    bounds.setSize(base::Size(std::min(bounds.width(), workArea.width()),
                              std::min(bounds.height(), workArea.height())));
    if (bounds.right() > workArea.right())
        bounds.moveRight(workArea.right());
    if (bounds.bottom() > workArea.bottom())
        bounds.moveBottom(workArea.bottom());
    if (bounds.x() < workArea.x())
        bounds.moveLeft(workArea.x());
    if (bounds.y() < workArea.y())
        bounds.moveTop(workArea.y());

    context.windowManager->setWindowOutput(window, target.waylandName);
    if (window->state()->isNormal())
        window->requestBounds(bounds);
    else
        window->state()->onEvent(wm::WMEvent(wm::WMEventType::WorkAreaChanged));
}

void lockScreen(const CommandContext &context)
{
    if (context.screenSaver)
        context.screenSaver->Lock();
    else if (context.session)
        context.session->requestLock();
}

void toggleAppList(const CommandContext &context)
{
    context.delegate->toggleAppList();
}

void takeScreenshot(const CommandContext &context, bool toClipboard)
{
    context.delegate->takeScreenshot(toClipboard);
}

void takePartialScreenshot(const CommandContext &context, bool toClipboard)
{
    context.delegate->takePartialScreenshot(toClipboard);
}

void startCaptureMode(const CommandContext &context)
{
    context.delegate->startCaptureMode();
}

void stopScreenRecording(const CommandContext &context)
{
    context.delegate->stopScreenRecording();
}

void toggleClipboardHistory(const CommandContext &context)
{
    context.delegate->toggleClipboardHistory();
}

void volumeDown(const CommandContext &context)
{
    context.delegate->volumeDown();
}

void volumeUp(const CommandContext &context)
{
    context.delegate->volumeUp();
}

void volumeMuteToggle(const CommandContext &context)
{
    context.delegate->toggleMute();
}

void microphoneMuteToggle(const CommandContext &context)
{
    context.delegate->toggleMicrophoneMute();
}

void brightnessDown(const CommandContext &context)
{
    context.delegate->brightnessDown();
}

void brightnessUp(const CommandContext &context)
{
    context.delegate->brightnessUp();
}

void keyboardBrightnessDown(const CommandContext &context)
{
    context.delegate->keyboardBrightnessDown();
}

void keyboardBrightnessUp(const CommandContext &context)
{
    context.delegate->keyboardBrightnessUp();
}

void mediaPlayPause(const CommandContext &context)
{
    context.delegate->mediaPlayPause();
}

void mediaNext(const CommandContext &context)
{
    context.delegate->mediaNext();
}

void mediaPrevious(const CommandContext &context)
{
    context.delegate->mediaPrevious();
}

void mediaStop(const CommandContext &context)
{
    context.delegate->mediaStop();
}

void activateDesk(const CommandContext &context, bool activateLeft)
{
    context.windowManager->desks()->activateAdjacentDesk(activateLeft,
                                                         wm::DesksSwitchSource::DeskSwitchShortcut);
}

void activateDeskAtIndex(const CommandContext &context, AcceleratorAction action)
{
    wm::DesksController *desks = context.windowManager->desks();
    // One desk animation at a time.
    if (desks->areDesksBeingModified())
        return;
    const int index = int(action) - int(AcceleratorAction::DesksActivate0);
    if (wm::Desk *desk = desks->deskAt(index))
        desks->activateDesk(desk, wm::DesksSwitchSource::IndexedDeskSwitchShortcut);
    else
        desks->performHitTheWallAnimation(/*goingLeft=*/false);
}

void moveActiveItem(const CommandContext &context, bool goingLeft)
{
    wm::DesksController *desks = context.windowManager->desks();
    if (desks->areDesksBeingModified())
        return;
    wm::Window *window = targetWindow(context);
    if (!window || !desks->belongsToActiveDesk(window))
        return;
    const int target = desks->activeDeskIndex() + (goingLeft ? -1 : 1);
    if (!desks->deskAt(target))
        return;
    desks->sendToDeskAtIndex(window, target);
}

void newDesk(const CommandContext &context)
{
    wm::DesksController *desks = context.windowManager->desks();
    if (!desks->canCreateDesks() || desks->areDesksBeingModified())
        return;
    // Add a desk and switch to it.
    if (wm::Desk *desk = desks->newDesk(wm::DesksCreationRemovalSource::Keyboard))
        desks->activateDesk(desk, wm::DesksSwitchSource::NewDeskShortcut);
}

void removeCurrentDesk(const CommandContext &context)
{
    wm::DesksController *desks = context.windowManager->desks();
    if (context.windowManager->eventHandler()->draggedWindow())
        return;
    if (!desks->canRemoveDesks() || desks->areDesksBeingModified())
        return;
    desks->removeDesk(desks->activeDesk(), wm::DesksCreationRemovalSource::Keyboard);
}

void toggleAssignToAllDesks(const CommandContext &context)
{
    wm::Window *window = targetWindow(context);
    wm::DesksController *desks = context.windowManager->desks();
    // Only a desk's own windows: an always-on-top window or a system dialog is
    // on every desk already.
    if (!window || (!desks->belongsToActiveDesk(window) && !desks->isVisibleOnAllDesks(window)))
        return;
    desks->setVisibleOnAllDesks(window, !desks->isVisibleOnAllDesks(window));
}

} // namespace Lingmo::Accelerators::commands
