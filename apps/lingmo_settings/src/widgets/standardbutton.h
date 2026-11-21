// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <optional>
#include <string>

#include "ui/view.h"

namespace Lingmo::ui {
class Label;
}

namespace settings {

// A full-width button standing alone between cards: a flat plate that only
// darkens while pressed.
class StandardButton : public Lingmo::ui::View
{
public:
    explicit StandardButton(std::string text = {});

    void setText(std::string text);
    // Unset: #363636 in dark mode, white otherwise.
    void setBackgroundColor(std::optional<Lingmo::ui::Color> color);

    Lingmo::ui::SizeF sizeHint() const override;
    bool acceptsPointer() const override { return true; }
    bool acceptsFocus() const override { return true; }

    std::function<void()> onClicked;

protected:
    void layout() override;
    void paint(Lingmo::ui::Painter& painter) override;
    void pointerPress(const Lingmo::ui::PointerEvent& event) override;
    void pointerRelease(const Lingmo::ui::PointerEvent& event) override;
    bool keyPress(const Lingmo::ui::KeyEvent& event) override;

private:
    Lingmo::ui::Color backgroundColor() const;

    Lingmo::ui::Label* m_label;
    std::optional<Lingmo::ui::Color> m_background;
    bool m_pressed = false;
};

} // namespace settings
