/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KGLOBALACCELD_H
#define KGLOBALACCELD_H

#include "kglobalacceld_export.h"

#include <kglobalshortcutinfo.h>

#include <KGlobalAccel>
#include <QDBusContext>
#include <QDBusObjectPath>
#include <QList>
#include <QStringList>

struct KGlobalAccelDPrivate;

/**
 * @note: Even though this is private API, KWin creates an object
 * of this type, check in KWin to see which methods are used before
 * removing them from here.
 *
 * @todo get rid of all of those QStringList parameters.
 */
class KGLOBALACCELD_EXPORT KGlobalAccelD : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KGlobalAccel")

public:
    enum SetShortcutFlag {
        SetPresent = 2,
        NoAutoloading = 4,
        IsDefault = 8,
    };
    Q_ENUM(SetShortcutFlag)
    Q_DECLARE_FLAGS(SetShortcutFlags, SetShortcutFlag)
    Q_FLAG(SetShortcutFlags)

    explicit KGlobalAccelD(QObject *parent = nullptr);
    ~KGlobalAccelD() override;

    bool init();

public Q_SLOTS:

    /**
     * Get the dbus path for all known components.
     *
     * The returned path is absolute. No need to prepend anything.
     */
    Q_SCRIPTABLE QList<QDBusObjectPath> allComponents() const;

    /**
     * Returns a list of QStringLists (one string list per known component,
     * with each string list containing four strings, one for each enumerator
     * in KGlobalAccel::actionIdFields).
     */
    Q_SCRIPTABLE QList<QStringList> allMainComponents() const;

    Q_SCRIPTABLE QList<QStringList> allActionsForComponent(const QStringList &actionId) const;

#if KGLOBALACCELD_ENABLE_DEPRECATED_SINCE(5, 90)
    KGLOBALACCELD_DEPRECATED_VERSION(5, 90, "Use actionList(const QKeySequence&, int) instead.")
    Q_SCRIPTABLE QStringList action(int key) const;
#endif
    Q_SCRIPTABLE QStringList actionList(const QKeySequence &key) const;

    // to be called by main components not owning the action
#if KGLOBALACCELD_ENABLE_DEPRECATED_SINCE(5, 90)
    KGLOBALACCELD_DEPRECATED_VERSION(5, 90, "Use shortcutKeys(const QStringList &) instead.")
    Q_SCRIPTABLE QList<int> shortcut(const QStringList &actionId) const;
#endif
    Q_SCRIPTABLE QList<QKeySequence> shortcutKeys(const QStringList &actionId) const;

    // to be called by main components not owning the action
#if KGLOBALACCELD_ENABLE_DEPRECATED_SINCE(5, 90)
    KGLOBALACCELD_DEPRECATED_VERSION(5, 90, "Use defaultShortcutKeys(const QStringList &) instead.")
    Q_SCRIPTABLE QList<int> defaultShortcut(const QStringList &actionId) const;
#endif
    Q_SCRIPTABLE QList<QKeySequence> defaultShortcutKeys(const QStringList &actionId) const;

    /**
     * Get the dbus path for @ componentUnique
     *
     * @param componentUnique the components unique identifier
     *
     * @return the absolute dbus path
     */
    Q_SCRIPTABLE QDBusObjectPath getComponent(const QString &componentUnique) const;

    // to be called by main components owning the action
#if KGLOBALACCELD_ENABLE_DEPRECATED_SINCE(5, 90)
    KGLOBALACCELD_DEPRECATED_VERSION(5, 90, "Use setShortcutKeys(const QStringList &, const QList<QKeySequence> &, uint) instead.")
    Q_SCRIPTABLE QList<int> setShortcut(const QStringList &actionId, const QList<int> &keys, uint flags);
#endif
    Q_SCRIPTABLE QList<QKeySequence> setShortcutKeys(const QStringList &actionId, const QList<QKeySequence> &keys, uint flags);

    // this is used if application A wants to change shortcuts of application B
#if KGLOBALACCELD_ENABLE_DEPRECATED_SINCE(5, 90)
    KGLOBALACCELD_DEPRECATED_VERSION(5, 90, "Use setForeignShortcutKeys(const QStringList &, const QList<QKeySequence> &) instead.")
    Q_SCRIPTABLE void setForeignShortcut(const QStringList &actionId, const QList<int> &keys);
#endif
    Q_SCRIPTABLE void setForeignShortcutKeys(const QStringList &actionId, const QList<QKeySequence> &keys);

    // to be called when a KAction is destroyed. The shortcut stays in the data structures for
    // conflict resolution but won't trigger.
    Q_SCRIPTABLE void setInactive(const QStringList &actionId);

    Q_SCRIPTABLE void doRegister(const QStringList &actionId);

#if KGLOBALACCELD_ENABLE_DEPRECATED_SINCE(4, 3)
    //! @deprecated Since 4.3, use KGlobalAccelD::unregister
    KGLOBALACCELD_DEPRECATED_VERSION(4, 3, "Use KGlobalAccelD::unregister(const QString&, const QString&")
    Q_SCRIPTABLE void unRegister(const QStringList &actionId);
#endif

    Q_SCRIPTABLE void activateGlobalShortcutContext(const QString &component, const QString &context);

#if KGLOBALACCELD_ENABLE_DEPRECATED_SINCE(5, 90)
    /**
     * Returns the shortcuts registered for @p key.
     *
     * If there is more than one shortcut they are guaranteed to be from the
     * same component but different contexts. All shortcuts are searched.
     *
     * @deprecated Since 5.90, use globalShortcutsByKey(const QKeySequence &, int) instead.
     */
    KGLOBALACCELD_DEPRECATED_VERSION(5, 90, "Use globalShortcutsByKey(const QKeySequence &, int) instead.")
    Q_SCRIPTABLE QList<KGlobalShortcutInfo> getGlobalShortcutsByKey(int key) const;
#endif

    /**
     * Returns the shortcuts registered for @p key.
     *
     * If there is more than one shortcut they are guaranteed to be from the
     * same component but different contexts. All shortcuts are searched.
     *
     * @since 5.90
     */
    Q_SCRIPTABLE QList<KGlobalShortcutInfo> globalShortcutsByKey(const QKeySequence &key, KGlobalAccel::MatchType type) const;

#if KGLOBALACCELD_ENABLE_DEPRECATED_SINCE(5, 90)
    /**
     * Returns true if the @p shortcut is available for @p component.
     *
     * @deprecated Since 5.90, use globalShortcutAvailable(const QKeySequence &, const QString &) instead.
     */
    KGLOBALACCELD_DEPRECATED_VERSION(5, 90, "Use globalShortcutAvailable(const QKeySequence &, const QString &) instead.")
    Q_SCRIPTABLE bool isGlobalShortcutAvailable(int key, const QString &component) const;
#endif

    /**
     * Returns true if the @p shortcut is available for @p component.
     *
     * @since 5.90
     */
    Q_SCRIPTABLE bool globalShortcutAvailable(const QKeySequence &key, const QString &component) const;

    /**
     * Delete the shortcut with @a component and @name.
     *
     * The shortcut is removed from the registry even if it is currently
     * present. It is removed from all contexts.
     *
     * @param componentUnique the components unique identifier
     * @param shortcutUnique the shortcut id
     *
     * @return @c true if the shortcuts was deleted, @c false if it didn't * exist.
     */
    Q_SCRIPTABLE bool unregister(const QString &componentUnique, const QString &shortcutUnique);

    Q_SCRIPTABLE void blockGlobalShortcuts(bool);

Q_SIGNALS:
#if KGLOBALACCELD_ENABLE_DEPRECATED_SINCE(5, 90)
    KGLOBALACCELD_DEPRECATED_VERSION(5, 90, "Use the yourShortcutsChanged(const QStringList &, const QList<QKeySequence> &) signal instead.")
    Q_SCRIPTABLE void yourShortcutGotChanged(const QStringList &actionId, const QList<int> &newKeys);
#endif

    Q_SCRIPTABLE void yourShortcutsChanged(const QStringList &actionId, const QList<QKeySequence> &newKeys);

private:
    void scheduleWriteSettings() const;

    KGlobalAccelDPrivate *const d;
};

#endif // KGLOBALACCELD_H
