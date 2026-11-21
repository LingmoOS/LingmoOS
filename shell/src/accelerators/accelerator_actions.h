// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/public/cpp/accelerator_actions.h
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65
//
// Adapted for Lingmo: the launcher, the lock screen, the window actions,
// the application menu's Quit, Hide and Hide Others, desks, screen capture and
// the volume, microphone, brightness and keyboard backlight keys, plus the
// media keys, which reach the session's MPRIS player. Browser, IME,
// accessibility, tablet and debug actions have nothing to act on.
#pragma once


#include <cstdint>
#include <optional>
#include <vector>

#include <string>
#include <string_view>

namespace Lingmo::Accelerators {

// Persisted and exposed over D-Bus by name, never by value, so the order here
// is free to change.
enum class AcceleratorAction : uint32_t {
    // General.
    ToggleAppList,
    ToggleSearch,
    ToggleScreenKeyboard,
    LockScreen,

    // Windows.
    CycleForwardMru,
    CycleBackwardMru,
    ToggleMaximized,
    WindowMinimize,
    // The application menu's Quit, Hide and Hide Others.
    QuitApplication,
    HideApplication,
    HideOtherApplications,
    ToggleFullscreen,
    WindowCycleSnapLeft,
    WindowCycleSnapRight,
    CloseWindow,
    MoveActiveWindowBetweenDisplays,
    ToggleOverview,
    ToggleShowDesktop,
    // Desks.
    DesksActivateDeskLeft,
    DesksActivateDeskRight,
    DesksMoveActiveItemLeft,
    DesksMoveActiveItemRight,
    DesksNewDesk,
    DesksRemoveCurrentDesk,
    DesksActivate0,
    DesksActivate1,
    DesksActivate2,
    DesksActivate3,
    DesksActivate4,
    DesksActivate5,
    DesksActivate6,
    DesksActivate7,
    DesksToggleAssignToAllDesks,

    // Screen capture, performed by the shell.
    TakeScreenshot,
    TakeScreenshotToClipboard,
    TakePartialScreenshot,
    TakePartialScreenshotToClipboard,
    StartCaptureMode,
    StopScreenRecording,

    // The clipboard history, performed by the shell.
    ToggleClipboardHistory,

    // Device keys, performed by the shell, which shows the new level.
    VolumeMuteToggle,
    VolumeDown,
    VolumeUp,
    MicrophoneMuteToggle,
    BrightnessDown,
    BrightnessUp,
    KeyboardBrightnessDown,
    KeyboardBrightnessUp,

    // The media keys, sent to the player the session is playing from over
    // MPRIS. The session's own, not the focused window's: a player in the
    // background still answers them, as it does on any other desktop.
    MediaPlayPause,
    MediaNext,
    MediaPrevious,
    MediaStop,
};

using AcceleratorActionId = uint32_t;

constexpr AcceleratorActionId toActionId(AcceleratorAction action)
{
    return static_cast<AcceleratorActionId>(action);
}

// The kebab-case identifier used in preferences and on the bus.
std::string acceleratorActionName(AcceleratorAction action);
std::optional<AcceleratorAction> acceleratorActionFromName(std::string_view name);

// Every action, in declaration order. An action need not have a default
// binding to exist: it is then listed with none until the user adds one.
std::vector<AcceleratorAction> allAcceleratorActions();

} // namespace Lingmo::Accelerators

