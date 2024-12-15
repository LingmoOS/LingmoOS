// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WL_CLIENT_ZWPVIRTUALKEYBOARDV1_H
#define WL_CLIENT_ZWPVIRTUALKEYBOARDV1_H

#include "Type.h"

namespace wl {
namespace client {

class ZwpVirtualKeyboardV1 : public Type<zwp_virtual_keyboard_v1>
{
public:
    ZwpVirtualKeyboardV1(zwp_virtual_keyboard_v1 *val);
    ~ZwpVirtualKeyboardV1();

    void keymap(uint32_t format, int32_t fd, uint32_t size);
    void key(uint32_t time, uint32_t key, uint32_t state);
    void modifiers(uint32_t modsDepressed, uint32_t modsLatched, uint32_t modsLocked, uint32_t group);
};

} // namespace client
} // namespace wl

#endif // !WL_CLIENT_ZWPVIRTUALKEYBOARDV1_H
