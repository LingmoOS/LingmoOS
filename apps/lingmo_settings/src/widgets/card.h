// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/painter.h"
#include "ui/cell_layout.h"

namespace settings {

// A rounded panel on a page in FishUI's second background colour: the
// RoundedItem's ColumnLayout, its rows its children. A list card holds
// ListRows edge to edge, without padding, so a row's press background
// reaches the card's edge and its corners follow the card's.
class Card : public Lingmo::ui::ColumnLayout
{
public:
    enum class Style {
        Padded,
        List,
    };

    explicit Card(Style style = Style::Padded);

protected:
    void paint(Lingmo::ui::Painter& painter) override;
};


// Kept for pages written before the toolkit had it; new code calls
// Lingmo::ui::themeCorners().
inline Lingmo::ui::Corners panelCorners()
{
    return Lingmo::ui::themeCorners();
}

} // namespace settings
