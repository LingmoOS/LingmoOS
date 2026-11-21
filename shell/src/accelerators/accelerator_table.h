// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/public/cpp/accelerators.h and
//           chromium/src/ash/accelerators/accelerator_table.h
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65
//
// Adapted for Lingmo: the default bindings and the action categories, with
// keys as xkb keysyms. Deprecated, debug and developer accelerators are not
// carried over.
//
// Lingmo has no key-ack extension, so a client can never report that it did
// not consume a key: every accelerator here is handled before the focused
// window sees it. Defaults therefore prefer Meta and the dedicated media and
// brightness keys, which applications do not claim, over Ctrl-based
// combinations that ChromeOS can afford to offer the browser first.
#pragma once


#include <array>

#include <xkbcommon/xkbcommon-keysyms.h>

#include "accelerators/accelerator.h"
#include "accelerators/accelerator_actions.h"

namespace Lingmo::Accelerators {

struct AcceleratorData {
    bool triggerOnPress;
    uint32_t keysym;
    uint32_t modifiers;
    AcceleratorAction action;
};

// There are three classes of accelerators, as in Ash:
//
// Reserved: handled whatever the focused window is, fullscreen included, and
// even while a window inhibits shortcuts.
// Preferred: handled whatever the focused window is, fullscreen included,
// unless a window inhibits shortcuts. Ash leaves these to a fullscreen window.
// Everything else: handled before the focused window unless it is fullscreen,
// since without a key-ack extension there is no post-target pass.
//
// As in Ash, a lone Meta opens the launcher on release; the
// AcceleratorLauncherStateMachine keeps Meta+L from doing it as well.
inline constexpr auto kAcceleratorData = std::to_array<AcceleratorData>({
        {true, XKB_KEY_space, MetaDown, AcceleratorAction::ToggleSearch},
        {true, XKB_KEY_k, MetaDown | ControlDown, AcceleratorAction::ToggleScreenKeyboard},
        {false, XKB_KEY_Super_L, NoModifier, AcceleratorAction::ToggleAppList},
        {true, XKB_KEY_l, MetaDown, AcceleratorAction::LockScreen},

        // Window management.
        {true, XKB_KEY_Tab, AltDown, AcceleratorAction::CycleForwardMru},
        {true, XKB_KEY_Tab, AltDown | ShiftDown, AcceleratorAction::CycleBackwardMru},
        {true, XKB_KEY_equal, AltDown, AcceleratorAction::ToggleMaximized},
        {true, XKB_KEY_Up, MetaDown, AcceleratorAction::ToggleMaximized},
        {true, XKB_KEY_minus, AltDown, AcceleratorAction::WindowMinimize},
        {true, XKB_KEY_Down, MetaDown, AcceleratorAction::WindowMinimize},
        {true, XKB_KEY_f, MetaDown, AcceleratorAction::ToggleFullscreen},
        {true, XKB_KEY_bracketleft, AltDown, AcceleratorAction::WindowCycleSnapLeft},
        {true, XKB_KEY_Left, MetaDown, AcceleratorAction::WindowCycleSnapLeft},
        {true, XKB_KEY_bracketright, AltDown, AcceleratorAction::WindowCycleSnapRight},
        {true, XKB_KEY_Right, MetaDown, AcceleratorAction::WindowCycleSnapRight},
        {true, XKB_KEY_F4, AltDown, AcceleratorAction::CloseWindow},
        {true, XKB_KEY_m, MetaDown | AltDown, AcceleratorAction::MoveActiveWindowBetweenDisplays},
        {true, XKB_KEY_Tab, MetaDown, AcceleratorAction::ToggleOverview},
        {true, XKB_KEY_XF86LaunchA, NoModifier, AcceleratorAction::ToggleOverview},

        // Desks, with Meta where upstream has Search.
        {true, XKB_KEY_bracketleft, MetaDown, AcceleratorAction::DesksActivateDeskLeft},
        {true, XKB_KEY_bracketright, MetaDown, AcceleratorAction::DesksActivateDeskRight},
        {true, XKB_KEY_bracketleft, MetaDown | ShiftDown,
         AcceleratorAction::DesksMoveActiveItemLeft},
        {true, XKB_KEY_bracketright, MetaDown | ShiftDown,
         AcceleratorAction::DesksMoveActiveItemRight},
        {true, XKB_KEY_equal, MetaDown | ShiftDown, AcceleratorAction::DesksNewDesk},
        {true, XKB_KEY_minus, MetaDown | ShiftDown, AcceleratorAction::DesksRemoveCurrentDesk},
        {true, XKB_KEY_1, MetaDown | ShiftDown, AcceleratorAction::DesksActivate0},
        {true, XKB_KEY_2, MetaDown | ShiftDown, AcceleratorAction::DesksActivate1},
        {true, XKB_KEY_3, MetaDown | ShiftDown, AcceleratorAction::DesksActivate2},
        {true, XKB_KEY_4, MetaDown | ShiftDown, AcceleratorAction::DesksActivate3},
        {true, XKB_KEY_5, MetaDown | ShiftDown, AcceleratorAction::DesksActivate4},
        {true, XKB_KEY_6, MetaDown | ShiftDown, AcceleratorAction::DesksActivate5},
        {true, XKB_KEY_7, MetaDown | ShiftDown, AcceleratorAction::DesksActivate6},
        {true, XKB_KEY_8, MetaDown | ShiftDown, AcceleratorAction::DesksActivate7},
        {true, XKB_KEY_a, MetaDown | ShiftDown, AcceleratorAction::DesksToggleAssignToAllDesks},

        // Screen capture.
        {true, XKB_KEY_Print, NoModifier, AcceleratorAction::StartCaptureMode},
        {true, XKB_KEY_Print, ShiftDown, AcceleratorAction::TakeScreenshot},
        {true, XKB_KEY_Print, MetaDown | ShiftDown, AcceleratorAction::TakePartialScreenshot},
        {true, XKB_KEY_Escape, ControlDown | MetaDown, AcceleratorAction::StopScreenRecording},

        // Clipboard history.
        {true, XKB_KEY_v, MetaDown, AcceleratorAction::ToggleClipboardHistory},

        // Device keys.
        {true, XKB_KEY_XF86AudioMute, NoModifier, AcceleratorAction::VolumeMuteToggle},
        {true, XKB_KEY_XF86AudioLowerVolume, NoModifier, AcceleratorAction::VolumeDown},
        {true, XKB_KEY_XF86AudioRaiseVolume, NoModifier, AcceleratorAction::VolumeUp},
        {true, XKB_KEY_XF86AudioMicMute, NoModifier, AcceleratorAction::MicrophoneMuteToggle},
        {true, XKB_KEY_XF86MonBrightnessDown, NoModifier, AcceleratorAction::BrightnessDown},
        {true, XKB_KEY_XF86MonBrightnessUp, NoModifier, AcceleratorAction::BrightnessUp},
        {true, XKB_KEY_XF86KbdBrightnessDown, NoModifier,
         AcceleratorAction::KeyboardBrightnessDown},
        {true, XKB_KEY_XF86KbdBrightnessUp, NoModifier, AcceleratorAction::KeyboardBrightnessUp},

        // Media keys. Play and Pause are the same key on most keyboards, but
        // some send one and some the other.
        {true, XKB_KEY_XF86AudioPlay, NoModifier, AcceleratorAction::MediaPlayPause},
        {true, XKB_KEY_XF86AudioPause, NoModifier, AcceleratorAction::MediaPlayPause},
        {true, XKB_KEY_XF86AudioNext, NoModifier, AcceleratorAction::MediaNext},
        {true, XKB_KEY_XF86AudioPrev, NoModifier, AcceleratorAction::MediaPrevious},
        {true, XKB_KEY_XF86AudioStop, NoModifier, AcceleratorAction::MediaStop},
});

// The defaults under input::ShortcutConvention::Command. Command reaches the
// compositor as Meta, so MetaDown reads as Command. Applications get Command
// and Control as Ctrl, so the system keeps to Control+Command, which they
// never see, apart from a few actions on Command alone: Command+Tab switches
// applications, Command+M minimizes, and Command+Q, Command+H and
// Command+Option+H are the application menu's Quit, Hide and Hide Others.
// Control+Up alone shows all windows; applications still get Ctrl+Up from
// Command+Up. Keys no application uses stay as under PC: XF86LaunchA, Print
// and its combinations, the device keys, and Control+Command+Escape stops a
// recording. Command+Shift+3, 4 and 5 capture as well, onto the clipboard with
// Control. The launcher has no default.
inline constexpr auto kCommandAcceleratorData = std::to_array<AcceleratorData>({
        {true, XKB_KEY_space, MetaDown, AcceleratorAction::ToggleSearch},
        {true, XKB_KEY_k, MetaDown | ControlDown, AcceleratorAction::ToggleScreenKeyboard},
        {true, XKB_KEY_q, ControlDown | MetaDown, AcceleratorAction::LockScreen},

        // Window management.
        {true, XKB_KEY_Tab, MetaDown, AcceleratorAction::CycleForwardMru},
        {true, XKB_KEY_Tab, MetaDown | ShiftDown, AcceleratorAction::CycleBackwardMru},
        {true, XKB_KEY_m, MetaDown, AcceleratorAction::WindowMinimize},
        {true, XKB_KEY_q, MetaDown, AcceleratorAction::QuitApplication},
        {true, XKB_KEY_h, MetaDown, AcceleratorAction::HideApplication},
        {true, XKB_KEY_h, MetaDown | AltDown, AcceleratorAction::HideOtherApplications},
        {true, XKB_KEY_f, ControlDown | MetaDown, AcceleratorAction::ToggleFullscreen},
        {true, XKB_KEY_Up, ControlDown | MetaDown, AcceleratorAction::ToggleMaximized},
        {true, XKB_KEY_Left, ControlDown | MetaDown, AcceleratorAction::WindowCycleSnapLeft},
        {true, XKB_KEY_Right, ControlDown | MetaDown, AcceleratorAction::WindowCycleSnapRight},
        {true, XKB_KEY_m, ControlDown | MetaDown,
         AcceleratorAction::MoveActiveWindowBetweenDisplays},
        {true, XKB_KEY_Up, ControlDown, AcceleratorAction::ToggleOverview},
        {true, XKB_KEY_XF86LaunchA, NoModifier, AcceleratorAction::ToggleOverview},

        // Desks, with Control+Command where PC has Meta.
        {true, XKB_KEY_bracketleft, ControlDown | MetaDown,
         AcceleratorAction::DesksActivateDeskLeft},
        {true, XKB_KEY_bracketright, ControlDown | MetaDown,
         AcceleratorAction::DesksActivateDeskRight},
        {true, XKB_KEY_bracketleft, ControlDown | MetaDown | ShiftDown,
         AcceleratorAction::DesksMoveActiveItemLeft},
        {true, XKB_KEY_bracketright, ControlDown | MetaDown | ShiftDown,
         AcceleratorAction::DesksMoveActiveItemRight},
        {true, XKB_KEY_equal, ControlDown | MetaDown, AcceleratorAction::DesksNewDesk},
        {true, XKB_KEY_minus, ControlDown | MetaDown, AcceleratorAction::DesksRemoveCurrentDesk},
        // Without Shift, which Control+Command+Shift+3 and 4 take for screenshots.
        {true, XKB_KEY_1, ControlDown | MetaDown, AcceleratorAction::DesksActivate0},
        {true, XKB_KEY_2, ControlDown | MetaDown, AcceleratorAction::DesksActivate1},
        {true, XKB_KEY_3, ControlDown | MetaDown, AcceleratorAction::DesksActivate2},
        {true, XKB_KEY_4, ControlDown | MetaDown, AcceleratorAction::DesksActivate3},
        {true, XKB_KEY_5, ControlDown | MetaDown, AcceleratorAction::DesksActivate4},
        {true, XKB_KEY_6, ControlDown | MetaDown, AcceleratorAction::DesksActivate5},
        {true, XKB_KEY_7, ControlDown | MetaDown, AcceleratorAction::DesksActivate6},
        {true, XKB_KEY_8, ControlDown | MetaDown, AcceleratorAction::DesksActivate7},
        {true, XKB_KEY_a, ControlDown | MetaDown, AcceleratorAction::DesksToggleAssignToAllDesks},

        // Screen capture.
        {true, XKB_KEY_3, MetaDown | ShiftDown, AcceleratorAction::TakeScreenshot},
        {true, XKB_KEY_3, ControlDown | MetaDown | ShiftDown,
         AcceleratorAction::TakeScreenshotToClipboard},
        {true, XKB_KEY_4, MetaDown | ShiftDown, AcceleratorAction::TakePartialScreenshot},
        {true, XKB_KEY_4, ControlDown | MetaDown | ShiftDown,
         AcceleratorAction::TakePartialScreenshotToClipboard},
        {true, XKB_KEY_5, MetaDown | ShiftDown, AcceleratorAction::StartCaptureMode},
        {true, XKB_KEY_Print, NoModifier, AcceleratorAction::StartCaptureMode},
        {true, XKB_KEY_Print, ShiftDown, AcceleratorAction::TakeScreenshot},
        {true, XKB_KEY_Print, MetaDown | ShiftDown, AcceleratorAction::TakePartialScreenshot},
        {true, XKB_KEY_Escape, ControlDown | MetaDown, AcceleratorAction::StopScreenRecording},

        // Clipboard history.
        {true, XKB_KEY_v, ControlDown | MetaDown, AcceleratorAction::ToggleClipboardHistory},

        // Device keys.
        {true, XKB_KEY_XF86AudioMute, NoModifier, AcceleratorAction::VolumeMuteToggle},
        {true, XKB_KEY_XF86AudioLowerVolume, NoModifier, AcceleratorAction::VolumeDown},
        {true, XKB_KEY_XF86AudioRaiseVolume, NoModifier, AcceleratorAction::VolumeUp},
        {true, XKB_KEY_XF86AudioMicMute, NoModifier, AcceleratorAction::MicrophoneMuteToggle},
        {true, XKB_KEY_XF86MonBrightnessDown, NoModifier, AcceleratorAction::BrightnessDown},
        {true, XKB_KEY_XF86MonBrightnessUp, NoModifier, AcceleratorAction::BrightnessUp},
        {true, XKB_KEY_XF86KbdBrightnessDown, NoModifier,
         AcceleratorAction::KeyboardBrightnessDown},
        {true, XKB_KEY_XF86KbdBrightnessUp, NoModifier, AcceleratorAction::KeyboardBrightnessUp},

        // Media keys. Play and Pause are the same key on most keyboards, but
        // some send one and some the other.
        {true, XKB_KEY_XF86AudioPlay, NoModifier, AcceleratorAction::MediaPlayPause},
        {true, XKB_KEY_XF86AudioPause, NoModifier, AcceleratorAction::MediaPlayPause},
        {true, XKB_KEY_XF86AudioNext, NoModifier, AcceleratorAction::MediaNext},
        {true, XKB_KEY_XF86AudioPrev, NoModifier, AcceleratorAction::MediaPrevious},
        {true, XKB_KEY_XF86AudioStop, NoModifier, AcceleratorAction::MediaStop},
});

// Switching applications always switches, over a fullscreen game too, and
// the overview comes up over one.
inline constexpr std::array kPreferredActions = {
        AcceleratorAction::ToggleSearch,
        AcceleratorAction::CycleBackwardMru,
        AcceleratorAction::CycleForwardMru,
        AcceleratorAction::ToggleOverview,
};

// Actions that are always handled.
inline constexpr std::array kReservedActions = {
        AcceleratorAction::LockScreen,
};

// Actions allowed while the session is blocked: the lock screen and the login
// screen.
inline constexpr std::array kActionsAllowedAtLockScreen = {
        AcceleratorAction::BrightnessDown,
        AcceleratorAction::BrightnessUp,
        AcceleratorAction::KeyboardBrightnessDown,
        AcceleratorAction::KeyboardBrightnessUp,
        // The keys reach the player, not the session, so they stay useful
        // while it is locked.
        AcceleratorAction::MediaNext,
        AcceleratorAction::MediaPlayPause,
        AcceleratorAction::MediaPrevious,
        AcceleratorAction::MediaStop,
        AcceleratorAction::MicrophoneMuteToggle,
        AcceleratorAction::VolumeDown,
        AcceleratorAction::VolumeMuteToggle,
        AcceleratorAction::VolumeUp,
};

// Actions allowed while a system-modal dialog is up.
inline constexpr std::array kActionsAllowedAtModalWindow = {
        AcceleratorAction::BrightnessDown,
        AcceleratorAction::BrightnessUp,
        AcceleratorAction::KeyboardBrightnessDown,
        AcceleratorAction::KeyboardBrightnessUp,
        AcceleratorAction::LockScreen,
        AcceleratorAction::MediaNext,
        AcceleratorAction::MediaPlayPause,
        AcceleratorAction::MediaPrevious,
        AcceleratorAction::MediaStop,
        AcceleratorAction::MicrophoneMuteToggle,
        AcceleratorAction::VolumeDown,
        AcceleratorAction::VolumeMuteToggle,
        AcceleratorAction::VolumeUp,
};

// Actions performed again while their key is held.
inline constexpr std::array kRepeatableActions = {
        AcceleratorAction::BrightnessDown,
        AcceleratorAction::BrightnessUp,
        AcceleratorAction::KeyboardBrightnessDown,
        AcceleratorAction::KeyboardBrightnessUp,
        AcceleratorAction::VolumeDown,
        AcceleratorAction::VolumeUp,
};

// Actions that require at least one window.
inline constexpr std::array kActionsNeedingWindow = {
        AcceleratorAction::DesksMoveActiveItemLeft,
        AcceleratorAction::DesksMoveActiveItemRight,
        AcceleratorAction::DesksToggleAssignToAllDesks,
        AcceleratorAction::CloseWindow,
        AcceleratorAction::MoveActiveWindowBetweenDisplays,
        AcceleratorAction::ToggleFullscreen,
        AcceleratorAction::ToggleMaximized,
        AcceleratorAction::WindowCycleSnapLeft,
        AcceleratorAction::WindowCycleSnapRight,
        AcceleratorAction::WindowMinimize,
        AcceleratorAction::QuitApplication,
        AcceleratorAction::HideApplication,
};

// Actions on the active window or its application, which do nothing while
// the overview has the keys.
inline constexpr std::array kActionsBlockedInOverview = {
        AcceleratorAction::DesksMoveActiveItemLeft,
        AcceleratorAction::DesksMoveActiveItemRight,
        AcceleratorAction::DesksToggleAssignToAllDesks,
        AcceleratorAction::CloseWindow,
        AcceleratorAction::MoveActiveWindowBetweenDisplays,
        AcceleratorAction::ToggleFullscreen,
        AcceleratorAction::ToggleMaximized,
        AcceleratorAction::WindowCycleSnapLeft,
        AcceleratorAction::WindowCycleSnapRight,
        AcceleratorAction::WindowMinimize,
        AcceleratorAction::QuitApplication,
        AcceleratorAction::HideApplication,
        AcceleratorAction::HideOtherApplications,
};

} // namespace Lingmo::Accelerators

