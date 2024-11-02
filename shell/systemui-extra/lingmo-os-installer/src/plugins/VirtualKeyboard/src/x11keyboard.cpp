/**
 * Copyright (C) 2018 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
**/
#include "x11keyboard.h"
#include <QDebug>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>



struct CharMap {
    QChar    name;
    KeySym   code;
};

struct CharMap XSpecialSymbolMap[] {
    {' ',   XK_space},
    {',',   XK_comma},
    {'.',   XK_period},
    {'\'',  XK_quoteright},
    {'@',   XK_at},
    {'#',   XK_numbersign},
    {'$', XK_dollar},
    {'%',   XK_percent},
    {'&',   XK_ampersand},
    {'*',   XK_asterisk},
    {'(',   XK_parenleft},
    {')',   XK_parenright},
    {'-',   XK_minus},
    {'+',   XK_plus},
    {'!',   XK_exclam},
    {'"',   XK_quotedbl},
    {'<',   XK_less},
    {'>',   XK_greater},
    {':',   XK_colon},
    {';',   XK_semicolon},
    {'/',   XK_slash},
    {'?',   XK_question},
    {'=',   XK_equal},
    {'.',   XK_kana_middledot},
    {'~',   XK_asciitilde},
    {'`',   XK_grave},
    {'|',   XK_bar},
    {'^',   XK_asciicircum},
    {'{',   XK_braceleft},
    {'}',   XK_braceright},
    {'[',   XK_bracketleft},
    {']',   XK_bracketright},
    {'_',   XK_underscore},
    {'\\',  XK_backslash},
};

QMap<FuncKey::FUNCKEY, KeySym> funckeyMap = {
    {FuncKey::SPACE,     XK_space},
    {FuncKey::BACKSPACE, XK_BackSpace},
    {FuncKey::ENTER,     XK_Return},
    {FuncKey::HOME,      XK_Home},
    {FuncKey::END,       XK_End},
    {FuncKey::PGUP,      XK_Page_Up},
    {FuncKey::PGDN,      XK_Page_Down},
    {FuncKey::INSERT,    XK_Insert},
    {FuncKey::DELETE,    XK_Delete},
    {FuncKey::UP,        XK_Up},
    {FuncKey::DOWN,      XK_Down},
    {FuncKey::LEFT,      XK_Left},
    {FuncKey::RIGHT,     XK_Right}
};

QMap<Modifier::MOD, KeySym> modifierMap = {
    {Modifier::CTRL,    XK_Control_L},
    {Modifier::ALT,     XK_Alt_L},
    {Modifier::SUPER,   XK_Super_L},
    {Modifier::SHIFT,   XK_Shift_L}
};

QVector<QChar> shiftKeyVec = {'~', '!', '@', '#', '$', '%', '^', '&', '*',
                              '(', ')', '_', '+', '{', '}', '|', ':', '"',
                              '>', '?'};

static Display *display = XOpenDisplay(0);
bool isShift = false;
bool isLetter = false;
unsigned int keyCodeOfChar(QChar c)
{
    QString text(c);

    KeySym keysym = XStringToKeysym(text.toLocal8Bit().data());

    if(keysym == NoSymbol) {
        int symbolCount = sizeof(XSpecialSymbolMap) / sizeof(struct CharMap);
        for(int i = 0; i < symbolCount; i++) {
            if(XSpecialSymbolMap[i].name == c) {
                keysym = XSpecialSymbolMap[i].code;
                break;
            }
        }
    }
    qDebug() << "keysym: " << keysym;

    isShift = shiftKeyVec.contains(c) || (c >= 'A' && c <= 'Z');

    isLetter = c.isLetter();

    KeyCode code = XKeysymToKeycode(display, keysym);

    return code;
}

X11Keyboard::X11Keyboard(QObject *parent)
    : QObject(parent)
{

}

X11Keyboard::~X11Keyboard()
{
    XCloseDisplay(display);
}

void X11Keyboard::addModifier(Modifier::MOD mod)
{
    modList.push_back(mod);
}

void X11Keyboard::removeModifier(Modifier::MOD mod)
{
    modList.removeOne(mod);
}

bool X11Keyboard::hasModifier(Modifier::MOD mod)
{
    return modList.contains(mod);
}

QList<Modifier::MOD> X11Keyboard::getAllModifier()
{
    return modList;
}

void X11Keyboard::clearModifier()
{
    modList.clear();
}


void X11Keyboard::onKeyPressed(QChar c)
{
    unsigned int keyCode = keyCodeOfChar(c);
    sendKey(keyCode);
}

void X11Keyboard::onKeyPressed(FuncKey::FUNCKEY key)
{
    KeyCode keyCode;
    KeySym keysym = funckeyMap[key];

    if(keysym != NoSymbol)
        keyCode = XKeysymToKeycode(display, keysym);

    sendKey(keyCode);
}

void X11Keyboard::sendKey(unsigned int keyCode)
{
    Window focusWindow;
    int revert;
    XGetInputFocus(display, &focusWindow, &revert);

    for(auto mod : modList){
        KeyCode keyCode = XKeysymToKeycode(display, modifierMap[mod]);
        XTestFakeKeyEvent(display, keyCode, True, 2);
    }

    //如果使用了修饰键（如ctrl、alt）且字符键是字母，则不起用shift键，否则快捷键不起作用
    if(!modList.isEmpty() && isLetter)
        isShift = false;

    if(isShift)
        XTestFakeKeyEvent(display, XKeysymToKeycode(display, XK_Shift_L), True, 2);

    XTestFakeKeyEvent(display, keyCode, True, CurrentTime);
    XTestFakeKeyEvent(display, keyCode, False, CurrentTime);

    if(isShift)
        XTestFakeKeyEvent(display, XKeysymToKeycode(display, XK_Shift_L), False, 2);
    for(auto mod : modList){
        KeyCode keyCode = XKeysymToKeycode(display, modifierMap[mod]);
        XTestFakeKeyEvent(display, keyCode, False, 2);
    }

    XFlush(display);
}
