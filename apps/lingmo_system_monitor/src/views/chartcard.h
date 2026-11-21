// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "components.h"

#include <cstdint>
#include <string>
#include <vector>

class HistoryGraph;

namespace Lingmo::ui {
class ColumnLayout;
}

// A resource's minute of history at full width, with its current value.
class ChartCard : public Plate
{
public:
    ChartCard();

    void setTitle(std::string title);
    void setValue(std::string value);
    void setSubtitle(std::string subtitle);
    void setColors(Lingmo::ui::Color primary, Lingmo::ui::Color secondary);
    void setSeries(std::vector<double> values, std::vector<double> secondaryValues, std::vector<int64_t> timestamps,
                   int64_t duration);
    void setMaximum(double maximum);
    // Unset: 100 % for a fixed maximum, the scale's rate otherwise.
    void setScaleLabel(std::string label);
    void setLegends(std::string primary, std::string secondary);
    void setDurationText(std::string text);

    HistoryGraph* graph() const { return m_graph; }

    Lingmo::ui::SizeF sizeHint() const override { return {0, 276}; }

protected:
    void layout() override;

private:
    void updateScale();

    Lingmo::ui::ColumnLayout* m_column;
    Lingmo::ui::Label* m_title;
    Lingmo::ui::Label* m_value;
    Lingmo::ui::Label* m_subtitle;
    Lingmo::ui::Label* m_scale;
    HistoryGraph* m_graph;
    Lingmo::ui::Label* m_duration;
    Lingmo::ui::Label* m_primaryLegend;
    Lingmo::ui::Label* m_secondaryLegend;
    Lingmo::ui::Label* m_now;
    std::string m_scaleText;
    double m_maximum = 100;
    Lingmo::ui::Color m_primary;
    Lingmo::ui::Color m_secondary;
};
