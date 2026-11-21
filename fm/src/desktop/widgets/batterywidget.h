// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

#include "base/timer.h"
#include "desktop/widgets/widget.h"

namespace Lingmo::ui {
class Label;
class Painter;
}

namespace fm {

// The battery's charge and what it is doing, sampled from
// /sys/class/power_supply; a machine without one says so instead.
class BatteryWidget : public DesktopWidget
{
public:
    BatteryWidget(WidgetLayer& layer, WidgetSize size);
    ~BatteryWidget() override;

protected:
    void layoutContent() override;
    void paintContent(ui::Painter& painter) override;
    void retranslateContent() override;

private:
    void refresh();

    ui::Label* m_title = nullptr;
    ui::Label* m_percent = nullptr;
    ui::Label* m_status = nullptr;
    base::Timer m_timer;
    bool m_present = false;
    int m_capacity = -1;
    std::string m_state;
    ui::RectF m_levelBar;
};

} // namespace fm
