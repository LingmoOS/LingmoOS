// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/webui/shortcut_customization_ui/backend/
//           accelerator_configuration_provider.h
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65
//
// Adapted for Lingmo: the mojo provider the Shortcuts app talks to becomes
// a D-Bus service, org.lingmo.Shortcuts1, carrying JSON. The layout table
// (categories, descriptions, order) lives with the Settings UI, which owns the
// translations; this side deals in action names and keysyms only.
#pragma once


#include <memory>
#include <optional>
#include <string>

#include "accelerators/accelerator.h"
#include "accelerators/accelerator_actions.h"
#include "accelerators/accelerator_configuration.h"
#include "base/dbus.h"
#include "base/json.h"
#include "base/signal.h"

namespace Lingmo::Input {
class DBusExport;
class XkbTracker;
}

namespace Lingmo::Accelerators {

class AcceleratorController;

class AcceleratorService
{
public:
    AcceleratorService(AcceleratorConfiguration *configuration, AcceleratorController *controller,
                       const input::XkbTracker *xkb);
    ~AcceleratorService();

    bool registerService();

    // "pc", or "command" when Meta is the Command key next to the space bar,
    // Alt is Option and Ctrl is Control; see input::ShortcutConvention.
    std::string shortcutConvention() const;

    // JSON for one accelerator, shared with the tests.
    base::json::Value acceleratorToJson(const Accelerator &accelerator,
                                        AcceleratorActionId id) const;

    // {"actions": [{"name", "accelerators": [...], "defaults": [...]}]}.
    std::string accelerators();

    // Every mutation answers {"result": <AcceleratorConfigResult name>,
    // "conflictAction": <action name, when the result names one>}. `keycode`
    // is the evdev code of the key the user pressed, 0 when unknown, and
    // `keysym` the keysym the toolkit reported for it. A Meta keycode with no
    // modifiers binds the Meta tap.
    std::string addAccelerator(const std::string &action, uint32_t keycode, uint32_t keysym,
                               uint32_t modifiers);
    std::string removeAccelerator(const std::string &action, uint32_t keysym, uint32_t modifiers);
    std::string replaceAccelerator(const std::string &action, uint32_t oldKeysym,
                                   uint32_t oldModifiers, uint32_t keycode, uint32_t keysym,
                                   uint32_t modifiers);
    std::string restoreDefault(const std::string &action);
    std::string restoreAllDefaults();

    // {"keysym", "keyName", "keyText", "modifierKey"} for a key the recorder
    // saw. A lone Meta (no other modifier) is accepted as a shortcut that
    // triggers on release; other modifiers on their own are not.
    std::string describeKey(uint32_t keycode, uint32_t keysym);

    // True while the user is recording a shortcut, so that the keys they
    // press do not also trigger anything. Also resets the confirmation
    // state of a pending conflict or warning. Over the bus the caller owns
    // the flag; it is lifted when the caller leaves the bus.
    void preventProcessingAccelerators(bool prevent);

    bool hasCustomAccelerators();

    base::Signal<> acceleratorsChanged;

private:
    // Which confirmation the last answer asked the user for.
    enum class ConflictErrorState {
        Standby,
        AwaitingConflictResolution,
        AwaitingNonMetaConfirmation,
    };

    struct PendingAccelerator {
        Accelerator accelerator;
        AcceleratorActionId action;
    };

    // Over the bus the caller owns the flag.
    void preventProcessingAccelerators(bool prevent, const std::string &owner);
    Accelerator resolve(uint32_t keycode, uint32_t keysym, uint32_t modifiers) const;
    std::optional<AcceleratorConfigResult> validateAction(const std::string &name,
                                                          AcceleratorActionId *id) const;
    std::optional<AcceleratorConfigResult> validateAccelerator(const Accelerator &accelerator) const;
    std::optional<base::json::Value> preprocessAddAccelerator(AcceleratorActionId id,
                                                              const Accelerator &accelerator);
    ConflictErrorState maybeHandleNonMetaAccelerator(const Accelerator &accelerator,
                                                     AcceleratorActionId id);
    std::string result(AcceleratorConfigResult result, const std::string &conflictAction = {}) const;
    void setPreventOwner(const std::string &owner);
    void releasePreventOwner(const std::string &owner);

    AcceleratorConfiguration *m_configuration = nullptr;
    AcceleratorController *m_controller = nullptr;
    const input::XkbTracker *m_xkb = nullptr;
    std::unique_ptr<input::DBusExport> m_object;
    base::Subscription m_configurationUpdates;

    std::string m_preventOwner;
    base::dbus::NameWatch m_preventOwnerWatch;

    std::optional<PendingAccelerator> m_pending;
    ConflictErrorState m_conflictState = ConflictErrorState::Standby;
};

} // namespace Lingmo::Accelerators

