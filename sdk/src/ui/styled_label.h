// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

#include "ui/label.h"

namespace Lingmo::ui {

// A Label with a raised or sunken style: the text drawn first in the style
// colour one logical pixel down (Raised) or up (Sunken), then
// the text over it. Layout and size are a Label's.
class StyledLabel : public Label
{
public:
    enum class Style {
        Normal,
        Raised,
        Sunken,
    };

    explicit StyledLabel(std::string text = {});

    void setStyle(Style style, Color color);
    Style style() const { return m_style; }

    RectF visualRect() const override;

protected:
    void paint(Painter& painter) override;
    // Text with a style, as with a shadow, is greyscale on this stack.
    bool usesSubpixel() const override { return m_style == Style::Normal && Label::usesSubpixel(); }

private:
    Style m_style = Style::Normal;
    Color m_styleColor;
};

} // namespace Lingmo::ui
