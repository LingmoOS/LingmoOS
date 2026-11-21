// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "KeyCodes.h"

#include <algorithm>
#include <vector>

#include <unicode/uchar.h>
#include <xkbcommon/xkbcommon.h>

#include "base/strings.h"

namespace Konsole
{

struct KeyName
{
    int key;
    const char* name;
};

// QKeySequence's untranslated key names of one word, in its order.
static const KeyName keyNames[] = {
    {Key_Space, "Space"},
    {Key_Escape, "Esc"},
    {Key_Tab, "Tab"},
    {Key_Backtab, "Backtab"},
    {Key_Backspace, "Backspace"},
    {Key_Return, "Return"},
    {Key_Enter, "Enter"},
    {Key_Insert, "Ins"},
    {Key_Delete, "Del"},
    {Key_Pause, "Pause"},
    {Key_Print, "Print"},
    {Key_SysReq, "SysReq"},
    {Key_Home, "Home"},
    {Key_End, "End"},
    {Key_Left, "Left"},
    {Key_Up, "Up"},
    {Key_Right, "Right"},
    {Key_Down, "Down"},
    {Key_PageUp, "PgUp"},
    {Key_PageDown, "PgDown"},
    {Key_CapsLock, "CapsLock"},
    {Key_NumLock, "NumLock"},
    {Key_ScrollLock, "ScrollLock"},
    {Key_Menu, "Menu"},
    {Key_Help, "Help"},
    {Key_Back, "Back"},
    {Key_Forward, "Forward"},
    {Key_Stop, "Stop"},
    {Key_Refresh, "Refresh"},
    {Key_Favorites, "Favorites"},
    {Key_Search, "Search"},
    {Key_Standby, "Standby"},
    {Key_Eject, "Eject"},
    {Key_ScreenSaver, "Screensaver"},
    {Key_WWW, "WWW"},
    {Key_Sleep, "Sleep"},
    {Key_LightBulb, "LightBulb"},
    {Key_Shop, "Shop"},
    {Key_History, "History"},
    {Key_Finance, "Finance"},
    {Key_Community, "Community"},
    {Key_Book, "Book"},
    {Key_CD, "CD"},
    {Key_Calculator, "Calculator"},
    {Key_Calendar, "Calendar"},
    {Key_Clear, "Clear"},
    {Key_Close, "Close"},
    {Key_Copy, "Copy"},
    {Key_Cut, "Cut"},
    {Key_Display, "Display"},
    {Key_DOS, "DOS"},
    {Key_Documents, "Documents"},
    {Key_Excel, "Spreadsheet"},
    {Key_Explorer, "Browser"},
    {Key_Game, "Game"},
    {Key_Go, "Go"},
    {Key_iTouch, "iTouch"},
    {Key_LogOff, "Logoff"},
    {Key_Market, "Market"},
    {Key_Meeting, "Meeting"},
    {Key_Memo, "Memo"},
    {Key_News, "News"},
    {Key_Option, "Option"},
    {Key_Paste, "Paste"},
    {Key_Phone, "Phone"},
    {Key_Reply, "Reply"},
    {Key_Reload, "Reload"},
    {Key_Save, "Save"},
    {Key_Send, "Send"},
    {Key_Spell, "Spellchecker"},
    {Key_Support, "Support"},
    {Key_Terminal, "Terminal"},
    {Key_Tools, "Tools"},
    {Key_Travel, "Travel"},
    {Key_Video, "Video"},
    {Key_Xfer, "XFer"},
    {Key_Away, "Away"},
    {Key_Messenger, "Messenger"},
    {Key_WebCam, "WebCam"},
    {Key_Pictures, "Pictures"},
    {Key_Music, "Music"},
    {Key_Battery, "Battery"},
    {Key_Bluetooth, "Bluetooth"},
    {Key_WLAN, "Wireless"},
    {Key_Subtitle, "Subtitle"},
    {Key_Time, "Time"},
    {Key_Hibernate, "Hibernate"},
    {Key_View, "View"},
    {Key_Suspend, "Suspend"},
    {Key_Red, "Red"},
    {Key_Green, "Green"},
    {Key_Yellow, "Yellow"},
    {Key_Blue, "Blue"},
    {Key_Guide, "Guide"},
    {Key_Info, "Info"},
    {Key_Settings, "Settings"},
    {Key_New, "New"},
    {Key_Open, "Open"},
    {Key_Find, "Find"},
    {Key_Undo, "Undo"},
    {Key_Redo, "Redo"},
    {Key_Insert, "Insert"},
    {Key_Delete, "Delete"},
    {Key_Escape, "Escape"},
    {Key_Select, "Select"},
    {Key_Yes, "Yes"},
    {Key_No, "No"},
    {Key_Context1, "Context1"},
    {Key_Context2, "Context2"},
    {Key_Context3, "Context3"},
    {Key_Context4, "Context4"},
    {Key_Call, "Call"},
    {Key_Hangup, "Hangup"},
    {Key_Flip, "Flip"},
    {Key_Kanji, "Kanji"},
    {Key_Muhenkan, "Muhenkan"},
    {Key_Henkan, "Henkan"},
    {Key_Romaji, "Romaji"},
    {Key_Hiragana, "Hiragana"},
    {Key_Katakana, "Katakana"},
    {Key_Zenkaku, "Zenkaku"},
    {Key_Hankaku, "Hankaku"},
    {Key_Touroku, "Touroku"},
    {Key_Massyo, "Massyo"},
    {Key_Hangul, "Hangul"},
    {Key_Cancel, "Cancel"},
    {Key_Printer, "Printer"},
    {Key_Execute, "Execute"},
    {Key_Play, "Play"},
    {Key_Zoom, "Zoom"},
    {Key_Exit, "Exit"},
    {Key_Shift, "Shift"},
    {Key_Control, "Control"},
    {Key_Alt, "Alt"},
    {Key_Meta, "Meta"},
};

struct KeysymKey
{
    uint32_t keysym;
    int key;
};

// QXkbCommon's table, in its order; looked up by keysym, the first entry wins.
static const KeysymKey keysymKeys[] = {
    {XKB_KEY_Escape, Key_Escape},
    {XKB_KEY_Tab, Key_Tab},
    {XKB_KEY_ISO_Left_Tab, Key_Backtab},
    {XKB_KEY_BackSpace, Key_Backspace},
    {XKB_KEY_Return, Key_Return},
    {XKB_KEY_Insert, Key_Insert},
    {XKB_KEY_Delete, Key_Delete},
    {XKB_KEY_Clear, Key_Delete},
    {XKB_KEY_Pause, Key_Pause},
    {XKB_KEY_Print, Key_Print},
    {XKB_KEY_Sys_Req, Key_SysReq},
    {0x1005FF60, Key_SysReq},
    {0x1007ff00, Key_SysReq},
    {XKB_KEY_Home, Key_Home},
    {XKB_KEY_End, Key_End},
    {XKB_KEY_Left, Key_Left},
    {XKB_KEY_Up, Key_Up},
    {XKB_KEY_Right, Key_Right},
    {XKB_KEY_Down, Key_Down},
    {XKB_KEY_Prior, Key_PageUp},
    {XKB_KEY_Next, Key_PageDown},
    {XKB_KEY_Shift_L, Key_Shift},
    {XKB_KEY_Shift_R, Key_Shift},
    {XKB_KEY_Shift_Lock, Key_Shift},
    {XKB_KEY_Control_L, Key_Control},
    {XKB_KEY_Control_R, Key_Control},
    {XKB_KEY_Meta_L, Key_Meta},
    {XKB_KEY_Meta_R, Key_Meta},
    {XKB_KEY_Alt_L, Key_Alt},
    {XKB_KEY_Alt_R, Key_Alt},
    {XKB_KEY_Caps_Lock, Key_CapsLock},
    {XKB_KEY_Num_Lock, Key_NumLock},
    {XKB_KEY_Scroll_Lock, Key_ScrollLock},
    {XKB_KEY_Super_L, Key_Super_L},
    {XKB_KEY_Super_R, Key_Super_R},
    {XKB_KEY_Menu, Key_Menu},
    {XKB_KEY_Hyper_L, Key_Hyper_L},
    {XKB_KEY_Hyper_R, Key_Hyper_R},
    {XKB_KEY_Help, Key_Help},
    {0x1000FF74, Key_Backtab},
    {0x1005FF10, Key_F11},
    {0x1005FF11, Key_F12},
    {XKB_KEY_KP_Space, Key_Space},
    {XKB_KEY_KP_Tab, Key_Tab},
    {XKB_KEY_KP_Enter, Key_Enter},
    {XKB_KEY_KP_Home, Key_Home},
    {XKB_KEY_KP_Left, Key_Left},
    {XKB_KEY_KP_Up, Key_Up},
    {XKB_KEY_KP_Right, Key_Right},
    {XKB_KEY_KP_Down, Key_Down},
    {XKB_KEY_KP_Prior, Key_PageUp},
    {XKB_KEY_KP_Next, Key_PageDown},
    {XKB_KEY_KP_End, Key_End},
    {XKB_KEY_KP_Begin, Key_Clear},
    {XKB_KEY_KP_Insert, Key_Insert},
    {XKB_KEY_KP_Delete, Key_Delete},
    {XKB_KEY_KP_Equal, Key_Equal},
    {XKB_KEY_KP_Multiply, Key_Asterisk},
    {XKB_KEY_KP_Add, Key_Plus},
    {XKB_KEY_KP_Separator, Key_Comma},
    {XKB_KEY_KP_Subtract, Key_Minus},
    {XKB_KEY_KP_Decimal, Key_Period},
    {XKB_KEY_KP_Divide, Key_Slash},
    {XKB_KEY_Undo, Key_Undo},
    {XKB_KEY_Redo, Key_Redo},
    {XKB_KEY_Find, Key_Find},
    {XKB_KEY_Cancel, Key_Cancel},
    {XKB_KEY_ISO_Level3_Shift, Key_AltGr},
    {XKB_KEY_Multi_key, Key_Multi_key},
    {XKB_KEY_Codeinput, Key_Codeinput},
    {XKB_KEY_SingleCandidate, Key_SingleCandidate},
    {XKB_KEY_MultipleCandidate, Key_MultipleCandidate},
    {XKB_KEY_PreviousCandidate, Key_PreviousCandidate},
    {XKB_KEY_Mode_switch, Key_Mode_switch},
    {XKB_KEY_script_switch, Key_Mode_switch},
    {XKB_KEY_Kanji, Key_Kanji},
    {XKB_KEY_Muhenkan, Key_Muhenkan},
    {XKB_KEY_Henkan_Mode, Key_Henkan},
    {XKB_KEY_Henkan, Key_Henkan},
    {XKB_KEY_Romaji, Key_Romaji},
    {XKB_KEY_Hiragana, Key_Hiragana},
    {XKB_KEY_Katakana, Key_Katakana},
    {XKB_KEY_Hiragana_Katakana, Key_Hiragana_Katakana},
    {XKB_KEY_Zenkaku, Key_Zenkaku},
    {XKB_KEY_Hankaku, Key_Hankaku},
    {XKB_KEY_Zenkaku_Hankaku, Key_Zenkaku_Hankaku},
    {XKB_KEY_Touroku, Key_Touroku},
    {XKB_KEY_Massyo, Key_Massyo},
    {XKB_KEY_Kana_Lock, Key_Kana_Lock},
    {XKB_KEY_Kana_Shift, Key_Kana_Shift},
    {XKB_KEY_Eisu_Shift, Key_Eisu_Shift},
    {XKB_KEY_Eisu_toggle, Key_Eisu_toggle},
    {XKB_KEY_Kanji_Bangou, Key_Codeinput},
    {XKB_KEY_Zen_Koho, Key_MultipleCandidate},
    {XKB_KEY_Mae_Koho, Key_PreviousCandidate},
    {XKB_KEY_Hangul, Key_Hangul},
    {XKB_KEY_Hangul_Start, Key_Hangul_Start},
    {XKB_KEY_Hangul_End, Key_Hangul_End},
    {XKB_KEY_Hangul_Hanja, Key_Hangul_Hanja},
    {XKB_KEY_Hangul_Jamo, Key_Hangul_Jamo},
    {XKB_KEY_Hangul_Romaja, Key_Hangul_Romaja},
    {XKB_KEY_Hangul_Codeinput, Key_Codeinput},
    {XKB_KEY_Hangul_Jeonja, Key_Hangul_Jeonja},
    {XKB_KEY_Hangul_Banja, Key_Hangul_Banja},
    {XKB_KEY_Hangul_PreHanja, Key_Hangul_PreHanja},
    {XKB_KEY_Hangul_PostHanja, Key_Hangul_PostHanja},
    {XKB_KEY_Hangul_SingleCandidate, Key_SingleCandidate},
    {XKB_KEY_Hangul_MultipleCandidate, Key_MultipleCandidate},
    {XKB_KEY_Hangul_PreviousCandidate, Key_PreviousCandidate},
    {XKB_KEY_Hangul_Special, Key_Hangul_Special},
    {XKB_KEY_Hangul_switch, Key_Mode_switch},
    {XKB_KEY_dead_grave, Key_Dead_Grave},
    {XKB_KEY_dead_acute, Key_Dead_Acute},
    {XKB_KEY_dead_circumflex, Key_Dead_Circumflex},
    {XKB_KEY_dead_tilde, Key_Dead_Tilde},
    {XKB_KEY_dead_macron, Key_Dead_Macron},
    {XKB_KEY_dead_breve, Key_Dead_Breve},
    {XKB_KEY_dead_abovedot, Key_Dead_Abovedot},
    {XKB_KEY_dead_diaeresis, Key_Dead_Diaeresis},
    {XKB_KEY_dead_abovering, Key_Dead_Abovering},
    {XKB_KEY_dead_doubleacute, Key_Dead_Doubleacute},
    {XKB_KEY_dead_caron, Key_Dead_Caron},
    {XKB_KEY_dead_cedilla, Key_Dead_Cedilla},
    {XKB_KEY_dead_ogonek, Key_Dead_Ogonek},
    {XKB_KEY_dead_iota, Key_Dead_Iota},
    {XKB_KEY_dead_voiced_sound, Key_Dead_Voiced_Sound},
    {XKB_KEY_dead_semivoiced_sound, Key_Dead_Semivoiced_Sound},
    {XKB_KEY_dead_belowdot, Key_Dead_Belowdot},
    {XKB_KEY_dead_hook, Key_Dead_Hook},
    {XKB_KEY_dead_horn, Key_Dead_Horn},
    {XKB_KEY_dead_stroke, Key_Dead_Stroke},
    {XKB_KEY_dead_abovecomma, Key_Dead_Abovecomma},
    {XKB_KEY_dead_abovereversedcomma, Key_Dead_Abovereversedcomma},
    {XKB_KEY_dead_doublegrave, Key_Dead_Doublegrave},
    {XKB_KEY_dead_belowring, Key_Dead_Belowring},
    {XKB_KEY_dead_belowmacron, Key_Dead_Belowmacron},
    {XKB_KEY_dead_belowcircumflex, Key_Dead_Belowcircumflex},
    {XKB_KEY_dead_belowtilde, Key_Dead_Belowtilde},
    {XKB_KEY_dead_belowbreve, Key_Dead_Belowbreve},
    {XKB_KEY_dead_belowdiaeresis, Key_Dead_Belowdiaeresis},
    {XKB_KEY_dead_invertedbreve, Key_Dead_Invertedbreve},
    {XKB_KEY_dead_belowcomma, Key_Dead_Belowcomma},
    {XKB_KEY_dead_currency, Key_Dead_Currency},
    {XKB_KEY_dead_a, Key_Dead_a},
    {XKB_KEY_dead_A, Key_Dead_A},
    {XKB_KEY_dead_e, Key_Dead_e},
    {XKB_KEY_dead_E, Key_Dead_E},
    {XKB_KEY_dead_i, Key_Dead_i},
    {XKB_KEY_dead_I, Key_Dead_I},
    {XKB_KEY_dead_o, Key_Dead_o},
    {XKB_KEY_dead_O, Key_Dead_O},
    {XKB_KEY_dead_u, Key_Dead_u},
    {XKB_KEY_dead_U, Key_Dead_U},
    {XKB_KEY_dead_small_schwa, Key_Dead_Small_Schwa},
    {XKB_KEY_dead_capital_schwa, Key_Dead_Capital_Schwa},
    {XKB_KEY_dead_greek, Key_Dead_Greek},
    {XKB_KEY_dead_lowline, Key_Dead_Lowline},
    {XKB_KEY_dead_aboveverticalline, Key_Dead_Aboveverticalline},
    {XKB_KEY_dead_belowverticalline, Key_Dead_Belowverticalline},
    {XKB_KEY_dead_longsolidusoverlay, Key_Dead_Longsolidusoverlay},
    {XKB_KEY_XF86Back, Key_Back},
    {XKB_KEY_XF86Forward, Key_Forward},
    {XKB_KEY_XF86Stop, Key_Stop},
    {XKB_KEY_XF86Refresh, Key_Refresh},
    {XKB_KEY_XF86Favorites, Key_Favorites},
    {XKB_KEY_XF86AudioMedia, Key_LaunchMedia},
    {XKB_KEY_XF86OpenURL, Key_OpenUrl},
    {XKB_KEY_XF86HomePage, Key_HomePage},
    {XKB_KEY_XF86Search, Key_Search},
    {XKB_KEY_XF86AudioLowerVolume, Key_VolumeDown},
    {XKB_KEY_XF86AudioMute, Key_VolumeMute},
    {XKB_KEY_XF86AudioRaiseVolume, Key_VolumeUp},
    {XKB_KEY_XF86AudioPlay, Key_MediaPlay},
    {XKB_KEY_XF86AudioStop, Key_MediaStop},
    {XKB_KEY_XF86AudioPrev, Key_MediaPrevious},
    {XKB_KEY_XF86AudioNext, Key_MediaNext},
    {XKB_KEY_XF86AudioRecord, Key_MediaRecord},
    {XKB_KEY_XF86AudioPause, Key_MediaPause},
    {XKB_KEY_XF86Mail, Key_LaunchMail},
    {XKB_KEY_XF86MyComputer, Key_LaunchMedia},
    {XKB_KEY_XF86Memo, Key_Memo},
    {XKB_KEY_XF86ToDoList, Key_ToDoList},
    {XKB_KEY_XF86Calendar, Key_Calendar},
    {XKB_KEY_XF86PowerDown, Key_PowerDown},
    {XKB_KEY_XF86ContrastAdjust, Key_ContrastAdjust},
    {XKB_KEY_XF86Standby, Key_Standby},
    {XKB_KEY_XF86MonBrightnessUp, Key_MonBrightnessUp},
    {XKB_KEY_XF86MonBrightnessDown, Key_MonBrightnessDown},
    {XKB_KEY_XF86KbdLightOnOff, Key_KeyboardLightOnOff},
    {XKB_KEY_XF86KbdBrightnessUp, Key_KeyboardBrightnessUp},
    {XKB_KEY_XF86KbdBrightnessDown, Key_KeyboardBrightnessDown},
    {XKB_KEY_XF86PowerOff, Key_PowerOff},
    {XKB_KEY_XF86WakeUp, Key_WakeUp},
    {XKB_KEY_XF86Eject, Key_Eject},
    {XKB_KEY_XF86ScreenSaver, Key_ScreenSaver},
    {XKB_KEY_XF86WWW, Key_WWW},
    {XKB_KEY_XF86Sleep, Key_Sleep},
    {XKB_KEY_XF86LightBulb, Key_LightBulb},
    {XKB_KEY_XF86Shop, Key_Shop},
    {XKB_KEY_XF86History, Key_History},
    {XKB_KEY_XF86AddFavorite, Key_AddFavorite},
    {XKB_KEY_XF86HotLinks, Key_HotLinks},
    {XKB_KEY_XF86BrightnessAdjust, Key_BrightnessAdjust},
    {XKB_KEY_XF86Finance, Key_Finance},
    {XKB_KEY_XF86Community, Key_Community},
    {XKB_KEY_XF86AudioRewind, Key_AudioRewind},
    {XKB_KEY_XF86BackForward, Key_BackForward},
    {XKB_KEY_XF86ApplicationLeft, Key_ApplicationLeft},
    {XKB_KEY_XF86ApplicationRight, Key_ApplicationRight},
    {XKB_KEY_XF86Book, Key_Book},
    {XKB_KEY_XF86CD, Key_CD},
    {XKB_KEY_XF86Calculater, Key_Calculator},
    {XKB_KEY_XF86Calculator, Key_Calculator},
    {XKB_KEY_XF86Clear, Key_Clear},
    {XKB_KEY_XF86ClearGrab, Key_ClearGrab},
    {XKB_KEY_XF86Close, Key_Close},
    {XKB_KEY_XF86Copy, Key_Copy},
    {XKB_KEY_XF86Cut, Key_Cut},
    {XKB_KEY_XF86Display, Key_Display},
    {XKB_KEY_XF86DOS, Key_DOS},
    {XKB_KEY_XF86Documents, Key_Documents},
    {XKB_KEY_XF86Excel, Key_Excel},
    {XKB_KEY_XF86Explorer, Key_Explorer},
    {XKB_KEY_XF86Game, Key_Game},
    {XKB_KEY_XF86Go, Key_Go},
    {XKB_KEY_XF86iTouch, Key_iTouch},
    {XKB_KEY_XF86LogOff, Key_LogOff},
    {XKB_KEY_XF86Market, Key_Market},
    {XKB_KEY_XF86Meeting, Key_Meeting},
    {XKB_KEY_XF86MenuKB, Key_MenuKB},
    {XKB_KEY_XF86MenuPB, Key_MenuPB},
    {XKB_KEY_XF86MySites, Key_MySites},
    {XKB_KEY_XF86New, Key_New},
    {XKB_KEY_XF86News, Key_News},
    {XKB_KEY_XF86OfficeHome, Key_OfficeHome},
    {XKB_KEY_XF86Open, Key_Open},
    {XKB_KEY_XF86Option, Key_Option},
    {XKB_KEY_XF86Paste, Key_Paste},
    {XKB_KEY_XF86Phone, Key_Phone},
    {XKB_KEY_XF86Reply, Key_Reply},
    {XKB_KEY_XF86Reload, Key_Reload},
    {XKB_KEY_XF86RotateWindows, Key_RotateWindows},
    {XKB_KEY_XF86RotationPB, Key_RotationPB},
    {XKB_KEY_XF86RotationKB, Key_RotationKB},
    {XKB_KEY_XF86Save, Key_Save},
    {XKB_KEY_XF86Send, Key_Send},
    {XKB_KEY_XF86Spell, Key_Spell},
    {XKB_KEY_XF86SplitScreen, Key_SplitScreen},
    {XKB_KEY_XF86Support, Key_Support},
    {XKB_KEY_XF86TaskPane, Key_TaskPane},
    {XKB_KEY_XF86Terminal, Key_Terminal},
    {XKB_KEY_XF86Tools, Key_Tools},
    {XKB_KEY_XF86Travel, Key_Travel},
    {XKB_KEY_XF86Video, Key_Video},
    {XKB_KEY_XF86Word, Key_Word},
    {XKB_KEY_XF86Xfer, Key_Xfer},
    {XKB_KEY_XF86ZoomIn, Key_ZoomIn},
    {XKB_KEY_XF86ZoomOut, Key_ZoomOut},
    {XKB_KEY_XF86Away, Key_Away},
    {XKB_KEY_XF86Messenger, Key_Messenger},
    {XKB_KEY_XF86WebCam, Key_WebCam},
    {XKB_KEY_XF86MailForward, Key_MailForward},
    {XKB_KEY_XF86Pictures, Key_Pictures},
    {XKB_KEY_XF86Music, Key_Music},
    {XKB_KEY_XF86Battery, Key_Battery},
    {XKB_KEY_XF86Bluetooth, Key_Bluetooth},
    {XKB_KEY_XF86WLAN, Key_WLAN},
    {XKB_KEY_XF86UWB, Key_UWB},
    {XKB_KEY_XF86AudioForward, Key_AudioForward},
    {XKB_KEY_XF86AudioRepeat, Key_AudioRepeat},
    {XKB_KEY_XF86AudioRandomPlay, Key_AudioRandomPlay},
    {XKB_KEY_XF86Subtitle, Key_Subtitle},
    {XKB_KEY_XF86AudioCycleTrack, Key_AudioCycleTrack},
    {XKB_KEY_XF86Time, Key_Time},
    {XKB_KEY_XF86Select, Key_Select},
    {XKB_KEY_XF86View, Key_View},
    {XKB_KEY_XF86TopMenu, Key_TopMenu},
    {XKB_KEY_XF86Red, Key_Red},
    {XKB_KEY_XF86Green, Key_Green},
    {XKB_KEY_XF86Yellow, Key_Yellow},
    {XKB_KEY_XF86Blue, Key_Blue},
    {XKB_KEY_XF86Bluetooth, Key_Bluetooth},
    {XKB_KEY_XF86Suspend, Key_Suspend},
    {XKB_KEY_XF86Hibernate, Key_Hibernate},
    {XKB_KEY_XF86TouchpadToggle, Key_TouchpadToggle},
    {XKB_KEY_XF86TouchpadOn, Key_TouchpadOn},
    {XKB_KEY_XF86TouchpadOff, Key_TouchpadOff},
    {XKB_KEY_XF86AudioMicMute, Key_MicMute},
    {XKB_KEY_XF86Launch0, Key_Launch0},
    {XKB_KEY_XF86Launch1, Key_Launch1},
    {XKB_KEY_XF86Launch2, Key_Launch2},
    {XKB_KEY_XF86Launch3, Key_Launch3},
    {XKB_KEY_XF86Launch4, Key_Launch4},
    {XKB_KEY_XF86Launch5, Key_Launch5},
    {XKB_KEY_XF86Launch6, Key_Launch6},
    {XKB_KEY_XF86Launch7, Key_Launch7},
    {XKB_KEY_XF86Launch8, Key_Launch8},
    {XKB_KEY_XF86Launch9, Key_Launch9},
    {XKB_KEY_XF86LaunchA, Key_LaunchA},
    {XKB_KEY_XF86LaunchB, Key_LaunchB},
    {XKB_KEY_XF86LaunchC, Key_LaunchC},
    {XKB_KEY_XF86LaunchD, Key_LaunchD},
    {XKB_KEY_XF86LaunchE, Key_LaunchE},
    {XKB_KEY_XF86LaunchF, Key_LaunchF},
};

static std::string lowerAscii(std::string text)
{
    for (char& ch : text)
    {
        if (ch >= 'A' && ch <= 'Z')
            ch = char(ch - 'A' + 'a');
    }
    return text;
}

int keyCodeFromName(const std::string& name)
{
    const std::string accel = lowerAscii(name);
    const std::u32string ucs4 = Lingmo::base::strings::utf8ToUtf32(accel);
    if (ucs4.size() == 1)
        return int(u_toupper(UChar32(ucs4[0])));

    if (accel.size() > 1 && accel[0] == 'f')
    {
        const auto number = Lingmo::base::strings::toNumber<int>(accel.substr(1));
        if (number && *number >= 1 && *number <= 35)
            return Key_F1 + *number - 1;
    }

    for (const KeyName& keyName : keyNames)
    {
        if (accel == lowerAscii(keyName.name))
            return keyName.key;
    }
    return Key_unknown;
}

std::string keyCodeName(int keyCode)
{
    if (keyCode >= Key_F1 && keyCode <= Key_F35)
        return "F" + std::to_string(keyCode - Key_F1 + 1);

    for (const KeyName& keyName : keyNames)
    {
        if (keyName.key == keyCode)
            return keyName.name;
    }

    std::string name;
    if (keyCode > 0 && keyCode < 0x110000)
        Lingmo::base::strings::appendUtf8(name, char32_t(keyCode));
    return name;
}

static bool isLatin1(uint32_t keysym)
{
    return keysym >= 0x20 && keysym <= 0xff;
}

bool isKeypadKeysym(uint32_t keysym)
{
    return keysym >= XKB_KEY_KP_Space && keysym <= XKB_KEY_KP_9;
}

int keyCodeFromKeysym(uint32_t keysym, uint32_t modifiers, const std::string& text)
{
    (void)modifiers;

    if (keysym >= XKB_KEY_F1 && keysym <= XKB_KEY_F35)
        return Key_F1 + int(keysym - XKB_KEY_F1);
    if (keysym >= XKB_KEY_KP_0 && keysym <= XKB_KEY_KP_9)
        return Key_0 + int(keysym - XKB_KEY_KP_0);
    if (isLatin1(keysym))
    {
        const uint32_t upper = xkb_keysym_to_upper(keysym);
        return int(isLatin1(upper) ? upper : keysym);
    }

    for (const KeysymKey& entry : keysymKeys)
    {
        if (entry.keysym == keysym)
            return entry.key;
    }

    // From the text: a digit of any script is its number, anything else its upper case.
    const std::u32string ucs4 = Lingmo::base::strings::utf8ToUtf32(text);
    if (ucs4.empty())
        return 0;
    if (u_charType(UChar32(ucs4[0])) == U_DECIMAL_DIGIT_NUMBER)
        return Key_0 + u_charDigitValue(UChar32(ucs4[0]));
    return int(u_toupper(UChar32(ucs4[0])));
}

}
