// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/webui/shortcut_customization_ui/backend/
//           accelerator_configuration_provider.cc
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "accelerators/accelerator_service.h"

#include <algorithm>

#include <xkbcommon/xkbcommon.h>

#include "accelerators/accelerator_controller.h"
#include "core/logging.h"
#include "input/dbus_export.h"
#include "input/xkb_tracker.h"

namespace Lingmo::Accelerators {

namespace {

constexpr auto kService = "org.lingmo.Shortcuts1";
constexpr auto kPath = "/org/lingmo/Shortcuts1";

constexpr size_t kMaxAcceleratorsAllowed = 5;

// Only these modifiers take part in a customized accelerator.
constexpr uint32_t kCustomizationModifierMask = ShiftDown | ControlDown | AltDown | MetaDown;

// Keys that may not be the activation key of a custom accelerator: power
// control, and the locks whose state xkb owns.
constexpr uint32_t kReservedKeys[] = {
        XKB_KEY_XF86PowerOff, XKB_KEY_XF86Sleep, XKB_KEY_Caps_Lock,
        XKB_KEY_Scroll_Lock,  XKB_KEY_Num_Lock,
};

bool isMetaKeysym(uint32_t keysym)
{
    return keysym == XKB_KEY_Super_L || keysym == XKB_KEY_Super_R;
}

std::string toJsonString(const base::json::Value &object)
{
    return object.serialize();
}

} // namespace

AcceleratorService::AcceleratorService(AcceleratorConfiguration *configuration,
                                       AcceleratorController *controller,
                                       const input::XkbTracker *xkb)
    : m_configuration(configuration)
    , m_controller(controller)
    , m_xkb(xkb)
{
    m_configurationUpdates = m_configuration->acceleratorsUpdated.connect([this] {
        acceleratorsChanged();
        if (m_object)
            m_object->emitSignal("acceleratorsChanged");
    });
}

AcceleratorService::~AcceleratorService() = default;

std::string AcceleratorService::shortcutConvention() const
{
    return m_configuration->convention() == input::ShortcutConvention::Command ? "command" : "pc";
}

bool AcceleratorService::registerService()
{
    if (m_object)
        return true;
    auto exported = input::DBusExport::create(kService, kPath, kService);
    if (!exported) {
        LM_WARNING(lingmoInput) << "Cannot export the shortcuts service: no session bus";
        return false;
    }
    base::dbus::Object &object = exported->object();
    object.addProperty("shortcutConvention", [this] { return shortcutConvention(); });
    object.addMethod("accelerators", [this] { return accelerators(); });
    object.addMethod("addAccelerator", [this](const std::string &action, uint32_t keycode,
                                              uint32_t keysym, uint32_t modifiers) {
        return addAccelerator(action, keycode, keysym, modifiers);
    }, base::dbus::ArgumentNames{{"action", "keycode", "keysym", "modifiers"}, {"result"}});
    object.addMethod("removeAccelerator",
                     [this](const std::string &action, uint32_t keysym, uint32_t modifiers) {
                         return removeAccelerator(action, keysym, modifiers);
                     },
                     base::dbus::ArgumentNames{{"action", "keysym", "modifiers"}, {"result"}});
    object.addMethod("replaceAccelerator",
                     [this](const std::string &action, uint32_t oldKeysym, uint32_t oldModifiers,
                            uint32_t keycode, uint32_t keysym, uint32_t modifiers) {
                         return replaceAccelerator(action, oldKeysym, oldModifiers, keycode,
                                                   keysym, modifiers);
                     },
                     base::dbus::ArgumentNames{{"action", "oldKeysym", "oldModifiers", "keycode",
                                                "keysym", "modifiers"},
                                               {"result"}});
    object.addMethod("restoreDefault",
                     [this](const std::string &action) { return restoreDefault(action); },
                     base::dbus::ArgumentNames{{"action"}, {"result"}});
    object.addMethod("restoreAllDefaults", [this] { return restoreAllDefaults(); });
    object.addMethod("describeKey", [this](uint32_t keycode, uint32_t keysym) {
        return describeKey(keycode, keysym);
    }, base::dbus::ArgumentNames{{"keycode", "keysym"}, {"result"}});
    object.addMethod("preventProcessingAccelerators", "b", "",
                     [this](base::dbus::MethodCall &call) {
                         bool prevent = false;
                         if (!call.read(prevent)) {
                             call.replyError("org.freedesktop.DBus.Error.InvalidArgs",
                                             "Unexpected arguments");
                             return;
                         }
                         preventProcessingAccelerators(prevent, call.message().sender());
                         call.reply();
                     },
                     base::dbus::ArgumentNames{{"prevent"}, {}});
    object.addMethod("hasCustomAccelerators", [this] { return hasCustomAccelerators(); });
    object.addSignal("acceleratorsChanged", "");
    if (!exported->publish("the shortcuts service"))
        return false;
    m_object = std::move(exported);
    return true;
}

// The members in sorted order, as the service has always sent them.
base::json::Value AcceleratorService::acceleratorToJson(const Accelerator &accelerator,
                                                        AcceleratorActionId id) const
{
    base::json::Value object = base::json::Value::object();
    object.insert("keyName", keysymName(accelerator.keysym()));
    object.insert("keyState",
                  accelerator.keyState() == Accelerator::KeyState::Pressed ? "pressed"
                                                                           : "released");
    object.insert("keyText", keysymText(accelerator.keysym()));
    object.insert("keysym", int64_t(accelerator.keysym()));
    object.insert("modifiers", int64_t(accelerator.modifiers()));
    const auto defaultId = m_configuration->idForDefaultAccelerator(accelerator);
    object.insert("type", defaultId && toActionId(*defaultId) == id ? "default" : "user");
    return object;
}

std::string AcceleratorService::accelerators()
{
    base::json::Value::Array actions;
    for (const auto &[id, accelerators] : m_configuration->acceleratorMapping()) {
        base::json::Value::Array current;
        for (const Accelerator &accelerator : accelerators)
            current.push_back(acceleratorToJson(accelerator, id));
        base::json::Value::Array defaults;
        for (const Accelerator &accelerator : m_configuration->defaultAcceleratorsForId(id))
            defaults.push_back(acceleratorToJson(accelerator, id));
        base::json::Value entry = base::json::Value::object();
        entry.insert("accelerators", std::move(current));
        entry.insert("defaults", std::move(defaults));
        entry.insert("name", acceleratorActionName(static_cast<AcceleratorAction>(id)));
        actions.push_back(std::move(entry));
    }
    base::json::Value root = base::json::Value::object();
    root.insert("actions", std::move(actions));
    return toJsonString(root);
}

Accelerator AcceleratorService::resolve(uint32_t keycode, uint32_t keysym, uint32_t modifiers) const
{
    // The compositor's keymap is what the client typed against, so the evdev
    // code gives the same base keysym the filter will see.
    uint32_t resolved = keycode != 0 && m_xkb ? m_xkb->baseKeysym(keycode) : 0;
    if (resolved == 0)
        resolved = xkb_keysym_to_lower(keysym);
    // On a positional layout a punctuation key is stored as the US key at its
    // position, which is what the filter will look it up by.
    if (m_configuration->usesPositionalLookup() && keycode != 0) {
        if (const uint32_t positional = positionalUsKeysym(keycode, resolved))
            resolved = positional;
    }
    // A lone Meta is Ash's launcher key: it triggers on release, so that
    // Meta+<key> can still be typed with Meta held, and the launcher state
    // machine decides whether the tap was really alone.
    const uint32_t customizationModifiers = modifiers & kCustomizationModifierMask;
    const bool metaTap = isMetaKeysym(resolved) && customizationModifiers == NoModifier;
    return Accelerator(resolved, keycode, customizationModifiers,
                       metaTap ? Accelerator::KeyState::Released : Accelerator::KeyState::Pressed);
}

std::optional<AcceleratorConfigResult>
AcceleratorService::validateAction(const std::string &name, AcceleratorActionId *id) const
{
    const auto action = acceleratorActionFromName(name);
    if (!action || !m_configuration->isValid(toActionId(*action)))
        return AcceleratorConfigResult::NotFound;
    *id = toActionId(*action);
    return std::nullopt;
}

std::optional<AcceleratorConfigResult>
AcceleratorService::validateAccelerator(const Accelerator &accelerator) const
{
    const uint32_t modifiers = accelerator.modifiers() & kCustomizationModifierMask;

    if (accelerator.keyState() == Accelerator::KeyState::Released)
        return std::nullopt;

    // Modifier + key, unless the key is a function key.
    if (modifiers == NoModifier && !isFunctionKeysym(accelerator.keysym()))
        return AcceleratorConfigResult::MissingModifier;

    if (std::find(std::begin(kReservedKeys), std::end(kReservedKeys), accelerator.keysym())
        != std::end(kReservedKeys)) {
        return AcceleratorConfigResult::ReservedKeyNotAllowed;
    }

    // A modifier cannot be the activation key of a custom accelerator.
    if (isModifierKeysym(accelerator.keysym()) || accelerator.keysym() == 0)
        return AcceleratorConfigResult::KeyNotAllowed;

    if (modifiers == ShiftDown)
        return AcceleratorConfigResult::ShiftOnlyNotAllowed;

    return std::nullopt;
}

std::optional<base::json::Value>
AcceleratorService::preprocessAddAccelerator(AcceleratorActionId id, const Accelerator &accelerator)
{
    const AcceleratorAction *found = m_configuration->findAcceleratorAction(accelerator);
    if (!found)
        return std::nullopt;

    const std::string conflictName = acceleratorActionName(*found);

    // Already one of this action's own accelerators.
    if (toActionId(*found) == id) {
        m_pending.reset();
        base::json::Value object = base::json::Value::object();
        object.insert("conflictAction", conflictName);
        object.insert("result", toString(AcceleratorConfigResult::Conflict));
        return object;
    }

    // Taking a combination from another action needs a second, identical
    // request as confirmation.
    if (!m_pending || m_pending->action != id || m_pending->accelerator != accelerator) {
        m_pending = PendingAccelerator {accelerator, id};
        m_conflictState = ConflictErrorState::AwaitingConflictResolution;
        base::json::Value object = base::json::Value::object();
        object.insert("conflictAction", conflictName);
        object.insert("result", toString(AcceleratorConfigResult::ConflictCanOverride));
        return object;
    }
    if (m_conflictState == ConflictErrorState::AwaitingConflictResolution)
        m_conflictState = ConflictErrorState::Standby;
    return std::nullopt;
}

AcceleratorService::ConflictErrorState
AcceleratorService::maybeHandleNonMetaAccelerator(const Accelerator &accelerator,
                                                  AcceleratorActionId id)
{
    // Re-adding a default is never warned about.
    const std::vector<Accelerator> defaults = m_configuration->defaultAcceleratorsForId(id);
    if (std::find(defaults.begin(), defaults.end(), accelerator) != defaults.end())
        return ConflictErrorState::Standby;

    if (m_conflictState != ConflictErrorState::AwaitingNonMetaConfirmation)
        m_pending.reset();

    // Applications never see Meta. Under the Command convention they get both
    // Command and Control as Ctrl, so only the two together are clear of them.
    const bool clear = m_configuration->convention() == input::ShortcutConvention::Command
            ? accelerator.isMetaDown() && accelerator.isControlDown()
            : accelerator.isMetaDown();
    // Function keys cannot be used with Meta, so they are not warned about;
    // neither is the Meta tap itself.
    if (!clear && !isFunctionKeysym(accelerator.keysym())
        && accelerator.keyState() == Accelerator::KeyState::Pressed) {
        if (!m_pending || m_pending->action != id || m_pending->accelerator != accelerator) {
            m_pending = PendingAccelerator {accelerator, id};
            return ConflictErrorState::AwaitingNonMetaConfirmation;
        }
    }
    return ConflictErrorState::Standby;
}

std::string AcceleratorService::result(AcceleratorConfigResult result,
                                       const std::string &conflictAction) const
{
    base::json::Value object = base::json::Value::object();
    if (!conflictAction.empty())
        object.insert("conflictAction", conflictAction);
    object.insert("result", toString(result));
    return toJsonString(object);
}

std::string AcceleratorService::addAccelerator(const std::string &action, uint32_t keycode,
                                               uint32_t keysym, uint32_t modifiers)
{
    AcceleratorActionId id = 0;
    if (const auto error = validateAction(action, &id))
        return result(*error);
    const Accelerator accelerator = resolve(keycode, keysym, modifiers);

    const std::vector<Accelerator> defaults = m_configuration->defaultAcceleratorsForId(id);
    if (std::find(defaults.begin(), defaults.end(), accelerator) == defaults.end()) {
        if (const auto error = validateAccelerator(accelerator)) {
            m_pending.reset();
            return result(*error);
        }
    }

    if (const auto current = m_configuration->acceleratorsForAction(id);
        current && current->size() >= kMaxAcceleratorsAllowed) {
        return result(AcceleratorConfigResult::MaximumAcceleratorsReached);
    }

    if (const auto conflict = preprocessAddAccelerator(id, accelerator))
        return toJsonString(*conflict);

    m_conflictState = maybeHandleNonMetaAccelerator(accelerator, id);
    if (m_conflictState != ConflictErrorState::Standby)
        return result(AcceleratorConfigResult::NonMetaAcceleratorWarning);

    m_pending.reset();
    return result(m_configuration->addUserAccelerator(id, accelerator));
}

std::string AcceleratorService::removeAccelerator(const std::string &action, uint32_t keysym,
                                                  uint32_t modifiers)
{
    AcceleratorActionId id = 0;
    if (const auto error = validateAction(action, &id))
        return result(*error);
    Accelerator accelerator = resolve(0, keysym, modifiers);
    // A default that triggers on release is one the caller does not know
    // about: it only ever sees presses.
    if (!m_configuration->findAcceleratorAction(accelerator)) {
        accelerator.setKeyState(Accelerator::KeyState::Released);
        if (!m_configuration->findAcceleratorAction(accelerator))
            accelerator.setKeyState(Accelerator::KeyState::Pressed);
    }
    return result(m_configuration->removeAccelerator(id, accelerator));
}

std::string AcceleratorService::replaceAccelerator(const std::string &action, uint32_t oldKeysym,
                                                   uint32_t oldModifiers, uint32_t keycode,
                                                   uint32_t keysym, uint32_t modifiers)
{
    AcceleratorActionId id = 0;
    if (const auto error = validateAction(action, &id))
        return result(*error);
    Accelerator oldAccelerator = resolve(0, oldKeysym, oldModifiers);
    if (!m_configuration->findAcceleratorAction(oldAccelerator)) {
        oldAccelerator.setKeyState(Accelerator::KeyState::Released);
        if (!m_configuration->findAcceleratorAction(oldAccelerator))
            oldAccelerator.setKeyState(Accelerator::KeyState::Pressed);
    }
    const Accelerator newAccelerator = resolve(keycode, keysym, modifiers);

    const std::vector<Accelerator> defaults = m_configuration->defaultAcceleratorsForId(id);
    if (std::find(defaults.begin(), defaults.end(), newAccelerator) == defaults.end()) {
        if (const auto error = validateAccelerator(newAccelerator))
            return result(*error);
    }

    const AcceleratorAction *oldId = m_configuration->findAcceleratorAction(oldAccelerator);
    if (!oldId || toActionId(*oldId) != id)
        return result(AcceleratorConfigResult::NotFound);

    if (const auto conflict = preprocessAddAccelerator(id, newAccelerator))
        return toJsonString(*conflict);

    m_conflictState = maybeHandleNonMetaAccelerator(newAccelerator, id);
    if (m_conflictState != ConflictErrorState::Standby)
        return result(AcceleratorConfigResult::NonMetaAcceleratorWarning);

    m_pending.reset();
    return result(m_configuration->replaceAccelerator(id, oldAccelerator, newAccelerator));
}

std::string AcceleratorService::restoreDefault(const std::string &action)
{
    AcceleratorActionId id = 0;
    if (const auto error = validateAction(action, &id))
        return result(*error);
    return result(m_configuration->restoreDefault(id));
}

std::string AcceleratorService::restoreAllDefaults()
{
    return result(m_configuration->restoreAllDefaults());
}

std::string AcceleratorService::describeKey(uint32_t keycode, uint32_t keysym)
{
    const Accelerator accelerator = resolve(keycode, keysym, NoModifier);
    base::json::Value object = base::json::Value::object();
    object.insert("keyName", keysymName(accelerator.keysym()));
    object.insert("keyText", keysymText(accelerator.keysym()));
    object.insert("keysym", int64_t(accelerator.keysym()));
    object.insert("modifierKey", isModifierKeysym(accelerator.keysym()));
    return toJsonString(object);
}

void AcceleratorService::preventProcessingAccelerators(bool prevent)
{
    preventProcessingAccelerators(prevent, {});
}

void AcceleratorService::preventProcessingAccelerators(bool prevent, const std::string &owner)
{
    // Ash resets the pending state whenever the recorder starts or stops.
    m_pending.reset();
    m_conflictState = ConflictErrorState::Standby;
    m_controller->setPreventProcessingAccelerators(prevent);
    setPreventOwner(prevent ? owner : std::string());
}

void AcceleratorService::setPreventOwner(const std::string &owner)
{
    if (owner == m_preventOwner)
        return;
    m_preventOwnerWatch = {};
    m_preventOwner = owner;
    if (owner.empty() || !m_object)
        return;
    // Also reports a caller that left before the watch was in place.
    m_preventOwnerWatch = m_object->connection()->watchName(
            owner, [this, owner](const std::string &current) {
                if (current.empty())
                    releasePreventOwner(owner);
            });
}

void AcceleratorService::releasePreventOwner(const std::string &owner)
{
    if (owner != m_preventOwner)
        return;
    preventProcessingAccelerators(false);
}

bool AcceleratorService::hasCustomAccelerators()
{
    return m_configuration->hasCustomAccelerators();
}

} // namespace Lingmo::Accelerators
