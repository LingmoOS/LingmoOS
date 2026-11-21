// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <ctime>
#include <string>
#include "base/timer.h"
#include "desktop/widgets/widget.h"
namespace Lingmo::ui { class Label; }
namespace fm {
struct WorldClockTime {
    bool valid=false;
    int hour=0,minute=0,second=0,dayDifference=0,offsetMinutes=0;
};
// ICU conversion does not change the process TZ or race other widget workers.
WorldClockTime worldClockTime(const std::string& zone,std::time_t now);
class WorldClockWidget : public DesktopWidget {
public:
    WorldClockWidget(WidgetLayer& layer,WidgetSize size);
    ~WorldClockWidget() override;
protected:
    void layoutContent() override;
    void paintContent(ui::Painter&) override;
    void retranslateContent() override;
private:
    void refresh();
    void loadZones();
    ui::Label* m_timeLabels[4]={};
    ui::Label* m_cityLabels[4]={};
    ui::Label* m_dayLabels[4]={};
    ui::RectF m_faces[4];
    std::array<WorldClockTime,4> m_times;
    base::Timer m_timer;
    std::string m_zones[4];
    int m_zoneCount=0;
};
}
