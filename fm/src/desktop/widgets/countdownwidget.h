// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <chrono>
#include <ctime>
#include <string>

#include "base/timer.h"
#include "desktop/widgets/widget.h"

namespace Lingmo::ui {
class Label;
}

namespace fm {

// Days/hours/minutes/seconds until a named event, read from
// filemanager/countdown.ini ([Countdown] Name= and TargetDate=YYYY-MM-DD).
// Small: days only. Medium: four columns. Large: four columns + event label
// and a decorative progress arc in the corner.
class CountdownWidget : public DesktopWidget
{
public:
    CountdownWidget(WidgetLayer& layer, WidgetSize size);
    ~CountdownWidget() override;

protected:
    void layoutContent() override;
    void paintContent(ui::Painter& painter) override;
    void retranslateContent() override;

private:
    void refresh();
    void loadConfig();
    static std::string settingsFile();

    ui::Label* m_eventLabel = nullptr;  // event name (Medium/Large)
    ui::Label* m_daysVal    = nullptr;
    ui::Label* m_hoursVal   = nullptr;
    ui::Label* m_minsVal    = nullptr;
    ui::Label* m_secsVal    = nullptr;
    ui::Label* m_daysUnit   = nullptr;
    ui::Label* m_hoursUnit  = nullptr;
    ui::Label* m_minsUnit   = nullptr;
    ui::Label* m_secsUnit   = nullptr;
    ui::Label* m_expired    = nullptr;  // shown when diff <= 0

    base::Timer  m_timer;
    std::string  m_eventName;
    std::time_t  m_targetTime = 0;
    bool         m_isExpired  = false;
};

} // namespace fm
