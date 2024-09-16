// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DSettings>

void GenerateSettingTranslate()
{
    auto group_baseName = QObject::tr("Basic");
    auto base_fontName = QObject::tr("Font Style");
    auto base_font_familyName = QObject::tr("Font");
    auto base_font_sizeName = QObject::tr("Font Size");
    auto group_shortcutsName = QObject::tr("Shortcuts");
    auto shortcuts_keymapName = QObject::tr("Keymap");
    auto shortcurs_keymap_keymap = QObject::tr("Keymap");
    auto shortcuts_windowName = QObject::tr("Window");
    auto shortcuts_window_addblanktabName = QObject::tr("New tab");
    auto shortcuts_window_newwindowName = QObject::tr("New window");
    auto shortcuts_window_savefileName = QObject::tr("Save");
    auto shortcuts_window_saveasfileName = QObject::tr("Save as");
    auto shortcuts_window_selectnexttabName = QObject::tr("Next tab");
    auto shortcuts_window_selectprevtabName = QObject::tr("Previous tab");
    auto shortcuts_window_closetabName = QObject::tr("Close tab");
    auto shortcuts_window_closeothertabsName = QObject::tr("Close other tabs");
    auto shortcuts_window_restoretabName = QObject::tr("Restore tab");
    auto shortcuts_window_openfileName = QObject::tr("Open file");
    auto shortcuts_window_incrementfontsizeName = QObject::tr("Increment font size");
    auto shortcuts_window_decrementfontsizeName = QObject::tr("Decrement font size");
    auto shortcuts_window_resetfontsizeName = QObject::tr("Reset font size");
    auto shortcuts_window_help = QObject::tr("Help");
    auto shortcuts_window_togglefullscreenName = QObject::tr("Toggle fullscreen");
    auto shortcuts_window_findName = QObject::tr("Find");
    Q_UNUSED(QObject::tr("Find Next"));
    Q_UNUSED(QObject::tr("Find Previous"));
    auto shortcuts_window_replaceName = QObject::tr("Replace");
    auto shortcuts_window_jumptolineName = QObject::tr("Go to line");
    auto shortcuts_window_savepositionName = QObject::tr("Save cursor position");
    auto shortcuts_window_restorepositionName = QObject::tr("Reset cursor position");
    auto shortcuts_window_escapeName = QObject::tr("Exit");
    auto shortcuts_window_displayshortcutsName = QObject::tr("Display shortcuts");
    auto shortcuts_window_printName = QObject::tr("Print");
    auto group_editorName = QObject::tr("Editor");
    auto shortcuts_editor_indentlineName = QObject::tr("Increase indent");
    auto shortcuts_editor_backindentlineName = QObject::tr("Decrease indent");
    auto shortcuts_editor_forwardcharName = QObject::tr("Forward character");
    auto shortcuts_editor_backwardcharName = QObject::tr("Backward character");
    auto shortcuts_editor_forwardwordName = QObject::tr("Forward word");
    auto shortcuts_editor_backwarwordName = QObject::tr("Backward word");
    auto shortcuts_editor_nextlineName = QObject::tr("Next line");
    auto shortcuts_editor_prevlineName = QObject::tr("Previous line");
    auto shortcuts_editor_newlineName = QObject::tr("New line");
    auto shortcuts_editor_opennewlineaboveName = QObject::tr("New line above");
    auto shortcuts_editor_opennewlinebelowName = QObject::tr("New line below");
    auto shortcuts_editor_duplicatelineName = QObject::tr("Duplicate line");
    auto shortcuts_editor_killlineName = QObject::tr("Delete to end of line");
    auto shortcuts_editor_killcurrentlineName = QObject::tr("Delete current line");
    auto shortcuts_editor_swaplineupName = QObject::tr("Swap line up");
    auto shortcuts_editor_swaplinedownName = QObject::tr("Swap line down");
    auto shortcuts_editor_scrolllineupName = QObject::tr("Scroll up one line");
    auto shortcuts_editor_scrolllinedownName = QObject::tr("Scroll down one line");
    auto shortcuts_editor_scrollupName = QObject::tr("Page up");
    auto shortcuts_editor_scrolldownName = QObject::tr("Page down");
    auto shortcuts_editor_movetoendoflineName = QObject::tr("Move to end of line");
    auto shortcuts_editor_movetostartoflineName = QObject::tr("Move to start of line");
    auto shortcuts_editor_movetoendName = QObject::tr("Move to end of text");
    auto shortcuts_editor_movetostartName = QObject::tr("Move to start of text");
    auto shortcuts_editor_movetolineindentationName = QObject::tr("Move to line indentation");
    auto shortcuts_editor_upcasewordName = QObject::tr("Upper case");
    auto shortcuts_editor_downcasewordName = QObject::tr("Lower case");
    auto shortcuts_editor_capitalizewordName = QObject::tr("Capitalize");
    auto shortcuts_editor_killbackwardwordName = QObject::tr("Delete backward word");
    auto shortcuts_editor_killforwardwordName = QObject::tr("Delete forward word");
    auto shortcuts_editor_forwardpairName = QObject::tr("Forward over a pair");
    auto shortcuts_editor_backwardpairName = QObject::tr("Backward over a pair");
    auto shortcuts_editor_selectallName = QObject::tr("Select all");
    auto shortcuts_editor_copyName = QObject::tr("Copy");
    auto shortcuts_editor_cutName = QObject::tr("Cut");
    auto shortcuts_editor_pasteName = QObject::tr("Paste");
    auto shortcuts_editor_transposecharName = QObject::tr("Transpose character");
    auto shortcuts_editor_setmarkName = QObject::tr("Mark");
    auto shortcuts_editor_exchangemarkName = QObject::tr("Unmark");
    auto shortcuts_editor_copylinesName = QObject::tr("Copy line");
    auto shortcuts_editor_cutlinesName = QObject::tr("Cut line");
    auto shortcuts_editor_joinlinesName = QObject::tr("Merge lines");
    auto shortcuts_editor_togglereadonlymodeName = QObject::tr("Read-Only mode");
    auto shortcuts_editor_togglecommentName = QObject::tr("Add comment");
    auto shortcuts_editor_removecommentName = QObject::tr("Remove comment");
    auto shortcuts_editor_undoName = QObject::tr("Undo");
    auto shortcuts_editor_redoName = QObject::tr("Redo");
    auto shortcuts_editor_switchbookmarkName = QObject::tr("Add/Remove bookmark");
    auto shortcuts_editor_movetoprebookmarkName = QObject::tr("Move to previous bookmark");
    auto shortcuts_editor_movetonextbookmarkName = QObject::tr("Move to next bookmark");
    auto group_advanceName = QObject::tr("Advanced");
    auto advance_windowName = QObject::tr("Window");
    auto advance_window_windowstateName = QObject::tr("Window size");
    auto advance_editor_tabspacenumberName = QObject::tr("Tab width");
    auto allow_midbutton_paste_name = QObject::tr("Paste by pressing a middle mouse button");

    auto advance_startup_name = QObject::tr("Startup");
    auto advance_startup_saveTabBeforeCloseName = QObject::tr("Reopen last closed tabs");

    auto advance_open_save_setting_name = QObject::tr("Open/Save Settings");

    auto base_font_wordwrapText = QObject::tr("Word wrap");
    auto base_font_codeflod = QObject::tr("Code folding flag");
    auto base_font_showLineNumber = QObject::tr("Show line numbers");
    auto base_font_showBookmark = QObject::tr("Show bookmarks icon");
    auto showblankcharacter = QObject::tr("Show whitespaces and tabs");
    auto base_font_highlightCurrentLine = QObject::tr("Highlight current line");
    auto shortcuts_editor_markName = QObject::tr("Color mark");

    //编码翻译
    auto Unicode = QObject::tr("Unicode");
    auto WesternEuropean = QObject::tr("WesternEuropean");
    auto CentralEuropean = QObject::tr("CentralEuropean");
    auto Baltic = QObject::tr("Baltic");
    auto Cyrillic = QObject::tr("Cyrillic");
    auto Arabic = QObject::tr("Arabic");
    auto Greek = QObject::tr("Greek");
    auto Hebrew = QObject::tr("Hebrew");
    auto Turkish = QObject::tr("Turkish");
    auto Thai = QObject::tr("Thai");
    auto Celtic = QObject::tr("Celtic");
    auto SouthEasternEuropean = QObject::tr("SouthEasternEuropean");
    auto Simplified = QObject::tr("ChineseSimplified");
    auto Traditional = QObject::tr("ChineseTraditional");
    auto Japanese = QObject::tr("Japanese");
    auto Korean = QObject::tr("Korean");
    auto Vietnamese = QObject::tr("Vietnamese");
}
