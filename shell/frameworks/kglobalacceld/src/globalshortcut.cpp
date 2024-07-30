/*
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "globalshortcut.h"

#include "kglobalshortcutinfo_p.h"

#include "component.h"
#include "globalshortcutcontext.h"
#include "globalshortcutsregistry.h"
#include "logging_p.h"

#include <QKeySequence>

GlobalShortcut::GlobalShortcut()
    : GlobalShortcut(QString{}, QString{}, nullptr)
{
}

GlobalShortcut::GlobalShortcut(const QString &uniqueName, const QString &friendlyName, GlobalShortcutContext *context)
    : _isPresent(false)
    , _isRegistered(false)
    , _isFresh(true)
    , _registry(GlobalShortcutsRegistry::self())
    , _context(context)
    , _uniqueName(uniqueName)
    , _friendlyName(friendlyName)
{
    if (_context) {
        _context->addShortcut(this);
    }
}

GlobalShortcut::~GlobalShortcut()
{
    setInactive();
}

GlobalShortcut::operator KGlobalShortcutInfo() const
{
    KGlobalShortcutInfo info;
    info.d->uniqueName = _uniqueName;
    info.d->friendlyName = _friendlyName;
    info.d->contextUniqueName = context()->uniqueName();
    info.d->contextFriendlyName = context()->friendlyName();
    info.d->componentUniqueName = context()->component()->uniqueName();
    info.d->componentFriendlyName = context()->component()->friendlyName();
    for (const QKeySequence &key : std::as_const(_keys)) {
        info.d->keys.append(key);
    }
    for (const QKeySequence &key : std::as_const(_defaultKeys)) {
        info.d->defaultKeys.append(key);
    }
    return info;
}

bool GlobalShortcut::isActive() const
{
    return _isRegistered;
}

bool GlobalShortcut::isFresh() const
{
    return _isFresh;
}

bool GlobalShortcut::isPresent() const
{
    return _isPresent;
}

bool GlobalShortcut::isSessionShortcut() const
{
    return uniqueName().startsWith(QLatin1String("_k_session:"));
}

void GlobalShortcut::setIsFresh(bool value)
{
    _isFresh = value;
}

void GlobalShortcut::setIsPresent(bool value)
{
    // (de)activate depending on old/new value
    _isPresent = value;
    if (_isPresent) {
        setActive();
    } else {
        setInactive();
    }
}

GlobalShortcutContext *GlobalShortcut::context()
{
    return _context;
}

GlobalShortcutContext const *GlobalShortcut::context() const
{
    return _context;
}

QString GlobalShortcut::uniqueName() const
{
    return _uniqueName;
}

void GlobalShortcut::unRegister()
{
    return _context->component()->unregisterShortcut(uniqueName());
}

QString GlobalShortcut::friendlyName() const
{
    return _friendlyName;
}

void GlobalShortcut::setFriendlyName(const QString &name)
{
    _friendlyName = name;
}

QList<QKeySequence> GlobalShortcut::keys() const
{
    return _keys;
}

void GlobalShortcut::setKeys(const QList<QKeySequence> &newKeys)
{
    bool active = _isRegistered;
    if (active) {
        setInactive();
    }

    _keys.clear();

    auto getKey = [this](const QKeySequence &key) {
        if (key.isEmpty()) {
            qCDebug(KGLOBALACCELD) << _uniqueName << "skipping because key is empty";
            return QKeySequence{};
        }

        if (_registry->getShortcutByKey(key) //
            || _registry->getShortcutByKey(key, KGlobalAccel::MatchType::Shadowed) //
            || _registry->getShortcutByKey(key, KGlobalAccel::MatchType::Shadows) //
        ) {
            qCDebug(KGLOBALACCELD) << _uniqueName << "skipping because key" << QKeySequence(key).toString() << "is already taken";
            return QKeySequence{};
        }

        return key;
    };

    std::transform(newKeys.cbegin(), newKeys.cend(), std::back_inserter(_keys), getKey);

    if (active) {
        setActive();
    }
}

QList<QKeySequence> GlobalShortcut::defaultKeys() const
{
    return _defaultKeys;
}

void GlobalShortcut::setDefaultKeys(const QList<QKeySequence> &newKeys)
{
    _defaultKeys = newKeys;
}

void GlobalShortcut::setActive()
{
    if (!_isPresent || _isRegistered) {
        // The corresponding application is not present or the keys are
        // already grabbed
        return;
    }

    for (const QKeySequence &key : std::as_const(_keys)) {
        if (!key.isEmpty() && !_registry->registerKey(key, this)) {
            qCDebug(KGLOBALACCELD) << uniqueName() << ": Failed to register " << QKeySequence(key).toString();
        }
    }

    _isRegistered = true;
}

void GlobalShortcut::setInactive()
{
    if (!_isRegistered) {
        // The keys are not grabbed currently
        return;
    }

    for (const QKeySequence &key : std::as_const(_keys)) {
        if (!key.isEmpty() && !_registry->unregisterKey(key, this)) {
            qCDebug(KGLOBALACCELD) << uniqueName() << ": Failed to unregister " << QKeySequence(key).toString();
        }
    }

    _isRegistered = false;
}
