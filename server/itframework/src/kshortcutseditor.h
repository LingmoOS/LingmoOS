/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1997 Nicolas Hadacek <hadacek@kde.org>
    SPDX-FileCopyrightText: 2001, 2001 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSHORTCUTSEDITOR_H
#define KSHORTCUTSEDITOR_H

#include <kxmlgui_export.h>

#include <QWidget>

class KActionCollection;
class KConfig;
class KConfigBase;
class KConfigGroup;
class KGlobalAccel;
class KShortcutsEditorPrivate;

// KShortcutsEditor expects that the list of existing shortcuts is already
// free of conflicts. If it is not, nothing will crash, but your users
// won't like the resulting behavior.

/**
 * @class KShortcutsEditor kshortcutseditor.h KShortcutsEditor
 *
 * @short Widget for configuration of KAccel and KGlobalAccel.
 *
 * Configure dictionaries of key/action associations for QActions,
 * including global shortcuts.
 *
 * The class takes care of all aspects of configuration, including
 * handling key conflicts internally. Connect to the allDefault()
 * slot if you want to set all configurable shortcuts to their
 * default values.
 *
 * @see KShortcutsDialog
 * @author Nicolas Hadacek <hadacek@via.ecp.fr>
 * @author Hamish Rodda <rodda@kde.org> (KDE 4 porting)
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class KXMLGUI_EXPORT KShortcutsEditor : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(ActionTypes actionTypes READ actionTypes WRITE setActionTypes)

public:
    /**
     * @see ActionTypes
     */
    enum ActionType {
        /// Actions which are triggered by any keypress in a widget which has the action added to it
        WidgetAction = Qt::WidgetShortcut /*0*/,
        /// Actions which are triggered by any keypress in a window which has the action added to it or its child widget(s)
        WindowAction = Qt::WindowShortcut /*1*/,
        /// Actions which are triggered by any keypress in the application
        ApplicationAction = Qt::ApplicationShortcut /*2*/,
        /// Actions which are triggered by any keypress in the windowing system
        /// @note Starting from 5.95, this flag is ignored if there are no actual Global shortcuts in any of the action collections that are added
        GlobalAction = 4,
        /// All actions
        AllActions = 0xffffffff,
    };
    /**
     * Stores a combination of #ActionType values.
     */
    Q_DECLARE_FLAGS(ActionTypes, ActionType)

    enum LetterShortcuts {
        /// Shortcuts without a modifier are not allowed,
        /// so 'A' would not be valid, whereas 'Ctrl+A' would be.
        /// This only applies to printable characters, however.
        /// 'F1', 'Insert' etc. could still be used.
        LetterShortcutsDisallowed = 0,
        /// Letter shortcuts are allowed
        LetterShortcutsAllowed,
    };

    /**
     * Constructor.
     *
     * @param collection the KActionCollection to configure
     * @param parent parent widget
     * @param actionTypes types of actions to display in this widget.
     * @param allowLetterShortcuts set to LetterShortcutsDisallowed if unmodified alphanumeric
     *  keys ('A', '1', etc.) are not permissible shortcuts.
     */
    KShortcutsEditor(KActionCollection *collection,
                     QWidget *parent,
                     ActionTypes actionTypes = AllActions,
                     LetterShortcuts allowLetterShortcuts = LetterShortcutsAllowed);

    /**
     * \overload
     *
     * Creates a key chooser without a starting action collection.
     *
     * @param parent parent widget
     * @param actionTypes types of actions to display in this widget.
     * @param allowLetterShortcuts set to LetterShortcutsDisallowed if unmodified alphanumeric
     *  keys ('A', '1', etc.) are not permissible shortcuts.
     */
    explicit KShortcutsEditor(QWidget *parent, ActionTypes actionTypes = AllActions, LetterShortcuts allowLetterShortcuts = LetterShortcutsAllowed);

    /// Destructor
    ~KShortcutsEditor() override;

    /**
     * Are the unsaved changes?
     */
    bool isModified() const;

    /**
     * Removes all action collections from the editor
     */
    void clearCollections();

    /**
     * Insert an action collection, i.e. add all its actions to the ones
     * already associated with the KShortcutsEditor object.
     * @param title subtree title of this collection of shortcut.
     */
    void addCollection(KActionCollection *, const QString &title = QString());

    /**
     * Undo all change made since the last commit().
     *
     * @since 5.75
     */
    void undo();

#if KXMLGUI_ENABLE_DEPRECATED_SINCE(5, 75)
    /**
     * Undo all change made since the last commit().
     * @deprecated Since 5.75, use undo()
     */
    KXMLGUI_DEPRECATED_VERSION(5, 75, "Use KShortcutsEditor::undo()")
    void undoChanges();
#endif

    /**
     * Save the changes.
     *
     * Before saving the changes are committed. This saves the actions to disk.
     * Any KActionCollection objects with the xmlFile() value set will be
     * written to an XML file.  All other will be written to the application's
     * rc file.
     */
    void save();

    /**
     * Commit the changes without saving.
     *
     * This commits the changes without saving.
     *
     * @since 4.2
     */
    void commit();

    /**
     * Removes all configured shortcuts.
     */
    void clearConfiguration();

    /**
     * Write the current settings to the \p config object.
     *
     * This does not initialize the \p config object. It adds the
     * configuration.
     *
     * @note This will not save the global configuration! globalaccel holds
     * that part of the configuration.
     * @see writeGlobalConfig()
     *
     * @param config Config object to save to or, or null to use the
     *               applications config object
     *
     */
    void writeConfiguration(KConfigGroup *config = nullptr) const;

    /**
     * Export the current setting to configuration @p config.
     *
     * This initializes the configuration object. This will export the global
     * configuration too.
     *
     * @param config Config object
     */
    void exportConfiguration(KConfigBase *config) const;
#if KXMLGUI_ENABLE_DEPRECATED_SINCE(5, 0)
    KXMLGUI_DEPRECATED_VERSION(5, 0, "Use KShortcutsEditor::exportConfiguration(KConfigBase *config)")
    void exportConfiguration(KConfig *config) const;
#endif

    /**
     * Import the settings from configuration @p config.
     *
     * This will remove all current setting before importing. All shortcuts
     * are set to QList<QKeySequence>() prior to importing from @p config!
     *
     * @param config Config object
     */
    void importConfiguration(KConfigBase *config);
#if KXMLGUI_ENABLE_DEPRECATED_SINCE(5, 0)
    KXMLGUI_DEPRECATED_VERSION(5, 0, "Use KShortcutsEditor::importConfiguration(KConfigBase *config)")
    void importConfiguration(KConfig *config);
#endif

    /**
     * Sets the types of actions to display in this widget.
     *
     * @param actionTypes New types of actions
     * @since 5.0
     */
    void setActionTypes(ActionTypes actionTypes);
    /**
     *
     * @return The types of actions currently displayed in this widget.
     * @since 5.0
     */
    ActionTypes actionTypes() const;

Q_SIGNALS:
    /**
     * Emitted when an action's shortcut has been changed.
     **/
    void keyChange();

public Q_SLOTS:
    /**
     * Resize columns to width required
     */
    void resizeColumns();

    /**
     * Set all shortcuts to their default values (bindings).
     **/
    void allDefault();

    /**
     * Opens a printing dialog to print all the shortcuts
     */
    void printShortcuts() const;

private:
    friend class KShortcutsDialog;
    friend class KShortcutsEditorPrivate;
    std::unique_ptr<KShortcutsEditorPrivate> const d;
    Q_DISABLE_COPY(KShortcutsEditor)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KShortcutsEditor::ActionTypes)

#endif // KSHORTCUTSEDITOR_H
