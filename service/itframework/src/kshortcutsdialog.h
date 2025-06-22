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

#ifndef KSHORTCUTSDIALOG_H
#define KSHORTCUTSDIALOG_H

#include <kxmlgui_export.h>

#include <QDialog>
#include <memory>

#include "kshortcutseditor.h"

/**
 * @class KShortcutsDialog kshortcutsdialog.h KShortcutsDialog
 *
 * @short Dialog for configuration of KActionCollection and KGlobalAccel.
 *
 * This dialog can be used to configure keyboard shortcuts associated with actions
 * in KActionCollection and KGlobalAccel. It integrates the KShortcutsEditor widget and
 * offers various functionalities related to keyboard shortcuts (e.g. reset all shortcuts
 * to defaults, manage shortcuts schemes ...etc).
 *
 * Several static methods are supplied, which provide a convenient interface to the dialog.
 * The most common, and most encouraged, use is with a KActionCollection.
 *
 * @code
 * KShortcutsDialog::configure(actionCollection(), KShortcutsEditor::LetterShortcutsAllowed, parent);
 *
 * // Alternatively, since 5.84, you can use:
 * KShortcutsDialog::showDialog(actionCollection(), KShortcutsEditor::LetterShortcutsAllowed, parent);
 * @endcode
 *
 * By default this dialog is modal (since 4.3). If you don't want that, call @c setModal(false)
 * and then the non-static configure() method to show the dialog.
 *
 * If you need to run some extra code when the dialog is closed, connect to the signals
 * emitted by @c QDialog (accepted(), rejected(), finished() ...etc.). However, note
 * that if that extra code depends on the changed settings having already been saved,
 * connect to the @c saved() signal instead to be on the safe side (if you connect to
 * e.g. @c QDialog::accepted() your function might be called before the changes have
 * actually been saved).
 *
 * @note Starting from 5.95, if there are no Global shortcuts in any of the action
 * collections shown in the dialog, the relevant columns ("Global" and "Global Alternate")
 * will be hidden.
 *
 * Example:
 * @code
 * // Create the dialog; alternatively you can use the other constructor if e.g.
 * // you need to only show certain action types, or disallow single letter shortcuts
 * KShortcutsDialog *dlg = new KShortcutsDialog(parent);
 *
 * // Set the Qt::WA_DeleteOnClose attribute, so that the dialog is automatically
 * // deleted after it's closed
 * dlg->setAttribute(Qt::WA_DeleteOnClose);
 *
 * // Add an action collection (otherwise the dialog would be empty)
 * dlg->addCollection(actionCollection());
 *
 * // Run some extra code after the settings are saved
 * connect(dlg, &KShortcutsDialog::saved, this, &ClassFoo::doExtraStuff);
 *
 * // Called with "true" so that the changes are saved if the dialog is accepted,
 * // see the configure(bool) method for more details
 * dlg->configure(true);
 * @endcode
 *
 * @image html kshortcutsdialog.png "KShortcutsDialog"
 *
 * @author Nicolas Hadacek <hadacek@via.ecp.fr>
 * @author Hamish Rodda <rodda@kde.org> (KDE 4 porting)
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class KXMLGUI_EXPORT KShortcutsDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructs a KShortcutsDialog as a child of @p parent.
     *
     * @param actionTypes sets the action types to be shown in the dialog,
     * this is an OR'ed combination of @c KShortcutsEditor::ActionTypes;
     * the default is @c KShortcutsEditor::AllActions
     * @param allowLetterShortcuts set to false if unmodified alphanumeric
     * keys ('A', '1', etc.) are not permissible shortcuts; @see @c KShortcutsEditor::LetterShortcuts
     * @param parent parent widget of the dialog, if not @c nullptr will be
     * used by the window manager to place the dialog relative to it
     */
    explicit KShortcutsDialog(KShortcutsEditor::ActionTypes actionTypes = KShortcutsEditor::AllActions,
                              KShortcutsEditor::LetterShortcuts allowLetterShortcuts = KShortcutsEditor::LetterShortcutsAllowed,
                              QWidget *parent = nullptr);
    /**
     * Constructs a KShortcutsDialog as a child of @p parent, with the default settings
     * (@ref KShortcutsEditor::AllActions and @ref KShortcutsEditor::LetterShortcutsAllowed).
     *
     * Typically a @ref KActionCollection is added by using @ref addCollection().
     *
     * @param parent the parent widget of the dialog, if not @c nullptr will be
     * used by the window manager to place the dialog relative to it
     *
     * @since 5.85
     */
    explicit KShortcutsDialog(QWidget *parent);

    /**
     * Destructor. Deletes all resources used by a KShortcutsDialog object.
     */
    ~KShortcutsDialog() override;

    /**
     * Add all actions of the collection to the ones displayed and configured
     * by the dialog.
     *
     * @param title the title associated with the collection (if null, the
     * KAboutData::progName() of the collection's componentData is used)
     */
    void addCollection(KActionCollection *collection, const QString &title = {});

    /**
     * @return the list of action collections that are available for configuration in the dialog.
     */
    QList<KActionCollection *> actionCollections() const;

    /**
     * Run the dialog and call writeSettings() on the action collections
     * that were added if @p saveSettings is true.
     *
     * If the dialog is modal this method will use @c QDialog::exec() to open the dialog,
     * otherwise it will use @c QDialog::show().
     */
    // TODO KF6 remove this method, see configure() static method for more details
    bool configure(bool saveSettings = true);

    /** @see QWidget::sizeHint() */
    QSize sizeHint() const override;

#if KXMLGUI_ENABLE_DEPRECATED_SINCE(5, 84)
    /**
     * Pops up a modal dialog for configuring key settings. The new
     * shortcut settings will become active if the user presses OK.
     *
     * @param collection the KActionCollection to configure
     * @param allowLetterShortcuts set to KShortcutsEditor::LetterShortcutsDisallowed if unmodified alphanumeric
     *  keys ('A', '1', etc.) are not permissible shortcuts.
     * @param parent the parent widget to attach to
     * @param bSaveSettings if true, the settings will also be saved back
     * by calling writeSettings() on the action collections that were added.
     *
     * @return Accept if the dialog was closed with OK, Reject otherwise.
     *
     * @deprecated since 5.84, use KShortcutsDialog::showDialog() instead.
     */
    KXMLGUI_DEPRECATED_VERSION(5, 84, "Use KShortcutsDialog::showDialog() instead, see API documentation for code examples.")
    static int configure(KActionCollection *collection,
                         KShortcutsEditor::LetterShortcuts allowLetterShortcuts = KShortcutsEditor::LetterShortcutsAllowed,
                         QWidget *parent = nullptr,
                         bool bSaveSettings = true);
#endif

    /**
     * This static method shows a modal dialog that can be used to configure
     * the shortcuts associated with each action in @p collection. The new
     * shortcut settings will be saved and applied if the dialog is accepted.
     *
     * The dialog is opened by using @c QDialog::show(), and it will be deleted
     * automatically when it's closed.
     *
     * Example:
     * @code
     * // Typical usage is with a KActionCollection:
     * KShortcutsDialog::showDialog(actionCollection(), KShortcutsEditor::LetterShortcutsAllowed, parent);
     * @endcode
     *
     * @param collection the KActionCollection to configure
     * @param allowLetterShortcuts set to @c KShortcutsEditor::LetterShortcutsDisallowed
     * if unmodified alphanumeric keys ('A', '1', etc.) are not permissible shortcuts
     * @param parent the parent widget of the dialog, if not @c nullptr it will be used
     * by the window manager to place the dialog relative to it
     *
     * @since 5.84
     */
    static void showDialog(KActionCollection *collection,
                           KShortcutsEditor::LetterShortcuts allowLetterShortcuts = KShortcutsEditor::LetterShortcutsAllowed,
                           QWidget *parent = nullptr);

    /**
     * Imports a shortcuts set up from @p path
     *
     * @since 5.15
     */
    void importConfiguration(const QString &path);

    /**
     * Exports a shortcuts set up from @p path
     *
     * @since 5.15
     */
    void exportConfiguration(const QString &path) const;

    /**
     * Reloads the list of schemes in the "Manage Schemes" section
     *
     * This is useful if the available scheme files changed for some reason
     * eg. because KNewStuff was used
     *
     * @since 5.97
     */
    void refreshSchemes();

    /**
     * This adds a @c QAction to the "More Actions" menu
     *
     * This is useful to add for example an action to download more
     * keyboard schemes via KNewStuff
     *
     * @since 5.97
     */
    void addActionToSchemesMoreButton(QAction *action);

public Q_SLOTS:
    /**
     * @reimp
     */
    void accept() override;

Q_SIGNALS:
    /**
     * Emitted after the dialog is accepted (by clicking the OK button) and settings are saved.
     * Connect to this signal if you need to run some extra code after the settings are saved.
     */
    void saved();

private:
    friend class KShortcutsDialogPrivate;
    std::unique_ptr<class KShortcutsDialogPrivate> const d;

    Q_DISABLE_COPY(KShortcutsDialog)
};

#endif // KSHORTCUTSDIALOG_H
