/*
    SPDX-FileCopyrightText: 2008 Erlend Hamberg <ehamberg@gmail.com>
    SPDX-FileCopyrightText: 2008 Evgeniy Ivanov <powerfox@kde.ru>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "definitions.h"
#include <QKeyEvent>
#include <QStringList>
#include <vimode/keyevent.h>
#include <vimode/keyparser.h>

using namespace KateVi;

KeyParser *KeyParser::m_instance = nullptr;

KeyParser::KeyParser()
{
    initKeyTables();
}

KeyParser *KeyParser::self()
{
    if (m_instance == nullptr) {
        m_instance = new KeyParser();
    }

    return m_instance;
}

void KeyParser::initKeyTables()
{
    m_qt2katevi = {
        {Qt::Key_Escape, QStringLiteral("esc")},
        {Qt::Key_Tab, QStringLiteral("tab")},
        {Qt::Key_Backtab, QStringLiteral("backtab")},
        {Qt::Key_Backspace, QStringLiteral("backspace")},
        {Qt::Key_Return, QStringLiteral("return")},
        {Qt::Key_Enter, QStringLiteral("enter")},
        {Qt::Key_Insert, QStringLiteral("insert")},
        {Qt::Key_Delete, QStringLiteral("delete")},
        {Qt::Key_Pause, QStringLiteral("pause")},
        {Qt::Key_Print, QStringLiteral("print")},
        {Qt::Key_SysReq, QStringLiteral("sysreq")},
        {Qt::Key_Clear, QStringLiteral("clear")},
        {Qt::Key_Home, QStringLiteral("home")},
        {Qt::Key_End, QStringLiteral("end")},
        {Qt::Key_Left, QStringLiteral("left")},
        {Qt::Key_Up, QStringLiteral("up")},
        {Qt::Key_Right, QStringLiteral("right")},
        {Qt::Key_Down, QStringLiteral("down")},
        {Qt::Key_PageUp, QStringLiteral("pageup")},
        {Qt::Key_PageDown, QStringLiteral("pagedown")},
        {Qt::Key_Shift, QStringLiteral("shift")},
        {Qt::Key_Control, QStringLiteral("control")},
        {Qt::Key_Meta, QStringLiteral("meta")},
        {Qt::Key_Alt, QStringLiteral("alt")},
        {Qt::Key_AltGr, QStringLiteral("altgr")},
        {Qt::Key_CapsLock, QStringLiteral("capslock")},
        {Qt::Key_NumLock, QStringLiteral("numlock")},
        {Qt::Key_ScrollLock, QStringLiteral("scrolllock")},
        {Qt::Key_F1, QStringLiteral("f1")},
        {Qt::Key_F2, QStringLiteral("f2")},
        {Qt::Key_F3, QStringLiteral("f3")},
        {Qt::Key_F4, QStringLiteral("f4")},
        {Qt::Key_F5, QStringLiteral("f5")},
        {Qt::Key_F6, QStringLiteral("f6")},
        {Qt::Key_F7, QStringLiteral("f7")},
        {Qt::Key_F8, QStringLiteral("f8")},
        {Qt::Key_F9, QStringLiteral("f9")},
        {Qt::Key_F10, QStringLiteral("f10")},
        {Qt::Key_F11, QStringLiteral("f11")},
        {Qt::Key_F12, QStringLiteral("f12")},
        {Qt::Key_F13, QStringLiteral("f13")},
        {Qt::Key_F14, QStringLiteral("f14")},
        {Qt::Key_F15, QStringLiteral("f15")},
        {Qt::Key_F16, QStringLiteral("f16")},
        {Qt::Key_F17, QStringLiteral("f17")},
        {Qt::Key_F18, QStringLiteral("f18")},
        {Qt::Key_F19, QStringLiteral("f19")},
        {Qt::Key_F20, QStringLiteral("f20")},
        {Qt::Key_F21, QStringLiteral("f21")},
        {Qt::Key_F22, QStringLiteral("f22")},
        {Qt::Key_F23, QStringLiteral("f23")},
        {Qt::Key_F24, QStringLiteral("f24")},
        {Qt::Key_F25, QStringLiteral("f25")},
        {Qt::Key_F26, QStringLiteral("f26")},
        {Qt::Key_F27, QStringLiteral("f27")},
        {Qt::Key_F28, QStringLiteral("f28")},
        {Qt::Key_F29, QStringLiteral("f29")},
        {Qt::Key_F30, QStringLiteral("f30")},
        {Qt::Key_F31, QStringLiteral("f31")},
        {Qt::Key_F32, QStringLiteral("f32")},
        {Qt::Key_F33, QStringLiteral("f33")},
        {Qt::Key_F34, QStringLiteral("f34")},
        {Qt::Key_F35, QStringLiteral("f35")},
        {Qt::Key_Super_L, QStringLiteral("super_l")},
        {Qt::Key_Super_R, QStringLiteral("super_r")},
        {Qt::Key_Menu, QStringLiteral("menu")},
        {Qt::Key_Hyper_L, QStringLiteral("hyper_l")},
        {Qt::Key_Hyper_R, QStringLiteral("hyper_r")},
        {Qt::Key_Help, QStringLiteral("help")},
        {Qt::Key_Direction_L, QStringLiteral("direction_l")},
        {Qt::Key_Direction_R, QStringLiteral("direction_r")},
        {Qt::Key_Multi_key, QStringLiteral("multi_key")},
        {Qt::Key_Codeinput, QStringLiteral("codeinput")},
        {Qt::Key_SingleCandidate, QStringLiteral("singlecandidate")},
        {Qt::Key_MultipleCandidate, QStringLiteral("multiplecandidate")},
        {Qt::Key_PreviousCandidate, QStringLiteral("previouscandidate")},
        {Qt::Key_Mode_switch, QStringLiteral("mode_switch")},
        {Qt::Key_Kanji, QStringLiteral("kanji")},
        {Qt::Key_Muhenkan, QStringLiteral("muhenkan")},
        {Qt::Key_Henkan, QStringLiteral("henkan")},
        {Qt::Key_Romaji, QStringLiteral("romaji")},
        {Qt::Key_Hiragana, QStringLiteral("hiragana")},
        {Qt::Key_Katakana, QStringLiteral("katakana")},
        {Qt::Key_Hiragana_Katakana, QStringLiteral("hiragana_katakana")},
        {Qt::Key_Zenkaku, QStringLiteral("zenkaku")},
        {Qt::Key_Hankaku, QStringLiteral("hankaku")},
        {Qt::Key_Zenkaku_Hankaku, QStringLiteral("zenkaku_hankaku")},
        {Qt::Key_Touroku, QStringLiteral("touroku")},
        {Qt::Key_Massyo, QStringLiteral("massyo")},
        {Qt::Key_Kana_Lock, QStringLiteral("kana_lock")},
        {Qt::Key_Kana_Shift, QStringLiteral("kana_shift")},
        {Qt::Key_Eisu_Shift, QStringLiteral("eisu_shift")},
        {Qt::Key_Eisu_toggle, QStringLiteral("eisu_toggle")},
        {Qt::Key_Hangul, QStringLiteral("hangul")},
        {Qt::Key_Hangul_Start, QStringLiteral("hangul_start")},
        {Qt::Key_Hangul_End, QStringLiteral("hangul_end")},
        {Qt::Key_Hangul_Hanja, QStringLiteral("hangul_hanja")},
        {Qt::Key_Hangul_Jamo, QStringLiteral("hangul_jamo")},
        {Qt::Key_Hangul_Romaja, QStringLiteral("hangul_romaja")},
        {Qt::Key_Hangul_Jeonja, QStringLiteral("hangul_jeonja")},
        {Qt::Key_Hangul_Banja, QStringLiteral("hangul_banja")},
        {Qt::Key_Hangul_PreHanja, QStringLiteral("hangul_prehanja")},
        {Qt::Key_Hangul_PostHanja, QStringLiteral("hangul_posthanja")},
        {Qt::Key_Hangul_Special, QStringLiteral("hangul_special")},
        {Qt::Key_Dead_Grave, QStringLiteral("dead_grave")},
        {Qt::Key_Dead_Acute, QStringLiteral("dead_acute")},
        {Qt::Key_Dead_Circumflex, QStringLiteral("dead_circumflex")},
        {Qt::Key_Dead_Tilde, QStringLiteral("dead_tilde")},
        {Qt::Key_Dead_Macron, QStringLiteral("dead_macron")},
        {Qt::Key_Dead_Breve, QStringLiteral("dead_breve")},
        {Qt::Key_Dead_Abovedot, QStringLiteral("dead_abovedot")},
        {Qt::Key_Dead_Diaeresis, QStringLiteral("dead_diaeresis")},
        {Qt::Key_Dead_Abovering, QStringLiteral("dead_abovering")},
        {Qt::Key_Dead_Doubleacute, QStringLiteral("dead_doubleacute")},
        {Qt::Key_Dead_Caron, QStringLiteral("dead_caron")},
        {Qt::Key_Dead_Cedilla, QStringLiteral("dead_cedilla")},
        {Qt::Key_Dead_Ogonek, QStringLiteral("dead_ogonek")},
        {Qt::Key_Dead_Iota, QStringLiteral("dead_iota")},
        {Qt::Key_Dead_Voiced_Sound, QStringLiteral("dead_voiced_sound")},
        {Qt::Key_Dead_Semivoiced_Sound, QStringLiteral("dead_semivoiced_sound")},
        {Qt::Key_Dead_Belowdot, QStringLiteral("dead_belowdot")},
        {Qt::Key_Dead_Hook, QStringLiteral("dead_hook")},
        {Qt::Key_Dead_Horn, QStringLiteral("dead_horn")},
        {Qt::Key_Back, QStringLiteral("back")},
        {Qt::Key_Forward, QStringLiteral("forward")},
        {Qt::Key_Stop, QStringLiteral("stop")},
        {Qt::Key_Refresh, QStringLiteral("refresh")},
        {Qt::Key_VolumeDown, QStringLiteral("volumedown")},
        {Qt::Key_VolumeMute, QStringLiteral("volumemute")},
        {Qt::Key_VolumeUp, QStringLiteral("volumeup")},
        {Qt::Key_BassBoost, QStringLiteral("bassboost")},
        {Qt::Key_BassUp, QStringLiteral("bassup")},
        {Qt::Key_BassDown, QStringLiteral("bassdown")},
        {Qt::Key_TrebleUp, QStringLiteral("trebleup")},
        {Qt::Key_TrebleDown, QStringLiteral("trebledown")},
        {Qt::Key_MediaPlay, QStringLiteral("mediaplay")},
        {Qt::Key_MediaStop, QStringLiteral("mediastop")},
        {Qt::Key_MediaPrevious, QStringLiteral("mediaprevious")},
        {Qt::Key_MediaNext, QStringLiteral("medianext")},
        {Qt::Key_MediaRecord, QStringLiteral("mediarecord")},
        {Qt::Key_HomePage, QStringLiteral("homepage")},
        {Qt::Key_Favorites, QStringLiteral("favorites")},
        {Qt::Key_Search, QStringLiteral("search")},
        {Qt::Key_Standby, QStringLiteral("standby")},
        {Qt::Key_OpenUrl, QStringLiteral("openurl")},
        {Qt::Key_LaunchMail, QStringLiteral("launchmail")},
        {Qt::Key_LaunchMedia, QStringLiteral("launchmedia")},
        {Qt::Key_Launch0, QStringLiteral("launch0")},
        {Qt::Key_Launch1, QStringLiteral("launch1")},
        {Qt::Key_Launch2, QStringLiteral("launch2")},
        {Qt::Key_Launch3, QStringLiteral("launch3")},
        {Qt::Key_Launch4, QStringLiteral("launch4")},
        {Qt::Key_Launch5, QStringLiteral("launch5")},
        {Qt::Key_Launch6, QStringLiteral("launch6")},
        {Qt::Key_Launch7, QStringLiteral("launch7")},
        {Qt::Key_Launch8, QStringLiteral("launch8")},
        {Qt::Key_Launch9, QStringLiteral("launch9")},
        {Qt::Key_LaunchA, QStringLiteral("launcha")},
        {Qt::Key_LaunchB, QStringLiteral("launchb")},
        {Qt::Key_LaunchC, QStringLiteral("launchc")},
        {Qt::Key_LaunchD, QStringLiteral("launchd")},
        {Qt::Key_LaunchE, QStringLiteral("launche")},
        {Qt::Key_LaunchF, QStringLiteral("launchf")},
        {Qt::Key_MediaLast, QStringLiteral("medialast")},
        {Qt::Key_unknown, QStringLiteral("unknown")},
        {Qt::Key_Call, QStringLiteral("call")},
        {Qt::Key_Context1, QStringLiteral("context1")},
        {Qt::Key_Context2, QStringLiteral("context2")},
        {Qt::Key_Context3, QStringLiteral("context3")},
        {Qt::Key_Context4, QStringLiteral("context4")},
        {Qt::Key_Flip, QStringLiteral("flip")},
        {Qt::Key_Hangup, QStringLiteral("hangup")},
        {Qt::Key_No, QStringLiteral("no")},
        {Qt::Key_Select, QStringLiteral("select")},
        {Qt::Key_Yes, QStringLiteral("yes")},
        {Qt::Key_Execute, QStringLiteral("execute")},
        {Qt::Key_Printer, QStringLiteral("printer")},
        {Qt::Key_Play, QStringLiteral("play")},
        {Qt::Key_Sleep, QStringLiteral("sleep")},
        {Qt::Key_Zoom, QStringLiteral("zoom")},
        {Qt::Key_Cancel, QStringLiteral("cancel")},
    };

    for (QHash<int, QString>::const_iterator i = m_qt2katevi.constBegin(); i != m_qt2katevi.constEnd(); ++i) {
        m_katevi2qt.insert(i.value(), i.key());
    }
    m_katevi2qt.insert(QStringLiteral("cr"), Qt::Key_Enter);

    m_nameToKeyCode = {
        {QStringLiteral("invalid"), -1},
        {QStringLiteral("esc"), 1},
        {QStringLiteral("tab"), 2},
        {QStringLiteral("backtab"), 3},
        {QStringLiteral("backspace"), 4},
        {QStringLiteral("return"), 5},
        {QStringLiteral("enter"), 6},
        {QStringLiteral("insert"), 7},
        {QStringLiteral("delete"), 8},
        {QStringLiteral("pause"), 9},
        {QStringLiteral("print"), 10},
        {QStringLiteral("sysreq"), 11},
        {QStringLiteral("clear"), 12},
        {QStringLiteral("home"), 13},
        {QStringLiteral("end"), 14},
        {QStringLiteral("left"), 15},
        {QStringLiteral("up"), 16},
        {QStringLiteral("right"), 17},
        {QStringLiteral("down"), 18},
        {QStringLiteral("pageup"), 19},
        {QStringLiteral("pagedown"), 20},
        {QStringLiteral("shift"), 21},
        {QStringLiteral("control"), 22},
        {QStringLiteral("meta"), 23},
        {QStringLiteral("alt"), 24},
        {QStringLiteral("altgr"), 25},
        {QStringLiteral("capslock"), 26},
        {QStringLiteral("numlock"), 27},
        {QStringLiteral("scrolllock"), 28},
        {QStringLiteral("f1"), 29},
        {QStringLiteral("f2"), 30},
        {QStringLiteral("f3"), 31},
        {QStringLiteral("f4"), 32},
        {QStringLiteral("f5"), 33},
        {QStringLiteral("f6"), 34},
        {QStringLiteral("f7"), 35},
        {QStringLiteral("f8"), 36},
        {QStringLiteral("f9"), 37},
        {QStringLiteral("f10"), 38},
        {QStringLiteral("f11"), 39},
        {QStringLiteral("f12"), 40},
        {QStringLiteral("f13"), 41},
        {QStringLiteral("f14"), 42},
        {QStringLiteral("f15"), 43},
        {QStringLiteral("f16"), 44},
        {QStringLiteral("f17"), 45},
        {QStringLiteral("f18"), 46},
        {QStringLiteral("f19"), 47},
        {QStringLiteral("f20"), 48},
        {QStringLiteral("f21"), 49},
        {QStringLiteral("f22"), 50},
        {QStringLiteral("f23"), 51},
        {QStringLiteral("f24"), 52},
        {QStringLiteral("f25"), 53},
        {QStringLiteral("f26"), 54},
        {QStringLiteral("f27"), 55},
        {QStringLiteral("f28"), 56},
        {QStringLiteral("f29"), 57},
        {QStringLiteral("f30"), 58},
        {QStringLiteral("f31"), 59},
        {QStringLiteral("f32"), 60},
        {QStringLiteral("f33"), 61},
        {QStringLiteral("f34"), 62},
        {QStringLiteral("f35"), 63},
        {QStringLiteral("super_l"), 64},
        {QStringLiteral("super_r"), 65},
        {QStringLiteral("menu"), 66},
        {QStringLiteral("hyper_l"), 67},
        {QStringLiteral("hyper_r"), 68},
        {QStringLiteral("help"), 69},
        {QStringLiteral("direction_l"), 70},
        {QStringLiteral("direction_r"), 71},
        {QStringLiteral("multi_key"), 172},
        {QStringLiteral("codeinput"), 173},
        {QStringLiteral("singlecandidate"), 174},
        {QStringLiteral("multiplecandidate"), 175},
        {QStringLiteral("previouscandidate"), 176},
        {QStringLiteral("mode_switch"), 177},
        {QStringLiteral("kanji"), 178},
        {QStringLiteral("muhenkan"), 179},
        {QStringLiteral("henkan"), 180},
        {QStringLiteral("romaji"), 181},
        {QStringLiteral("hiragana"), 182},
        {QStringLiteral("katakana"), 183},
        {QStringLiteral("hiragana_katakana"), 184},
        {QStringLiteral("zenkaku"), 185},
        {QStringLiteral("hankaku"), 186},
        {QStringLiteral("zenkaku_hankaku"), 187},
        {QStringLiteral("touroku"), 188},
        {QStringLiteral("massyo"), 189},
        {QStringLiteral("kana_lock"), 190},
        {QStringLiteral("kana_shift"), 191},
        {QStringLiteral("eisu_shift"), 192},
        {QStringLiteral("eisu_toggle"), 193},
        {QStringLiteral("hangul"), 194},
        {QStringLiteral("hangul_start"), 195},
        {QStringLiteral("hangul_end"), 196},
        {QStringLiteral("hangul_hanja"), 197},
        {QStringLiteral("hangul_jamo"), 198},
        {QStringLiteral("hangul_romaja"), 199},
        {QStringLiteral("hangul_jeonja"), 200},
        {QStringLiteral("hangul_banja"), 201},
        {QStringLiteral("hangul_prehanja"), 202},
        {QStringLiteral("hangul_posthanja"), 203},
        {QStringLiteral("hangul_special"), 204},
        {QStringLiteral("dead_grave"), 205},
        {QStringLiteral("dead_acute"), 206},
        {QStringLiteral("dead_circumflex"), 207},
        {QStringLiteral("dead_tilde"), 208},
        {QStringLiteral("dead_macron"), 209},
        {QStringLiteral("dead_breve"), 210},
        {QStringLiteral("dead_abovedot"), 211},
        {QStringLiteral("dead_diaeresis"), 212},
        {QStringLiteral("dead_abovering"), 213},
        {QStringLiteral("dead_doubleacute"), 214},
        {QStringLiteral("dead_caron"), 215},
        {QStringLiteral("dead_cedilla"), 216},
        {QStringLiteral("dead_ogonek"), 217},
        {QStringLiteral("dead_iota"), 218},
        {QStringLiteral("dead_voiced_sound"), 219},
        {QStringLiteral("dead_semivoiced_sound"), 220},
        {QStringLiteral("dead_belowdot"), 221},
        {QStringLiteral("dead_hook"), 222},
        {QStringLiteral("dead_horn"), 223},
        {QStringLiteral("back"), 224},
        {QStringLiteral("forward"), 225},
        {QStringLiteral("stop"), 226},
        {QStringLiteral("refresh"), 227},
        {QStringLiteral("volumedown"), 228},
        {QStringLiteral("volumemute"), 229},
        {QStringLiteral("volumeup"), 230},
        {QStringLiteral("bassboost"), 231},
        {QStringLiteral("bassup"), 232},
        {QStringLiteral("bassdown"), 233},
        {QStringLiteral("trebleup"), 234},
        {QStringLiteral("trebledown"), 235},
        {QStringLiteral("mediaplay"), 236},
        {QStringLiteral("mediastop"), 237},
        {QStringLiteral("mediaprevious"), 238},
        {QStringLiteral("medianext"), 239},
        {QStringLiteral("mediarecord"), 240},
        {QStringLiteral("homepage"), 241},
        {QStringLiteral("favorites"), 242},
        {QStringLiteral("search"), 243},
        {QStringLiteral("standby"), 244},
        {QStringLiteral("openurl"), 245},
        {QStringLiteral("launchmail"), 246},
        {QStringLiteral("launchmedia"), 247},
        {QStringLiteral("launch0"), 248},
        {QStringLiteral("launch1"), 249},
        {QStringLiteral("launch2"), 250},
        {QStringLiteral("launch3"), 251},
        {QStringLiteral("launch4"), 252},
        {QStringLiteral("launch5"), 253},
        {QStringLiteral("launch6"), 254},
        {QStringLiteral("launch7"), 255},
        {QStringLiteral("launch8"), 256},
        {QStringLiteral("launch9"), 257},
        {QStringLiteral("launcha"), 258},
        {QStringLiteral("launchb"), 259},
        {QStringLiteral("launchc"), 260},
        {QStringLiteral("launchd"), 261},
        {QStringLiteral("launche"), 262},
        {QStringLiteral("launchf"), 263},
        {QStringLiteral("medialast"), 264},
        {QStringLiteral("unknown"), 265},
        {QStringLiteral("call"), 266},
        {QStringLiteral("context1"), 267},
        {QStringLiteral("context2"), 268},
        {QStringLiteral("context3"), 269},
        {QStringLiteral("context4"), 270},
        {QStringLiteral("flip"), 271},
        {QStringLiteral("hangup"), 272},
        {QStringLiteral("no"), 273},
        {QStringLiteral("select"), 274},
        {QStringLiteral("yes"), 275},
        {QStringLiteral("execute"), 276},
        {QStringLiteral("printer"), 277},
        {QStringLiteral("play"), 278},
        {QStringLiteral("sleep"), 279},
        {QStringLiteral("zoom"), 280},
        {QStringLiteral("cancel"), 281},

        {QStringLiteral("a"), 282},
        {QStringLiteral("b"), 283},
        {QStringLiteral("c"), 284},
        {QStringLiteral("d"), 285},
        {QStringLiteral("e"), 286},
        {QStringLiteral("f"), 287},
        {QStringLiteral("g"), 288},
        {QStringLiteral("h"), 289},
        {QStringLiteral("i"), 290},
        {QStringLiteral("j"), 291},
        {QStringLiteral("k"), 292},
        {QStringLiteral("l"), 293},
        {QStringLiteral("m"), 294},
        {QStringLiteral("n"), 295},
        {QStringLiteral("o"), 296},
        {QStringLiteral("p"), 297},
        {QStringLiteral("q"), 298},
        {QStringLiteral("r"), 299},
        {QStringLiteral("s"), 300},
        {QStringLiteral("t"), 301},
        {QStringLiteral("u"), 302},
        {QStringLiteral("v"), 303},
        {QStringLiteral("w"), 304},
        {QStringLiteral("x"), 305},
        {QStringLiteral("y"), 306},
        {QStringLiteral("z"), 307},
        {QStringLiteral("`"), 308},
        {QStringLiteral("!"), 309},
        {QStringLiteral("\""), 310},
        {QStringLiteral("$"), 311},
        {QStringLiteral("%"), 312},
        {QStringLiteral("^"), 313},
        {QStringLiteral("&"), 314},
        {QStringLiteral("*"), 315},
        {QStringLiteral("("), 316},
        {QStringLiteral(")"), 317},
        {QStringLiteral("-"), 318},
        {QStringLiteral("_"), 319},
        {QStringLiteral("="), 320},
        {QStringLiteral("+"), 321},
        {QStringLiteral("["), 322},
        {QStringLiteral("]"), 323},
        {QStringLiteral("{"), 324},
        {QStringLiteral("}"), 325},
        {QStringLiteral(":"), 326},
        {QStringLiteral(";"), 327},
        {QStringLiteral("@"), 328},
        {QStringLiteral("'"), 329},
        {QStringLiteral("#"), 330},
        {QStringLiteral("~"), 331},
        {QStringLiteral("\\"), 332},
        {QStringLiteral("|"), 333},
        {QStringLiteral(","), 334},
        {QStringLiteral("."), 335},
        // { QLatin1String( ">" ), 336 },
        {QStringLiteral("/"), 337},
        {QStringLiteral("?"), 338},
        {QStringLiteral(" "), 339},
        // { QLatin1String( "<" ), 341 },
        {QStringLiteral("0"), 340},
        {QStringLiteral("1"), 341},
        {QStringLiteral("2"), 342},
        {QStringLiteral("3"), 343},
        {QStringLiteral("4"), 344},
        {QStringLiteral("5"), 345},
        {QStringLiteral("6"), 346},
        {QStringLiteral("7"), 347},
        {QStringLiteral("8"), 348},
        {QStringLiteral("9"), 349},
        {QStringLiteral("cr"), 350},
        {QStringLiteral("leader"), 351},
        {QStringLiteral("nop"), 352},
    };

    for (QHash<QString, int>::const_iterator i = m_nameToKeyCode.constBegin(); i != m_nameToKeyCode.constEnd(); ++i) {
        m_keyCodeToName.insert(i.value(), i.key());
    }
}

QString KeyParser::qt2vi(int key) const
{
    auto it = m_qt2katevi.constFind(key);
    if (it != m_qt2katevi.cend()) {
        return it.value();
    }
    return QStringLiteral("invalid");
}

int KeyParser::vi2qt(const QString &keypress) const
{
    auto it = m_katevi2qt.constFind(keypress);
    if (it != m_katevi2qt.cend()) {
        return it.value();
    }
    return -1;
}

int KeyParser::encoded2qt(const QString &keypress) const
{
    QString key = KeyParser::self()->decodeKeySequence(keypress);

    if (key.length() > 2 && key.at(0) == QLatin1Char('<') && key.at(key.length() - 1) == QLatin1Char('>')) {
        key = key.mid(1, key.length() - 2);
    }
    auto it = m_katevi2qt.constFind(key);
    if (it != m_katevi2qt.cend())
        return it.value();
    return -1;
}

const QString KeyParser::encodeKeySequence(const QString &keys) const
{
    QString encodedSequence;
    unsigned int keyCodeTemp = 0;

    const QStringView keysView(keys);
    bool insideTag = false;
    QChar c;
    for (int i = 0; i < keys.length(); i++) {
        c = keys.at(i);
        if (insideTag) {
            if (c == QLatin1Char('>')) {
                QChar code(0xE000 + keyCodeTemp);
                encodedSequence.append(code);
                keyCodeTemp = 0;
                insideTag = false;
                continue;
            } else {
                // contains modifiers
                if (keysView.mid(i).indexOf(QLatin1Char('-')) != -1 && keysView.mid(i).indexOf(QLatin1Char('-')) < keysView.mid(i).indexOf(QLatin1Char('>'))) {
                    // Perform something similar to a split on '-', except that we want to keep the occurrences of '-'
                    // e.g. <c-s-a> will equate to the list of tokens "c-", "s-", "a".
                    // A straight split on '-' would give us "c", "s", "a", in which case we lose the piece of information that
                    // 'a' is just the 'a' key, not the 'alt' modifier.
                    const QString untilClosing = keys.mid(i, keysView.mid(i).indexOf(QLatin1Char('>'))).toLower();
                    QStringList tokens;
                    int currentPos = 0;
                    int nextHypen = -1;
                    while ((nextHypen = untilClosing.indexOf(QLatin1Char('-'), currentPos)) != -1) {
                        tokens << untilClosing.mid(currentPos, nextHypen - currentPos + 1);
                        currentPos = nextHypen + 1;
                    }
                    tokens << untilClosing.mid(currentPos);

                    for (const QString &str : std::as_const(tokens)) {
                        if (str == QLatin1String("s-") && (keyCodeTemp & 0x01) != 0x1) {
                            keyCodeTemp += 0x1;
                        } else if (str == QLatin1String("c-") && (keyCodeTemp & 0x02) != 0x2) {
                            keyCodeTemp += 0x2;
                        } else if (str == QLatin1String("a-") && (keyCodeTemp & 0x04) != 0x4) {
                            keyCodeTemp += 0x4;
                        } else if (str == QLatin1String("m-") && (keyCodeTemp & 0x08) != 0x8) {
                            keyCodeTemp += 0x8;
                        } else {
                            if (m_nameToKeyCode.contains(str) || (str.length() == 1 && str.at(0).isLetterOrNumber())) {
                                if (m_nameToKeyCode.contains(str)) {
                                    keyCodeTemp += m_nameToKeyCode.value(str) * 0x10;
                                } else {
                                    keyCodeTemp += str.at(0).unicode() * 0x10;
                                }
                            } else {
                                int endOfBlock = keys.indexOf(QLatin1Char('>'));
                                if (endOfBlock -= -1) {
                                    endOfBlock = keys.length() - 1;
                                }
                                encodedSequence.clear();
                                encodedSequence.append(QChar::fromUcs4(m_nameToKeyCode.value(QStringLiteral("invalid"))));
                                break;
                            }
                        }
                    }
                } else {
                    const QString str = keys.mid(i, keys.indexOf(QLatin1Char('>'), i) - i).toLower();
                    if (keys.indexOf(QLatin1Char('>'), i) != -1 && (m_nameToKeyCode.contains(str) || (str.length() == 1 && str.at(0).isLetterOrNumber()))) {
                        if (m_nameToKeyCode.contains(str)) {
                            keyCodeTemp += m_nameToKeyCode.value(str) * 0x10;
                        } else {
                            keyCodeTemp += str.at(0).unicode() * 0x10;
                        }
                    } else {
                        int endOfBlock = keys.indexOf(QLatin1Char('>'));
                        if (endOfBlock -= -1) {
                            endOfBlock = keys.length() - 1;
                        }
                        encodedSequence.clear();
                        keyCodeTemp = m_nameToKeyCode.value(QStringLiteral("invalid")) * 0x10;
                    }
                }
                i += keysView.mid(i, keysView.mid(i).indexOf(QLatin1Char('>'))).length() - 1;
            }
        } else {
            if (c == QLatin1Char('<')) {
                // If there's no closing '>', or if there is an opening '<' before the next '>', interpret as a literal '<'
                // If we are <space>, encode as a literal " ".
                const QStringView rest = keysView.mid(i);
                if (rest.indexOf(QLatin1Char('>'), 1) != -1 && rest.mid(1, rest.indexOf(QLatin1Char('>'), 1) - 1) == QLatin1String("space")) {
                    encodedSequence.append(QLatin1Char(' '));
                    i += rest.indexOf(QLatin1Char('>'), 1);
                    continue;
                } else if (rest.indexOf(QLatin1Char('>'), 1) == -1
                           || (rest.indexOf(QLatin1Char('<'), 1) < rest.indexOf(QLatin1Char('>'), 1) && rest.indexOf(QLatin1Char('<'), 1) != -1)) {
                    encodedSequence.append(c);
                    continue;
                }
                insideTag = true;
                continue;
            } else {
                encodedSequence.append(c);
            }
        }
    }

    return encodedSequence;
}

const QString KeyParser::decodeKeySequence(const QString &keys) const
{
    QString ret;
    ret.reserve(keys.length());

    for (int i = 0; i < keys.length(); i++) {
        QChar c = keys.at(i);
        int keycode = c.unicode();

        if ((keycode & 0xE000) != 0xE000) {
            ret.append(c);
        } else {
            ret.append(QLatin1Char('<'));

            if ((keycode & 0x1) == 0x1) {
                ret.append(QLatin1String("s-"));
                // keycode -= 0x1;
            }
            if ((keycode & 0x2) == 0x2) {
                ret.append(QLatin1String("c-"));
                // keycode -= 0x2;
            }
            if ((keycode & 0x4) == 0x4) {
                ret.append(QLatin1String("a-"));
                // keycode -= 0x4;
            }
            if ((keycode & 0x8) == 0x8) {
                ret.append(QLatin1String("m-"));
                // keycode -= 0x8;
            }

            if ((keycode & 0xE000) == 0xE000) {
                ret.append(m_keyCodeToName.value((keycode - 0xE000) / 0x10));
            } else {
                ret.append(QChar::fromUcs4(keycode));
            }
            ret.append(QLatin1Char('>'));
        }
    }

    return ret;
}

const QChar KeyParser::KeyEventToQChar(const QKeyEvent &keyEvent)
{
    return KeyEventToQChar(keyEvent.key(), keyEvent.text(), keyEvent.modifiers());
}

const QChar KeyParser::KeyEventToQChar(const KeyEvent &keyEvent)
{
    return KeyEventToQChar(keyEvent.key(), keyEvent.text(), keyEvent.modifiers());
}

const QChar KeyParser::KeyEventToQChar(int keyCode, const QString &text, Qt::KeyboardModifiers mods) const
{
    // If previous key press was AltGr, return key value right away and don't go
    // down the "handle modifiers" code path. AltGr is really confusing...
    if (mods & Qt::GroupSwitchModifier) {
        return (!text.isEmpty()) ? text.at(0) : QChar();
    }

    if (text.isEmpty() || (text.length() == 1 && text.at(0) < QChar(0x20)) || keyCode == Qt::Key_Delete
        || (mods != Qt::NoModifier && mods != Qt::ShiftModifier && mods != Qt::KeypadModifier)) {
        QString keyPress;
        keyPress.reserve(11);

        keyPress.append(QLatin1Char('<'));
        keyPress.append((mods & Qt::ShiftModifier) ? QStringLiteral("s-") : QString());
        keyPress.append((mods & CONTROL_MODIFIER) ? QStringLiteral("c-") : QString());
        keyPress.append((mods & Qt::AltModifier) ? QStringLiteral("a-") : QString());
        keyPress.append((mods & META_MODIFIER) ? QStringLiteral("m-") : QString());
        keyPress.append(keyCode <= 0xFF ? QChar(keyCode) : qt2vi(keyCode));
        keyPress.append(QLatin1Char('>'));

        return encodeKeySequence(keyPress).at(0);
    } else {
        return text.at(0);
    }
}
