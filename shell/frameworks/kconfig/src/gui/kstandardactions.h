/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999, 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2001, 2002 Ellis Whitehead <ellis@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#ifndef KGUISTANDARDACTION_H
#define KGUISTANDARDACTION_H

#include <QAction>
#include <QList>
#include <QStringList>

#include <KStandardShortcut>
#include <kconfiggui_export.h>

#include <optional>
#include <type_traits>

class QObject;
class QAction;

/**
 * Convenience methods to access all standard KDE actions.
 *
 * These actions should be used instead of hardcoding menubar and
 * toolbar items.  Using these actions helps your application easily
 * conform to the <a href="https://develop.kde.org/hig/">KDE Human Interface Guidelines</a>.
 *
 * All of the documentation for QAction holds for KStandardActions
 * also.  When in doubt on how things work, check the QAction
 * documentation first.
 * Please note that calling any of these methods automatically adds the action
 * to the actionCollection() of the QObject given by the 'parent' parameter.
 *
 * <b>Simple Example:</b>\n
 *
 * In general, using standard actions should be a drop in replacement
 * for regular actions. For example, if you previously had:
 * @code
 * QAction *newAct = new QAction(QIcon::fromTheme("document-new"),
 *                               i18n("&New"),
 *                               this);
 * newAct->setShortcut(KStandardShortcut::shortcut(KStandardShortcut::New).constFirst());
 * connect(newAct, &QAction::triggered, this, &ClassFoo::fileNew);
 * @endcode
 *
 * You can replace it with:
 * @code
 * QAction *newAct = KStandardActions::openNew(this, &ClassFoo::fileNew, this);
 * @endcode
 *
 * Alternatively you can instantiate the action using the StandardAction enums
 * provided.  This author can't think of a reason why you would want to, but, hey,
 * if you do, here's how:
 *
 * \code
 * QAction *newAct = KStandardActions::create(KStandardActions::New, this, &ClassFoo::fileNew, this);
 * \endcode
 *
 * <b>Relationship with KActionCollection from KXMLGui</b>\n
 *
 * If a KActionCollection is passed as the parent then the action will be
 * automatically added to that collection:
 * \code
 * QAction *cut = KStandardActions::cut(this, &ClassFoo::editCut, actionCollection());
 * \endcode
 *
 * Each action has a unique internal name which can be queried using the
 * name method.  For example KStandardActions::name(KStandardActions::Cut)
 * would return 'edit_cut'.  This name can be used as a unique identifier
 * for the actions.  So if you wanted to add an existing standard action
 * to an action collection you can do so like this:
 * \code
 * QAction *cut = KStandardActions::cut(this, &ClassFoo::editCut, this);
 * actionCollection()->addAction(KStandardActions::name(KStandardActions::Cut), cut);
 * \endcode
 *
 * You can then get a pointer to the action using
 * \code
 * QAction *cutPtr = actionCollection()->action(KStandardActions::name(KStandardActions::Cut));
 * \endcode
 *
 * @author Kurt Granroth <granroth@kde.org>
 * @since 6.3
 */
namespace KStandardActions
{
/**
 * The standard menubar and toolbar actions.
 */
enum StandardAction {
    ActionNone,
    // File Menu
    New, ///< Create a new document or window.
    Open, ///< Open an existing file.
    OpenRecent, ///< Open a recently used document.
    Save, ///< Save the current document.
    SaveAs, ///< Save the current document under a different name.
    Revert, ///< Revert the current document to the last saved version.
    Close, ///< Close the current document.
    Print, ///< Print the current document.
    PrintPreview, ///< Show a print preview of the current document.
    Mail, ///< Send the current document by mail.
    Quit, ///< Quit the program.
    // Edit Menu
    Undo, ///< Undo the last operation.
    Redo, ///< Redo the last operation.
    Cut, ///< Cut selected area and store it in the clipboard.
    Copy, ///< Copy selected area and store it in the clipboard.
    Paste, ///< Paste the contents of clipboard at the current mouse or cursor.
    SelectAll, ///< Select all elements in the current document.
    Deselect, ///< Deselect any selected elements in the current document.
    Find, ///< Initiate a 'find' request in the current document.
    FindNext, ///< Find the next instance of a stored 'find'
    FindPrev, ///< Find a previous instance of a stored 'find'.
    Replace, ///< Find and replace matches.
    // View Menu
    ActualSize, ///< View the document at its actual size.
    FitToPage, ///< Fit the document view to the size of the current window.
    FitToWidth, ///< Fit the document view to the width of the current window.
    FitToHeight, ///< Fit the document view to the height of the current window.
    ZoomIn, ///< Zoom in the current document.
    ZoomOut, ///< Zoom out the current document.
    Zoom, ///< Select the current zoom level.
    Redisplay, ///< Redisplay or redraw the document.
    // Go Menu
    Up, ///< Move up (web style menu).
    Back, ///< Move back (web style menu).
    Forward, ///< Move forward (web style menu).
    Home, ///< Go to the "Home" position or document.
    Prior, ///< Scroll up one page.
    Next, ///< Scroll down one page.
    Goto, ///< Jump to some specific location in the document.
    GotoPage, ///< Go to a specific page.
    GotoLine, ///< Go to a specific line.
    FirstPage, ///< Jump to the first page.
    LastPage, ///< Jump to the last page.
    DocumentBack, ///< Move back (document style menu).
    DocumentForward, ///< Move forward (document style menu).
    // Bookmarks Menu
    AddBookmark, ///< Add the current page to the bookmarks tree.
    EditBookmarks, ///< Edit the application bookmarks.
    // Tools Menu
    Spelling, ///< Pop up the spell checker.
    // Settings Menu
    ShowMenubar, ///< Show/Hide the menubar.
    ShowToolbar, ///< Show/Hide the toolbar.
    ShowStatusbar, ///< Show/Hide the statusbar.
    KeyBindings, ///< Display the configure key bindings dialog.
    Preferences, ///< Display the preferences/options dialog.
    ConfigureToolbars, ///< Display the toolbar configuration dialog.
    // Help Menu
    HelpContents, ///< Display the handbook of the application.
    WhatsThis, ///< Trigger the What's This cursor.
    ReportBug, ///< Open up the Report Bug dialog.
    AboutApp, ///< Display the application's About box.
    AboutKDE, ///< Display the About KDE dialog.
    // Other standard actions
    ConfigureNotifications, ///< Display the notifications configuration dialog.
    FullScreen, ///< Switch to/from full screen mode.
    Clear, ///< Clear the content of the focus widget.
    SwitchApplicationLanguage, ///< Display the Switch Application Language dialog.
    DeleteFile, ///< Permanently deletes files or folders.
    RenameFile, ///< Renames files or folders.
    MoveToTrash, ///< Moves files or folders to the trash.
    Donate, ///< Open donation page on kde.org.
    HamburgerMenu ///< Opens a menu that substitutes the menubar.
    // To keep in sync with KConfigWidgets::KStandardAction
};

/**
 * @internal
 */
KCONFIGGUI_EXPORT QAction *_kgui_createInternal(StandardAction id, QObject *parent);

/**
 * Creates an action corresponding to one of the
 * KStandardActions::StandardAction actions, which is connected to the given
 * object and @p slot, and is owned by @p parent.
 *
 * If not explicitly specified, @p connectionType will be AutoConnection for all actions
 * except for ConfigureToolbars it will be QueuedConnection.
 *
 * @see create(StandardAction, const QObject *, const char *, QObject *)
 */
template<class Receiver, class Func>
inline QAction *create(StandardAction id, const Receiver *recvr, Func slot, QObject *parent, std::optional<Qt::ConnectionType> connectionType = std::nullopt)
{
    QAction *action = _kgui_createInternal(id, parent);
    // ConfigureToolbars is special because of bug #200815
    const Qt::ConnectionType defaultConnectionType = (id == ConfigureToolbars) ? Qt::QueuedConnection : Qt::AutoConnection;
    QObject::connect(action, &QAction::triggered, recvr, slot, connectionType.value_or(defaultConnectionType));
    return action;
}

/**
 * This will return the internal name of a given standard action.
 */
KCONFIGGUI_EXPORT QString name(StandardAction id);

/**
 * Returns a list of all actionIds.
 */
KCONFIGGUI_EXPORT QList<StandardAction> actionIds();

/**
 * Returns the standardshortcut associated with @a actionId.
 *
 * @param id    The identifier whose associated shortcut is wanted.
 */
KCONFIGGUI_EXPORT KStandardShortcut::StandardShortcut shortcutForActionId(StandardAction id);

// clang-format off
#define KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(name, enumValue) \
    template<class Receiver, class Func> \
    inline QAction *name(const Receiver *recvr, Func slot, QObject *parent) \
    { return create(enumValue, recvr, slot, parent); }
// clang-format on

/**
 * Create a new document or window.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(openNew, New)

/**
 * Open an existing file.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(open, Open)

/**
 * Save the current document.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(save, Save)

/**
 * Save the current document under a different name.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(saveAs, SaveAs)

/**
 * Revert the current document to the last saved version
 * (essentially will undo all changes).
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(revert, Revert)

/**
 * Close the current document.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(close, Close)

/**
 * Print the current document.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(print, Print)

/**
 * Show a print preview of the current document.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(printPreview, PrintPreview)

/**
 * Mail this document.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(mail, Mail)

/**
 * Quit the program.
 *
 * Note that you probably want to connect this action to either QWidget::close()
 * or QApplication::closeAllWindows(), but not QApplication::quit(), so that
 * KMainWindow::queryClose() is called on any open window (to warn the user
 * about unsaved changes for example).
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(quit, Quit)

/**
 * Undo the last operation.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(undo, Undo)

/**
 * Redo the last operation.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(redo, Redo)

/**
 * Cut selected area and store it in the clipboard.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(cut, Cut)

/**
 * Copy the selected area into the clipboard.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(copy, Copy)

/**
 * Paste the contents of clipboard at the current mouse or cursor
 * position.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(paste, Paste)

/**
 * Clear the content of the focus widget
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(clear, Clear)

/**
 * Select all elements in the current document.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(selectAll, SelectAll)

/**
 * Deselect any selected elements in the current document.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(deselect, Deselect)

/**
 * Initiate a 'find' request in the current document.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(find, Find)

/**
 * Find the next instance of a stored 'find'.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(findNext, FindNext)

/**
 * Find a previous instance of a stored 'find'.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(findPrev, FindPrev)

/**
 * Find and replace matches.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(replace, Replace)

/**
 * View the document at its actual size.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(actualSize, ActualSize)

/**
 * Fit the document view to the size of the current window.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(fitToPage, FitToPage)

/**
 * Fit the document view to the width of the current window.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(fitToWidth, FitToWidth)

/**
 * Fit the document view to the height of the current window.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(fitToHeight, FitToHeight)

/**
 * Zoom in the current document view.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(zoomIn, ZoomIn)

/**
 * Zoom out the current document view.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(zoomOut, ZoomOut)

/**
 * Select the current zoom level.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(zoom, Zoom)

/**
 * Redisplay or redraw the document.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(redisplay, Redisplay)

/**
 * Move up (web style menu).
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(up, Up)

/**
 * Move back (web style menu).
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(back, Back)

/**
 * Move forward (web style menu).
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(forward, Forward)

/**
 * Go to the "Home" position or document.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(home, Home)

/**
 * Scroll up one page.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(prior, Prior)

/**
 * Scroll down one page.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(next, Next)

/**
 * Jump to some specific location in the document.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(goTo, Goto)

/**
 * Go to a specific page.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(gotoPage, GotoPage)

/**
 * Go to a specific line.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(gotoLine, GotoLine)

/**
 * Jump to the first page.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(firstPage, FirstPage)

/**
 * Jump to the last page.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(lastPage, LastPage)

/**
 * Move back (document style menu).
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(documentBack, DocumentBack)

/**
 * Move forward (document style menu).
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(documentForward, DocumentForward)

/**
 * Add the current page to the bookmarks tree.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(addBookmark, AddBookmark)

/**
 * Edit the application bookmarks.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(editBookmarks, EditBookmarks)

/**
 * Pop up the spell checker.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(spelling, Spelling)

/**
 * Display the configure key bindings dialog.
 *
 * Note that you might be able to use the pre-built KXMLGUIFactory's function:
 * @code
 * KStandardActions::keyBindings(guiFactory(), &KXMLGUIFactory::showConfigureShortcutsDialog, actionCollection());
 * @endcode
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(keyBindings, KeyBindings)

/**
 * Display the preferences/options dialog.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(preferences, Preferences)

/**
 * Display the toolbar configuration dialog.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(configureToolbars, ConfigureToolbars)

/**
 * Display the notifications configuration dialog.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(configureNotifications, ConfigureNotifications)

/**
 * Display the Switch Application Language dialog.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(switchApplicationLanguage, SwitchApplicationLanguage)

/**
 * Display the handbook of the application.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(helpContents, HelpContents)

/**
 * Trigger the What's This cursor.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(whatsThis, WhatsThis)

/**
 * Open up the Report Bug dialog.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(reportBug, ReportBug)

/**
 * Display the application's About box.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(aboutApp, AboutApp)

/**
 * Display the About KDE dialog.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(aboutKDE, AboutKDE)

/**
 * Permanently deletes files or folders.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(deleteFile, DeleteFile)

/**
 * Renames files or folders.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(renameFile, RenameFile)

/**
 * Moves files or folders to the trash.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(moveToTrash, MoveToTrash)

/**
 * Open donation page on kde.org.
 */
KGUISTANDARDACTION_WITH_NEW_STYLE_CONNECT(donate, Donate)
}

#endif // KSTDACTION_H
