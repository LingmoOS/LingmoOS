// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "InputMethodContextV1.h"

#include "InputMethodV1.h"
#include "Server.h"
#include "inputmethodv1/InputMethodGrabV1.h"
#include "inputmethodv1/ZwpInputMethodContextV1.h"

WL_ADDONS_BASE_USE_NAMESPACE

InputMethodContextV1::InputMethodContextV1(InputMethodV1 *inputMethodV1)
    : ZwpInputMethodContextV1(inputMethodV1)
{
    init(inputMethodV1->getResource()->client(), 0);
}

InputMethodContextV1::~InputMethodContextV1() = default;

void InputMethodContextV1::sendContentType(uint32_t hint, uint32_t purpose)
{
    send_content_type(getResource()->handle, hint, purpose);
}

void InputMethodContextV1::sendSurroundingText(const char *text, uint32_t cursor, uint32_t anchor)
{
    send_surrounding_text(getResource()->handle, text, cursor, anchor);
}

void InputMethodContextV1::setCursorRectangle(int x, int y, int width, int height) { }

void InputMethodContextV1::sendKey(uint32_t keycode, bool isRelease)
{
    uint32_t ts = getTimestamp();

    getKeyboardGrab()->sendKey(0,
                               ts,
                               keycode,
                               isRelease ? WL_KEYBOARD_KEY_STATE_RELEASED
                                         : WL_KEYBOARD_KEY_STATE_PRESSED);
}
