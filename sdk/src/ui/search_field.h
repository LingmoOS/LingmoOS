// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>
#include <string>

#include "ui/text_field.h"

namespace Lingmo::ui {

class Icon;

// The applications' pill search field: a search glyph before the text, a
// clear button after it while there is text, and the focus border and ring.
class SearchField : public TextField
{
public:
    explicit SearchField(std::string placeholder = {});

    // The pill's colour; the theme's second background unless set.
    void setFill(std::optional<ColorRecipe> fill);
    // The glyphs' distance from the ends, the gap between a glyph and the
    // text, and how far the clear button's hit area reaches past its glyph.
    // 12, 8 and 7 unless set.
    void setIconMetrics(double inset, double gap, double hitMargin);
    // The search and clear glyphs in this colour; the theme's text unless set.
    void setGlyphColor(std::optional<ColorRecipe> color);

protected:
    void paint(Painter& painter) override;
    void layout() override;
    void textChanged() override;

private:
    class ClearButton;

    void updateMargins();

    std::optional<ColorRecipe> m_fill;
    double m_inset = 12;
    double m_gap = 8;
    double m_hitMargin = 7;
    Icon* m_search;
    ClearButton* m_clear;
};

} // namespace Lingmo::ui
