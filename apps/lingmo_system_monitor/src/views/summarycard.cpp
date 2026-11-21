// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "summarycard.h"

#include "components.h"
#include "historygraph.h"
#include "textmetrics.h"

#include "base/i18n.h"
#include "ui/icon.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/theme.h"

#include <algorithm>

using namespace Lingmo::ui;
using Lingmo::i18n::tr;

static constexpr double kPadding = 12;
static constexpr double kIcon = 16;

void SidebarButton::setSelected(bool selected)
{
    if (m_selected == selected)
        return;
    m_selected = selected;
    requestLayout();
    requestPaint();
}

void SidebarButton::paint(Painter& painter)
{
    const Theme& theme = Theme::current();
    const Color accent = accentColor();
    // FishUI's RoundedRectangle with a border of 1 / Dpi.ratio: one device pixel.
    const bool bordered = m_selected || hasVisibleFocus();
    painter.drawRoundedRect(localRect(), {theme.mediumRadius, 0, themeCorners()},
                            m_selected ? accent.withAlpha(theme.darkMode ? 0.15 : 0.09) : theme.secondBackground,
                            bordered ? 1 / painter.scale() : 0, accent.withAlpha(0.5));
}

void SidebarButton::pointerPress(const PointerEvent& event)
{
    m_pressed = event.button == MouseButton::Left;
}

void SidebarButton::pointerRelease(const PointerEvent& event)
{
    const bool pressed = m_pressed;
    m_pressed = false;
    if (pressed && localRect().contains(event.position) && onClicked)
        onClicked();
}

bool SidebarButton::keyPress(const KeyEvent& event)
{
    if (event.modifiers != NoModifier || event.key != key::Space)
        return false;
    m_spacePressed = true;
    return true;
}

bool SidebarButton::keyRelease(const KeyEvent& event)
{
    if (event.key != key::Space || !m_spacePressed)
        return false;
    m_spacePressed = false;
    if (onClicked)
        onClicked();
    return true;
}

ProcessesButton::ProcessesButton()
    : m_icon(addChild<Icon>("processes", kIcon))
    , m_title(addLabel(this, tr("Processes"), 0, 500))
    , m_count(addLabel(this, {}, 9))
{
}

void ProcessesButton::setCount(std::string count)
{
    if (count == m_count->text())
        return;
    m_count->setText(std::move(count));
    requestLayout();
}

Color ProcessesButton::accentColor() const
{
    return Theme::current().highlight;
}

void ProcessesButton::layout()
{
    const Theme& theme = Theme::current();
    const double height = geometry().height;
    const double right = geometry().width - kPadding;
    // Capitals centred in the row, not the line box.
    const TextMetrics title = textMetrics(m_title);
    const double baseline = (height + title.capHeight) / 2;
    const double capCenter = snapToDevice(this, snapToDevice(this, baseline) - title.capHeight / 2);
    m_icon->setColor(isSelected() ? theme.highlight : theme.text);
    m_icon->setGeometry({kPadding, capCenter - kIcon / 2, kIcon, kIcon});
    const TextMetrics count = textMetrics(m_count);
    const double countWidth = m_count->sizeHint().width;
    m_count->setColor(theme.disabledText);
    m_count->setGeometry({right - countWidth, snapToDevice(this, baseline - count.ascent), countWidth, count.lineHeight});
    const double x = kPadding + kIcon + 8;
    m_title->setGeometry({x, snapToDevice(this, baseline - title.ascent), std::max(0.0, right - countWidth - 8 - x),
                          title.lineHeight});
}

SummaryCard::SummaryCard(std::string title, std::string icon, Color color)
    : m_color(color)
    , m_icon(addChild<Icon>(std::move(icon), kIcon))
    , m_title(addLabel(this, std::move(title), 9, 500))
    , m_value(addLabel(this, "—", 9, 500))
    , m_subtitle(addLabel(this, tr("Collecting data…"), 8))
    , m_graph(addChild<HistoryGraph>())
{
    m_icon->setColor(color);
    m_graph->setColor(color);
    m_graph->setCurved(true);
    m_graph->setLineWidth(1.25);
}

void SummaryCard::setValue(std::string value)
{
    if (value == m_value->text())
        return;
    m_value->setText(std::move(value));
    requestLayout();
}

void SummaryCard::setSubtitle(std::string subtitle)
{
    if (subtitle == m_subtitle->text())
        return;
    m_subtitle->setText(std::move(subtitle));
}

void SummaryCard::setHistory(std::vector<double> values, std::vector<int64_t> timestamps, int64_t duration)
{
    m_graph->setValues(std::move(values));
    m_graph->setTimestamps(std::move(timestamps));
    m_graph->setDuration(duration);
}

void SummaryCard::setMaximum(double maximum)
{
    m_graph->setMaximum(maximum);
}

void SummaryCard::setScaleFloor(double floor)
{
    m_graph->setScaleFloor(floor);
}

void SummaryCard::layout()
{
    const RectF content = localRect().adjusted(kPadding, kPadding, -kPadding, -kPadding);
    // The capitals' top, not the line box, sits on the card's padding.
    const TextMetrics title = textMetrics(m_title);
    const TextMetrics value = textMetrics(m_value);
    const double baseline = title.capHeight;
    const double capCenter = snapToDevice(this, snapToDevice(this, baseline) - title.capHeight / 2);
    const double iconTop = capCenter - kIcon / 2;
    const double titleTop = snapToDevice(this, baseline - title.ascent);
    const double valueTop = snapToDevice(this, baseline - value.ascent);
    // The row is as tall as what reaches lowest in it.
    const double row = std::max({iconTop + kIcon, titleTop + title.lineHeight, valueTop + value.lineHeight});
    m_icon->setGeometry({content.x, content.y + iconTop, kIcon, kIcon});
    const double valueWidth = m_value->sizeHint().width;
    m_value->setGeometry({content.right() - valueWidth, content.y + valueTop, valueWidth, value.lineHeight});
    const double titleX = kIcon + 8;
    m_title->setGeometry({content.x + titleX, content.y + titleTop,
                          std::max(0.0, content.width - titleX - valueWidth - 8), title.lineHeight});

    const double subtitleTop = row + 5;
    const double subtitleHeight = m_subtitle->sizeHint().height;
    m_subtitle->setColor(Theme::current().disabledText);
    m_subtitle->setGeometry({content.x + titleX, content.y + subtitleTop, content.width - titleX, subtitleHeight});
    const double graphTop = subtitleTop + subtitleHeight + 5;
    m_graph->setGeometry({content.x, content.y + graphTop, content.width, std::max(0.0, content.height - graphTop)});
}
