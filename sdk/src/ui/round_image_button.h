// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <optional>
#include <string>

#include "ui/view.h"

namespace Lingmo::ui {

class ThemeIcon;

// FishUI's RoundImageButton: an icon on a round plate that shows on hover
// and press, kept on the icon's pixel parity so both share a centre.
class RoundImageButton : public View
{
public:
    // One of the toolkit's glyphs, tinted with the icon colour.
    explicit RoundImageButton(std::string iconName = {}, double size = 32);

    // The button's width and height.
    void setButtonSize(double size);
    double buttonSize() const { return m_size; }
    // Around the icon; the icon is the button less these unless sized.
    void setIconMargins(double margins);
    // The size the icon is drawn at; 0 follows the button and the margins.
    void setIconSize(double size);
    double iconSize() const;
    void setIconOffset(PointF offset);

    void setIconName(std::string name);
    const std::string& iconName() const { return m_iconName; }
    // A theme icon name or a file, drawn untinted, when no glyph is set.
    void setSource(std::string source);
    const std::string& source() const { return m_source; }
    // The glyph's colour; unset follows the theme's text.
    void setIconColor(std::optional<Color> color);
    // The plate at rest, transparent unless set.
    void setBackgroundColor(std::optional<Color> color);
    // Unset, the text colour at 0.11 (0.12 dark) hovered, 0.21 (0.06 dark)
    // pressed.
    void setHoveredColor(std::optional<Color> color);
    void setPressedColor(std::optional<Color> color);
    // The hovered plate without the pointer, as for a drop target.
    void setHighlighted(bool highlighted);
    bool isHighlighted() const { return m_highlighted; }
    // The pressed plate while set, as while the button's menu is open.
    void setChecked(bool checked);
    bool isChecked() const { return m_checked; }
    // Unset, 1 enabled and 0.35 disabled.
    void setIconOpacity(std::optional<double> opacity);

    bool isPressed() const { return m_pressed; }

    SizeF sizeHint() const override { return {m_size, m_size}; }
    bool acceptsPointer() const override { return true; }

    std::function<void()> onClicked;

protected:
    void layout() override;
    void paint(Painter& painter) override;
    void enabledChanged() override;
    void pointerEnter() override;
    void pointerLeave() override;
    void pointerPress(const PointerEvent& event) override;
    void pointerRelease(const PointerEvent& event) override;

private:
    // The icon's top left in the button, on whole logical pixels.
    PointF iconPosition() const;
    void updateIcon();

    std::string m_iconName;
    std::string m_source;
    double m_size;
    double m_iconMargins = 0;
    double m_iconSize = 0;
    PointF m_iconOffset;
    std::optional<Color> m_iconColor;
    std::optional<Color> m_backgroundColor;
    std::optional<Color> m_hoveredColor;
    std::optional<Color> m_pressedColor;
    std::optional<double> m_iconOpacity;
    bool m_highlighted = false;
    bool m_checked = false;
    bool m_hovered = false;
    bool m_pressed = false;
    ThemeIcon* m_image;
};

} // namespace Lingmo::ui
