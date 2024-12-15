/*
 Copyright (C) 2008 Carlo Segato <brandon.ml@gmail.com>

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "kkeyserver_win.h"

#include <windows.h>

#include <QDebug>

namespace KKeyServer
{

struct TransKey {
    uint keySymQt;
    uint keySymWin;
};

static const TransKey KeyTbl[] = {
    { Qt::Key_Cancel,       VK_CANCEL },
    { Qt::Key_Backspace,    VK_BACK },
    { Qt::Key_Tab,          VK_TAB },
    { Qt::Key_Clear,        VK_CLEAR },
    { Qt::Key_Return,       VK_RETURN },
    { Qt::Key_Shift,            VK_SHIFT },
    { Qt::Key_Control,          VK_CONTROL },
    { Qt::Key_Alt,          VK_MENU },
    { Qt::Key_Pause,            VK_PAUSE },
    { Qt::Key_CapsLock,         VK_CAPITAL },
    { Qt::Key_Escape,           VK_ESCAPE },
//FIXME: under wince VK_MODECHANGE is not defined
#ifndef _WIN32_WCE
    { Qt::Key_Mode_switch,          VK_MODECHANGE },
#endif
    { Qt::Key_Space,            VK_SPACE },
    { Qt::Key_PageUp,           VK_PRIOR },
    { Qt::Key_PageDown,         VK_NEXT },
    { Qt::Key_End,          VK_END },
    { Qt::Key_Home,         VK_HOME },
    { Qt::Key_Left,         VK_LEFT },
    { Qt::Key_Up,           VK_UP },
    { Qt::Key_Right,            VK_RIGHT },
    { Qt::Key_Down,         VK_DOWN },
    { Qt::Key_Select,           VK_SELECT },
    { Qt::Key_Printer,          VK_PRINT },
    { Qt::Key_Execute,          VK_EXECUTE },
    { Qt::Key_Print,            VK_SNAPSHOT },
    { Qt::Key_Insert,           VK_INSERT },
    { Qt::Key_Delete,           VK_DELETE },
    { Qt::Key_Help,         VK_HELP },
    /*        { 0,            VK_0 },
            { 0,            VK_1 },
            { 0,            VK_2 },
            { 0,            VK_3 },
            { 0,            VK_4 },
            { 0,            VK_5 },
            { 0,            VK_6 },
            { 0,            VK_7 },
            { 0,            VK_8 },
            { 0,            VK_9 },
            { 0,            VK_A },
            { 0,            VK_B },
            { 0,            VK_C },
            { 0,            VK_D },
            { 0,            VK_E },
            { 0,            VK_F },
            { 0,            VK_G },
            { 0,            VK_H },
            { 0,            VK_I },
            { 0,            VK_J },
            { 0,            VK_K },
            { 0,            VK_L },
            { 0,            VK_M },
            { 0,            VK_N },
            { 0,            VK_O },
            { 0,            VK_P },
            { 0,            VK_Q },
            { 0,            VK_R },
            { 0,            VK_S },
            { 0,            VK_T },
            { 0,            VK_U },
            { 0,            VK_V },
            { 0,            VK_W },
            { 0,            VK_X },
            { 0,            VK_Y },
            { 0,            VK_Z },*/
    { Qt::Key_Meta,         VK_LWIN },
    { Qt::Key_Meta,         VK_RWIN },
    { Qt::Key_Menu,         VK_APPS },
    { Qt::Key_Sleep,            VK_SLEEP },
    { Qt::Key_0,            VK_NUMPAD0 },
    { Qt::Key_1,            VK_NUMPAD1 },
    { Qt::Key_2,            VK_NUMPAD2 },
    { Qt::Key_3,            VK_NUMPAD3 },
    { Qt::Key_4,            VK_NUMPAD4 },
    { Qt::Key_5,            VK_NUMPAD5 },
    { Qt::Key_6,            VK_NUMPAD6 },
    { Qt::Key_7,            VK_NUMPAD7 },
    { Qt::Key_8,            VK_NUMPAD8 },
    { Qt::Key_9,            VK_NUMPAD9 },
    { Qt::Key_Asterisk,         VK_MULTIPLY },
    { Qt::Key_Plus,         VK_ADD },
    { Qt::Key_Comma,            VK_SEPARATOR },
    { Qt::Key_Minus,            VK_SUBTRACT },
    { Qt::Key_Period,           VK_DECIMAL },
    { Qt::Key_Slash,            VK_DIVIDE },
    { Qt::Key_F1,           VK_F1 },
    { Qt::Key_F2,           VK_F2 },
    { Qt::Key_F3,           VK_F3 },
    { Qt::Key_F4,           VK_F4 },
    { Qt::Key_F5,           VK_F5 },
    { Qt::Key_F6,           VK_F6 },
    { Qt::Key_F7,           VK_F7 },
    { Qt::Key_F8,           VK_F8 },
    { Qt::Key_F9,           VK_F9 },
    { Qt::Key_F10,          VK_F10 },
    { Qt::Key_F11,          VK_F11 },
    { Qt::Key_F12,          VK_F12 },
    { Qt::Key_F13,          VK_F13 },
    { Qt::Key_F14,          VK_F14 },
    { Qt::Key_F15,          VK_F15 },
    { Qt::Key_F16,          VK_F16 },
    { Qt::Key_F17,          VK_F17 },
    { Qt::Key_F18,          VK_F18 },
    { Qt::Key_F19,          VK_F19 },
    { Qt::Key_F20,          VK_F20 },
    { Qt::Key_F21,          VK_F21 },
    { Qt::Key_F22,          VK_F22 },
    { Qt::Key_F23,          VK_F23 },
    { Qt::Key_F24,          VK_F24 },
    { Qt::Key_NumLock,          VK_NUMLOCK },
    { Qt::Key_ScrollLock,           VK_SCROLL },
//         { 0,            VK_OEM_FJ_JISHO },
//         { Qt::Key_Massyo,           VK_OEM_FJ_MASSHOU },
//         { Qt::Key_Touroku,          VK_OEM_FJ_TOUROKU },
//         { 0,            VK_OEM_FJ_LOYA },
//         { 0,            VK_OEM_FJ_ROYA },
    { Qt::Key_Shift,            VK_LSHIFT },
    { Qt::Key_Shift,            VK_RSHIFT },
    { Qt::Key_Control,          VK_LCONTROL },
    { Qt::Key_Control,          VK_RCONTROL },
    { Qt::Key_Alt,          VK_LMENU },
    { Qt::Key_Alt,          VK_RMENU },

//  winuser.h from psdk w2kserver2003R2 defines the following constants only for _WIN32_WINNT >= 0x0500
// and we should also do so - otherwise the constants may not be available
#if(_WIN32_WINNT >= 0x0500)
    { Qt::Key_Back,         VK_BROWSER_BACK },
    { Qt::Key_Forward,          VK_BROWSER_FORWARD },
    { Qt::Key_Refresh,          VK_BROWSER_REFRESH },
    { Qt::Key_Stop,         VK_BROWSER_STOP },
    { Qt::Key_Search,           VK_BROWSER_SEARCH },
    { Qt::Key_Favorites,            VK_BROWSER_FAVORITES },
    { Qt::Key_HomePage,         VK_BROWSER_HOME },
    { Qt::Key_VolumeMute,           VK_VOLUME_MUTE },
    { Qt::Key_VolumeDown,           VK_VOLUME_DOWN },
    { Qt::Key_VolumeUp,         VK_VOLUME_UP },
    { Qt::Key_MediaNext,            VK_MEDIA_NEXT_TRACK },
    { Qt::Key_MediaPrevious,            VK_MEDIA_PREV_TRACK },
    { Qt::Key_MediaStop,            VK_MEDIA_STOP },
    { Qt::Key_MediaPlay,            VK_MEDIA_PLAY_PAUSE },
    { Qt::Key_LaunchMail,           VK_LAUNCH_MAIL },
    { Qt::Key_LaunchMedia,          VK_LAUNCH_MEDIA_SELECT },
    { Qt::Key_Launch0,          VK_LAUNCH_APP1 },
    { Qt::Key_Launch1,          VK_LAUNCH_APP2 },
#endif
    /*        { 0,            VK_OEM_1 },
            { 0,            VK_OEM_PLUS },
            { 0,            VK_OEM_COMMA },
            { 0,            VK_OEM_MINUS },
            { 0,            VK_OEM_PERIOD },
            { 0,            VK_OEM_2 },
            { 0,            VK_OEM_3 },
            { 0,            VK_OEM_4 },
            { 0,            VK_OEM_5 },
            { 0,            VK_OEM_6 },
            { 0,            VK_OEM_7 },
            { 0,            VK_OEM_8 },*/
    { Qt::Key_Play,         VK_PLAY },
    { Qt::Key_Zoom,         VK_ZOOM },
    { Qt::Key_Clear,            VK_OEM_CLEAR },
//         { 0
};

bool keyQtToModWin(int keyQt, uint *mod)
{
    *mod = 0;
    if (keyQt & Qt::ShiftModifier) {
        *mod |= MOD_SHIFT;
    }
    if (keyQt & Qt::ControlModifier) {
        *mod |= MOD_CONTROL;
    }
    if (keyQt & Qt::AltModifier) {
        *mod |= MOD_ALT;
    }
    if (keyQt & Qt::MetaModifier) {
        *mod |= MOD_WIN;
    }

    return true;
}

bool modWinToKeyQt(uint mod, int *keyQt)
{
    *keyQt = 0;
    if (mod & MOD_SHIFT) {
        *keyQt |= Qt::ShiftModifier;
    }
    if (mod & MOD_CONTROL) {
        *keyQt |= Qt::ControlModifier;
    }
    if (mod & MOD_ALT) {
        *keyQt |= Qt::AltModifier;
    }
    if (mod & MOD_WIN) {
        *keyQt |= Qt::MetaModifier;
    }

    return true;
}

bool keyQtToCodeWin(int keyQt, uint *sym)
{
    int symQt = keyQt & ~Qt::KeyboardModifierMask;

    if (symQt >= Qt::Key_A && symQt <= Qt::Key_Z) {
        *sym = symQt;
        return true;
    }

    for (uint i = 0; i < sizeof(KeyTbl); i++) {
        if (KeyTbl[i].keySymQt == symQt) {
            *sym = KeyTbl[i].keySymWin;
            return true;
        }
    }

    *sym = 0;
    if (symQt != Qt::Key_Shift && symQt != Qt::Key_Control && symQt != Qt::Key_Alt &&
            symQt != Qt::Key_Meta && symQt != Qt::Key_Direction_L && symQt != Qt::Key_Direction_R) {
        // qDebug() << "Sym::initQt( " << QString::number(keyQt,16) << " ): failed to convert key.";
    }
    return false;
}

bool codeWinToKeyQt(uint sym, int *keyQt)
{
    if (sym >= Qt::Key_A && sym <= Qt::Key_Z) {
        *keyQt = sym;
        return true;
    }

    for (uint i = 0; i < sizeof(KeyTbl); i++) {
        if (KeyTbl[i].keySymWin == sym) {
            *keyQt = KeyTbl[i].keySymQt;
            return true;
        }
    }

    *keyQt = 0;
    return false;
}

}
