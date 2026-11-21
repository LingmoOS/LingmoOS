// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "desktop/widgets/worldclockwidget.h"
#include "base/i18n.h"
#include "base/settings.h"
#include "core/config.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>
#include <numbers>
#include <unicode/calendar.h>
#include <unicode/timezone.h>
using Lingmo::I18n::tr;
namespace fm {
WorldClockTime worldClockTime(const std::string& zone,std::time_t now) {
    WorldClockTime out;
    std::unique_ptr<icu::TimeZone> tz(icu::TimeZone::createTimeZone(icu::UnicodeString::fromUTF8(zone)));
    icu::UnicodeString id;tz->getID(id);if(id==icu::UnicodeString("Etc/Unknown"))return out;
    UErrorCode error=U_ZERO_ERROR;
    std::unique_ptr<icu::Calendar> target(icu::Calendar::createInstance(tz.release(),error));
    std::unique_ptr<icu::Calendar> local(icu::Calendar::createInstance(error));
    if(U_FAILURE(error)||!target||!local)return out;
    target->setTime(double(now)*1000,error);local->setTime(double(now)*1000,error);
    out.hour=target->get(UCAL_HOUR_OF_DAY,error);out.minute=target->get(UCAL_MINUTE,error);out.second=target->get(UCAL_SECOND,error);
    out.dayDifference=target->get(UCAL_JULIAN_DAY,error)-local->get(UCAL_JULIAN_DAY,error);
    const int targetOffset=target->get(UCAL_ZONE_OFFSET,error)+target->get(UCAL_DST_OFFSET,error);
    const int localOffset=local->get(UCAL_ZONE_OFFSET,error)+local->get(UCAL_DST_OFFSET,error);
    out.offsetMinutes=(targetOffset-localOffset)/60000;out.valid=U_SUCCESS(error);return out;
}
namespace {
std::string cityName(const std::string& zone) {
    if(zone=="Asia/Shanghai")return tr("Shanghai");
    if(zone=="America/New_York")return tr("New York");
    if(zone=="Europe/London")return tr("London");
    if(zone=="Asia/Tokyo")return tr("Tokyo");
    if(zone=="Australia/Sydney")return tr("Sydney");
    auto city=zone.substr(zone.rfind('/')==std::string::npos?0:zone.rfind('/')+1);
    std::replace(city.begin(),city.end(),'_',' ');return city;
}
}
WorldClockWidget::WorldClockWidget(WidgetLayer& layer,WidgetSize size):DesktopWidget(layer,WidgetKind::WorldClock,size) {
    setName("world-clock-widget");
    for(int i=0;i<4;++i){
        m_timeLabels[i]=addChild<ui::Label>();m_cityLabels[i]=addChild<ui::Label>();m_dayLabels[i]=addChild<ui::Label>();
        m_cityLabels[i]->setName("world-city-"+std::to_string(i));
        for(auto* label:{m_timeLabels[i],m_cityLabels[i],m_dayLabels[i]}){label->setTextAlignment(ui::Label::TextAlignment::Center);label->setElide(ui::Label::Elide::Right);}
    }
    m_timer.setInterval(std::chrono::seconds(15));m_timer.onTimeout=[this]{refresh();};m_timer.start();loadZones();refresh();
}
WorldClockWidget::~WorldClockWidget()=default;
void WorldClockWidget::loadZones(){
    const base::Settings settings(fm::settingsFile());m_zoneCount=0;
    for(int i=0;i<4;++i){const auto zone=settings.stringValue("WorldClock/Zone"+std::to_string(i+1));if(!zone.empty())m_zones[m_zoneCount++]=zone;}
    if(!m_zoneCount){const char* zones[]={"America/New_York","Europe/London","Asia/Shanghai","Asia/Tokyo"};for(int i=0;i<4;++i)m_zones[i]=zones[i];m_zoneCount=4;}
    for(int i=0;i<m_zoneCount;++i){m_cityLabels[i]->setText(cityName(m_zones[i]));m_cityLabels[i]->setToolTip(m_zones[i]);}
}
void WorldClockWidget::refresh(){
    const auto now=std::time(nullptr);
    for(int i=0;i<m_zoneCount;++i){
        auto& time=m_times[i];time=worldClockTime(m_zones[i],now);
        char value[24];std::snprintf(value,sizeof(value),"%02d:%02d",time.hour,time.minute);
        m_timeLabels[i]->setText(time.valid?value:"—");
        if(!time.valid)m_dayLabels[i]->setText(tr("Invalid time zone"));
        else {
            const char* day=time.dayDifference<0?tr("Yesterday"):time.dayDifference>0?tr("Tomorrow"):tr("Today");
            const int delta=time.offsetMinutes;char offset[24];std::snprintf(offset,sizeof(offset),"%c%d:%02d",delta<0?'-':'+',std::abs(delta)/60,std::abs(delta)%60);
            m_dayLabels[i]->setText(std::string(day)+" "+offset);
        }
    }
    requestLayout();requestPaint();
}
void WorldClockWidget::layoutContent(){
    const auto r=contentRect();const bool small=size()==WidgetSize::Small,large=size()==WidgetSize::Large;
    const int visible=small?std::min(1,m_zoneCount):m_zoneCount;
    const int columns=small?1:large?2:std::max(1,visible);const int rows=large?2:1;
    const double gap=small?0:8,cw=(r.width-gap*(columns-1))/columns,ch=(r.height-gap*(rows-1))/rows;
    for(int i=0;i<4;++i){
        const bool shown=i<visible;for(auto* label:{m_cityLabels[i],m_timeLabels[i],m_dayLabels[i]})label->setVisible(shown);
        m_faces[i]={};if(!shown)continue;
        const double x=r.x+(i%columns)*(cw+gap),y=r.y+(i/columns)*(ch+gap);
        const double face=std::max(0.0,std::min(cw,ch-54));m_faces[i]={x+(cw-face)/2,y,face,face};
        const double baseline=y+face+6;
        m_cityLabels[i]->setFontSize(small||large?12:10);m_cityLabels[i]->setWeight(600);m_cityLabels[i]->setGeometry({x,baseline,cw,16});
        m_timeLabels[i]->setFontSize(12);m_timeLabels[i]->setWeight(500);m_timeLabels[i]->setGeometry({x,baseline+17,cw,16});
        m_dayLabels[i]->setFontSize(small||large?10:8);m_dayLabels[i]->setColor(widgetTheme(this).text.withAlpha(.65));m_dayLabels[i]->setGeometry({x,baseline+34,cw,15});
    }
}
void WorldClockWidget::paintContent(ui::Painter& p){
    const auto& theme=widgetTheme(this);
    for(int i=0;i<4;++i){const auto r=m_faces[i];if(r.width<=0)continue;const auto time=m_times[i];
        const bool night=time.hour<6||time.hour>=18;const auto face=night?theme.text.withAlpha(.11):theme.highlight.withAlpha(.10);
        p.fillEllipse(r,face);const double cx=r.x+r.width/2,cy=r.y+r.height/2,rad=r.width/2;
        for(int n=0;n<12;++n){const double angle=n*std::numbers::pi/6;const double inner=rad*(n%3==0?.73:.82);
            p.drawLine({cx+std::sin(angle)*inner,cy-std::cos(angle)*inner},{cx+std::sin(angle)*rad*.9,cy-std::cos(angle)*rad*.9},theme.text.withAlpha(n%3==0?.65:.25),p.scale());}
        if(!time.valid)continue;
        auto hand=[&](double turn,double length,double stroke){const auto a=turn*2*std::numbers::pi;p.drawLine({cx,cy},{cx+std::sin(a)*rad*length,cy-std::cos(a)*rad*length},theme.text,stroke*p.scale());};
        hand((time.hour%12+time.minute/60.0)/12,.48,2.5);hand(time.minute/60.0,.72,1.6);
        p.fillEllipse({cx-2,cy-2,4,4},theme.highlight);
    }
}
void WorldClockWidget::retranslateContent(){loadZones();refresh();}
}
