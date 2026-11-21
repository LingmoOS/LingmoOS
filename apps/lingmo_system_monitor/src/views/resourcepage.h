// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/signal.h"
#include "ui/scroll_view.h"

#include <vector>

class ChartCard;
class InfoRow;
class ListCard;
class Monitor;
class SectionLabel;

namespace Lingmo::ui {
class Column;
class ColumnLayout;
class GridLayout;
class Label;
class ProgressBar;
} // namespace Lingmo::ui

// A resource's chart and details: CPU (1), memory (2), network (3), disks (4)
// or GPUs (5).
class ResourcePage : public Lingmo::ui::ScrollView
{
public:
    ResourcePage(Monitor& monitor, int page);

    void setPage(int page);
    int page() const { return m_page; }

protected:
    void layout() override;

private:
    struct Section
    {
        Lingmo::ui::ColumnLayout* column = nullptr;
        SectionLabel* title = nullptr;
        ListCard* card = nullptr;
        Lingmo::ui::ProgressBar* usage = nullptr;
        std::vector<InfoRow*> rows;
    };
    struct Tile;

    void rebuild();
    void update();
    void updateChart();
    Section addSection(Lingmo::ui::View* parent, size_t rows);
    Lingmo::ui::Label* addHeading(const char* text);

    Monitor& m_monitor;
    int m_page;
    Lingmo::ui::Column* m_content;
    // As the Qt version's ColumnLayout, inside the scrolled margins.
    Lingmo::ui::ColumnLayout* m_layout;
    ChartCard* m_chart = nullptr;
    Lingmo::ui::GridLayout* m_cores = nullptr;
    std::vector<Tile*> m_tiles;
    ListCard* m_card = nullptr;
    std::vector<Section> m_sections;
    std::vector<Section> m_volumes;
    size_t m_built[3] = {0, 0, 0};
    Lingmo::base::Subscription m_updated;
};
