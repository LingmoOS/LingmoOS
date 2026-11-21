// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

#include "desktop/widgets/widget.h"
#include "base/timer.h"

namespace Lingmo::ui {
class Label;
}

namespace fm {

class MonthArrow;

// A month at a glance: the locale's month title, its weekday names and its
// days, with today marked. The arrows step a month either way.
class CalendarWidget : public DesktopWidget
{
public:
    CalendarWidget(WidgetLayer& layer, WidgetSize size);
    ~CalendarWidget() override;

protected:
    void layoutContent() override;
    void paintContent(ui::Painter& painter) override;
    void retranslateContent() override;

private:
    void showMonth(int year, int month);
    void shiftMonth(int delta);
    void updateDayLabels();

    ui::Label* m_title = nullptr;
    base::Timer m_dayTimer;
    int m_lastDay=0;
    std::vector<ui::Label*> m_weekdays;
    std::vector<ui::Label*> m_days;
    MonthArrow* m_previous = nullptr;
    MonthArrow* m_next = nullptr;
    int m_year = 0;
    int m_month = 1;
    int m_todayIndex = -1;
};

} // namespace fm
