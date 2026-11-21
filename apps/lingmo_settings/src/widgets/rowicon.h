// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

#include "ui/view.h"

namespace Lingmo::ui {
class Icon;
}

namespace settings {

class Picture;

// The sidebar's tile: a rounded square with a white glyph, for the sidebar
// and a list row's leading slot. The glyph is one of settings' sidebar
// pictures, or a toolkit glyph when the name has no ".svg".
class RowIcon : public Lingmo::ui::View
{
public:
    RowIcon(std::string glyph, Lingmo::ui::Color tint, Lingmo::ui::Color accent = {});

    void set(std::string glyph, Lingmo::ui::Color tint);
    // The current sidebar entry draws its tile inverted: white behind the
    // glyph in the accent, so that it still reads against the accent fill the
    // row itself is drawn with.
    void setInverted(bool inverted);

    Lingmo::ui::SizeF sizeHint() const override { return {24, 24}; }

protected:
    void layout() override;
    void paint(Lingmo::ui::Painter& painter) override;

private:
    void applyGlyphColor();

    Lingmo::ui::Color m_tint;
    Lingmo::ui::Color m_accent;
    bool m_inverted = false;
    bool m_pictureGlyph = false;
    Picture* m_picture;
    Lingmo::ui::Icon* m_glyph;
};

} // namespace settings
