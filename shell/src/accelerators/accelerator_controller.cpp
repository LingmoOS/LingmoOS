// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/accelerators/accelerator_controller_impl.cc
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "accelerators/accelerator_controller.h"

#include <xkbcommon/xkbcommon-keysyms.h>

#include "accelerators/accelerator_configuration.h"
#include "accelerators/accelerator_delegate.h"
#include "accelerators/accelerator_table.h"
#include "core/logging.h"
#include "session/session_controller.h"
#include "wm/mru_window_tracker.h"
#include "wm/overview/overview_controller.h"
#include "wm/window.h"
#include "wm/window_manager.h"

namespace Lingmo::Accelerators {

namespace {

bool canHandleToggleAppList(const Accelerator &accelerator,
                            const AcceleratorLauncherStateMachine &launcherStateMachine)
{
    // Meta on its own goes through the state machine; a combination that
    // happens to be bound to the launcher does not.
    if (accelerator.keysym() != XKB_KEY_Super_L && accelerator.keysym() != XKB_KEY_Super_R)
        return true;
    return launcherStateMachine.canHandleLauncher();
}

} // namespace

AcceleratorController::AcceleratorController(AcceleratorConfiguration *configuration,
                                             const CommandContext &context,
                                             std::function<bool()> anyKeysPressed)
    : m_launcherStateMachine(std::move(anyKeysPressed))
    , m_configuration(configuration)
    , m_context(context)
{
    init();
    m_configurationUpdates =
            m_configuration->acceleratorsUpdated.connect([this] { onAcceleratorsUpdated(); });
}

AcceleratorController::~AcceleratorController() = default;

void AcceleratorController::init()
{
    m_actionsAllowedAtLockScreen.insert(kActionsAllowedAtLockScreen.begin(),
                                        kActionsAllowedAtLockScreen.end());
    m_actionsAllowedAtModalWindow.insert(kActionsAllowedAtModalWindow.begin(),
                                         kActionsAllowedAtModalWindow.end());
    m_preferredActions.insert(kPreferredActions.begin(), kPreferredActions.end());
    m_reservedActions.insert(kReservedActions.begin(), kReservedActions.end());
    m_actionsNeedingWindow.insert(kActionsNeedingWindow.begin(), kActionsNeedingWindow.end());
    m_actionsBlockedInOverview.insert(kActionsBlockedInOverview.begin(),
                                      kActionsBlockedInOverview.end());
    m_repeatableActions.insert(kRepeatableActions.begin(), kRepeatableActions.end());
    registerAccelerators(m_configuration->allAccelerators());
}

void AcceleratorController::registerAccelerators(std::vector<Accelerator> accelerators)
{
    m_manager.registerAccelerators(accelerators, AcceleratorManager::NormalPriority, this);
}

void AcceleratorController::onAcceleratorsUpdated()
{
    m_manager.unregisterAll(this);
    registerAccelerators(m_configuration->allAccelerators());
}

bool AcceleratorController::process(const Accelerator &accelerator)
{
    return m_manager.process(accelerator);
}

bool AcceleratorController::isReserved(const Accelerator &accelerator) const
{
    const AcceleratorAction *action = m_configuration->findAcceleratorAction(accelerator);
    return action && m_reservedActions.contains(*action);
}

bool AcceleratorController::isPreferred(const Accelerator &accelerator) const
{
    const AcceleratorAction *action = m_configuration->findAcceleratorAction(accelerator);
    return action && m_preferredActions.contains(*action);
}

bool AcceleratorController::isRepeatable(const Accelerator &accelerator) const
{
    const AcceleratorAction *action = m_configuration->findAcceleratorAction(accelerator);
    return action && m_repeatableActions.contains(*action);
}

bool AcceleratorController::performActionIfEnabled(AcceleratorAction action,
                                                   const Accelerator &accelerator)
{
    if (!canPerformAction(action, accelerator))
        return false;
    performAction(action, accelerator);
    return true;
}

bool AcceleratorController::performActionWithoutKey(AcceleratorAction action)
{
    if (acceleratorProcessingRestriction(action) != Restriction::None)
        return false;
    return performActionIfEnabled(action, Accelerator());
}

bool AcceleratorController::doesAcceleratorMatchAction(const Accelerator &accelerator,
                                                       AcceleratorAction action) const
{
    const AcceleratorAction *found = m_configuration->findAcceleratorAction(accelerator);
    return found && *found == action;
}

void AcceleratorController::setUsePositionalLookup(bool use)
{
    m_configuration->setUsePositionalLookup(use);
    m_manager.setUsePositionalLookup(use);
}

void AcceleratorController::setPreventProcessingAccelerators(bool prevent)
{
    m_preventProcessing = prevent;
}

bool AcceleratorController::acceleratorPressed(const Accelerator &accelerator)
{
    const AcceleratorAction *action = m_configuration->findAcceleratorAction(accelerator);
    if (!action || !canPerformAction(*action, accelerator))
        return false;
    performAction(*action, accelerator);
    // ShouldActionConsumeKeyEvent: adding exceptions is strongly discouraged.
    return true;
}

bool AcceleratorController::canPerformAction(AcceleratorAction action,
                                             const Accelerator &accelerator) const
{
    const Restriction restriction = acceleratorProcessingRestriction(action);
    if (restriction != Restriction::None)
        return restriction == Restriction::PreventProcessingAndPropagation;

    // True when running the action does something; false gives the focused
    // window a chance at the key instead.
    switch (action) {
    case AcceleratorAction::CycleBackwardMru:
    case AcceleratorAction::CycleForwardMru:
        return commands::canCycleMru(m_context);
    case AcceleratorAction::LockScreen:
        return commands::canLock(m_context);
    case AcceleratorAction::MoveActiveWindowBetweenDisplays:
        return commands::canMoveActiveWindowBetweenDisplays(m_context);
    case AcceleratorAction::ToggleAppList:
        return canHandleToggleAppList(accelerator, m_launcherStateMachine);
    case AcceleratorAction::WindowCycleSnapLeft:
    case AcceleratorAction::WindowCycleSnapRight:
        return commands::canWindowSnap(m_context);
    case AcceleratorAction::CloseWindow:
        return commands::canCloseWindow(m_context);
    case AcceleratorAction::ToggleOverview:
        return commands::canToggleOverview(m_context);
    case AcceleratorAction::ToggleShowDesktop:
        return commands::canToggleShowDesktop(m_context);

    // The following are always enabled.
    case AcceleratorAction::ToggleScreenKeyboard:
    case AcceleratorAction::ToggleSearch:
    case AcceleratorAction::DesksActivateDeskLeft:
    case AcceleratorAction::DesksActivateDeskRight:
    case AcceleratorAction::DesksMoveActiveItemLeft:
    case AcceleratorAction::DesksMoveActiveItemRight:
    case AcceleratorAction::DesksNewDesk:
    case AcceleratorAction::DesksRemoveCurrentDesk:
    case AcceleratorAction::DesksActivate0:
    case AcceleratorAction::DesksActivate1:
    case AcceleratorAction::DesksActivate2:
    case AcceleratorAction::DesksActivate3:
    case AcceleratorAction::DesksActivate4:
    case AcceleratorAction::DesksActivate5:
    case AcceleratorAction::DesksActivate6:
    case AcceleratorAction::DesksActivate7:
    case AcceleratorAction::DesksToggleAssignToAllDesks:
    case AcceleratorAction::ToggleMaximized:
    case AcceleratorAction::WindowMinimize:
    case AcceleratorAction::QuitApplication:
    case AcceleratorAction::HideApplication:
    case AcceleratorAction::HideOtherApplications:
    case AcceleratorAction::ToggleFullscreen:
    case AcceleratorAction::TakeScreenshot:
    case AcceleratorAction::TakeScreenshotToClipboard:
    case AcceleratorAction::TakePartialScreenshot:
    case AcceleratorAction::TakePartialScreenshotToClipboard:
    case AcceleratorAction::StartCaptureMode:
    case AcceleratorAction::StopScreenRecording:
    case AcceleratorAction::ToggleClipboardHistory:
    case AcceleratorAction::VolumeMuteToggle:
    case AcceleratorAction::VolumeDown:
    case AcceleratorAction::VolumeUp:
    case AcceleratorAction::MicrophoneMuteToggle:
    case AcceleratorAction::BrightnessDown:
    case AcceleratorAction::BrightnessUp:
    case AcceleratorAction::KeyboardBrightnessDown:
    case AcceleratorAction::KeyboardBrightnessUp:
    case AcceleratorAction::MediaPlayPause:
    case AcceleratorAction::MediaNext:
    case AcceleratorAction::MediaPrevious:
    case AcceleratorAction::MediaStop:
        return true;
    }
    return false;
}

void AcceleratorController::performAction(AcceleratorAction action, const Accelerator &accelerator)
{
    if (acceleratorProcessingRestriction(action) != Restriction::None)
        return;

    LM_DEBUG(lingmoInput) << "Accelerator" << accelerator.shortcutText() << "->"
                     << acceleratorActionName(action);

    switch (action) {
    case AcceleratorAction::CycleBackwardMru:
        commands::cycleBackwardMru(m_context);
        break;
    case AcceleratorAction::CycleForwardMru:
        commands::cycleForwardMru(m_context);
        break;
    case AcceleratorAction::LockScreen:
        commands::lockScreen(m_context);
        break;
    case AcceleratorAction::MoveActiveWindowBetweenDisplays:
        commands::moveActiveWindowBetweenDisplays(m_context);
        break;
    case AcceleratorAction::ToggleAppList:
        commands::toggleAppList(m_context);
        break;
    case AcceleratorAction::ToggleSearch:
        m_context.delegate->toggleSearch();
        break;
    case AcceleratorAction::ToggleScreenKeyboard:
        m_context.delegate->toggleScreenKeyboard();
        break;
    case AcceleratorAction::ToggleFullscreen:
        commands::toggleFullscreen(m_context);
        break;
    case AcceleratorAction::ToggleMaximized:
        commands::toggleMaximized(m_context);
        break;
    case AcceleratorAction::WindowCycleSnapLeft:
    case AcceleratorAction::WindowCycleSnapRight:
        commands::windowSnap(m_context, action);
        break;
    case AcceleratorAction::WindowMinimize:
        commands::windowMinimize(m_context);
        break;
    case AcceleratorAction::QuitApplication:
        commands::quitApplication(m_context);
        break;
    case AcceleratorAction::HideApplication:
        commands::hideApplication(m_context);
        break;
    case AcceleratorAction::HideOtherApplications:
        commands::hideOtherApplications(m_context);
        break;
    case AcceleratorAction::CloseWindow:
        commands::closeWindow(m_context);
        break;
    case AcceleratorAction::ToggleOverview:
        commands::toggleOverview(m_context);
        break;
    case AcceleratorAction::ToggleShowDesktop:
        commands::toggleShowDesktop(m_context);
        break;
    case AcceleratorAction::DesksActivateDeskLeft:
    case AcceleratorAction::DesksActivateDeskRight:
        commands::activateDesk(m_context, action == AcceleratorAction::DesksActivateDeskLeft);
        break;
    case AcceleratorAction::DesksMoveActiveItemLeft:
    case AcceleratorAction::DesksMoveActiveItemRight:
        commands::moveActiveItem(m_context, action == AcceleratorAction::DesksMoveActiveItemLeft);
        break;
    case AcceleratorAction::DesksNewDesk:
        commands::newDesk(m_context);
        break;
    case AcceleratorAction::DesksRemoveCurrentDesk:
        commands::removeCurrentDesk(m_context);
        break;
    case AcceleratorAction::DesksActivate0:
    case AcceleratorAction::DesksActivate1:
    case AcceleratorAction::DesksActivate2:
    case AcceleratorAction::DesksActivate3:
    case AcceleratorAction::DesksActivate4:
    case AcceleratorAction::DesksActivate5:
    case AcceleratorAction::DesksActivate6:
    case AcceleratorAction::DesksActivate7:
        commands::activateDeskAtIndex(m_context, action);
        break;
    case AcceleratorAction::DesksToggleAssignToAllDesks:
        commands::toggleAssignToAllDesks(m_context);
        break;
    case AcceleratorAction::TakeScreenshot:
    case AcceleratorAction::TakeScreenshotToClipboard:
        commands::takeScreenshot(m_context,
                                 action == AcceleratorAction::TakeScreenshotToClipboard);
        break;
    case AcceleratorAction::TakePartialScreenshot:
    case AcceleratorAction::TakePartialScreenshotToClipboard:
        commands::takePartialScreenshot(
                m_context, action == AcceleratorAction::TakePartialScreenshotToClipboard);
        break;
    case AcceleratorAction::StartCaptureMode:
        commands::startCaptureMode(m_context);
        break;
    case AcceleratorAction::StopScreenRecording:
        commands::stopScreenRecording(m_context);
        break;
    case AcceleratorAction::ToggleClipboardHistory:
        commands::toggleClipboardHistory(m_context);
        break;
    case AcceleratorAction::VolumeMuteToggle:
        commands::volumeMuteToggle(m_context);
        break;
    case AcceleratorAction::VolumeDown:
        commands::volumeDown(m_context);
        break;
    case AcceleratorAction::VolumeUp:
        commands::volumeUp(m_context);
        break;
    case AcceleratorAction::MicrophoneMuteToggle:
        commands::microphoneMuteToggle(m_context);
        break;
    case AcceleratorAction::BrightnessDown:
        commands::brightnessDown(m_context);
        break;
    case AcceleratorAction::BrightnessUp:
        commands::brightnessUp(m_context);
        break;
    case AcceleratorAction::KeyboardBrightnessDown:
        commands::keyboardBrightnessDown(m_context);
        break;
    case AcceleratorAction::KeyboardBrightnessUp:
        commands::keyboardBrightnessUp(m_context);
        break;
    case AcceleratorAction::MediaPlayPause:
        commands::mediaPlayPause(m_context);
        break;
    case AcceleratorAction::MediaNext:
        commands::mediaNext(m_context);
        break;
    case AcceleratorAction::MediaPrevious:
        commands::mediaPrevious(m_context);
        break;
    case AcceleratorAction::MediaStop:
        commands::mediaStop(m_context);
        break;
    }
}

AcceleratorController::Restriction
AcceleratorController::acceleratorProcessingRestriction(AcceleratorAction action) const
{
    if (m_preventProcessing)
        return Restriction::PreventProcessing;
    if (m_context.session && m_context.session->isBlocked()
        && !m_actionsAllowedAtLockScreen.contains(action)) {
        return Restriction::PreventProcessing;
    }
    if (isSystemModalWindowOpen() && !m_actionsAllowedAtModalWindow.contains(action)) {
        // Propagation is stopped too, so that Alt+Tab does not end up cycling
        // through the dialog's own widgets.
        return Restriction::PreventProcessingAndPropagation;
    }
    // The overview takes the key instead: it has no active window.
    if (m_actionsBlockedInOverview.contains(action) && m_context.windowManager
        && m_context.windowManager->overview()->capturesInput()) {
        return Restriction::PreventProcessing;
    }
    if (m_actionsNeedingWindow.contains(action) && m_context.windowManager) {
        bool anyWindow = false;
        for (wm::Window *window : m_context.windowManager->windows()) {
            if (wm::canIncludeWindowInMruList(window)) {
                anyWindow = true;
                break;
            }
        }
        if (!anyWindow)
            return Restriction::PreventProcessingAndPropagation;
    }
    return Restriction::None;
}

bool AcceleratorController::isSystemModalWindowOpen() const
{
    if (!m_context.windowManager)
        return false;
    for (wm::Window *window : m_context.windowManager->windows()) {
        if (window->isSystemDialog() && window->isVisible())
            return true;
    }
    return false;
}

} // namespace Lingmo::Accelerators
