// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/public/cpp/accelerator_actions.cc
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65

#include "accelerators/accelerator_actions.h"

#include <array>
#include <utility>
#include <string_view>

namespace Lingmo::Accelerators {

namespace {

constexpr auto kActionNames = std::to_array<std::pair<AcceleratorAction, const char *>>({
        {AcceleratorAction::ToggleAppList, "toggle-app-list"},
        {AcceleratorAction::ToggleSearch, "toggle-search"},
        {AcceleratorAction::ToggleScreenKeyboard, "toggle-screen-keyboard"},
        {AcceleratorAction::LockScreen, "lock-screen"},
        {AcceleratorAction::CycleForwardMru, "cycle-forward-mru"},
        {AcceleratorAction::CycleBackwardMru, "cycle-backward-mru"},
        {AcceleratorAction::ToggleMaximized, "toggle-maximized"},
        {AcceleratorAction::WindowMinimize, "window-minimize"},
        {AcceleratorAction::QuitApplication, "quit-application"},
        {AcceleratorAction::HideApplication, "hide-application"},
        {AcceleratorAction::HideOtherApplications, "hide-other-applications"},
        {AcceleratorAction::ToggleFullscreen, "toggle-fullscreen"},
        {AcceleratorAction::WindowCycleSnapLeft, "window-cycle-snap-left"},
        {AcceleratorAction::WindowCycleSnapRight, "window-cycle-snap-right"},
        {AcceleratorAction::CloseWindow, "close-window"},
        {AcceleratorAction::MoveActiveWindowBetweenDisplays, "move-active-window-between-displays"},
        {AcceleratorAction::ToggleOverview, "toggle-overview"},
        {AcceleratorAction::ToggleShowDesktop, "toggle-show-desktop"},
        {AcceleratorAction::DesksActivateDeskLeft, "desks-activate-desk-left"},
        {AcceleratorAction::DesksActivateDeskRight, "desks-activate-desk-right"},
        {AcceleratorAction::DesksMoveActiveItemLeft, "desks-move-active-item-left"},
        {AcceleratorAction::DesksMoveActiveItemRight, "desks-move-active-item-right"},
        {AcceleratorAction::DesksNewDesk, "desks-new-desk"},
        {AcceleratorAction::DesksRemoveCurrentDesk, "desks-remove-current-desk"},
        {AcceleratorAction::DesksActivate0, "desks-activate-0"},
        {AcceleratorAction::DesksActivate1, "desks-activate-1"},
        {AcceleratorAction::DesksActivate2, "desks-activate-2"},
        {AcceleratorAction::DesksActivate3, "desks-activate-3"},
        {AcceleratorAction::DesksActivate4, "desks-activate-4"},
        {AcceleratorAction::DesksActivate5, "desks-activate-5"},
        {AcceleratorAction::DesksActivate6, "desks-activate-6"},
        {AcceleratorAction::DesksActivate7, "desks-activate-7"},
        {AcceleratorAction::DesksToggleAssignToAllDesks, "desks-toggle-assign-to-all-desks"},
        {AcceleratorAction::TakeScreenshot, "take-screenshot"},
        {AcceleratorAction::TakeScreenshotToClipboard, "take-screenshot-to-clipboard"},
        {AcceleratorAction::TakePartialScreenshot, "take-partial-screenshot"},
        {AcceleratorAction::TakePartialScreenshotToClipboard,
         "take-partial-screenshot-to-clipboard"},
        {AcceleratorAction::StartCaptureMode, "start-capture-mode"},
        {AcceleratorAction::StopScreenRecording, "stop-screen-recording"},
        {AcceleratorAction::ToggleClipboardHistory, "toggle-clipboard-history"},
        {AcceleratorAction::VolumeMuteToggle, "volume-mute-toggle"},
        {AcceleratorAction::VolumeDown, "volume-down"},
        {AcceleratorAction::VolumeUp, "volume-up"},
        {AcceleratorAction::MicrophoneMuteToggle, "microphone-mute-toggle"},
        {AcceleratorAction::BrightnessDown, "brightness-down"},
        {AcceleratorAction::BrightnessUp, "brightness-up"},
        {AcceleratorAction::KeyboardBrightnessDown, "keyboard-brightness-down"},
        {AcceleratorAction::KeyboardBrightnessUp, "keyboard-brightness-up"},
        {AcceleratorAction::MediaPlayPause, "media-play-pause"},
        {AcceleratorAction::MediaNext, "media-next"},
        {AcceleratorAction::MediaPrevious, "media-previous"},
        {AcceleratorAction::MediaStop, "media-stop"},
});

} // namespace

std::string acceleratorActionName(AcceleratorAction action)
{
    for (const auto &[candidate, name] : kActionNames) {
        if (candidate == action)
            return name;
    }
    return {};
}

std::vector<AcceleratorAction> allAcceleratorActions()
{
    std::vector<AcceleratorAction> actions;
    actions.reserve(kActionNames.size());
    for (const auto &[action, name] : kActionNames)
        actions.push_back(action);
    return actions;
}

std::optional<AcceleratorAction> acceleratorActionFromName(std::string_view name)
{
    for (const auto &[action, candidate] : kActionNames) {
        if (name == candidate)
            return action;
    }
    return std::nullopt;
}

} // namespace Lingmo::Accelerators
