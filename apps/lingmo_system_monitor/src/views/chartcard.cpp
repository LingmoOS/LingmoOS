// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "chartcard.h"

#include "historygraph.h"
#include "monitor.h"

#include "base/i18n.h"
#include "ui/cell_layout.h"
#include "ui/theme.h"

#include <algorithm>

using namespace Lingmo::ui;
using Lingmo::i18n::tr;

static constexpr double kPadding = 16;
static constexpr double kSpacing = 10;

ChartCard::ChartCard()
    : Plate(Theme::current().bigRadius)
    , m_column(addChild<ColumnLayout>())
{
    m_column->setSpacing(kSpacing);
    auto* top = m_column->addChild<RowLayout>();
    top->setSpacing(16);
    top->setLayoutHints({.fillWidth = true});
    m_title = addLabel(top, {}, 11, 500);
    m_title->setLayoutHints({.fillWidth = true});
    m_value = addLabel(top, {}, 18, 500);
    m_subtitle = addLabel(m_column, {}, 9);
    m_subtitle->setLayoutHints({.fillWidth = true});
    m_scale = addLabel(m_column, {}, 8);
    m_scale->setName("chartScaleLabel");
    m_scale->setLayoutHints({.horizontalAlignment = Alignment::End});
    m_graph = m_column->addChild<HistoryGraph>();
    m_graph->setName("historyGraph");
    m_graph->setLayoutHints({.fillWidth = true, .fillHeight = true});
    auto* bottom = m_column->addChild<RowLayout>();
    bottom->setSpacing(12);
    bottom->setLayoutHints({.fillWidth = true});
    m_duration = addLabel(bottom, {}, 8);
    bottom->addChild<View>()->setLayoutHints({.fillWidth = true});
    m_primaryLegend = addLabel(bottom, {}, 8);
    m_secondaryLegend = addLabel(bottom, {}, 8);
    m_now = addLabel(bottom, tr("Now"), 8);
}

void ChartCard::setTitle(std::string title)
{
    m_title->setText(std::move(title));
}

void ChartCard::setValue(std::string value)
{
    if (value == m_value->text())
        return;
    m_value->setText(std::move(value));
    requestLayout();
}

void ChartCard::setSubtitle(std::string subtitle)
{
    m_subtitle->setText(std::move(subtitle));
}

void ChartCard::setColors(Color primary, Color secondary)
{
    m_primary = primary;
    m_secondary = secondary;
    m_graph->setColor(primary);
    m_graph->setSecondaryColor(secondary);
    requestLayout();
}

void ChartCard::setSeries(std::vector<double> values, std::vector<double> secondaryValues,
                          std::vector<int64_t> timestamps, int64_t duration)
{
    m_graph->setValues(std::move(values));
    m_graph->setSecondaryValues(std::move(secondaryValues));
    m_graph->setTimestamps(std::move(timestamps));
    m_graph->setDuration(duration);
    updateScale();
}

void ChartCard::setMaximum(double maximum)
{
    m_maximum = maximum;
    m_graph->setMaximum(maximum);
    updateScale();
}

void ChartCard::setScaleLabel(std::string label)
{
    m_scaleText = std::move(label);
    updateScale();
}

void ChartCard::setLegends(std::string primary, std::string secondary)
{
    m_primaryLegend->setText(std::move(primary));
    m_secondaryLegend->setText(std::move(secondary));
    m_primaryLegend->setVisible(!m_primaryLegend->text().empty());
    m_secondaryLegend->setVisible(!m_secondaryLegend->text().empty());
    requestLayout();
}

void ChartCard::setDurationText(std::string text)
{
    m_duration->setText(std::move(text));
}

void ChartCard::updateScale()
{
    const std::string text = !m_scaleText.empty() ? m_scaleText
                              : m_maximum > 0     ? std::string("100%")
                                                  : Monitor::formatRate(m_graph->effectiveMaximum());
    if (text == m_scale->text())
        return;
    m_scale->setText(text);
    requestLayout();
}

void ChartCard::layout()
{
    const Theme& theme = Theme::current();
    m_subtitle->setColor(theme.disabledText);
    m_scale->setColor(theme.disabledText);
    m_duration->setColor(theme.disabledText);
    m_now->setColor(theme.disabledText);
    m_primaryLegend->setColor(m_primary);
    m_secondaryLegend->setColor(m_secondary);
    m_graph->setGridColor(theme.text.withAlpha(0.06));
    m_column->setGeometry(localRect().adjusted(kPadding, kPadding, -kPadding, -kPadding));
}
