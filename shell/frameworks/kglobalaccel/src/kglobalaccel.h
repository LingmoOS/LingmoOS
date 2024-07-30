/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2001, 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KGLOBALACCEL_H_
#define _KGLOBALACCEL_H_

#include "kglobalshortcutinfo.h"
#include <kglobalaccel_export.h>

#include <QKeySequence>
#include <QList>
#include <QObject>

class QAction;
class OrgKdeKglobalaccelComponentInterface;

/**
 * @short Configurable global shortcut support
 *
 * KGlobalAccel allows you to have global accelerators that are independent of
 * the focused window.  Unlike regular shortcuts, the application's window does not need focus
 * for them to be activated.
 *
 * @see KKeyChooser
 * @see KKeyDialog
 */
class KGLOBALACCEL_EXPORT KGlobalAccel : public QObject
{
    Q_OBJECT

public:
    /**
     * An enum about global shortcut setter semantics
     */
    enum GlobalShortcutLoading {
        /// Look up the action in global settings (using its main component's name and text())
        /// and set the shortcut as saved there.
        /// @see setGlobalShortcut()
        Autoloading = 0x0,
        /// Prevent autoloading of saved global shortcut for action
        NoAutoloading = 0x4,
    };

    /**
     * Index for actionId QStringLists
     */
    enum actionIdFields {
        ComponentUnique = 0, //!< Components Unique Name (ID)
        ActionUnique = 1, //!< Actions Unique Name(ID)
        ComponentFriendly = 2, //!< Components Friendly Translated Name
        ActionFriendly = 3, //!< Actions Friendly Translated Name
    };

    /**
     * Keysequence match semantics.
     *
     * Assuming we have an Emacs-style shortcut, for example (Alt+B, Alt+F, Alt+G) already assigned,
     * how a new shortcut is compared depends on which value of the enum is used:
     * @c Equal : Exact matching: (Alt+B, Alt+F, Alt+G)
     * @c Shadows : Sequence shadowing: (Alt+B, Alt+F), (Alt+F, Alt+G)
     * @c Shadowed : Sequence being shadowed: (Alt+B, Alt+F, Alt+G, <any key>), (<any key>, Alt+B, Alt+F, Alt+G)
     *
     * @since 5.90
     */
    enum MatchType {
        Equal,
        Shadows,
        Shadowed,
    };
    Q_ENUM(MatchType)

    /**
     * Returns (and creates if necessary) the singleton instance
     */
    static KGlobalAccel *self();

    /**
     * Take away the given shortcut from the named action it belongs to.
     * This applies to all actions with global shortcuts in any KDE application.
     *
     * @see promptStealShortcutSystemwide()
     */
    static void stealShortcutSystemwide(const QKeySequence &seq);

    /**
     * Clean the shortcuts for component @a componentUnique.
     *
     * If the component is not active all global shortcut registrations are
     * purged and the component is removed completely.
     *
     * If the component is active all global shortcut registrations not in use
     * will be purged. If there is no shortcut registration left the component
     * is purged too.
     *
     * If a purged component or shortcut is activated the next time it will
     * reregister itself. All you probably will lose on wrong usage are the
     * user's set shortcuts.
     *
     * If you make sure your component is running and all global shortcuts it
     * has are active this function can be used to clean up the registry.
     *
     * Handle with care!
     *
     * If the method return @c true at least one shortcut was purged so handle
     * all previously acquired information with care.
     */
    static bool cleanComponent(const QString &componentUnique);

    /**
     * Check if @a component is active.
     *
     * @param componentUnique the components unique identifier
     * @return @c true if active, @false if not
     */
    static bool isComponentActive(const QString &componentName);

    /**
     * Returns a list of global shortcuts registered for the shortcut @p seq.
     *
     * If the list contains more that one entry it means the component
     * that registered the shortcuts uses global shortcut contexts. All
     * returned shortcuts belong to the same component.
     *
     * @param type a value from the KGlobalAccel::MatchType enum
     *
     * @since 5.90
     */
    static QList<KGlobalShortcutInfo> globalShortcutsByKey(const QKeySequence &seq, MatchType type = Equal);

    /**
     * Check if the shortcut @seq is available for the @p component. The
     * component is only of interest if the current application uses global shortcut
     * contexts. In that case a global shortcut by @p component in an inactive
     * global shortcut contexts does not block the @p seq for us.
     *
     * @since 4.2
     */
    static bool isGlobalShortcutAvailable(const QKeySequence &seq, const QString &component = QString());

    /**
     * Show a messagebox to inform the user that a global shortcut is already occupied,
     * and ask to take it away from its current action(s). This is GUI only, so nothing will
     * be actually changed.
     *
     * @see stealShortcutSystemwide()
     *
     * @since 4.2
     */
    static bool promptStealShortcutSystemwide(QWidget *parent, const QList<KGlobalShortcutInfo> &shortcuts, const QKeySequence &seq);

    /**
     * Assign a default global shortcut for a given QAction.
     * For more information about global shortcuts @see setShortcut
     * Upon shortcut change the globalShortcutChanged will be triggered so other applications get notified
     *
     * @sa globalShortcutChanged
     *
     * @since 5.0
     */
    bool setDefaultShortcut(QAction *action, const QList<QKeySequence> &shortcut, GlobalShortcutLoading loadFlag = Autoloading);

    /**
     * Assign a global shortcut for the given action. Global shortcuts
     * allow an action to respond to key shortcuts independently of the focused window,
     * i.e. the action will trigger if the keys were pressed no matter where in the X session.
     *
     * The action must have a per main component unique
     * action->objectName() to enable cross-application bookkeeping. If the action->objectName() is empty this method will
     * do nothing and will return false.
     *
     * It is mandatory that the action->objectName() doesn't change once the shortcut has been successfully registered.
     *
     * \note KActionCollection::insert(name, action) will set action's objectName to name so you often
     * don't have to set an objectName explicitly.
     *
     * When an action, identified by main component name and objectName(), is assigned
     * a global shortcut for the first time on a KDE installation the assignment will
     * be saved. The shortcut will then be restored every time setGlobalShortcut() is
     * called with @p loading == Autoloading.
     *
     * If you actually want to change the global shortcut you have to set
     * @p loading to NoAutoloading. The new shortcut will be automatically saved again.
     *
     * @param action the action for which the shortcut will be assigned
     * @param shortcut global shortcut(s) to assign. Will be ignored unless @p loading is set to NoAutoloading or this is the first time ever you call this
     * method (see above).
     * @param loadFlag if Autoloading, assign the global shortcut this action has previously had if any.
     *                   That way user preferences and changes made to avoid clashes will be conserved.
     *                if NoAutoloading the given shortcut will be assigned without looking up old values.
     *                   You should only do this if the user wants to change the shortcut or if you have
     *                   another very good reason. Key combinations that clash with other shortcuts will be
     *                   dropped.
     *
     * \note the default shortcut will never be influenced by autoloading - it will be set as given.
     * @sa shortcut()
     * @sa globalShortcutChanged
     * @since 5.0
     */
    bool setShortcut(QAction *action, const QList<QKeySequence> &shortcut, GlobalShortcutLoading loadFlag = Autoloading);

    /**
     * Convenient method to set both active and default shortcut.
     *
     * If more control for loading the shortcuts is needed use the variants offering more control.
     *
     * @sa setShortcut
     * @sa setDefaultShortcut
     * @since 5.0
     **/
    static bool setGlobalShortcut(QAction *action, const QList<QKeySequence> &shortcut);

    /**
     * Convenient method to set both active and default shortcut.
     *
     * This method is suited for the case that only one shortcut is to be configured.
     *
     * If more control for loading the shortcuts is needed use the variants offering more control.
     *
     * @sa setShortcut
     * @sa setDefaultShortcut
     * @since 5.0
     **/
    static bool setGlobalShortcut(QAction *action, const QKeySequence &shortcut);

    /**
     * Get the global default shortcut for this action, if one exists. Global shortcuts
     * allow your actions to respond to accellerators independently of the focused window.
     * Unlike regular shortcuts, the application's window does not need focus
     * for them to be activated.
     *
     * @sa setDefaultShortcut()
     * @since 5.0
     */
    QList<QKeySequence> defaultShortcut(const QAction *action) const;

    /**
     * Get the global shortcut for this action, if one exists. Global shortcuts
     * allow your actions to respond to accellerators independently of the focused window.
     * Unlike regular shortcuts, the application's window does not need focus
     * for them to be activated.
     *
     * @note that this method only works together with setShortcut() because the action pointer
     * is used to retrieve the result. If you would like to retrieve the shortcut as stored
     * in the global settings, use the globalShortcut(componentName, actionId) instead.
     *
     * @sa setShortcut()
     * @since 5.0
     */
    QList<QKeySequence> shortcut(const QAction *action) const;

    /**
     * Retrieves the shortcut as defined in global settings by
     * componentName (e.g. "kwin") and actionId (e.g. "Kill Window").
     *
     * @since 5.10
     */
    QList<QKeySequence> globalShortcut(const QString &componentName, const QString &actionId) const;

    /**
     * Unregister and remove all defined global shortcuts for the given action.
     *
     * @since 5.0
     */
    void removeAllShortcuts(QAction *action);

    /**
     * Returns true if a shortcut or a default shortcut has been registered for the given action
     *
     * @since 5.0
     */
    bool hasShortcut(const QAction *action) const;

Q_SIGNALS:
    /**
     * Emitted when the global shortcut is changed. A global shortcut is
     * subject to be changed by the global shortcuts kcm.
     *
     * @param action pointer to the action for which the changed shortcut was registered
     * @param seq the key sequence that corresponds to the changed shortcut
     *
     * @see setGlobalShortcut
     * @see setDefaultShortcut
     * @since 5.0
     */
    void globalShortcutChanged(QAction *action, const QKeySequence &seq);
    void globalShortcutActiveChanged(QAction *action, bool active);

private:
    /// Creates a new KGlobalAccel object
    KGLOBALACCEL_NO_EXPORT KGlobalAccel();

    /// Destructor
    KGLOBALACCEL_NO_EXPORT ~KGlobalAccel() override;

    //! get component @p componentUnique
    KGLOBALACCEL_NO_EXPORT OrgKdeKglobalaccelComponentInterface *getComponent(const QString &componentUnique);

    class KGlobalAccelPrivate *const d;

    friend class KGlobalAccelSingleton;
};

KGLOBALACCEL_EXPORT QDBusArgument &operator<<(QDBusArgument &argument, const KGlobalAccel::MatchType &type);
KGLOBALACCEL_EXPORT const QDBusArgument &operator>>(const QDBusArgument &argument, KGlobalAccel::MatchType &type);

#endif // _KGLOBALACCEL_H_
