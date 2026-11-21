// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string>

#include "base/timer.h"
#include "desktop/widgets/widget.h"

namespace Lingmo::ui {
class Label;
class Painter;
}

namespace fm {

// How busy the machine is: the CPU and the memory, sampled a second at a
// time from /proc, with a bar for each.
class SystemWidget : public DesktopWidget
{
public:
    SystemWidget(WidgetLayer& layer, WidgetSize size);
    ~SystemWidget() override;

protected:
    void layoutContent() override;
    void paintContent(ui::Painter& painter) override;

private:
    void sample();

    ui::Label* m_cpu = nullptr;
    ui::Label* m_memory = nullptr;
    base::Timer m_timer;
    double m_cpuUsage = 0;
    double m_memoryUsage = 0;
    uint64_t m_previousTotal = 0;
    uint64_t m_previousIdle = 0;
    ui::RectF m_cpuBar;
    ui::RectF m_memoryBar;
};

} // namespace fm
