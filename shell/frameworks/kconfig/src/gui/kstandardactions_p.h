/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999, 2000 Kurt Granroth <granroth@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#pragma once

#include <QAction>
#include <QGuiApplication>

#include <KStandardActions>
#include <KStandardShortcut>

#include <string>

namespace KStandardActions
{

/**
 * Helper class for storing raw data in static tables which can be used for QString instance
 * creation at runtime without copying/converting to new memalloc'ed memory, as well as avoiding
 * that way storing the strings directly as QStrings resulting in non-constexpr init code on
 * library loading
 * @internal
 */
struct RawStringData {
    template<std::size_t StringSize>
    constexpr inline RawStringData(const char16_t (&_data)[StringSize])
        : data(_data)
        , size(std::char_traits<char16_t>::length(_data))
    {
    }
    constexpr inline RawStringData(std::nullptr_t)
    {
    }
    constexpr inline RawStringData() = default;

    inline QString toString() const
    {
        if (!data) {
            return QString();
        }

        return Qt::Literals::StringLiterals::operator""_s(data, size);
    }

private:
    const char16_t *const data = nullptr;
    const std::size_t size = 0;
};

/**
 * @internal
 */
struct KStandardActionsInfo {
    KStandardActions::StandardAction id;
    KStandardShortcut::StandardShortcut idAccel;
    const RawStringData psName;
    const char *psLabel;
    const char *psToolTip;
    const RawStringData psIconName;
};
// clang-format off
static constexpr KStandardActionsInfo g_rgActionInfo[] = {
    { New,           KStandardShortcut::New, u"file_new", QT_TRANSLATE_NOOP("KStandardActions", "&New"), QT_TRANSLATE_NOOP("KStandardActions", "Create new document"), u"document-new" },
    { Open,          KStandardShortcut::Open, u"file_open", QT_TRANSLATE_NOOP("KStandardActions", "&Open…"), QT_TRANSLATE_NOOP("KStandardActions", "Open an existing document"), u"document-open" },
    { OpenRecent,    KStandardShortcut::AccelNone, u"file_open_recent", QT_TRANSLATE_NOOP("KStandardActions", "Open &Recent"), QT_TRANSLATE_NOOP("KStandardActions", "Open a document which was recently opened"), u"document-open-recent" },
    { Save,          KStandardShortcut::Save, u"file_save", QT_TRANSLATE_NOOP("KStandardActions", "&Save"), QT_TRANSLATE_NOOP("KStandardActions", "Save document"), u"document-save" },
    { SaveAs,        KStandardShortcut::SaveAs, u"file_save_as", QT_TRANSLATE_NOOP("KStandardActions", "Save &As…"), QT_TRANSLATE_NOOP("KStandardActions", "Save document under a new name"), u"document-save-as" },
    { Revert,        KStandardShortcut::Revert, u"file_revert", QT_TRANSLATE_NOOP("KStandardActions", "Re&vert"), QT_TRANSLATE_NOOP("KStandardActions", "Revert unsaved changes made to document"), u"document-revert" },
    { Close,         KStandardShortcut::Close, u"file_close", QT_TRANSLATE_NOOP("KStandardActions", "&Close"), QT_TRANSLATE_NOOP("KStandardActions", "Close document"), u"document-close" },
    { Print,         KStandardShortcut::Print, u"file_print", QT_TRANSLATE_NOOP("KStandardActions", "&Print…"), QT_TRANSLATE_NOOP("KStandardActions", "Print document"), u"document-print" },
    { PrintPreview,  KStandardShortcut::PrintPreview, u"file_print_preview", QT_TRANSLATE_NOOP("KStandardActions", "Print Previe&w"), QT_TRANSLATE_NOOP("KStandardActions", "Show a print preview of document"), u"document-print-preview" },
    { Mail,          KStandardShortcut::Mail, u"file_mail", QT_TRANSLATE_NOOP("KStandardActions", "&Mail…"), QT_TRANSLATE_NOOP("KStandardActions", "Send document by mail"), u"mail-send" },
    { Quit,          KStandardShortcut::Quit, u"file_quit", QT_TRANSLATE_NOOP("KStandardActions", "&Quit"), QT_TRANSLATE_NOOP("KStandardActions", "Quit application"), u"application-exit" },

    { Undo,          KStandardShortcut::Undo, u"edit_undo", QT_TRANSLATE_NOOP("KStandardActions", "&Undo"), QT_TRANSLATE_NOOP("KStandardActions", "Undo last action"), u"edit-undo" },
    { Redo,          KStandardShortcut::Redo, u"edit_redo", QT_TRANSLATE_NOOP("KStandardActions", "Re&do"), QT_TRANSLATE_NOOP("KStandardActions", "Redo last undone action"), u"edit-redo" },
    { Cut,           KStandardShortcut::Cut, u"edit_cut", QT_TRANSLATE_NOOP("KStandardActions", "Cu&t"), QT_TRANSLATE_NOOP("KStandardActions", "Cut selection to clipboard"), u"edit-cut" },
    { Copy,          KStandardShortcut::Copy, u"edit_copy", QT_TRANSLATE_NOOP("KStandardActions", "&Copy"), QT_TRANSLATE_NOOP("KStandardActions", "Copy selection to clipboard"), u"edit-copy" },
    { Paste,         KStandardShortcut::Paste, u"edit_paste", QT_TRANSLATE_NOOP("KStandardActions", "&Paste"), QT_TRANSLATE_NOOP("KStandardActions", "Paste clipboard content"), u"edit-paste" },
    { Clear,         KStandardShortcut::Clear, u"edit_clear", QT_TRANSLATE_NOOP("KStandardActions", "C&lear"), {}, u"edit-clear" },
    { SelectAll,     KStandardShortcut::SelectAll, u"edit_select_all", QT_TRANSLATE_NOOP("KStandardActions", "Select &All"), {}, u"edit-select-all" },
    { Deselect,      KStandardShortcut::Deselect, u"edit_deselect", QT_TRANSLATE_NOOP("KStandardActions", "Dese&lect"), {}, u"edit-select-none" },
    { Find,          KStandardShortcut::Find, u"edit_find", QT_TRANSLATE_NOOP("KStandardActions", "&Find…"), {}, u"edit-find" },
    { FindNext,      KStandardShortcut::FindNext, u"edit_find_next", QT_TRANSLATE_NOOP("KStandardActions", "Find &Next"), {}, u"go-down-search" },
    { FindPrev,      KStandardShortcut::FindPrev, u"edit_find_prev", QT_TRANSLATE_NOOP("KStandardActions", "Find Pre&vious"), {}, u"go-up-search" },
    { Replace,       KStandardShortcut::Replace, u"edit_replace", QT_TRANSLATE_NOOP("KStandardActions", "&Replace…"), {}, u"edit-find-replace" },

    { ActualSize,    KStandardShortcut::ActualSize, u"view_actual_size", QT_TRANSLATE_NOOP("KStandardActions", "Zoom to &Actual Size"), QT_TRANSLATE_NOOP("KStandardActions", "View document at its actual size"), u"zoom-original" },
    { FitToPage,     KStandardShortcut::FitToPage, u"view_fit_to_page", QT_TRANSLATE_NOOP("KStandardActions", "&Fit to Page"), QT_TRANSLATE_NOOP("KStandardActions", "Zoom to fit page in window"), u"zoom-fit-page" },
    { FitToWidth,    KStandardShortcut::FitToWidth, u"view_fit_to_width", QT_TRANSLATE_NOOP("KStandardActions", "Fit to Page &Width"), QT_TRANSLATE_NOOP("KStandardActions", "Zoom to fit page width in window"), u"zoom-fit-width" },
    { FitToHeight,   KStandardShortcut::FitToHeight, u"view_fit_to_height", QT_TRANSLATE_NOOP("KStandardActions", "Fit to Page &Height"), QT_TRANSLATE_NOOP("KStandardActions", "Zoom to fit page height in window"), u"zoom-fit-height" },
    { ZoomIn,        KStandardShortcut::ZoomIn, u"view_zoom_in", QT_TRANSLATE_NOOP("KStandardActions", "Zoom &In"), {}, u"zoom-in" },
    { ZoomOut,       KStandardShortcut::ZoomOut, u"view_zoom_out", QT_TRANSLATE_NOOP("KStandardActions", "Zoom &Out"), {}, u"zoom-out" },
    { Zoom,          KStandardShortcut::Zoom, u"view_zoom", QT_TRANSLATE_NOOP("KStandardActions", "&Zoom…"), QT_TRANSLATE_NOOP("KStandardActions", "Select zoom level"), u"zoom" },
    { Redisplay,     KStandardShortcut::Reload, u"view_redisplay", QT_TRANSLATE_NOOP("KStandardActions", "&Refresh"), QT_TRANSLATE_NOOP("KStandardActions", "Refresh document"), u"view-refresh" },

    { Up,            KStandardShortcut::Up, u"go_up", QT_TRANSLATE_NOOP("KStandardActions", "&Up"), QT_TRANSLATE_NOOP("KStandardActions", "Go up"), u"go-up" },
    // The following three have special i18n() needs for sLabel
    { Back,          KStandardShortcut::Back, u"go_back", {}, {}, u"go-previous" },
    { Forward,       KStandardShortcut::Forward, u"go_forward", {}, {}, u"go-next" },
    { Home,          KStandardShortcut::Home, u"go_home", {}, {}, u"go-home" },
    { Prior,         KStandardShortcut::Prior, u"go_previous", QT_TRANSLATE_NOOP("KStandardActions", "&Previous Page"), QT_TRANSLATE_NOOP("KStandardActions", "Go to previous page"), u"go-previous-view-page" },
    { Next,          KStandardShortcut::Next, u"go_next", QT_TRANSLATE_NOOP("KStandardActions", "&Next Page"), QT_TRANSLATE_NOOP("KStandardActions", "Go to next page"), u"go-next-view-page" },
    { Goto,          KStandardShortcut::Goto, u"go_goto", QT_TRANSLATE_NOOP("KStandardActions", "&Go To…"), {}, {} },
    { GotoPage,      KStandardShortcut::GotoPage, u"go_goto_page", QT_TRANSLATE_NOOP("KStandardActions", "&Go to Page…"), {}, u"go-jump" },
    { GotoLine,      KStandardShortcut::GotoLine, u"go_goto_line", QT_TRANSLATE_NOOP("KStandardActions", "&Go to Line…"), {}, {} },
    { FirstPage,     KStandardShortcut::Begin, u"go_first", QT_TRANSLATE_NOOP("KStandardActions", "&First Page"), QT_TRANSLATE_NOOP("KStandardActions", "Go to first page"), u"go-first-view-page" },
    { LastPage,      KStandardShortcut::End, u"go_last", QT_TRANSLATE_NOOP("KStandardActions", "&Last Page"), QT_TRANSLATE_NOOP("KStandardActions", "Go to last page"), u"go-last-view-page" },
    { DocumentBack,  KStandardShortcut::DocumentBack, u"go_document_back", QT_TRANSLATE_NOOP("KStandardActions", "&Back"), QT_TRANSLATE_NOOP("KStandardActions", "Go back in document"), u"go-previous" },
    { DocumentForward, KStandardShortcut::DocumentForward, u"go_document_forward", QT_TRANSLATE_NOOP("KStandardActions", "&Forward"), QT_TRANSLATE_NOOP("KStandardActions", "Go forward in document"), u"go-next" },

    { AddBookmark,   KStandardShortcut::AddBookmark, u"bookmark_add", QT_TRANSLATE_NOOP("KStandardActions", "&Add Bookmark"), {}, u"bookmark-new" },
    { EditBookmarks, KStandardShortcut::EditBookmarks, u"bookmark_edit", QT_TRANSLATE_NOOP("KStandardActions", "&Edit Bookmarks…"), {}, u"bookmarks-organize" },

    { Spelling,      KStandardShortcut::Spelling, u"tools_spelling", QT_TRANSLATE_NOOP("KStandardActions", "&Spelling…"), QT_TRANSLATE_NOOP("KStandardActions", "Check spelling in document"), u"tools-check-spelling" },

    { ShowMenubar,   KStandardShortcut::ShowMenubar, u"options_show_menubar", QT_TRANSLATE_NOOP("KStandardActions", "Show &Menubar"), QT_TRANSLATE_NOOP("KStandardActions", "Show or hide menubar"), u"show-menu" },
    { ShowToolbar,   KStandardShortcut::ShowToolbar, u"options_show_toolbar", QT_TRANSLATE_NOOP("KStandardActions", "Show &Toolbar"), QT_TRANSLATE_NOOP("KStandardActions", "Show or hide toolbar"), {} },
    { ShowStatusbar, KStandardShortcut::ShowStatusbar, u"options_show_statusbar", QT_TRANSLATE_NOOP("KStandardActions", "Show St&atusbar"), QT_TRANSLATE_NOOP("KStandardActions", "Show or hide statusbar"), {} },
    { FullScreen,    KStandardShortcut::FullScreen, u"fullscreen", QT_TRANSLATE_NOOP("KStandardActions", "F&ull Screen Mode"), {}, u"view-fullscreen" },
    { KeyBindings,   KStandardShortcut::KeyBindings, u"options_configure_keybinding", QT_TRANSLATE_NOOP("KStandardActions", "Configure Keyboard S&hortcuts…"), {}, u"configure-shortcuts" },
    { Preferences,   KStandardShortcut::Preferences, u"options_configure", QT_TRANSLATE_NOOP("KStandardActions", "&Configure %1…"), {}, u"configure" },
    { ConfigureToolbars, KStandardShortcut::ConfigureToolbars, u"options_configure_toolbars", QT_TRANSLATE_NOOP("KStandardActions", "Configure Tool&bars…"), {}, u"configure-toolbars" },
    { ConfigureNotifications, KStandardShortcut::ConfigureNotifications, u"options_configure_notifications", QT_TRANSLATE_NOOP("KStandardActions", "Configure &Notifications…"), {}, u"preferences-desktop-notification" },

    // the idea here is that Contents is used in menus, and Help in dialogs, so both share the same
    // shortcut
    { HelpContents,  KStandardShortcut::Help, u"help_contents", QT_TRANSLATE_NOOP("KStandardActions", "%1 &Handbook"), {}, u"help-contents" },
    { WhatsThis,     KStandardShortcut::WhatsThis, u"help_whats_this", QT_TRANSLATE_NOOP("KStandardActions", "What's &This?"), {}, u"help-contextual" },
    { ReportBug,     KStandardShortcut::ReportBug, u"help_report_bug", QT_TRANSLATE_NOOP("KStandardActions", "&Report Bug…"), {}, u"tools-report-bug" },
    { SwitchApplicationLanguage, KStandardShortcut::SwitchApplicationLanguage, u"switch_application_language", QT_TRANSLATE_NOOP("KStandardActions", "Configure &Language…"), {}, u"preferences-desktop-locale" },
    { AboutApp,      KStandardShortcut::AccelNone, u"help_about_app", QT_TRANSLATE_NOOP("KStandardActions", "&About %1"), {}, nullptr },
    { AboutKDE,      KStandardShortcut::AccelNone, u"help_about_kde", QT_TRANSLATE_NOOP("KStandardActions", "About &KDE"), {}, u"kde" },
    { DeleteFile,    KStandardShortcut::DeleteFile, u"deletefile", QT_TRANSLATE_NOOP("KStandardActions", "&Delete"), {}, u"edit-delete" },
    { RenameFile,    KStandardShortcut::RenameFile, u"renamefile", QT_TRANSLATE_NOOP("KStandardActions", "&Rename…"), {}, u"edit-rename" },
    { MoveToTrash,   KStandardShortcut::MoveToTrash, u"movetotrash", QT_TRANSLATE_NOOP("KStandardActions", "&Move to Trash"), {}, u"trash-empty" },
    { Donate,        KStandardShortcut::Donate, u"help_donate", QT_TRANSLATE_NOOP("KStandardActions", "&Donate"), {}, u"help-donate"},
    { HamburgerMenu, KStandardShortcut::OpenMainMenu, u"hamburger_menu", QT_TRANSLATE_NOOP("KStandardActions", "Open &Menu"), {}, u"application-menu" },
};
// clang-format on

/**
 * @internal
 */
inline const KStandardActionsInfo *infoPtr(StandardAction id)
{
    for (const auto &action : g_rgActionInfo) {
        if (action.id == id) {
            return &action;
        }
    }

    return nullptr;
}

/**
 * @internal
 */
static inline QStringList internal_stdNames()
{
    QStringList result;

    for (const auto &action : g_rgActionInfo) {
        if (!QCoreApplication::translate("KStandardActions", action.psLabel).isEmpty()) {
            if (QByteArrayView(action.psLabel).contains("%1"))
            // Prevents KLocalizedString::toString() from complaining about unsubstituted placeholder.
            {
                result.append(QCoreApplication::translate("KStandardActions", action.psLabel).arg(QString()));
            } else {
                result.append(QCoreApplication::translate("KStandardActions", action.psLabel));
            }
        }
    }

    return result;
}
}
