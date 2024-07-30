/*
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef GLOBALSHORTCUTSREGISTRY_H
#define GLOBALSHORTCUTSREGISTRY_H

#include "kglobalaccel.h"

#include "component.h"
#include "kserviceactioncomponent.h"

#include <KSharedConfig>

#include <QDBusObjectPath>
#include <QHash>
#include <QKeySequence>
#include <QObject>
#include <QTimer>

#include <chrono>

#include "kglobalaccel_export.h"

class Component;
class GlobalShortcut;
class KGlobalAccelInterface;

/**
 * Global Shortcut Registry.
 *
 * Shortcuts are registered by component. A component is for example kmail or
 * amarok.
 *
 * A component can have contexts. Currently on plasma is planned to support
 * that feature. A context enables plasma to keep track of global shortcut
 * settings when switching containments.
 *
 * A shortcut (WIN+x) can be registered by one component only. The component
 * is allowed to register it more than once in different contexts.
 *
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class KGLOBALACCEL_EXPORT GlobalShortcutsRegistry : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "org.kde.KdedGlobalAccel.GlobalShortcutsRegistry")

public:
    /**
     * Use GlobalShortcutsRegistry::self()
     *
     * @internal
     */
    GlobalShortcutsRegistry();
    ~GlobalShortcutsRegistry() override;

    /**
     * Activate all shortcuts having their application present.
     */
    void activateShortcuts();

    /**
     * Returns a list of D-Bus paths of registered Components.
     *
     * The returned paths are absolute (i.e. no need to prepend anything).
     */
    QList<QDBusObjectPath> componentsDbusPaths() const;

    /**
     * Returns a list of QStringLists (one string list per registered component,
     * with each string list containing four strings, one for each enumerator in
     * KGlobalAccel::actionIdFields).
     */
    QList<QStringList> allComponentNames() const;

    /**
     * Return the root dbus path for the registry.
     */
    QDBusObjectPath dbusPath() const;

    /**
     * Deactivate all currently active shortcuts.
     */
    void deactivateShortcuts(bool temporarily = false);

    /**
     */
    Component *getComponent(const QString &uniqueName);

    /**
     * Get the shortcut corresponding to key. Active and inactive shortcuts
     * are considered. But if the matching application uses contexts only one
     * shortcut is returned.
     *
     * @see getShortcutsByKey(int key)
     */
    GlobalShortcut *getShortcutByKey(const QKeySequence &key, KGlobalAccel::MatchType type = KGlobalAccel::MatchType::Equal) const;

    /**
     * Get the shortcuts corresponding to key. Active and inactive shortcuts
     * are considered.
     *
     * @see getShortcutsByKey(int key)
     */
    QList<GlobalShortcut *> getShortcutsByKey(const QKeySequence &key, KGlobalAccel::MatchType type) const;

    /**
     * Checks if @p shortcut is available for @p component.
     *
     * It is available if not used by another component in any context or used
     * by @p component only in not active contexts.
     */
    bool isShortcutAvailable(const QKeySequence &shortcut, const QString &component, const QString &context) const;

    static GlobalShortcutsRegistry *self();

    bool registerKey(const QKeySequence &key, GlobalShortcut *shortcut);

    void setDBusPath(const QDBusObjectPath &path);

    bool unregisterKey(const QKeySequence &key, GlobalShortcut *shortcut);

public Q_SLOTS:

    void clear();

    void loadSettings();

    void writeSettings();

    // Grab the keys
    void grabKeys();

    // Ungrab the keys
    void ungrabKeys();

private:
    friend struct KGlobalAccelDPrivate;
    friend class Component;
    friend class KGlobalAccelInterface;

    Component *createComponent(const QString &uniqueName, const QString &friendlyName);
    KServiceActionComponent *createServiceActionComponent(const QString &uniqueName);
    KServiceActionComponent *createServiceActionComponent(KService::Ptr service);
    void migrateConfig();
    void migrateKHotkeys();
    void scheduleRefreshServices();
    void refreshServices();
    void detectAppsWithShortcuts();

    static void unregisterComponent(Component *component);
    using ComponentPtr = std::unique_ptr<Component, decltype(&unregisterComponent)>;

    Component *registerComponent(ComponentPtr component);

    // called by the implementation to inform us about key presses
    // returns true if the key was handled
    bool keyPressed(int keyQt);
    bool keyReleased(int keyQt);
    bool pointerPressed(Qt::MouseButtons pointerButtons);
    bool axisTriggered(int axis);

    bool processKey(int keyQt);

    QHash<QKeySequence, GlobalShortcut *> _active_keys;
    QKeySequence _active_sequence;
    QHash<int, int> _keys_count;

    // State machine:
    // Any -> PressingModifierOnly when a modifier is pressed
    // PressingModifierOnly -> Normal when any modifier key is released; and emit the shortcut
    // PressingModifierOnly -> Normal when a non-modifier key is pressed
    enum { Normal, PressingModifierOnly } m_state = Normal;

    using ComponentVec = std::vector<ComponentPtr>;
    ComponentVec m_components;
    ComponentVec::const_iterator findByName(const QString &name) const
    {
        return std::find_if(m_components.cbegin(), m_components.cend(), [&name](const ComponentPtr &comp) {
            return comp->uniqueName() == name;
        });
    }

    KGlobalAccelInterface *_manager = nullptr;

    mutable KConfig _config;

    QDBusObjectPath _dbusPath;
    GlobalShortcut *m_lastShortcut = nullptr;
    QTimer m_refreshServicesTimer;
};

#endif /* #ifndef GLOBALSHORTCUTSREGISTRY_H */
