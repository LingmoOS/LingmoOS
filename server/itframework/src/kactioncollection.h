/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 Nicolas Hadacek <haadcek@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2000 Michael Koch <koch@kde.org>
    SPDX-FileCopyrightText: 2001 Holger Freyther <freyther@kde.org>
    SPDX-FileCopyrightText: 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KACTIONCOLLECTION_H
#define KACTIONCOLLECTION_H

#include <KStandardAction>
#include <kxmlgui_export.h>

#include <QAction>
#include <QObject>
#include <memory>

class KXMLGUIClient;
class KConfigGroup;
class QActionGroup;
class QString;

/**
 * @class KActionCollection kactioncollection.h KActionCollection
 *
 * \short A container for a set of QAction objects.
 *
 * KActionCollection manages a set of QAction objects.  It
 * allows them to be grouped for organized presentation of configuration to the user,
 * saving + loading of configuration, and optionally for automatic plugging into
 * specified widget(s).
 *
 * Additionally, KActionCollection provides several convenience functions for locating
 * named actions, and actions grouped by QActionGroup.
 *
 * \note If you create your own action collection and need to assign shortcuts
 * to the actions within, you have to call associateWidget() or
 * addAssociatedWidget() to have them working.
 */
class KXMLGUI_EXPORT KActionCollection : public QObject
{
    friend class KXMLGUIClient;

    Q_OBJECT

    Q_PROPERTY(QString configGroup READ configGroup WRITE setConfigGroup)
    Q_PROPERTY(bool configIsGlobal READ configIsGlobal WRITE setConfigGlobal)

public:
    /**
     * Constructor.  Allows specification of a component name other than the default
     * application name, where needed (remember to call setComponentDisplayName() too).
     */
    explicit KActionCollection(QObject *parent, const QString &cName = QString());

    /**
     * Destructor.
     */
    ~KActionCollection() override;

    /**
     * Access the list of all action collections in existence for this app
     */
    static const QList<KActionCollection *> &allCollections();

    /**
     * Clears the entire action collection, deleting all actions.
     */
    void clear();

    /**
     * Associate all actions in this collection to the given @p widget.
     * Unlike addAssociatedWidget(), this method only adds all current actions
     * in the collection to the given widget. Any action added after this call
     * will not be added to the given widget automatically.
     * So this is just a shortcut for a foreach loop and a widget->addAction call.
     */
    void associateWidget(QWidget *widget) const;

    /**
     * Associate all actions in this collection to the given @p widget, including any actions
     * added after this association is made.
     *
     * This does not change the action's shortcut context, so if you need to have the actions only
     * trigger when the widget has focus, you'll need to set the shortcut context on each action
     * to Qt::WidgetShortcut (or better still, Qt::WidgetWithChildrenShortcut with Qt 4.4+)
     */
    void addAssociatedWidget(QWidget *widget);

    /**
     * Remove an association between all actions in this collection and the given @p widget, i.e.
     * remove those actions from the widget, and stop associating newly added actions as well.
     */
    void removeAssociatedWidget(QWidget *widget);

    /**
     * Return a list of all associated widgets.
     */
    QList<QWidget *> associatedWidgets() const;

    /**
     * Clear all associated widgets and remove the actions from those widgets.
     */
    void clearAssociatedWidgets();

    /**
     * Returns the KConfig group with which settings will be loaded and saved.
     */
    QString configGroup() const;

    /**
     * Returns whether this action collection's configuration should be global to KDE ( @c true ),
     * or specific to the application ( @c false ).
     */
    bool configIsGlobal() const;

    /**
     * Sets @p group as the KConfig group with which settings will be loaded and saved.
     */
    void setConfigGroup(const QString &group);

    /**
     * Set whether this action collection's configuration should be global to KDE ( @c true ),
     * or specific to the application ( @c false ).
     */
    void setConfigGlobal(bool global);

    /**
     * Read all key associations from @p config.
     *
     * If @p config is zero, read all key associations from the
     * application's configuration file KSharedConfig::openConfig(),
     * in the group set by setConfigGroup().
     */
    void readSettings(KConfigGroup *config = nullptr);

    /**
     * Import from @p config all configurable global key associations.
     *
     * \since 4.1
     *
     * \param config Config object to read from
     */
    void importGlobalShortcuts(KConfigGroup *config);

    /**
     * Export the current configurable global key associations to @p config.
     *
     * \since 4.1
     *
     * \param config Config object to save to
     * \param writeDefaults set to true to write settings which are already at defaults.
     */
    void exportGlobalShortcuts(KConfigGroup *config, bool writeDefaults = false) const;

    /**
     * Write the current configurable key associations to @p config. What the
     * function does if @p config is zero depends. If this action collection
     * belongs to a KXMLGUIClient the setting are saved to the kxmlgui
     * definition file. If not the settings are written to the applications
     * config file.
     *
     * \note @p oneAction and @p writeDefaults have no meaning for the kxmlgui
     * configuration file.
     *
     * \param config Config object to save to, or null (see above)
     * \param writeDefaults set to true to write settings which are already at defaults.
     * \param oneAction pass an action here if you just want to save the values for one action, eg.
     *                  if you know that action is the only one which has changed.
     */
    void writeSettings(KConfigGroup *config = nullptr, bool writeDefaults = false, QAction *oneAction = nullptr) const;

    /**
     * Returns the number of actions in the collection.
     *
     * This is equivalent to actions().count().
     */
    int count() const;

    /**
     * Returns whether the action collection is empty or not.
     */
    bool isEmpty() const;

    /**
     * Return the QAction* at position @p index in the action collection.
     *
     * This is equivalent to actions().value(index);
     */
    QAction *action(int index) const;

    /**
     * Get the action with the given \p name from the action collection.
     *
     * This won't return the action for the menus defined using a "<Menu>" tag
     * in XMLGUI files (e.g. "<Menu name="menuId">" in "applicationNameui.rc").
     * To access menu actions defined like this, use e.g.
     * \code
     * qobject_cast<QMenu *>(guiFactory()->container("menuId", this));
     * \endcode
     * after having called setupGUI() or createGUI().
     *
     * @param name Name of the QAction
     * @return A pointer to the QAction in the collection which matches the parameters or
     * null if nothing matches.
     */
    QAction *action(const QString &name) const;

    /**
     * Returns the list of QActions which belong to this action collection.
     *
     * The list is guaranteed to be in the same order the action were put into
     * the collection.
     */
    QList<QAction *> actions() const;

    /**
     * Returns the list of QActions without an QAction::actionGroup() which belong to this action collection.
     */
    const QList<QAction *> actionsWithoutGroup() const;

    /**
     * Returns the list of all QActionGroups associated with actions in this action collection.
     */
    const QList<QActionGroup *> actionGroups() const;

    /**
     * Set the @p componentName associated with this action collection.
     *
     * \warning Don't call this method on a KActionCollection that contains
     * actions. This is not supported.
     *
     * \param componentData the name which is to be associated with this action collection,
     * or QString() to indicate the app name. This is used to load/save settings into XML files.
     * KXMLGUIClient::setComponentName takes care of calling this.
     */
    void setComponentName(const QString &componentName);

    /** The component name with which this class is associated. */
    QString componentName() const;

    /**
     * Set the component display name associated with this action collection.
     * (e.g. for the toolbar editor)
     * KXMLGUIClient::setComponentName takes care of calling this.
     */
    void setComponentDisplayName(const QString &displayName);

    /** The display name for the associated component. */
    QString componentDisplayName() const;

    /**
     * The parent KXMLGUIClient, or null if not available.
     */
    const KXMLGUIClient *parentGUIClient() const;

Q_SIGNALS:
    /**
     * Indicates that @p action was inserted into this action collection.
     */
    void inserted(QAction *action);

#if KXMLGUI_ENABLE_DEPRECATED_SINCE(5, 0)
    /**
     * Indicates that @p action was removed from this action collection.
     * @deprecated Since 5.0, use changed() (added in 5.66) instead.
     */
    KXMLGUI_DEPRECATED_VERSION(5, 0, "removed() is sometimes emitted with partially destroyed objects; use changed() instead (added in 5.66)")
    QT_MOC_COMPAT void removed(QAction *action);
#endif

    /**
     * Emitted when an action has been inserted into, or removed from, this action collection.
     * @since 5.66
     */
    void changed();

#if KXMLGUI_ENABLE_DEPRECATED_SINCE(5, 0)
    /**
     * Indicates that @p action was highlighted (hovered over).
     * @deprecated Since 5.0. Replaced by actionHovered(QAction* action);
     */
    KXMLGUI_DEPRECATED_VERSION(5, 0, "Use KActionCollection::actionHovered(QAction* action)")
    QT_MOC_COMPAT void actionHighlighted(QAction *action);
#endif

    /**
     * Indicates that @p action was hovered.
     */
    void actionHovered(QAction *action);

    /**
     * Indicates that @p action was triggered
     */
    void actionTriggered(QAction *action);

protected:
    /// Overridden to perform connections when someone wants to know whether an action was highlighted or triggered
    void connectNotify(const QMetaMethod &signal) override;

protected Q_SLOTS:
    virtual void slotActionTriggered();

#if KXMLGUI_ENABLE_DEPRECATED_SINCE(5, 0)
    /**
     * @internal
     * @deprecated Since 5.0. Replaced by slotActionHovered();
     */
    KXMLGUI_DEPRECATED_VERSION(5, 0, "Use KActionCollection::slotActionHovered()")
    QT_MOC_COMPAT virtual void slotActionHighlighted();
#endif

private Q_SLOTS:
    void slotActionHovered();

public:
    /**
     * Add an action under the given name to the collection.
     *
     * Inserting an action that was previously inserted under a different name will replace the
     * old entry, i.e. the action will not be available under the old name anymore but only under
     * the new one.
     *
     * Inserting an action under a name that is already used for another action will replace
     * the other action in the collection (but will not delete it).
     *
     * If KAuthorized::authorizeAction() reports that the action is not
     * authorized, it will be disabled and hidden.
     *
     * The ownership of the action object is not transferred.
     * If the action is destroyed it will be removed automatically from the KActionCollection.
     *
     * @param name The name by which the action be retrieved again from the collection.
     * @param action The action to add.
     * @return the same as the action given as parameter. This is just for convenience
     * (chaining calls) and consistency with the other addAction methods, you can also
     * simply ignore the return value.
     */
    Q_INVOKABLE QAction *addAction(const QString &name, QAction *action);

    /**
     * Adds a list of actions to the collection.
     *
     * The objectName of the actions is used as their internal name in the collection.
     *
     * The ownership of the action objects is not transferred.
     * If the action is destroyed it will be removed automatically from the KActionCollection.
     *
     * Uses addAction(const QString&, QAction*).
     *
     * @param actions the list of the actions to add.
     *
     * @see addAction()
     * @since 5.0
     */
    void addActions(const QList<QAction *> &actions);

    /**
     * Removes an action from the collection and deletes it.
     * @param action The action to remove.
     */
    void removeAction(QAction *action);

    /**
     * Removes an action from the collection.
     *
     * The ownership of the action object is not changed.
     *
     * @param action the action to remove.
     */
    QAction *takeAction(QAction *action);

    /**
     * Creates a new standard action, adds it to the collection and connects the
     * action's triggered(bool) signal to the specified receiver/member. The
     * newly created action is also returned.
     *
     * @note Using KStandardAction::OpenRecent will cause a different signal than
     * triggered(bool) to be used, see KStandardAction for more information.
     *
     * The action can be retrieved later from the collection by its standard name as per
     * KStandardAction::stdName.
     *
     * The KActionCollection takes ownership of the action object.
     *
     * @param actionType The standard action type of the action to create.
     * @param receiver The QObject to connect the triggered(bool) signal to.  Leave nullptr if no
     *                 connection is desired.
     * @param member The SLOT to connect the triggered(bool) signal to.  Leave nullptr if no
     *               connection is desired.
     * @return new action of the given type ActionType.
     */
    QAction *addAction(KStandardAction::StandardAction actionType, const QObject *receiver = nullptr, const char *member = nullptr);

    /**
     * Creates a new standard action, adds to the collection under the given name
     * and connects the action's triggered(bool) signal to the specified
     * receiver/member. The newly created action is also returned.
     *
     * @note Using KStandardAction::OpenRecent will cause a different signal than
     * triggered(bool) to be used, see KStandardAction for more information.
     *
     * The action can be retrieved later from the collection by the specified name.
     *
     * The KActionCollection takes ownership of the action object.
     *
     * @param actionType The standard action type of the action to create.
     * @param name The name by which the action be retrieved again from the collection.
     * @param receiver The QObject to connect the triggered(bool) signal to.  Leave nullptr if no
     *                 connection is desired.
     * @param member The SLOT to connect the triggered(bool) signal to.  Leave nullptr if no
     *               connection is desired.
     * @return new action of the given type ActionType.
     */
    QAction *addAction(KStandardAction::StandardAction actionType, const QString &name, const QObject *receiver = nullptr, const char *member = nullptr);

/**
 * This is the same as addAction(KStandardAction::StandardAction actionType, const QString &name, const QObject *receiver, const char *member) using
 * new style connect syntax.
 *
 * @param actionType The standard action type of the action to create.
 * @param name The name by which the action be retrieved again from the collection.
 * @param receiver The QObject to connect the triggered(bool) signal to.
 * @param slot The slot or lambda to connect the triggered(bool) signal to.
 * @return new action of the given type ActionType.
 *
 * @see addAction(KStandardAction::StandardAction, const QString &, const QObject *, const char *)
 * @since 5.80
 */
#ifdef K_DOXYGEN
    inline QAction *addAction(KStandardAction::StandardAction actionType, const QString &name, const Receiver *receiver, Func slot)
#else
    template<class Receiver, class Func>
    inline typename std::enable_if<!std::is_convertible<Func, const char *>::value, QAction>::type *
    addAction(KStandardAction::StandardAction actionType, const QString &name, const Receiver *receiver, Func slot)
#endif
    {
        QAction *action = KStandardAction::create(actionType, receiver, slot, nullptr);
        action->setParent(this);
        action->setObjectName(name);
        return addAction(name, action);
    }

    /**
     * Creates a new action under the given name to the collection and connects
     * the action's triggered(bool) signal to the specified receiver/member. The
     * newly created action is returned.
     *
     * NOTE: KDE prior to 4.2 used the triggered() signal instead of the triggered(bool)
     * signal.
     *
     * Inserting an action that was previously inserted under a different name will replace the
     * old entry, i.e. the action will not be available under the old name anymore but only under
     * the new one.
     *
     * Inserting an action under a name that is already used for another action will replace
     * the other action in the collection.
     *
     * The KActionCollection takes ownership of the action object.
     *
     * @param name The name by which the action be retrieved again from the collection.
     * @param receiver The QObject to connect the triggered(bool) signal to.  Leave nullptr if no
     *                 connection is desired.
     * @param member The SLOT to connect the triggered(bool) signal to.  Leave nullptr if no
     *               connection is desired.
     * @return new action of the given type ActionType.
     */
    QAction *addAction(const QString &name, const QObject *receiver = nullptr, const char *member = nullptr);

    /**
     * Creates a new action under the given name, adds it to the collection and connects the action's triggered(bool)
     * signal to the specified receiver/member. The receiver slot may accept either a bool or no
     * parameters at all (i.e. slotTriggered(bool) or slotTriggered() ).
     * The type of the action is specified by the template parameter ActionType.
     *
     * NOTE: KDE prior to 4.2 connected the triggered() signal instead of the triggered(bool)
     * signal.
     *
     * The KActionCollection takes ownership of the action object.
     *
     * @param name The internal name of the action (e.g. "file-open").
     * @param receiver The QObject to connect the triggered(bool) signal to.  Leave nullptr if no
     *                 connection is desired.
     * @param member The SLOT to connect the triggered(bool) signal to.  Leave nullptr if no
     *               connection is desired.
     * @return new action of the given type ActionType.
     *
     * @see addAction()
     */
    template<class ActionType>
    ActionType *add(const QString &name, const QObject *receiver = nullptr, const char *member = nullptr)
    {
        ActionType *a = new ActionType(this);
        if (receiver && member) {
            connect(a, SIGNAL(triggered(bool)), receiver, member);
        }
        addAction(name, a);
        return a;
    }

/**
 * This is the same as add(const QString &name, const QObject *receiver, const char *member) using
 * new style connect syntax.
 *
 * @param name The internal name of the action (e.g. "file-open").
 * @param receiver The QObject to connect the triggered(bool) signal to.
 * @param slot The slot or lambda to connect the triggered(bool) signal to.
 * @return new action of the given type ActionType.
 *
 * @see add(const QString &, const QObject *, const char *)
 * @since 5.28
 */
#ifdef K_DOXYGEN
    template<class ActionType>
    inline ActionType *add(const QString &name, const Receiver *receiver, Func slot)
#else
    template<class ActionType, class Receiver, class Func>
    inline typename std::enable_if<!std::is_convertible<Func, const char *>::value, ActionType>::type *
    add(const QString &name, const Receiver *receiver, Func slot)
#endif
    {
        ActionType *a = new ActionType(this);
        connect(a, &QAction::triggered, receiver, slot);
        addAction(name, a);
        return a;
    }

/**
 * This is the same as addAction(const QString &name, const QObject *receiver, const char *member) using
 * new style connect syntax.
 *
 * @param name The internal name of the action (e.g. "file-open").
 * @param receiver The QObject to connect the triggered(bool) signal to.
 * @param slot The slot or lambda to connect the triggered(bool) signal to.
 * @return new action of the given type ActionType.
 *
 * @see addAction(const QString &, const QObject *, const char *)
 * @since 5.28
 */
#ifdef K_DOXYGEN
    inline QAction *addAction(const QString &name, const Receiver *receiver, Func slot)
#else
    template<class Receiver, class Func>
    inline typename std::enable_if<!std::is_convertible<Func, const char *>::value, QAction>::type *
    addAction(const QString &name, const Receiver *receiver, Func slot)
#endif
    {
        return add<QAction>(name, receiver, slot);
    }

    /**
     * Get the default primary shortcut for the given action.
     *
     * @param action the action for which the default primary shortcut should be returned.
     * @return the default primary shortcut of the given action
     * @since 5.0
     */
    QKeySequence defaultShortcut(QAction *action) const;

    /**
     * Get the default shortcuts for the given action.
     *
     * @param action the action for which the default shortcuts should be returned.
     * @return the default shortcuts of the given action
     * @since 5.0
     */
    QList<QKeySequence> defaultShortcuts(QAction *action) const;

    // TODO KF6: Make setDefaultShortcut static
    /**
     * Set the default shortcut for the given action.
     * Since 5.2, this also calls action->setShortcut(shortcut), i.e. the default shortcut is
     * made active initially.
     *
     * @param action the action for which the default shortcut should be set.
     * @param shortcut the shortcut to use for the given action in its specified shortcutContext()
     * @since 5.0
     */
    void setDefaultShortcut(QAction *action, const QKeySequence &shortcut);

    /**
     * Set the default shortcuts for the given action.
     * Since 5.2, this also calls action->setShortcuts(shortcuts), i.e. the default shortcut is
     * made active initially.
     *
     * @param action the action for which the default shortcut should be set.
     * @param shortcuts the shortcuts to use for the given action in its specified shortcutContext()
     * @since 5.0
     */
    Q_INVOKABLE void setDefaultShortcuts(QAction *action, const QList<QKeySequence> &shortcuts);

    /**
     * Returns true if the given action's shortcuts may be configured by the user.
     *
     * @param action the action for the hint should be verified.
     * @since 5.0
     */
    bool isShortcutsConfigurable(QAction *action) const;

    /**
     * Indicate whether the user may configure the action's shortcuts.
     *
     * @param action the action for the hint should be verified.
     * @param configurable set to true if the shortcuts of the given action may be configured by the user, otherwise false.
     * @since 5.0
     */
    void setShortcutsConfigurable(QAction *action, bool configurable);

private:
    KActionCollection(const KXMLGUIClient *parent); // used by KXMLGUIClient

    friend class KActionCollectionPrivate;
    std::unique_ptr<class KActionCollectionPrivate> const d;
};

#endif
