// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/shortcuts/shortcutkeys.h"

#include "ui/color_provider.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/cell_layout.h"
#include "ui/theme.h"
#include "widgets/colors.h"
#include "widgets/metrics.h"

using namespace Lingmo::ui;

namespace settings {

static constexpr double kCapHeight = 24;
static constexpr double kCapPadding = 14;
static constexpr double kCapRadius = 6;

ShortcutKeys::ShortcutKeys(std::vector<std::string> parts)
{
    setParts(std::move(parts));
}

void ShortcutKeys::setParts(std::vector<std::string> parts)
{
    while (!children().empty())
        takeChild(children().back().get());
    m_parts.clear();
    for (size_t i = 0; i < parts.size(); ++i) {
        Part part;
        if (i > 0) {
            part.plus = addChild<Label>("+");
            part.plus->setFontSize(metrics::points(9));
        }
        part.cap = addChild<Label>(parts[i]);
        part.cap->setFontSize(metrics::points(9));
        m_parts.push_back(part);
    }
    requestLayout();
    requestPaint();
}

void ShortcutKeys::setDimmed(bool dimmed)
{
    if (dimmed == m_dimmed)
        return;
    m_dimmed = dimmed;
    for (const Part& part : m_parts)
        part.cap->setOpacity(dimmed ? 0.5 : 1);
    requestPaint();
}

double ShortcutKeys::capWidth(const Part& part) const
{
    return part.cap->sizeHint().width + kCapPadding;
}

SizeF ShortcutKeys::sizeHint() const
{
    double width = 0;
    for (size_t i = 0; i < m_parts.size(); ++i) {
        if (i > 0)
            width += metrics::xs + m_parts[i].plus->sizeHint().width + metrics::xs;
        width += capWidth(m_parts[i]);
    }
    return {width, m_parts.empty() ? 0 : kCapHeight};
}

// A Row of Rows as the Qt version laid it out: positions add up unrounded,
// and anchors centre each text in its 24 px cap.
void ShortcutKeys::layout()
{
    const Theme& theme = Theme::current();
    double x = 0;
    for (Part& part : m_parts) {
        if (part.plus) {
            x += metrics::xs;
            part.plus->setColor(theme.disabledText);
            const SizeF plus = part.plus->sizeHint();
            part.plus->setGeometry({x, rounding::centeredStart(kCapHeight, plus.height), plus.width, plus.height});
            x += plus.width + metrics::xs;
        }
        part.cap->setColor(theme.text);
        const SizeF label = part.cap->sizeHint();
        const double width = capWidth(part);
        part.capX = x;
        part.cap->setGeometry({x + rounding::centeredStart(width, label.width),
                               rounding::centeredStart(kCapHeight, label.height), label.width, label.height});
        x += width;
    }
}

void ShortcutKeys::paint(Painter& painter)
{
    const Color cap = color(SettingsColor::KeyCap);
    for (const Part& part : m_parts) {
        painter.save();
        painter.multiplyOpacity(m_dimmed ? 0.5 : 1);
        painter.drawRoundedRect({part.capX, 0, capWidth(part), kCapHeight}, {kCapRadius, 0, themeCorners()}, cap);
        painter.restore();
    }
}

} // namespace settings
