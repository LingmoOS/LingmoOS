// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

#include "ui/header_button.h"
#include "ui/view.h"

namespace Lingmo::ui {

// A row of header buttons on a single shared plate, one square each, with a
// hairline divider between neighbours.  Hidden buttons give up their square.
// The plate uses the same frosted-glass style as an individual HeaderButton
// but groups them so they read as one control.
class HeaderButtonGroup : public View
{
public:
    // Each button is kSegment × kSegment logical pixels.
    static constexpr double kSegment = 26;

    HeaderButtonGroup();

    // Add a button showing one of the toolkit glyphs, e.g. "search" or "sidebar".
    HeaderButton* addButton(std::string glyph, double glyphSize = 16);
    const std::vector<HeaderButton*>& buttons() const { return m_buttons; }

    SizeF sizeHint() const override;

protected:
    void layout() override;
    void paint(Painter& painter) override;

private:
    std::vector<HeaderButton*> m_buttons;
};

} // namespace Lingmo::ui
