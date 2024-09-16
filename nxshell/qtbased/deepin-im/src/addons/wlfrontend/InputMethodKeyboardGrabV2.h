// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INPUTMETHODKEYBOARDV2_H
#define INPUTMETHODKEYBOARDV2_H

#include "wayland-input-method-unstable-v2-client-protocol.h"
#include "wl/client/ZwpInputMethodKeyboardGrabV2.h"

#include <dimcore/InputContext.h>
#include <xkbcommon/xkbcommon.h>

#include <QObject>

#include <memory>

namespace org {
namespace deepin {
namespace dim {

class InputMethodKeyboardGrabV2QObject : public QObject
{
    Q_OBJECT

signals:
    void keymap(uint32_t format, int32_t fd, uint32_t size);
    void key(uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
    void modifiers(uint32_t serial,
                   uint32_t mods_depressed,
                   uint32_t mods_latched,
                   uint32_t mods_locked,
                   uint32_t group);
    void repeatInfo(int32_t rate, int32_t delay);
};

class InputMethodKeyboardGrabV2 : public wl::client::ZwpInputMethodKeyboardGrabV2
{
public:
    explicit InputMethodKeyboardGrabV2(zwp_input_method_keyboard_grab_v2 *val);

    InputMethodKeyboardGrabV2QObject *qobject() { return qobject_.get(); }

protected:
    void zwp_input_method_keyboard_grab_v2_keymap(uint32_t format,
                                                  int32_t fd,
                                                  uint32_t size) override;
    void zwp_input_method_keyboard_grab_v2_key(uint32_t serial,
                                               uint32_t time,
                                               uint32_t key,
                                               uint32_t state) override;
    void zwp_input_method_keyboard_grab_v2_modifiers(uint32_t serial,
                                                     uint32_t mods_depressed,
                                                     uint32_t mods_latched,
                                                     uint32_t mods_locked,
                                                     uint32_t group) override;
    void zwp_input_method_keyboard_grab_v2_repeat_info(int32_t rate, int32_t delay) override;

private:
    std::unique_ptr<InputMethodKeyboardGrabV2QObject> qobject_;
};

} // namespace dim
} // namespace deepin
} // namespace org

#endif // !INPUTMETHODKEYBOARDV2_H
