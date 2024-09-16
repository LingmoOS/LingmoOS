// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "common/common.h"

#include <dimcore/InputMethodAddon.h>
#include <xkbcommon/xkbcommon.h>

class QDomElement;

namespace org {
namespace deepin {
namespace dim {

class Keyboard : public InputMethodAddon
{
public:
    explicit Keyboard(Dim *dim);
    ~Keyboard() override;

    const QList<InputMethodEntry> &getInputMethods() override;
    void initInputMethods() override;
    bool keyEvent(const InputMethodEntry &entry, InputContextKeyEvent &keyEvent) override;
    void updateSurroundingText(InputContextEvent &event) override;

private:
    void parseRule(const QString &file);
    void parseLayoutList(const QDomElement &layoutListEle);
    void parseVariantList(const std::string &layoutName, const QDomElement &variantListEle);

private:
    std::unique_ptr<struct xkb_context, Deleter<xkb_context_unref>> ctx_;
    std::unique_ptr<struct xkb_keymap, Deleter<xkb_keymap_unref>> keymap_;
    std::unique_ptr<struct xkb_state, Deleter<xkb_state_unref>> state_;
    QList<InputMethodEntry> keyboards_;
};

} // namespace dim
} // namespace deepin
} // namespace org

#endif // !KEYBOARD_H
