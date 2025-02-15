/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999, 2000 Kurt Granroth <granroth@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KSTANDARDACTION_PRIVATE_H
#define KSTANDARDACTION_PRIVATE_H

#include <QAction>
#include <QApplication>

#include <KLazyLocalizedString>
#include <KLocalizedString>
#include <KStandardShortcut>

namespace KStandardAction
{
struct KStandardActionInfo {
    StandardAction id;
    KStandardShortcut::StandardShortcut idAccel;
    const char *psName;
    const KLazyLocalizedString psLabel;
    const KLazyLocalizedString psToolTip;
    const char *psIconName;
};
// clang-format off
static const KStandardActionInfo g_rgActionInfo[] = {
    { New,           KStandardShortcut::New, "file_new", kli18n("&New"), kli18n("Create new document"), "document-new" },
    { Open,          KStandardShortcut::Open, "file_open", kli18n("&Open..."), kli18n("Open an existing document"), "document-open" },
    { OpenRecent,    KStandardShortcut::AccelNone, "file_open_recent", kli18n("Open &Recent"), kli18n("Open a document which was recently opened"), "document-open-recent" },
    { Save,          KStandardShortcut::Save, "file_save", kli18n("&Save"), kli18n("Save document"), "document-save" },
    { SaveAs,        KStandardShortcut::SaveAs, "file_save_as", kli18n("Save &As..."), kli18n("Save document under a new name"), "document-save-as" },
    { Revert,        KStandardShortcut::Revert, "file_revert", kli18n("Re&vert"), kli18n("Revert unsaved changes made to document"), "document-revert" },
    { Close,         KStandardShortcut::Close, "file_close", kli18n("&Close"), kli18n("Close document"), "document-close" },
    { Print,         KStandardShortcut::Print, "file_print", kli18n("&Print..."), kli18n("Print document"), "document-print" },
    { PrintPreview,  KStandardShortcut::PrintPreview, "file_print_preview", kli18n("Print Previe&w"), kli18n("Show a print preview of document"), "document-print-preview" },
    { Mail,          KStandardShortcut::Mail, "file_mail", kli18n("&Mail..."), kli18n("Send document by mail"), "mail-send" },
    { Quit,          KStandardShortcut::Quit, "file_quit", kli18n("&Quit"), kli18n("Quit application"), "application-exit" },

    { Undo,          KStandardShortcut::Undo, "edit_undo", kli18n("&Undo"), kli18n("Undo last action"), "edit-undo" },
    { Redo,          KStandardShortcut::Redo, "edit_redo", kli18n("Re&do"), kli18n("Redo last undone action"), "edit-redo" },
    { Cut,           KStandardShortcut::Cut, "edit_cut", kli18n("Cu&t"), kli18n("Cut selection to clipboard"), "edit-cut" },
    { Copy,          KStandardShortcut::Copy, "edit_copy", kli18n("&Copy"), kli18n("Copy selection to clipboard"), "edit-copy" },
    { Paste,         KStandardShortcut::Paste, "edit_paste", kli18n("&Paste"), kli18n("Paste clipboard content"), "edit-paste" },
#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 39)
    { PasteText,     KStandardShortcut::Paste, "edit_paste", kli18n("&Paste"), kli18n("Paste clipboard content"), "edit-paste" },
#endif
    { Clear,         KStandardShortcut::Clear, "edit_clear", kli18n("C&lear"), {}, "edit-clear" },
    { SelectAll,     KStandardShortcut::SelectAll, "edit_select_all", kli18n("Select &All"), {}, "edit-select-all" },
    { Deselect,      KStandardShortcut::Deselect, "edit_deselect", kli18n("Dese&lect"), {}, "edit-select-none" },
    { Find,          KStandardShortcut::Find, "edit_find", kli18n("&Find..."), {}, "edit-find" },
    { FindNext,      KStandardShortcut::FindNext, "edit_find_next", kli18n("Find &Next"), {}, "go-down-search" },
    { FindPrev,      KStandardShortcut::FindPrev, "edit_find_prev", kli18n("Find Pre&vious"), {}, "go-up-search" },
    { Replace,       KStandardShortcut::Replace, "edit_replace", kli18n("&Replace..."), {}, "edit-find-replace" },

    { ActualSize,    KStandardShortcut::ActualSize, "view_actual_size", kli18n("Zoom to &Actual Size"), kli18n("View document at its actual size"), "zoom-original" },
    { FitToPage,     KStandardShortcut::FitToPage, "view_fit_to_page", kli18n("&Fit to Page"), kli18n("Zoom to fit page in window"), "zoom-fit-page" },
    { FitToWidth,    KStandardShortcut::FitToWidth, "view_fit_to_width", kli18n("Fit to Page &Width"), kli18n("Zoom to fit page width in window"), "zoom-fit-width" },
    { FitToHeight,   KStandardShortcut::FitToHeight, "view_fit_to_height", kli18n("Fit to Page &Height"), kli18n("Zoom to fit page height in window"), "zoom-fit-height" },
    { ZoomIn,        KStandardShortcut::ZoomIn, "view_zoom_in", kli18n("Zoom &In"), {}, "zoom-in" },
    { ZoomOut,       KStandardShortcut::ZoomOut, "view_zoom_out", kli18n("Zoom &Out"), {}, "zoom-out" },
    { Zoom,          KStandardShortcut::Zoom, "view_zoom", kli18n("&Zoom..."), kli18n("Select zoom level"), "zoom" },
    { Redisplay,     KStandardShortcut::Reload, "view_redisplay", kli18n("&Refresh"), kli18n("Refresh document"), "view-refresh" },

    { Up,            KStandardShortcut::Up, "go_up", kli18n("&Up"), kli18n("Go up"), "go-up" },
    // The following three have special i18n() needs for sLabel
    { Back,          KStandardShortcut::Back, "go_back", {}, {}, "go-previous" },
    { Forward,       KStandardShortcut::Forward, "go_forward", {}, {}, "go-next" },
    { Home,          KStandardShortcut::Home, "go_home", {}, {}, "go-home" },
    { Prior,         KStandardShortcut::Prior, "go_previous", kli18n("&Previous Page"), kli18n("Go to previous page"), "go-previous-view-page" },
    { Next,          KStandardShortcut::Next, "go_next", kli18n("&Next Page"), kli18n("Go to next page"), "go-next-view-page" },
    { Goto,          KStandardShortcut::Goto, "go_goto", kli18n("&Go To..."), {}, nullptr },
    { GotoPage,      KStandardShortcut::GotoPage, "go_goto_page", kli18n("&Go to Page..."), {}, "go-jump" },
    { GotoLine,      KStandardShortcut::GotoLine, "go_goto_line", kli18n("&Go to Line..."), {}, nullptr },
    { FirstPage,     KStandardShortcut::Begin, "go_first", kli18n("&First Page"), kli18n("Go to first page"), "go-first-view-page" },
    { LastPage,      KStandardShortcut::End, "go_last", kli18n("&Last Page"), kli18n("Go to last page"), "go-last-view-page" },
    { DocumentBack,  KStandardShortcut::DocumentBack, "go_document_back", kli18n("&Back"), kli18n("Go back in document"), "go-previous" },
    { DocumentForward, KStandardShortcut::DocumentForward, "go_document_forward", kli18n("&Forward"), kli18n("Go forward in document"), "go-next" },

    { AddBookmark,   KStandardShortcut::AddBookmark, "bookmark_add", kli18n("&Add Bookmark"), {}, "bookmark-new" },
    { EditBookmarks, KStandardShortcut::EditBookmarks, "bookmark_edit", kli18n("&Edit Bookmarks..."), {}, "bookmarks-organize" },

    { Spelling,      KStandardShortcut::Spelling, "tools_spelling", kli18n("&Spelling..."), kli18n("Check spelling in document"), "tools-check-spelling" },

    { ShowMenubar,   KStandardShortcut::ShowMenubar, "options_show_menubar", kli18n("Show &Menubar"), kli18n("Show or hide menubar"), "show-menu" },
    { ShowToolbar,   KStandardShortcut::ShowToolbar, "options_show_toolbar", kli18n("Show &Toolbar"), kli18n("Show or hide toolbar"), nullptr },
    { ShowStatusbar, KStandardShortcut::ShowStatusbar, "options_show_statusbar", kli18n("Show St&atusbar"), kli18n("Show or hide statusbar"), nullptr },
    { FullScreen,    KStandardShortcut::FullScreen, "fullscreen", kli18n("F&ull Screen Mode"), {}, "view-fullscreen" },
#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 38)
    { SaveOptions,   KStandardShortcut::SaveOptions_DEPRECATED_DO_NOT_USE, "options_save_options", kli18n("&Save Settings"), {}, nullptr },
#endif
    { KeyBindings,   KStandardShortcut::KeyBindings, "options_configure_keybinding", kli18n("Configure Keyboard S&hortcuts..."), {}, "configure-shortcuts" },
    { Preferences,   KStandardShortcut::Preferences, "options_configure", kli18n("&Configure %1..."), {}, "configure" },
    { ConfigureToolbars, KStandardShortcut::ConfigureToolbars, "options_configure_toolbars", kli18n("Configure Tool&bars..."), {}, "configure-toolbars" },
    { ConfigureNotifications, KStandardShortcut::ConfigureNotifications, "options_configure_notifications", kli18n("Configure &Notifications..."), {}, "preferences-desktop-notification" },

    // the idea here is that Contents is used in menus, and Help in dialogs, so both share the same
    // shortcut
#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 38)
    { Help,          KStandardShortcut::Help, "help", {}, {}, "help-contents" },
#endif
    { HelpContents,  KStandardShortcut::Help, "help_contents", kli18n("%1 &Handbook"), {}, "help-contents" },
    { WhatsThis,     KStandardShortcut::WhatsThis, "help_whats_this", kli18n("What's &This?"), {}, "help-contextual" },
    { TipofDay,      KStandardShortcut::TipofDay, "help_show_tip", kli18n("Tip of the &Day"), {}, "help-hint" },
    { ReportBug,     KStandardShortcut::ReportBug, "help_report_bug", kli18n("&Report Bug..."), {}, "tools-report-bug" },
    { SwitchApplicationLanguage, KStandardShortcut::SwitchApplicationLanguage, "switch_application_language", kli18n("Configure &Language..."), {}, "preferences-desktop-locale" },
    { AboutApp,      KStandardShortcut::AccelNone, "help_about_app", kli18n("&About %1"), {}, nullptr },
    { AboutKDE,      KStandardShortcut::AccelNone, "help_about_kde", kli18n("About &KDE"), {}, "kde" },
    { DeleteFile,    KStandardShortcut::DeleteFile, "deletefile", kli18n("&Delete"), {}, "edit-delete" },
    { RenameFile,    KStandardShortcut::RenameFile, "renamefile", kli18n("&Rename..."), {}, "edit-rename" },
    { MoveToTrash,   KStandardShortcut::MoveToTrash, "movetotrash", kli18n("&Move to Trash"), {}, "trash-empty" },
    { Donate,        KStandardShortcut::Donate, "help_donate", kli18n("&Donate"), {}, "help-donate"},
    { HamburgerMenu, KStandardShortcut::AccelNone, "hamburger_menu", kli18n("Open &Menu"), {}, "application-menu" },
    { ActionNone,    KStandardShortcut::AccelNone, nullptr, {}, {}, nullptr }
};
// clang-format on

inline const KStandardActionInfo *infoPtr(StandardAction id)
{
    for (uint i = 0; g_rgActionInfo[i].id != ActionNone; i++) {
        if (g_rgActionInfo[i].id == id) {
            return &g_rgActionInfo[i];
        }
    }

    return nullptr;
}

static inline QStringList internal_stdNames()
{
    QStringList result;

    for (uint i = 0; g_rgActionInfo[i].id != ActionNone; i++)
        if (!g_rgActionInfo[i].psLabel.isEmpty()) {
            if (QByteArray(g_rgActionInfo[i].psLabel.untranslatedText()).contains("%1"))
            // Prevents KLocalizedString::toString() from complaining about unsubstituted placeholder.
            {
                result.append(g_rgActionInfo[i].psLabel.subs(QString()).toString());
            } else {
                result.append(g_rgActionInfo[i].psLabel.toString());
            }
        }

    return result;
}

class AutomaticAction : public QAction
{
    Q_OBJECT

public:
    AutomaticAction(const QIcon &icon, const QString &text, KStandardShortcut::StandardShortcut standardShortcut, const char *slot, QObject *parent);
public Q_SLOTS:
    inline void cut()
    {
        invokeEditSlot("cut");
    }
    inline void copy()
    {
        invokeEditSlot("copy");
    }
    inline void paste()
    {
        invokeEditSlot("paste");
    }
    inline void clear()
    {
        invokeEditSlot("clear");
    }
    inline void selectAll()
    {
        invokeEditSlot("selectAll");
    }

    void invokeEditSlot(const char *slot)
    {
        if (qApp->focusWidget()) {
            QMetaObject::invokeMethod(qApp->focusWidget(), slot);
        }
    }
};

}

#endif
