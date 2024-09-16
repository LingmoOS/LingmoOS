// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INPUTMETHODCONTEXTV1_H
#define INPUTMETHODCONTEXTV1_H

#include "inputmethodv1/ZwpInputMethodContextV1.h"

WL_ADDONS_BASE_BEGIN_NAMESPACE

class Server;
class InputMethodV1;

class InputMethodContextV1 : public ZwpInputMethodContextV1
{
public:
    explicit InputMethodContextV1(InputMethodV1 *inputMethodV1);
    ~InputMethodContextV1() override;

    void sendContentType(uint32_t hint, uint32_t purpose);
    void sendSurroundingText(const char *text, uint32_t cursor, uint32_t anchor);
    void setCursorRectangle(int x, int y, int width, int height);
    void sendKey(uint32_t keycode, bool isRelease);
};

WL_ADDONS_BASE_END_NAMESPACE

#endif // !INPUTMETHODCONTEXTV1_H
