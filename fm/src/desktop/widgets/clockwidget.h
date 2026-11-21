// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

#include "base/timer.h"
#include "desktop/widgets/widget.h"

namespace Lingmo::ui {
class Label;
}

namespace fm {

// The time and the date, in the locale's own words, a minute at a time.
class ClockWidget : public DesktopWidget
{
public:
    ClockWidget(WidgetLayer& layer, WidgetSize size);
    ~ClockWidget() override;

protected:
    void layoutContent() override;
    void retranslateContent() override;

private:
    void refresh();

    ui::Label* m_time = nullptr;
    ui::Label* m_date = nullptr;
    base::Timer m_timer;
    std::string m_lastTime;
    std::string m_lastDate;
};

} // namespace fm
