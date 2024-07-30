/*
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef COMPONENT_H
#define COMPONENT_H

#include "globalshortcut.h"
#include "kglobalshortcutinfo.h"

#include "kconfiggroup.h"

#include <KGlobalAccel>
#include <QHash>
#include <QObject>

class GlobalShortcut;
class GlobalShortcutContext;
class GlobalShortcutsRegistry;

/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class Component : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "org.kde.kglobalaccel.Component")

    /* clang-format off */
    Q_SCRIPTABLE Q_PROPERTY(QString friendlyName READ friendlyName)
    Q_SCRIPTABLE Q_PROPERTY(QString uniqueName READ uniqueName)


public:


    ~Component() override;
    /* clang-format on */

    bool activateGlobalShortcutContext(const QString &uniqueName);

    void activateShortcuts();

    //! Returns all shortcuts in context @context
    QList<GlobalShortcut *> allShortcuts(const QString &context = QStringLiteral("default")) const;

    //! Creates the new global shortcut context @p context
    bool createGlobalShortcutContext(const QString &context, const QString &friendlyName = QString());

    //! Return the current context
    GlobalShortcutContext *currentContext();

    //! Return uniqueName converted to a valid dbus path
    QDBusObjectPath dbusPath() const;

    //! Deactivate all currently active shortcuts
    void deactivateShortcuts(bool temporarily = false);

    //! Returns the friendly name
    QString friendlyName() const;

    //! Returns the currently active shortcut for key
    GlobalShortcut *getShortcutByKey(const QKeySequence &key, KGlobalAccel::MatchType type) const;

    //! Returns the shortcut context @p name or nullptr
    GlobalShortcutContext *shortcutContext(const QString &name);
    GlobalShortcutContext const *shortcutContext(const QString &name) const;

    /**
     * Returns the list of shortcuts (different context) registered with @p
     * key.
     */
    QList<GlobalShortcut *> getShortcutsByKey(const QKeySequence &key, KGlobalAccel::MatchType type) const;

    //! Returns the shortcut by unique name. Only the active context is
    //! searched.
    GlobalShortcut *getShortcutByName(const QString &uniqueName, const QString &context = QStringLiteral("default")) const;

    /**
     * Check if @a key is available for component @p component
     */
    bool isShortcutAvailable(const QKeySequence &key, const QString &component, const QString &context) const;

    //! Load the settings from config group @p config
    virtual void loadSettings(KConfigGroup &config);

    //! Sets the human readable name for this component.
    void setFriendlyName(const QString &);

    QString uniqueName() const;

    //! Unregister @a shortcut. This will remove its siblings from all contexts
    void unregisterShortcut(const QString &uniqueName);

    virtual void writeSettings(KConfigGroup &config) const;

protected:
    friend class ::GlobalShortcutsRegistry;

    //! Constructs a component. This is a private constructor, to create a component
    //! use GlobalShortcutsRegistry::self()->createComponent().
    Component(const QString &uniqueName, const QString &friendlyName);

    /**
     * Create a new globalShortcut by its name
     * @param uniqueName internal unique name to identify the shortcut
     * @param friendlyName name for the shortcut to be presented to the user
     * @param shortcutString string representation of the shortcut, such as "CTRL+S"
     * @param defaultShortcutString string representation of the default shortcut,
     *                   such as "CTRL+S", when the user choses to reset to default
     *                   the keyboard shortcut will return to this one.
     */
    GlobalShortcut *
    registerShortcut(const QString &uniqueName, const QString &friendlyName, const QString &shortcutString, const QString &defaultShortcutString);

    static QString stringFromKeys(const QList<QKeySequence> &keys);
    static QList<QKeySequence> keysFromString(const QString &str);

public Q_SLOTS:

    // For dbus Q_SCRIPTABLE has to be on slots. Scriptable methods are not
    // exported.

    /**
     * Remove all currently not used global shortcuts registrations for this
     * component and if nothing is left the component too.
     *
     * If the method returns true consider all information previously acquired
     * from this component as void.
     *
     * The method will cleanup in all contexts.
     *
     * @return @c true if a change was made, @c false if not.
     */
    Q_SCRIPTABLE virtual bool cleanUp();

    /**
     * Check if the component is currently active.
     *
     * A component is active if at least one of it's global shortcuts is
     * currently present.
     */
    Q_SCRIPTABLE bool isActive() const;

    //! Get all shortcutnames living in @a context
    Q_SCRIPTABLE QStringList shortcutNames(const QString &context = QStringLiteral("default")) const;

    //! Returns all shortcut in @a context
    Q_SCRIPTABLE QList<KGlobalShortcutInfo> allShortcutInfos(const QString &context = QStringLiteral("default")) const;

    //! Returns the shortcut contexts available for the component.
    Q_SCRIPTABLE QStringList getShortcutContexts() const;

    virtual void emitGlobalShortcutPressed(const GlobalShortcut &shortcut);
    virtual void emitGlobalShortcutReleased(const GlobalShortcut &shortcut);

    Q_SCRIPTABLE void invokeShortcut(const QString &shortcutName, const QString &context = QStringLiteral("default"));

Q_SIGNALS:

    //! Signals that a action for this component was triggered
    Q_SCRIPTABLE void globalShortcutPressed(const QString &componentUnique, const QString &shortcutUnique, qlonglong timestamp);

    //! Signals that a action for this component is not triggered anymore
    Q_SCRIPTABLE void globalShortcutReleased(const QString &componentUnique, const QString &shortcutUnique, qlonglong timestamp);

protected:
    QString _uniqueName;
    // the name as it would be found in a magazine article about the application,
    // possibly localized if a localized name exists.
    QString _friendlyName;

    GlobalShortcutsRegistry *_registry;

    GlobalShortcutContext *_current;
    QHash<QString, GlobalShortcutContext *> _contexts;
};

#endif /* #ifndef COMPONENT_H */
