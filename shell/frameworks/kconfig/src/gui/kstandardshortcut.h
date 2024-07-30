/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1997 Stefan Taferner <taferner@kde.org>
    SPDX-FileCopyrightText: 2000 Nicolas Hadacek <hadacek@kde.org>
    SPDX-FileCopyrightText: 2001, 2002 Ellis Whitehead <ellis@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KSTANDARDSHORTCUT_H
#define KSTANDARDSHORTCUT_H

#include <QKeySequence>
#include <QString>

#include <kconfiggui_export.h>

/**
 * \namespace KStandardShortcut
 * Convenient methods for access to the common accelerator keys in
 * the key configuration. These are the standard keybindings that should
 * be used in all KDE applications. They will be configurable,
 * so do not hardcode the default behavior.
 */
namespace KStandardShortcut
{
// STUFF WILL BREAK IF YOU DON'T READ THIS!!!
/*
 *Always add new std-accels to the end of this enum, never in the middle!
 *Don't forget to add the corresponding entries in g_infoStandardShortcut[] in kstandardshortcut.cpp, too.
 *Values of elements here and positions of the corresponding entries in
 *the big array g_infoStandardShortcut[] ABSOLUTELY MUST BE THE SAME.
 * !!!    !!!!   !!!!!    !!!!
 *    !!!!    !!!     !!!!    !!!!
 *
 * Other Rules:
 *
 * - Never change the name of an existing shortcut
 * - Never translate the name of a shortcut
 */

/**
 * Defines the identifier of all standard accelerators.
 */
enum StandardShortcut {
    // C++ requires that the value of an enum symbol be one more than the previous one.
    // This means that everything will be well-ordered from here on.
    AccelNone = 0,
    // File menu
    Open, ///< Open file.
    New, ///< Create a new document.
    Close, ///< Close current document.
    Save, ///< Save current document.
    // The Print item
    Print, ///< Print current document.
    Quit, ///< Quit the program.
    // Edit menu
    Undo, ///< Undo last operation.
    Redo, ///< Redo last operation.
    Cut, ///< Cut selected area and store it in the clipboard.
    Copy, ///< Copy selected area into the clipboard.
    Paste, ///< Paste contents of clipboard at mouse/cursor position.
    PasteSelection, ///< Paste the selection at mouse/cursor position.
    SelectAll, ///< Select all.
    Deselect, ///< Deselect any selected elements.
    DeleteWordBack, ///< Delete a word back from mouse/cursor position.
    DeleteWordForward, ///< Delete a word forward from mouse/cursor position.
    Find, ///< Initiate a 'find' request in the current document.
    FindNext, ///< Find the next instance of a stored 'find'.
    FindPrev, ///< Find a previous instance of a stored 'find'.
    Replace, ///< Find and replace matches.
    // Navigation
    Home, ///< Go to home page.
    Begin, ///< Go to beginning of the document.
    End, ///< Go to end of the document.
    Prior, ///< Scroll up one page.
    Next, ///< Scroll down one page.
    Up, ///< Up.
    Back, ///< Back.
    Forward, ///< Forward.
    Reload, ///< Reload.
    // Text Navigation
    BeginningOfLine, ///< Go to beginning of current line.
    EndOfLine, ///< Go to end of current line.
    GotoLine, ///< Go to line.
    BackwardWord, ///< BackwardWord.
    ForwardWord, ///< ForwardWord.
    // View parameters
    AddBookmark, ///< Add current page to bookmarks.
    ZoomIn, ///< Zoom in.
    ZoomOut, ///< Zoom out.
    FullScreen, ///< Full Screen mode.
    ShowMenubar, ///< Show Menu Bar.
    // Tabular navigation
    TabNext, ///< Next Tab.
    TabPrev, ///< Previous Tab.
    // Help menu
    Help, ///< Help the user in the current situation.
    WhatsThis, ///< What's This button.
    // Text completion
    TextCompletion, ///< Complete text in input widgets.
    PrevCompletion, ///< Iterate through a list when completion returns multiple items.
    NextCompletion, ///< Iterate through a list when completion returns multiple items.
    SubstringCompletion, ///< Find a string within another string or list of strings.
    RotateUp, ///< Help users iterate through a list of entries.
    RotateDown, ///< Help users iterate through a list of entries.
    OpenRecent, ///< Open a recently used document.
    SaveAs, ///< Save the current document under a different name.
    Revert, ///< Revert the current document to the last saved version.
    PrintPreview, ///< Show a print preview of the current document.
    Mail, ///< Send the current document by mail.
    Clear, ///< Clear the content of the focus widget.
    ActualSize, ///< View the document at its actual size.
    FitToPage, ///< Fit the document view to the size of the current window.
    FitToWidth, ///< Fit the document view to the width of the current window.
    FitToHeight, ///< Fit the document view to the height of the current window.
    Zoom, ///< Select the current zoom level.
    Goto, ///< Jump to some specific location in the document.
    GotoPage, ///< Go to a specific page.
    DocumentBack, ///< Move back (document style menu).
    DocumentForward, ///< Move forward (document style menu).
    EditBookmarks, ///< Edit the application bookmarks.
    Spelling, ///< Pop up the spell checker.
    ShowToolbar, ///< Show/Hide the toolbar.
    ShowStatusbar, ///< Show/Hide the statusbar.
    KeyBindings, ///< Display the configure key bindings dialog.
    Preferences, ///< Display the preferences/options dialog.
    ConfigureToolbars, ///< Display the toolbar configuration dialog.
    ConfigureNotifications, ///< Display the notifications configuration dialog.
    ReportBug, ///< Display the Report Bug dialog.
    SwitchApplicationLanguage, ///< Display the Switch Application Language dialog.
    AboutApp, ///< Display the application's About dialog.
    AboutKDE, ///< Display the About KDE dialog.
    DeleteFile, ///< Permanently delete files or folders. @since 5.25
    RenameFile, ///< Rename files or folders. @since 5.25
    MoveToTrash, ///< Move files or folders to the trash. @since 5.25
    Donate, ///< Open donation page on kde.org. @since 5.26
    ShowHideHiddenFiles, ///< Toggle showing or hiding hidden files @since 5.70
    CreateFolder, ///< Create a folder. @since 5.74
    OpenMainMenu, ///< Open a main menu like the menu bar or a hamburger menu. Necessary for accessibility. @since 6.0
    OpenContextMenu, ///< Open a context menu for the object with keyboard focus. Necessary for accessibility. @since 6.0
    // Insert new items here!

    StandardShortcutCount, // number of standard shortcuts
};

/**
 * Categories in which the standard shortcuts can be classified
 * @since 5.74
 */
enum class Category {
    InvalidCategory = -1,
    File,
    Edit,
    Navigation,
    View,
    Settings,
    Help,
};

/**
 * Returns the keybinding for @p accel.
 * On X11, if QApplication was initialized with GUI disabled, the
 * default keybinding will always be returned.
 * @param id the id of the accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &shortcut(StandardShortcut id);

/**
 * Returns a unique name for the given accel.
 * @param id the id of the accelerator
 * @return the unique name of the accelerator
 */
KCONFIGGUI_EXPORT QString name(StandardShortcut id);

/**
 * Returns a localized label for user-visible display.
 * @param id the id of the accelerator
 * @return a localized label for the accelerator
 */
KCONFIGGUI_EXPORT QString label(StandardShortcut id);

/**
 * Returns an extended WhatsThis description for the given accelerator.
 * @param id the id of the accelerator
 * @return a localized description of the accelerator
 */
KCONFIGGUI_EXPORT QString whatsThis(StandardShortcut id);

/**
 * Return the StandardShortcut id of the standard accel action which
 * uses this key sequence, or AccelNone if none of them do.
 * This is used by class KKeyChooser.
 * @param keySeq the key sequence to search
 * @return the id of the standard accelerator, or AccelNone if there
 *          is none
 */
KCONFIGGUI_EXPORT StandardShortcut find(const QKeySequence &keySeq);

/**
 * Return the StandardShortcut id of the standard accelerator action which
 * has \p name as its name, or AccelNone if none of them do.
 * @param name the name as returned by  name(StandardShortcut id)
 * @return the id of the standard accelerator with the given name or AccelNone
 * if there is no such accelerator
 * @since 5.71
 */
KCONFIGGUI_EXPORT StandardShortcut findByName(const QString &name);

/**
 * Returns the hardcoded default shortcut for @p id.
 * This does not take into account the user's configuration.
 * @param id the id of the accelerator
 * @return the default shortcut of the accelerator
 */
KCONFIGGUI_EXPORT QList<QKeySequence> hardcodedDefaultShortcut(StandardShortcut id);

/**
 * Saves the new shortcut \a cut for standard accel \a id.
 */
KCONFIGGUI_EXPORT void saveShortcut(StandardShortcut id, const QList<QKeySequence> &newShortcut);

/**
 * Returns the appropriate category for the given StandardShortcut \p id.
 * @since 5.73
 */
KCONFIGGUI_EXPORT Category category(StandardShortcut id);

/**
 * Open file. Default: Ctrl-o
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &open();

/**
 * Create a new document (or whatever). Default: Ctrl-n
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &openNew();

/**
 * Close current document. Default: Ctrl-w
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &close();

/**
 * Save current document. Default: Ctrl-s
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &save();

/**
 * Print current document. Default: Ctrl-p
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &print();

/**
 * Quit the program. Default: Ctrl-q
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &quit();

/**
 * Undo last operation. Default: Ctrl-z
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &undo();

/**
 * Redo last operation. Default: Shift-Ctrl-z
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &redo();

/**
 * Cut selected area and store it in the clipboard. Default: Ctrl-x
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &cut();

/**
 * Copy selected area into the clipboard. Default: Ctrl-c
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &copy();

/**
 * Paste contents of clipboard at mouse/cursor position. Default: Ctrl-v
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &paste();

/**
 * Paste the selection at mouse/cursor position. Default: Ctrl-Shift-Insert
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &pasteSelection();

/**
 * Select all. Default: Ctrl-A
 * @return the shortcut of the standard accelerator
 **/
KCONFIGGUI_EXPORT const QList<QKeySequence> &selectAll();

/**
 * Delete a word back from mouse/cursor position. Default: Ctrl-Backspace
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &deleteWordBack();

/**
 * Delete a word forward from mouse/cursor position. Default: Ctrl-Delete
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &deleteWordForward();

/**
 * Initiate a 'find' request in the current document. Default: Ctrl-f
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &find();

/**
 * Find the next instance of a stored 'find' Default: F3
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &findNext();

/**
 * Find a previous instance of a stored 'find'. Default: Shift-F3
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &findPrev();

/**
 * Find and replace matches. Default: Ctrl-r
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &replace();

/**
 * Zoom in. Default: Ctrl-Plus
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &zoomIn();

/**
 * Zoom out. Default: Ctrl-Minus
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &zoomOut();

/**
 * Go to home page. Default: Alt-Home
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &home();

/**
 * Go to beginning of the document. Default: Ctrl-Home
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &begin();

/**
 * Go to end of the document. Default: Ctrl-End
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &end();

/**
 * Go to beginning of current line. Default: Home
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &beginningOfLine();

/**
 * Go to end of current line. Default: End
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &endOfLine();

/**
 * Scroll up one page. Default: Prior
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &prior();

/**
 * Scroll down one page. Default: Next
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &next();

/**
 * Go to line. Default: Ctrl+G
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &gotoLine();

/**
 * Add current page to bookmarks. Default: Ctrl+B
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &addBookmark();

/**
 * Next Tab. Default: Ctrl-<
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &tabNext();

/**
 * Previous Tab. Default: Ctrl->
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &tabPrev();

/**
 * Full Screen Mode. Default: Ctrl+Shift+F
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &fullScreen();

/**
 * Help the user in the current situation. Default: F1
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &help();

/**
 * Complete text in input widgets. Default Ctrl+E
 * @return the shortcut of the standard accelerator
 **/
KCONFIGGUI_EXPORT const QList<QKeySequence> &completion();

/**
 * Iterate through a list when completion returns
 * multiple items. Default: Ctrl+Up
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &prevCompletion();

/**
 * Iterate through a list when completion returns
 * multiple items. Default: Ctrl+Down
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &nextCompletion();

/**
 * Find a string within another string or list of strings.
 * Default: Ctrl-T
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &substringCompletion();

/**
 * Help users iterate through a list of entries. Default: Up
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &rotateUp();

/**
 * Help users iterate through a list of entries. Default: Down
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &rotateDown();

/**
 * What's This button. Default: Shift+F1
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &whatsThis();

/**
 * Reload. Default: F5
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &reload();

/**
 * Up. Default: Alt+Up
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &up();

/**
 * Back. Default: Alt+Left
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &back();

/**
 * Forward. Default: ALT+Right
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &forward();

/**
 * BackwardWord. Default: Ctrl+Left
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &backwardWord();

/**
 * ForwardWord. Default: Ctrl+Right
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &forwardWord();

/**
 * Show Menu Bar.  Default: Ctrl-M
 * @return the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &showMenubar();

/**
 * Permanently delete files or folders. Default: Shift+Delete
 * @return the shortcut of the standard accelerator
 * @since 5.25
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &deleteFile();

/**
 * Rename files or folders. Default: F2
 * @return the shortcut of the standard accelerator
 * @since 5.25
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &renameFile();

/**
 * Create a folder. Default: Ctrl+Shift+N
 * @return the shortcut of the standard accelerator
 * @since 5.74
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &createFolder();

/**
 * Moves files or folders to the trash. Default: Delete
 * @return the shortcut of the standard accelerator
 * @since 5.25
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &moveToTrash();

/**
 * Opens the app's settings window. Default: Ctrl+Shift+Comma
 * @return the shortcut of the standard accelerator
 * @since 5.64
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &preferences();

/**
 * Shows or hides hidden files. Defaults: Ctrl+H, Alt+.
 * @return the shortcut of the standard accelerator
 * @since 5.70
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &showHideHiddenFiles();

/**
 * Open a main menu like the menu bar or a hamburger menu.
 * Necessary for accessibility. Default: F10
 * @return the shortcut of the standard accelerator
 * @since 6.0
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &openMainMenu();

/**
 * Open a context menu for the object with keyboard focus.
 * Necessary for accessibility. Default: Menu, Shift+F10
 * @return the shortcut of the standard accelerator
 * @since 6.0
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &openContextMenu();
}

#endif // KSTANDARDSHORTCUT_H
