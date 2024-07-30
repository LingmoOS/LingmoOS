/*
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "component.h"

#include "globalshortcutcontext.h"
#include "globalshortcutsregistry.h"
#include "kglobalaccel_interface.h"
#include "logging_p.h"
#include <config-kglobalaccel.h>

#include <QKeySequence>
#include <QStringList>

#if HAVE_X11
#include <private/qtx11extras_p.h>
#endif

QList<QKeySequence> Component::keysFromString(const QString &str)
{
    QList<QKeySequence> ret;
    if (str == QLatin1String("none")) {
        return ret;
    }
    const QStringList strList = str.split(QLatin1Char('\t'));
    for (const QString &s : strList) {
        QKeySequence key = QKeySequence::fromString(s, QKeySequence::PortableText);
        if (!key.isEmpty()) { // sanity check just in case
            ret.append(key);
        }
    }
    return ret;
}

QString Component::stringFromKeys(const QList<QKeySequence> &keys)
{
    if (keys.isEmpty()) {
        return QStringLiteral("none");
    }
    QString ret;
    for (const QKeySequence &key : keys) {
        ret.append(key.toString(QKeySequence::PortableText));
        ret.append(QLatin1Char('\t'));
    }
    ret.chop(1);
    return ret;
}

Component::Component(const QString &uniqueName, const QString &friendlyName)
    : _uniqueName(uniqueName)
    , _friendlyName(friendlyName)
    , _registry(GlobalShortcutsRegistry::self())
{
    // Make sure we do no get uniquenames still containing the context
    Q_ASSERT(uniqueName.indexOf(QLatin1Char('|')) == -1);

    const QString DEFAULT(QStringLiteral("default"));
    createGlobalShortcutContext(DEFAULT, QStringLiteral("Default Context"));
    _current = _contexts.value(DEFAULT);
}

Component::~Component()
{
    // We delete all shortcuts from all contexts
    qDeleteAll(_contexts);
}

bool Component::activateGlobalShortcutContext(const QString &uniqueName)
{
    if (!_contexts.value(uniqueName)) {
        createGlobalShortcutContext(uniqueName, QStringLiteral("TODO4"));
        return false;
    }

    // Deactivate the current contexts shortcuts
    deactivateShortcuts();

    // Switch the context
    _current = _contexts.value(uniqueName);

    return true;
}

void Component::activateShortcuts()
{
    for (GlobalShortcut *shortcut : std::as_const(_current->_actionsMap)) {
        shortcut->setActive();
    }
}

QList<GlobalShortcut *> Component::allShortcuts(const QString &contextName) const
{
    GlobalShortcutContext *context = _contexts.value(contextName);
    return context ? context->_actionsMap.values() : QList<GlobalShortcut *>{};
}

QList<KGlobalShortcutInfo> Component::allShortcutInfos(const QString &contextName) const
{
    GlobalShortcutContext *context = _contexts.value(contextName);
    return context ? context->allShortcutInfos() : QList<KGlobalShortcutInfo>{};
}

bool Component::cleanUp()
{
    bool changed = false;

    const auto actions = _current->_actionsMap;
    for (GlobalShortcut *shortcut : actions) {
        qCDebug(KGLOBALACCELD) << _current->_actionsMap.size();
        if (!shortcut->isPresent()) {
            changed = true;
            shortcut->unRegister();
        }
    }

    if (changed) {
        _registry->writeSettings();
        // We could be destroyed after this call!
    }

    return changed;
}

bool Component::createGlobalShortcutContext(const QString &uniqueName, const QString &friendlyName)
{
    if (_contexts.value(uniqueName)) {
        qCDebug(KGLOBALACCELD) << "Shortcut Context " << uniqueName << "already exists for component " << _uniqueName;
        return false;
    }
    _contexts.insert(uniqueName, new GlobalShortcutContext(uniqueName, friendlyName, this));
    return true;
}

GlobalShortcutContext *Component::currentContext()
{
    return _current;
}

QDBusObjectPath Component::dbusPath() const
{
    auto isNonAscii = [](QChar ch) {
        const char c = ch.unicode();
        const bool isAscii = c == '_' //
            || (c >= 'A' && c <= 'Z') //
            || (c >= 'a' && c <= 'z') //
            || (c >= '0' && c <= '9');
        return !isAscii;
    };

    QString dbusPath = _uniqueName;
    // DBus path can only contain ASCII characters, any non-alphanumeric char should
    // be turned into '_'
    std::replace_if(dbusPath.begin(), dbusPath.end(), isNonAscii, QLatin1Char('_'));

    // QDBusObjectPath could be a little bit easier to handle :-)
    return QDBusObjectPath(_registry->dbusPath().path() + QLatin1String("component/") + dbusPath);
}

void Component::deactivateShortcuts(bool temporarily)
{
    for (GlobalShortcut *shortcut : std::as_const(_current->_actionsMap)) {
        if (temporarily //
            && _uniqueName == QLatin1String("kwin") //
            && shortcut->uniqueName() == QLatin1String("Block Global Shortcuts")) {
            continue;
        }
        shortcut->setInactive();
    }
}

void Component::emitGlobalShortcutPressed(const GlobalShortcut &shortcut)
{
#if HAVE_X11
    // pass X11 timestamp
    const long timestamp = QX11Info::appTime();
#else
    const long timestamp = 0;
#endif

    if (shortcut.context()->component() != this) {
        return;
    }

    Q_EMIT globalShortcutPressed(shortcut.context()->component()->uniqueName(), shortcut.uniqueName(), timestamp);
}

void Component::emitGlobalShortcutReleased(const GlobalShortcut &shortcut)
{
#if HAVE_X11
    // pass X11 timestamp
    const long timestamp = QX11Info::appTime();
#else
    const long timestamp = 0;
#endif

    if (shortcut.context()->component() != this) {
        return;
    }

    Q_EMIT globalShortcutReleased(shortcut.context()->component()->uniqueName(), shortcut.uniqueName(), timestamp);
}

void Component::invokeShortcut(const QString &shortcutName, const QString &context)
{
    GlobalShortcut *shortcut = getShortcutByName(shortcutName, context);
    if (shortcut) {
        emitGlobalShortcutPressed(*shortcut);
    }
}

QString Component::friendlyName() const
{
    return !_friendlyName.isEmpty() ? _friendlyName : _uniqueName;
}

GlobalShortcut *Component::getShortcutByKey(const QKeySequence &key, KGlobalAccel::MatchType type) const
{
    return _current->getShortcutByKey(key, type);
}

QList<GlobalShortcut *> Component::getShortcutsByKey(const QKeySequence &key, KGlobalAccel::MatchType type) const
{
    QList<GlobalShortcut *> rc;
    for (GlobalShortcutContext *context : std::as_const(_contexts)) {
        GlobalShortcut *sc = context->getShortcutByKey(key, type);
        if (sc) {
            rc.append(sc);
        }
    }
    return rc;
}

GlobalShortcut *Component::getShortcutByName(const QString &uniqueName, const QString &context) const
{
    const GlobalShortcutContext *shortcutContext = _contexts.value(context);
    return shortcutContext ? shortcutContext->_actionsMap.value(uniqueName) : nullptr;
}

QStringList Component::getShortcutContexts() const
{
    return _contexts.keys();
}

bool Component::isActive() const
{
    // The component is active if at least one of it's global shortcuts is
    // present.
    for (GlobalShortcut *shortcut : std::as_const(_current->_actionsMap)) {
        if (shortcut->isPresent()) {
            return true;
        }
    }
    return false;
}

bool Component::isShortcutAvailable(const QKeySequence &key, const QString &component, const QString &context) const
{
    qCDebug(KGLOBALACCELD) << key.toString() << component;

    // if this component asks for the key. only check the keys in the same
    // context
    if (component == uniqueName()) {
        return shortcutContext(context)->isShortcutAvailable(key);
    } else {
        for (auto it = _contexts.cbegin(), endIt = _contexts.cend(); it != endIt; ++it) {
            const GlobalShortcutContext *ctx = it.value();
            if (!ctx->isShortcutAvailable(key)) {
                return false;
            }
        }
    }
    return true;
}

GlobalShortcut *
Component::registerShortcut(const QString &uniqueName, const QString &friendlyName, const QString &shortcutString, const QString &defaultShortcutString)
{
    // The shortcut will register itself with us
    GlobalShortcut *shortcut = new GlobalShortcut(uniqueName, friendlyName, currentContext());

    const QList<QKeySequence> keys = keysFromString(shortcutString);
    shortcut->setDefaultKeys(keysFromString(defaultShortcutString));
    shortcut->setIsFresh(false);
    QList<QKeySequence> newKeys = keys;
    for (const QKeySequence &key : keys) {
        if (!key.isEmpty()) {
            if (_registry->getShortcutByKey(key)) {
                // The shortcut is already used. The config file is
                // broken. Ignore the request.
                newKeys.removeAll(key);
                qCWarning(KGLOBALACCELD) << "Shortcut found twice in kglobalshortcutsrc." << key;
            }
        }
    }
    shortcut->setKeys(keys);
    return shortcut;
}

void Component::loadSettings(KConfigGroup &configGroup)
{
    // GlobalShortcutsRegistry::loadSettings handles contexts.
    const auto listKeys = configGroup.keyList();
    for (const QString &confKey : listKeys) {
        const QStringList entry = configGroup.readEntry(confKey, QStringList());
        if (entry.size() != 3) {
            continue;
        }

        registerShortcut(confKey, entry[2], entry[0], entry[1]);
    }
}

void Component::setFriendlyName(const QString &name)
{
    _friendlyName = name;
}

GlobalShortcutContext *Component::shortcutContext(const QString &contextName)
{
    return _contexts.value(contextName);
}

GlobalShortcutContext const *Component::shortcutContext(const QString &contextName) const
{
    return _contexts.value(contextName);
}

QStringList Component::shortcutNames(const QString &contextName) const
{
    const GlobalShortcutContext *context = _contexts.value(contextName);
    return context ? context->_actionsMap.keys() : QStringList{};
}

QString Component::uniqueName() const
{
    return _uniqueName;
}

void Component::unregisterShortcut(const QString &uniqueName)
{
    // Now wrote all contexts
    for (GlobalShortcutContext *context : std::as_const(_contexts)) {
        if (context->_actionsMap.value(uniqueName)) {
            delete context->takeShortcut(context->_actionsMap.value(uniqueName));
        }
    }
}

void Component::writeSettings(KConfigGroup &configGroup) const
{
    // If we don't delete the current content global shortcut
    // registrations will never not deleted after forgetGlobalShortcut()
    configGroup.deleteGroup();

    // Now write all contexts
    for (GlobalShortcutContext *context : std::as_const(_contexts)) {
        KConfigGroup contextGroup;

        if (context->uniqueName() == QLatin1String("default")) {
            contextGroup = configGroup;
            // Write the friendly name
            contextGroup.writeEntry("_k_friendly_name", friendlyName());
        } else {
            contextGroup = KConfigGroup(&configGroup, context->uniqueName());
            // Write the friendly name
            contextGroup.writeEntry("_k_friendly_name", context->friendlyName());
        }

        // qCDebug(KGLOBALACCELD) << "writing group " << _uniqueName << ":" << context->uniqueName();

        for (const GlobalShortcut *shortcut : std::as_const(context->_actionsMap)) {
            // qCDebug(KGLOBALACCELD) << "writing" << shortcut->uniqueName();

            // We do not write fresh shortcuts.
            // We do not write session shortcuts
            if (shortcut->isFresh() || shortcut->isSessionShortcut()) {
                continue;
            }
            // qCDebug(KGLOBALACCELD) << "really writing" << shortcut->uniqueName();

            QStringList entry(stringFromKeys(shortcut->keys()));
            entry.append(stringFromKeys(shortcut->defaultKeys()));
            entry.append(shortcut->friendlyName());

            contextGroup.writeEntry(shortcut->uniqueName(), entry);
        }
    }
}
