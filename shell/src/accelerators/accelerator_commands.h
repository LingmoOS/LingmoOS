// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/accelerators/accelerator_commands.h
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65
//
// Adapted for Lingmo: the commands that have a Lingmo counterpart, with
// the Shell::Get() singletons replaced by an explicit context. The launcher
// lives in the shell process and is reached through the delegate.
#pragma once


#include "accelerators/accelerator_actions.h"

namespace Lingmo::Session {
class ScreenSaverService;
class SessionController;
} // namespace Lingmo::Session

namespace Lingmo::WM {
class Window;
class WindowCycleController;
class WindowManager;
} // namespace Lingmo::WM

namespace Lingmo::Accelerators {

class AcceleratorDelegate;

struct CommandContext {
    wm::WindowManager *windowManager = nullptr;
    wm::WindowCycleController *windowCycle = nullptr;
    session::SessionController *session = nullptr;
    session::ScreenSaverService *screenSaver = nullptr;
    AcceleratorDelegate *delegate = nullptr;
};

namespace commands {

// The window an action applies to: the active one, when it is a visible
// application window rather than one of the shell's own.
wm::Window *targetWindow(const CommandContext &context);

bool canCycleMru(const CommandContext &context);
bool canLock(const CommandContext &context);
bool canWindowSnap(const CommandContext &context);
bool canMoveActiveWindowBetweenDisplays(const CommandContext &context);
bool canCloseWindow(const CommandContext &context);
bool canToggleOverview(const CommandContext &context);
bool canToggleShowDesktop(const CommandContext &context);

void cycleForwardMru(const CommandContext &context);
void cycleBackwardMru(const CommandContext &context);
void toggleMaximized(const CommandContext &context);
// Minimizes the active window; ash's ToggleMinimized restores a minimized top
// window instead, which cannot be active here.
void windowMinimize(const CommandContext &context);
// The application menu's Quit, Hide and Hide Others. Quit waits 100 ms first.
void quitApplication(const CommandContext &context);
void hideApplication(const CommandContext &context);
void hideOtherApplications(const CommandContext &context);
void toggleFullscreen(const CommandContext &context);
// Snaps to the requested side, or restores when already snapped there.
void windowSnap(const CommandContext &context, AcceleratorAction action);
void closeWindow(const CommandContext &context);
void moveActiveWindowBetweenDisplays(const CommandContext &context);
void toggleOverview(const CommandContext &context);
// Slides the application windows aside to show the desktop, or back.
void toggleShowDesktop(const CommandContext &context);
// Desks.
void activateDesk(const CommandContext &context, bool activateLeft);
void activateDeskAtIndex(const CommandContext &context, AcceleratorAction action);
void moveActiveItem(const CommandContext &context, bool goingLeft);
void newDesk(const CommandContext &context);
void removeCurrentDesk(const CommandContext &context);
void toggleAssignToAllDesks(const CommandContext &context);
void lockScreen(const CommandContext &context);
void toggleAppList(const CommandContext &context);
// Screen capture belongs to the shell; these only ask it.
void takeScreenshot(const CommandContext &context, bool toClipboard);
void takePartialScreenshot(const CommandContext &context, bool toClipboard);
void startCaptureMode(const CommandContext &context);
void stopScreenRecording(const CommandContext &context);
// So does the clipboard history.
void toggleClipboardHistory(const CommandContext &context);
// The levels belong to the shell too, which shows the change.
void volumeDown(const CommandContext &context);
void volumeUp(const CommandContext &context);
void volumeMuteToggle(const CommandContext &context);
void microphoneMuteToggle(const CommandContext &context);
void brightnessDown(const CommandContext &context);
void brightnessUp(const CommandContext &context);
void keyboardBrightnessDown(const CommandContext &context);
void keyboardBrightnessUp(const CommandContext &context);
// The session's media player, over MPRIS. Nothing happens when none is
// running, which is the answer a key with nothing to control should give.
void mediaPlayPause(const CommandContext &context);
void mediaNext(const CommandContext &context);
void mediaPrevious(const CommandContext &context);
void mediaStop(const CommandContext &context);

} // namespace commands
} // namespace Lingmo::Accelerators

