// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "show_buttons.h"
#include "keydefine.h"
#include <QDebug>


const ShowButtons::KeyDescription ShowButtons::keyDescriptions[] = {
    { "ESC", KEY_ESCAPE, Qt::Key_Escape},
    { "F1", KEY_F1, Qt::Key_F1},
    { "F2", KEY_F2, Qt::Key_F2},
    { "F3", KEY_F3, Qt::Key_F3},

    { "F4", KEY_F4, Qt::Key_F4},
    { "F5", KEY_F5, Qt::Key_F5},
    { "F6", KEY_F6, Qt::Key_F6},
    { "F7", KEY_F7, Qt::Key_F7},

    { "F8", KEY_F8, Qt::Key_F8},
    { "F9", KEY_F9, Qt::Key_F9},
    { "F10", KEY_F10, Qt::Key_F10},
    { "F11", KEY_F11, Qt::Key_F11},

    { "F12", KEY_F12, Qt::Key_F12},
    { "1", KEY_1, Qt::Key_1},
    { "2", KEY_2, Qt::Key_2},
    { "3", KEY_3, Qt::Key_3},

    { "4", KEY_4, Qt::Key_4},
    { "5", KEY_5, Qt::Key_5},
    { "6", KEY_6, Qt::Key_6},
    { "7", KEY_7, Qt::Key_7},
    { "8", KEY_8, Qt::Key_8},
    { "9", KEY_9, Qt::Key_9},
    { "0", KEY_0, Qt::Key_0},
    { "-", KEY_MINUS, Qt::Key_Minus},

    { "+", KEY_PLUS, Qt::Key_Plus},
    { "=", KEY_EQUAL, Qt::Key_Equal},
    { "A", KEY_A, Qt::Key_A},
    { "B", KEY_B, Qt::Key_B},

    { "C", KEY_C, Qt::Key_C},
    { "D", KEY_D, Qt::Key_D},
    { "E", KEY_E, Qt::Key_E},
    { "F", KEY_F, Qt::Key_F},

    { "G", KEY_G, Qt::Key_G},
    { "H", KEY_H, Qt::Key_H},
    { "I", KEY_I, Qt::Key_I},
    { "J", KEY_J, Qt::Key_J},

    { "K", KEY_K, Qt::Key_K},
    { "L", KEY_L, Qt::Key_L},
    { "M", KEY_M, Qt::Key_M},
    { "N", KEY_N, Qt::Key_N},
    { "O", KEY_O, Qt::Key_O},
    { "P", KEY_P, Qt::Key_P},
    { "Q", KEY_Q, Qt::Key_Q},
    { "R", KEY_R, Qt::Key_R},

    { "S", KEY_S, Qt::Key_S},
    { "T", KEY_T, Qt::Key_T},
    { "U", KEY_U, Qt::Key_U},
    { "V", KEY_V, Qt::Key_V},

    { "W", KEY_W, Qt::Key_W},
    { "X", KEY_X, Qt::Key_X},
    { "Y", KEY_Y, Qt::Key_Y},
    { "Z", KEY_Z, Qt::Key_Z},

    { "[", KEY_BRACKET_OPEN, Qt::Key_BracketLeft},
    { "]", KEY_BRACKET_CLOSE, Qt::Key_BracketRight},
    { ";", KEY_SEMICOLON, Qt::Key_Semicolon},
    { "'", KEY_QUOTE, Qt::Key_QuoteDbl},

    { "\\", KEY_BACKSLASH, Qt::Key_Backslash},
    { "|", KEY_VBAR,Qt::Key_Backslash },
    { ",", KEY_COMMA, Qt::Key_Comma},
    { ".", KEY_PERIOD, Qt::Key_Period},
    { "/", KEY_SLASH,  Qt::Key_Question},
    { "?", KEY_QUESTION, Qt::Key_Question},
    { "TAB", KEY_TAB, Qt::Key_Tab},
    { "Aa", KEY_CAPSLOCK, Qt::Key_CapsLock},

    { "LSHIFT", KEY_LSHIFT, Qt::Key_Shift},
    { "LCTRL", KEY_LCONTROL, Qt::Key_Control},
    { "LALT", KEY_LALT, Qt::Key_Alt},
    { "SPACE", KEY_SPACE, Qt::Key_Space},

    { "RALT", KEY_RALT, Qt::Key_Alt},
//    { "WIN", KEY_RWIN, Qt::Key_Menu},
//    { "MENU", KEY_MENU, Qt::Key_Menu},
    { "RCTRL", KEY_RCTRL, Qt::Key_Control},

    { "RSHIFT", KEY_RSHIFT, Qt::Key_Shift},
    { "ENTER", KEY_ENTER, Qt::Key_Enter},
    { "BACKSPACE", KEY_BACKSPACE, Qt::Key_Backspace},
    { "SCRLOCK", KEY_SCROLLLOCK, Qt::Key_ScrollLock},
    { "PAUSE", KEY_PAUSE, Qt::Key_Pause},

    { "INSERT", KEY_INSERT, Qt::Key_Insert},
    { "HOME", KEY_HOME, Qt::Key_Home},
    { "PAGEUP", KEY_PAGEUP, Qt::Key_PageUp},
    { "DELETE", KEY_DELETE, Qt::Key_Delete},

    { "END", KEY_END, Qt::Key_End},
    { "PAGEDOWN", KEY_PAGEDOWN, Qt::Key_PageDown},
    { "LEFT", KEY_LEFT, Qt::Key_Left},
    { "UP", KEY_UP, Qt::Key_Up},

    { "RIGHT", KEY_RIGHT, Qt::Key_Right},
    { "DOWN", KEY_DOWN, Qt::Key_Down},
    { "NUM", KEY_NUMLOCK, Qt::Key_NumLock},
    { "NUM /", KEY_DIVIDE, Qt::Key_Slash},
    { "NUM *", KEY_MULTIPLY, Qt::Key_0},
    { "NUM 7", KEY_NUM7, Qt::Key_7},
    { "NUM 8", KEY_NUM8, Qt::Key_8},

    { "NUM 9", KEY_NUM9, Qt::Key_9},
    { "NUM -", KEY_NUMMINUS, Qt::Key_Minus},
    { "NUM 4", KEY_NUM4, Qt::Key_4},
    { "NUM 5", KEY_NUM5, Qt::Key_5},

    { "NUM 6", KEY_NUM6, Qt::Key_6},
    { "NUM +", KEY_NUMPLUS, Qt::Key_Plus},
    { "NUM 1", KEY_NUM1, Qt::Key_1},
    { "NUM 2", KEY_NUM2, Qt::Key_2},
    { "NUM 3", KEY_NUM3, Qt::Key_3},
    { "NUM 0", KEY_NUM0, Qt::Key_0},
    { "NUM DEL", KEY_NUMDELETE, Qt::Key_Delete},
    { "NUM =", KEY_NUMENTER, Qt::Key_Enter}
};

ShowButtons::ShowButtons(DWidget *parent) : DWidget(parent)
{
}

void ShowButtons::showContentButtons(const int key)
{
    QString t_keyCode = "";
    t_keyCode = getKeyCodeFromEventWayland(key);

    if (t_keyCode != "") {
        if (m_keyCodeVec.contains(t_keyCode)) {
            return;
        } else {
            m_keyCodeVec.append(t_keyCode);
            qDebug() << t_keyCode << " key press";
            emit keyShowSignal(t_keyCode);
        }
    }

}

void ShowButtons::releaseContentButtons(const int key)
{
    QString t_keyCode = "";
    t_keyCode = getKeyCodeFromEventWayland(key);

    if (t_keyCode != "") {
        if (m_keyCodeVec.contains(t_keyCode)) {
            m_keyCodeVec.removeOne(t_keyCode);
            return;
        }
    }
}

void ShowButtons::showContentButtons(unsigned char keyCode)
{
    QString t_keyCode = "";
    t_keyCode = getKeyCodeFromEvent(keyCode);

    if (t_keyCode != "") {
        if (m_keyCodeVec.contains(t_keyCode)) {
            return;
        }

        else {
            m_keyCodeVec.append(t_keyCode);
            qDebug() << t_keyCode << " key press";
            emit keyShowSignal(t_keyCode);
        }
    }
}

void ShowButtons::releaseContentButtons(unsigned char keyCode)
{
    QString t_keyCode = "";
    t_keyCode = getKeyCodeFromEvent(keyCode);

    if (t_keyCode != "") {
        if (m_keyCodeVec.contains(t_keyCode)) {
            m_keyCodeVec.removeOne(t_keyCode);
            return;
        }
    }
}

QString ShowButtons::getKeyCodeFromEvent(unsigned char keyCode)
{
    QString t_keyCode = "";
    for (unsigned long var = 0; var < sizeof(ShowButtons::keyDescriptions) / sizeof(ShowButtons::keyDescriptions[0]); ++var) {
        if(ShowButtons::keyDescriptions[var].x11Key == keyCode) {
            t_keyCode = ShowButtons::keyDescriptions[var].text;
            break;
        }
    }
    return t_keyCode;
}

QString ShowButtons::getKeyCodeFromEventWayland(const int keyCode)
{
    QString t_keyCode = "";
    for (unsigned long var = 0; var < sizeof(ShowButtons::keyDescriptions) / sizeof(ShowButtons::keyDescriptions[0]); ++var) {
        if(ShowButtons::keyDescriptions[var].x11Key == keyCode) {
            t_keyCode = ShowButtons::keyDescriptions[var].text;
            break;
        }
    }
    return t_keyCode;
}
