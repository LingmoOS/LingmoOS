// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgets/card.h"

#include "ui/painter.h"
#include "ui/theme.h"
#include "widgets/metrics.h"

using namespace Lingmo::ui;

namespace settings {

Card::Card(Style style)
{
    if (style == Style::Padded) {
        setPadding({metrics::cardHorizontalPadding, metrics::cardVerticalPadding, metrics::cardHorizontalPadding,
                    metrics::cardVerticalPadding});
        setSpacing(metrics::cardSpacing);
    } else {
        setSpacing(0);
    }
    setLayoutHints({.fillWidth = true});
}

void Card::paint(Painter& painter)
{
    const Theme& theme = Theme::current();
    painter.drawRoundedRect(localRect(), {18, 0, themeCorners()}, theme.darkMode ? theme.secondBackground : theme.background);
}


} // namespace settings
