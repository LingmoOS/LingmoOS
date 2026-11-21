// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/accelerators/ash_accelerator_configuration.cc
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "accelerators/accelerator_configuration.h"

#include <algorithm>
#include <cassert>
#include <map>

#include "core/logging.h"
#include "core/preferences.h"

namespace Lingmo::Accelerators {

namespace {

// Meta is a different key under each convention, so each keeps its own.
constexpr auto kOverridesGroup = "Shortcuts";
constexpr auto kCommandOverridesGroup = "CommandShortcuts";
constexpr auto kKeyField = "key";
constexpr auto kModifiersField = "modifiers";
constexpr auto kStateField = "state";
constexpr auto kActionField = "action";

const std::array<std::pair<uint32_t, const char *>, 5> kModifierNames = {{
        {ControlDown, "Ctrl"},
        {AltDown, "Alt"},
        {ShiftDown, "Shift"},
        {MetaDown, "Meta"},
        {AltGrDown, "AltGr"},
}};

base::json::Value modifiersToJson(uint32_t modifiers)
{
    base::json::Value::Array names;
    for (const auto &[flag, name] : kModifierNames) {
        if (modifiers & flag)
            names.emplace_back(name);
    }
    return names;
}

uint32_t modifiersFromJson(const base::json::Value::Array &names)
{
    uint32_t modifiers = NoModifier;
    for (const base::json::Value &value : names) {
        for (const auto &[flag, name] : kModifierNames) {
            if (value.toString() == name)
                modifiers |= flag;
        }
    }
    return modifiers;
}

// Keys sorted at every level, as the file has always had them; overrides read from
// the file keep their own order otherwise.
base::json::Value sortedKeys(const base::json::Value &value)
{
    if (value.isArray()) {
        base::json::Value::Array result;
        for (const auto &item : value.toArray())
            result.push_back(sortedKeys(item));
        return base::json::Value(std::move(result));
    }
    if (!value.isObject())
        return value;
    std::map<std::string, base::json::Value> members;
    for (const auto &[key, member] : value.toObject())
        members.insert_or_assign(key, sortedKeys(member));
    base::json::Value::Object result;
    for (auto &[key, member] : members)
        result.emplace_back(key, std::move(member));
    return base::json::Value::object(std::move(result));
}

// The members in sorted order, as the files have always had them.
base::json::Value modificationToJson(const Accelerator &accelerator,
                                     AcceleratorConfiguration::ModificationAction action)
{
    base::json::Value object = base::json::Value::object();
    object.insert(kActionField,
                  action == AcceleratorConfiguration::ModificationAction::Add ? "add" : "remove");
    object.insert(kKeyField, keysymName(accelerator.keysym()));
    object.insert(kModifiersField, modifiersToJson(accelerator.modifiers()));
    object.insert(kStateField,
                  accelerator.keyState() == Accelerator::KeyState::Pressed ? "pressed"
                                                                           : "released");
    return object;
}

struct Modification {
    Accelerator accelerator;
    AcceleratorConfiguration::ModificationAction action;
};

std::optional<Modification> modificationFromJson(const base::json::Value &object)
{
    if (!object.isObject())
        return std::nullopt;
    const uint32_t keysym = keysymFromName(object[kKeyField].toString());
    if (keysym == 0)
        return std::nullopt;
    const std::string action = object[kActionField].toString();
    if (action != "add" && action != "remove")
        return std::nullopt;
    Accelerator accelerator(keysym, modifiersFromJson(object[kModifiersField].toArray()));
    if (object[kStateField].toString() == "released")
        accelerator.setKeyState(Accelerator::KeyState::Released);
    return Modification {accelerator,
                         action == "add" ? AcceleratorConfiguration::ModificationAction::Add
                                         : AcceleratorConfiguration::ModificationAction::Remove};
}

void setLookupMaps(std::span<const AcceleratorData> accelerators,
                   ActionIdToAcceleratorsMap &idToAccelerator,
                   AcceleratorMap<AcceleratorAction> &acceleratorToId)
{
    for (const AcceleratorData &data : accelerators) {
        Accelerator accelerator(data.keysym, data.modifiers);
        accelerator.setKeyState(data.triggerOnPress ? Accelerator::KeyState::Pressed
                                                    : Accelerator::KeyState::Released);
        if (!acceleratorToId.insertNew(accelerator, data.action)) {
            LM_WARNING(lingmoInput) << "Duplicate default accelerator" << accelerator.shortcutText();
            continue;
        }
        idToAccelerator[toActionId(data.action)].push_back(accelerator);
    }
}

} // namespace

const char *toString(AcceleratorConfigResult result)
{
    switch (result) {
    case AcceleratorConfigResult::Success:
        return "success";
    case AcceleratorConfigResult::Conflict:
        return "conflict";
    case AcceleratorConfigResult::ConflictCanOverride:
        return "conflict-can-override";
    case AcceleratorConfigResult::NotFound:
        return "not-found";
    case AcceleratorConfigResult::KeyNotAllowed:
        return "key-not-allowed";
    case AcceleratorConfigResult::MissingModifier:
        return "missing-modifier";
    case AcceleratorConfigResult::ShiftOnlyNotAllowed:
        return "shift-only-not-allowed";
    case AcceleratorConfigResult::MaximumAcceleratorsReached:
        return "maximum-accelerators-reached";
    case AcceleratorConfigResult::RestoreSuccessWithConflicts:
        return "restore-success-with-conflicts";
    case AcceleratorConfigResult::NonMetaAcceleratorWarning:
        return "non-meta-accelerator-warning";
    case AcceleratorConfigResult::ReservedKeyNotAllowed:
        return "reserved-key-not-allowed";
    }
    return "unknown";
}

AcceleratorConfiguration::AcceleratorConfiguration(const std::string &settingsFile)
    : m_settings(settingsFile.empty() ? preferencesFile("compositor") : settingsFile)
{
}

AcceleratorConfiguration::~AcceleratorConfiguration() = default;

void AcceleratorConfiguration::initialize()
{
    // The defaults' own update saves the override cache, so it must already
    // hold what the file has.
    loadOverrides();
    if (m_convention == input::ShortcutConvention::Command)
        initialize(kCommandAcceleratorData);
    else
        initialize(kAcceleratorData);
    applyOverrides();
}

void AcceleratorConfiguration::setConvention(input::ShortcutConvention convention)
{
    if (m_convention == convention)
        return;
    m_convention = convention;
    initialize();
}

void AcceleratorConfiguration::initialize(std::span<const AcceleratorData> accelerators)
{
    m_accelerators.clear();
    m_idToAccelerators.clear();
    m_acceleratorToId.clear();
    m_defaultAcceleratorsToId.clear();
    m_defaultIdToAccelerators.clear();

    setLookupMaps(accelerators, m_defaultIdToAccelerators, m_defaultAcceleratorsToId);
    // Actions without a default are still actions the user can bind.
    for (const AcceleratorAction action : allAcceleratorActions()) {
        m_defaultIdToAccelerators.try_emplace(toActionId(action));
        m_idToAccelerators.try_emplace(toActionId(action));
    }
    addAccelerators(accelerators);
}

void AcceleratorConfiguration::addAccelerators(std::span<const AcceleratorData> accelerators)
{
    setLookupMaps(accelerators, m_idToAccelerators, m_acceleratorToId);
    updateAndNotifyAccelerators();
}

std::optional<std::vector<Accelerator>>
AcceleratorConfiguration::acceleratorsForAction(AcceleratorActionId id) const
{
    const auto it = m_idToAccelerators.find(id);
    if (it == m_idToAccelerators.end())
        return std::nullopt;
    return it->second;
}

const AcceleratorAction *
AcceleratorConfiguration::findAcceleratorAction(const Accelerator &accelerator) const
{
    return m_acceleratorToId.find(accelerator);
}

std::optional<AcceleratorAction>
AcceleratorConfiguration::idForDefaultAccelerator(const Accelerator &accelerator) const
{
    const AcceleratorAction *action = m_defaultAcceleratorsToId.find(accelerator);
    return action ? std::optional<AcceleratorAction>(*action) : std::nullopt;
}

std::vector<Accelerator>
AcceleratorConfiguration::defaultAcceleratorsForId(AcceleratorActionId id) const
{
    const auto it = m_defaultIdToAccelerators.find(id);
    return it == m_defaultIdToAccelerators.end() ? std::vector<Accelerator>() : it->second;
}

bool AcceleratorConfiguration::isValid(AcceleratorActionId id) const
{
    return m_idToAccelerators.contains(id) && m_defaultIdToAccelerators.contains(id);
}

bool AcceleratorConfiguration::hasCustomAccelerators() const
{
    return totalNumberOfModifications() > 0;
}

AcceleratorConfigResult AcceleratorConfiguration::addUserAccelerator(AcceleratorActionId id,
                                                                     const Accelerator &accelerator)
{
    const AcceleratorConfigResult result = doAddAccelerator(id, accelerator, /*saveOverride=*/true);
    if (result == AcceleratorConfigResult::Success)
        updateAndNotifyAccelerators();
    LM_DEBUG(lingmoInput) << "addAccelerator" << id << accelerator.shortcutText() << toString(result);
    return result;
}

AcceleratorConfigResult AcceleratorConfiguration::removeAccelerator(AcceleratorActionId id,
                                                                    const Accelerator &accelerator)
{
    const AcceleratorConfigResult result =
            doRemoveAccelerator(id, accelerator, /*saveOverride=*/true);
    if (result == AcceleratorConfigResult::Success) {
        restoreDefaultOwner(m_acceleratorToId.remapForLookup(accelerator));
        updateAndNotifyAccelerators();
    }
    LM_DEBUG(lingmoInput) << "removeAccelerator" << id << accelerator.shortcutText() << toString(result);
    return result;
}

AcceleratorConfigResult
AcceleratorConfiguration::replaceAccelerator(AcceleratorActionId id,
                                             const Accelerator &oldAccelerator,
                                             const Accelerator &newAccelerator)
{
    const AcceleratorConfigResult result = doReplaceAccelerator(id, oldAccelerator, newAccelerator);
    if (result == AcceleratorConfigResult::Success)
        updateAndNotifyAccelerators();
    LM_DEBUG(lingmoInput) << "replaceAccelerator" << id << oldAccelerator.shortcutText() << "->"
                     << newAccelerator.shortcutText() << toString(result);
    return result;
}

AcceleratorConfigResult AcceleratorConfiguration::restoreDefault(AcceleratorActionId id)
{
    const auto current = m_idToAccelerators.find(id);
    if (current == m_idToAccelerators.end())
        return AcceleratorConfigResult::NotFound;

    std::vector<Accelerator> &acceleratorsForId = current->second;
    // Clear the reverse mapping first.
    for (const Accelerator &accelerator : acceleratorsForId) {
        const AcceleratorAction *found = m_acceleratorToId.find(accelerator);
        if (!found || toActionId(*found) != id)
            return AcceleratorConfigResult::NotFound;
        m_acceleratorToId.erase(accelerator);
    }
    const std::vector<Accelerator> released = std::move(acceleratorsForId);
    acceleratorsForId.clear();

    m_overrides.erase(acceleratorActionName(static_cast<AcceleratorAction>(id)));

    // Whatever this action had taken from another one goes back there; its
    // own defaults are brought back below.
    for (const Accelerator &accelerator : released) {
        if (idForDefaultAccelerator(accelerator) != static_cast<AcceleratorAction>(id))
            restoreDefaultOwner(accelerator);
    }

    // Only bring back the defaults nobody else is using now. The user re-adds
    // the others by hand, which is what makes the conflict visible.
    const auto defaults = m_defaultIdToAccelerators.find(id);
    assert(defaults != m_defaultIdToAccelerators.end());

    AcceleratorConfigResult result = AcceleratorConfigResult::Success;
    for (const Accelerator &accelerator : defaults->second) {
        if (!m_acceleratorToId.find(accelerator)) {
            acceleratorsForId.push_back(accelerator);
            m_acceleratorToId.insertNew(accelerator, static_cast<AcceleratorAction>(id));
        } else {
            result = AcceleratorConfigResult::RestoreSuccessWithConflicts;
        }
    }

    updateAndNotifyAccelerators();
    LM_DEBUG(lingmoInput) << "restoreDefault" << id << toString(result);
    return result;
}

AcceleratorConfigResult AcceleratorConfiguration::restoreAllDefaults()
{
    resetAllAccelerators();
    m_overrides.clear();
    updateAndNotifyAccelerators();
    return AcceleratorConfigResult::Success;
}

AcceleratorConfigResult AcceleratorConfiguration::doRemoveAccelerator(
        AcceleratorActionId id, const Accelerator &accelerator, bool saveOverride)
{
    const AcceleratorAction *foundId = m_acceleratorToId.find(accelerator);
    const auto foundAccelerators = m_idToAccelerators.find(id);
    if (foundAccelerators == m_idToAccelerators.end() || !foundId)
        return AcceleratorConfigResult::NotFound;
    if (toActionId(*foundId) != id)
        return AcceleratorConfigResult::NotFound;

    const Accelerator stored = m_acceleratorToId.remapForLookup(accelerator);
    std::erase(foundAccelerators->second, stored);
    m_acceleratorToId.erase(stored);

    // A press/release pair belongs together: removing one removes the other.
    Accelerator reverse(stored);
    reverse.setKeyState(stored.keyState() == Accelerator::KeyState::Pressed
                                ? Accelerator::KeyState::Released
                                : Accelerator::KeyState::Pressed);
    const AcceleratorAction *reverseId = m_acceleratorToId.find(reverse);
    if (reverseId && toActionId(*reverseId) == id) {
        std::erase(foundAccelerators->second, reverse);
        m_acceleratorToId.erase(reverse);
    }

    if (saveOverride)
        updateOverrides(id, stored, ModificationAction::Remove);
    return AcceleratorConfigResult::Success;
}

AcceleratorConfigResult AcceleratorConfiguration::doAddAccelerator(AcceleratorActionId id,
                                                                   const Accelerator &accelerator,
                                                                   bool saveOverride)
{
    const auto accelerators = m_idToAccelerators.find(id);
    if (accelerators == m_idToAccelerators.end())
        return AcceleratorConfigResult::NotFound;

    // Whoever holds the combination now loses it. A default that is taken
    // from its own action is not recorded as a removal: applying the add
    // override again reproduces it.
    if (const AcceleratorAction *conflict = findAcceleratorAction(accelerator)) {
        bool saveRemoveOverride = true;
        if (const auto defaultId = idForDefaultAccelerator(accelerator))
            saveRemoveOverride = *defaultId != *conflict;
        const AcceleratorConfigResult removed =
                doRemoveAccelerator(toActionId(*conflict), accelerator, saveRemoveOverride);
        if (removed != AcceleratorConfigResult::Success)
            return removed;
    }

    accelerators->second.push_back(accelerator);
    m_acceleratorToId.insertNew(accelerator, static_cast<AcceleratorAction>(id));

    if (saveOverride)
        updateOverrides(id, accelerator, ModificationAction::Add);
    return AcceleratorConfigResult::Success;
}

AcceleratorConfigResult
AcceleratorConfiguration::doReplaceAccelerator(AcceleratorActionId id,
                                               const Accelerator &oldAccelerator,
                                               const Accelerator &newAccelerator)
{
    const AcceleratorAction *foundId = m_acceleratorToId.find(oldAccelerator);
    if (!foundId || toActionId(*foundId) != id)
        return AcceleratorConfigResult::NotFound;

    const AcceleratorConfigResult removed =
            doRemoveAccelerator(id, oldAccelerator, /*saveOverride=*/true);
    if (removed != AcceleratorConfigResult::Success)
        return removed;
    restoreDefaultOwner(oldAccelerator);
    return doAddAccelerator(id, newAccelerator, /*saveOverride=*/true);
}

void AcceleratorConfiguration::setUsePositionalLookup(bool use)
{
    m_acceleratorToId.setUsePositionalLookup(use);
    m_defaultAcceleratorsToId.setUsePositionalLookup(use);
}

void AcceleratorConfiguration::updateAndNotifyAccelerators()
{
    m_accelerators.clear();
    m_accelerators.reserve(m_acceleratorToId.size());
    for (const auto &[accelerator, action] : m_acceleratorToId)
        m_accelerators.push_back(accelerator);

    saveOverrides();
    acceleratorsUpdated();
}

std::string AcceleratorConfiguration::overridesGroup() const
{
    return m_convention == input::ShortcutConvention::Command ? kCommandOverridesGroup
                                                              : kOverridesGroup;
}

void AcceleratorConfiguration::loadOverrides()
{
    m_overrides.clear();
    m_settings.sync();
    const std::string group = overridesGroup();
    for (const std::string &action : m_settings.childKeys(group)) {
        const auto document =
                base::json::Value::parse(m_settings.stringValue(group + "/" + action));
        if (document && document->isArray())
            m_overrides[action] = document->toArray();
    }
}

void AcceleratorConfiguration::saveOverrides()
{
    const std::string group = overridesGroup();
    m_settings.remove(group);
    for (const auto &[action, list] : m_overrides)
        m_settings.setValue(group + "/" + action, sortedKeys(base::json::Value(list)).serialize());
    if (!m_settings.sync())
        LM_WARNING(lingmoInput) << "Cannot persist shortcut overrides to" << m_settings.path();
}

void AcceleratorConfiguration::applyOverrides()
{
    std::vector<std::string> actionsToRemove;

    for (auto &[actionName, list] : m_overrides) {
        const auto action = acceleratorActionFromName(actionName);
        if (!action || !isValid(toActionId(*action))) {
            LM_INFO(lingmoInput) << "Dropping shortcut overrides for unknown action"
                            << actionName;
            actionsToRemove.push_back(actionName);
            continue;
        }
        if (list.empty()) {
            actionsToRemove.push_back(actionName);
            continue;
        }
        const AcceleratorActionId id = toActionId(*action);

        for (size_t i = 0; i < list.size();) {
            const auto modification = modificationFromJson(list[i]);
            if (!modification) {
                LM_WARNING(lingmoInput) << "Malformed shortcut override for" << actionName;
                list.erase(list.begin() + i);
                continue;
            }
            if (modification->action == ModificationAction::Remove) {
                // A removal of something that no longer exists is stale; it
                // may also have been re-added to another action meanwhile.
                const AcceleratorAction *found =
                        m_acceleratorToId.find(modification->accelerator);
                if (!found) {
                    list.erase(list.begin() + i);
                    continue;
                }
                if (toActionId(*found) == id)
                    doRemoveAccelerator(id, modification->accelerator, /*saveOverride=*/false);
            } else {
                doAddAccelerator(id, modification->accelerator, /*saveOverride=*/false);
            }
            ++i;
        }
        if (list.empty())
            actionsToRemove.push_back(actionName);
    }

    for (const std::string &action : actionsToRemove)
        m_overrides.erase(action);

    if (!areAcceleratorsValid()) {
        LM_WARNING(lingmoInput) << "Shortcut overrides are inconsistent; restoring the defaults";
        restoreAllDefaults();
        return;
    }
    updateAndNotifyAccelerators();
}

void AcceleratorConfiguration::updateOverrides(AcceleratorActionId id,
                                               const Accelerator &accelerator,
                                               ModificationAction action)
{
    const std::string key = acceleratorActionName(static_cast<AcceleratorAction>(id));
    auto entry = m_overrides.find(key);
    if (entry == m_overrides.end()) {
        m_overrides[key] = {modificationToJson(accelerator, action)};
        return;
    }

    base::json::Value::Array &list = entry->second;
    for (size_t i = 0; i < list.size(); ++i) {
        const auto modification = modificationFromJson(list[i]);
        if (modification && modification->accelerator == accelerator) {
            // The opposite modification of the same combination cancels it:
            // a custom accelerator removed again, or a default re-enabled.
            assert(modification->action != action);
            list.erase(list.begin() + i);
            if (list.empty())
                m_overrides.erase(entry);
            return;
        }
    }
    list.push_back(modificationToJson(accelerator, action));
}

bool AcceleratorConfiguration::hasOverride(AcceleratorActionId id, const Accelerator &accelerator,
                                           ModificationAction action) const
{
    const auto entry = m_overrides.find(acceleratorActionName(static_cast<AcceleratorAction>(id)));
    if (entry == m_overrides.end())
        return false;
    for (const base::json::Value &value : entry->second) {
        const auto modification = modificationFromJson(value);
        if (modification && modification->accelerator == accelerator
            && modification->action == action)
            return true;
    }
    return false;
}

// A default that was moved to another action was not recorded as removed from
// its own, so the override file says nothing about it once the user takes it
// away from the new owner again. The running state has to follow the file and
// give it back, unless the user had disabled it on its own action.
void AcceleratorConfiguration::restoreDefaultOwner(const Accelerator &accelerator)
{
    const auto owner = idForDefaultAccelerator(accelerator);
    if (!owner)
        return;

    const AcceleratorActionId id = toActionId(*owner);
    const auto accelerators = m_idToAccelerators.find(id);
    const auto defaults = m_defaultIdToAccelerators.find(id);
    if (accelerators == m_idToAccelerators.end() || defaults == m_defaultIdToAccelerators.end())
        return;

    Accelerator reverse(accelerator);
    reverse.setKeyState(accelerator.keyState() == Accelerator::KeyState::Pressed
                                ? Accelerator::KeyState::Released
                                : Accelerator::KeyState::Pressed);

    // The press/release pair was removed together, so it comes back together.
    for (const Accelerator &candidate : defaults->second) {
        if (candidate != accelerator && candidate != reverse)
            continue;
        if (m_acceleratorToId.find(candidate)
            || hasOverride(id, candidate, ModificationAction::Remove))
            continue;
        accelerators->second.push_back(candidate);
        m_acceleratorToId.insertNew(candidate, *owner);
    }
}

bool AcceleratorConfiguration::areAcceleratorsValid()
{
    for (const auto &[id, accelerators] : m_idToAccelerators) {
        for (const Accelerator &accelerator : accelerators) {
            const AcceleratorAction *found = m_acceleratorToId.find(accelerator);
            if (!found || toActionId(*found) != id)
                return false;
        }
    }
    for (const auto &[accelerator, action] : m_acceleratorToId) {
        const auto it = m_idToAccelerators.find(toActionId(action));
        if (it == m_idToAccelerators.end())
            return false;
        if (std::find(it->second.begin(), it->second.end(), accelerator) == it->second.end())
            return false;
    }
    return true;
}

void AcceleratorConfiguration::resetAllAccelerators()
{
    m_accelerators.clear();
    m_idToAccelerators = m_defaultIdToAccelerators;
    m_acceleratorToId = m_defaultAcceleratorsToId;
}

int AcceleratorConfiguration::totalNumberOfModifications() const
{
    int count = 0;
    for (const auto &[action, list] : m_overrides)
        count += int(list.size());
    return count;
}

} // namespace Lingmo::Accelerators
