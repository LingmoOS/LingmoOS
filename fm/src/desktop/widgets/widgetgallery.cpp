// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "desktop/widgets/widgetgallery.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include "base/i18n.h"
#include "ui/button.h"
#include "ui/icon.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/painter.h"
#include "ui/scroll_view.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include "ui/window.h"
static const char* tr(const char* text) { return Lingmo::i18n::dtr("lingmo-filemanager", text); }
namespace fm {
namespace {
constexpr std::array kinds{WidgetKind::Clock,WidgetKind::Calendar,WidgetKind::WorldClock,
    WidgetKind::Weather,WidgetKind::System,WidgetKind::Battery,WidgetKind::NowPlaying,
    WidgetKind::Notes,WidgetKind::Countdown,WidgetKind::NetworkSpeed,WidgetKind::Todo,WidgetKind::Launcher};
std::string title(WidgetKind kind) {
    switch(kind) {
    case WidgetKind::Clock:return tr("Clock"); case WidgetKind::Calendar:return tr("Calendar");
    case WidgetKind::WorldClock:return tr("World Clock");case WidgetKind::Weather:return tr("Weather");
    case WidgetKind::System:return tr("System");case WidgetKind::Battery:return tr("Battery");
    case WidgetKind::NowPlaying:return tr("Now Playing");case WidgetKind::Notes:return tr("Notes");
    case WidgetKind::Countdown:return tr("Countdown");case WidgetKind::NetworkSpeed:return tr("Network");
    case WidgetKind::Todo:return tr("To-Do");case WidgetKind::Launcher:return tr("Launcher");
    } return {};
}
std::string categoryGlyph(WidgetKind kind) {
    const char* shape = "";
    switch(kind) {
    case WidgetKind::Clock:shape=R"(<circle cx="12" cy="12" r="8"/><path d="M12 7v5l3 2"/>)";break;
    case WidgetKind::Calendar:shape=R"(<rect x="4" y="5" width="16" height="15" rx="2"/><path d="M4 10h16M8 3v4m8-4v4M8 14h2m4 0h2m-8 3h2"/>)";break;
    case WidgetKind::WorldClock:shape=R"(<circle cx="12" cy="12" r="8"/><ellipse cx="12" cy="12" rx="4" ry="8"/><path d="M4 12h16"/>)";break;
    case WidgetKind::Weather:shape=R"(<circle cx="10" cy="9" r="4"/><path d="M10 2v1m0 12v1M3 9H2m16 0h-1M5 4l1 1m8-1-1 1M6 20h12a3 3 0 0 0 0-6 4 4 0 0 0-7-1 4 4 0 0 0-5 7Z"/>)";break;
    case WidgetKind::System:shape=R"(<path d="M5 19V9m7 10V4m7 15v-7"/>)";break;
    case WidgetKind::Battery:shape=R"(<rect x="3" y="7" width="16" height="10" rx="2"/><path d="M21 10v4M7 10v4m4-4v4m4-4v4"/>)";break;
    case WidgetKind::NowPlaying:shape=R"(<path d="M9 17V6l10-2v11M9 9l10-2"/><ellipse cx="6" cy="18" rx="3" ry="2"/><ellipse cx="16" cy="16" rx="3" ry="2"/>)";break;
    case WidgetKind::Notes:shape=R"(<rect x="5" y="3" width="14" height="18" rx="2"/><path d="M8 8h8M8 12h8m-8 4h5"/>)";break;
    case WidgetKind::Countdown:shape=R"(<circle cx="12" cy="14" r="7"/><path d="M9 3h6m-3 0v4m0 3v4l3 2m3-9 2-2"/>)";break;
    case WidgetKind::NetworkSpeed:shape=R"(<path d="M7 3v17m-4-4 4 4 4-4m6 5V4m-4 4 4-4 4 4"/>)";break;
    case WidgetKind::Todo:shape=R"(<path d="m3 6 2 2 4-4m3 2h9M3 13l2 2 4-4m3 2h9M3 20h6m3 0h9"/>)";break;
    case WidgetKind::Launcher:shape=R"(<rect x="4" y="4" width="6" height="6" rx="1"/><rect x="14" y="4" width="6" height="6" rx="1"/><rect x="4" y="14" width="6" height="6" rx="1"/><rect x="14" y="14" width="6" height="6" rx="1"/>)";break;
    }
    const auto name=std::string("widget-category-")+widgetKindKey(kind);
    ui::Icon::registerGlyph(name,std::string(R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="black" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round">)")+shape+"</svg>");
    return name;
}
std::string folded(std::string text) {
    for(char& c:text) if(static_cast<unsigned char>(c)<128)c=static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return text;
}
ui::Label* label(ui::View* parent,std::string text,double font=13,int weight=400) {
    auto* result=parent->addChild<ui::Label>(std::move(text));result->setFontSize(font);result->setWeight(weight);
    result->setVerticallyCentered(true);return result;
}
}
class WidgetGallery::Category final:public ui::View {
public:
    explicit Category(WidgetKind value):kind(value) {
        setName(std::string("widget-category:")+widgetKindKey(kind));
        m_icon=addChild<ui::Icon>(categoryGlyph(kind),18);m_label=label(this,title(kind),13,500);
        setCursorShape(ui::CursorShape::Pointer);
    }
    WidgetKind kind; bool selected=false; std::function<void()> onClicked;
    ui::SizeF sizeHint()const override{return {176,36};}
    bool acceptsPointer()const override{return true;}
    bool acceptsFocus()const override{return true;}
    void retranslate(){m_label->setText(title(kind));requestLayout();}
    void layout()override {
        m_icon->setGeometry({12,9,18,18});m_icon->setColor(ui::ColorRecipe(ui::Theme::current().highlight));m_label->setGeometry({42,0,std::max(0.0,geometry().width-48),36});
        m_label->setColor(ui::ColorRecipe(selected?ui::Theme::current().highlight:ui::Theme::current().text));
    }
    void paint(ui::Painter& p)override {
        const auto& t=ui::Theme::current();p.fillRoundedRect({9,6,24,24},6,t.highlight.withAlpha(.14));if(selected||m_hovered||hasFocus())
            p.fillRoundedRect(localRect(),9,selected?t.highlight.withAlpha(.18):t.text.withAlpha(.06));
    }
protected:
    void pointerEnter()override{m_hovered=true;requestPaint();}
    void pointerLeave()override{m_hovered=false;requestPaint();}
    void pointerPress(const ui::PointerEvent& e)override{m_pressed=e.button==ui::MouseButton::Left;}
    void pointerRelease(const ui::PointerEvent& e)override {
        const bool activate=m_pressed&&e.button==ui::MouseButton::Left&&localRect().contains(e.position);
        m_pressed=false;if(activate&&onClicked)onClicked();
    }
    bool keyPress(const ui::KeyEvent& e)override {
        if(e.key==ui::key::Return||e.key==ui::key::Space){if(onClicked)onClicked();return true;}return false;
    }
private:ui::Icon* m_icon;ui::Label* m_label;bool m_pressed=false,m_hovered=false;
};
class WidgetGallery::PanelView final:public ui::View {
public:
    double sidebar=204;
    PanelView(){setName("widget-gallery-panel");setClipsChildren(true);setChildClipRadius(24);}
    bool acceptsPointer()const override{return true;}
    void paint(ui::Painter& p)override {
        const auto& t=ui::Theme::current();const auto r=localRect();
        p.drawShadow(r,24,22,{0,0,0,75},{0,8});
        p.save();p.clipRoundedRect(r,24,ui::Corners::Continuous);
        const bool glass=window()&&window()->backgroundGlassAvailable();
        p.fillRect({0,0,sidebar,r.height},t.darkMode?ui::Color{35,35,44,uint8_t(glass?160:255)}:ui::Color{239,240,249,uint8_t(glass?160:255)});
        p.fillRect({sidebar,0,r.width-sidebar,r.height},t.darkMode?ui::Color{30,30,36,255}:ui::Color{250,250,253,255});
        p.fillRect({sidebar,0,1,r.height},t.text.withAlpha(.08));p.restore();
        p.drawRoundedRect(r,{24,0,ui::Corners::Continuous},{0,0,0,0},1,t.text.withAlpha(.12));
    }
};
// Sample content is deliberately independent of DesktopWidget: no timers,
// settings writes, media subscriptions or network requests while browsing.
class WidgetGallery::PreviewCard final:public ui::View {
public:
    PreviewCard(WidgetKind kind,WidgetSize size):m_size(size){setName(std::string("widget-preview:")+widgetSizeKey(size));setCursorShape(ui::CursorShape::Pointer);setClipsChildren(true);setChildClipRadius(20);setKind(kind);}
    std::function<void()> onClicked;
    std::function<void(ui::PointF,bool)> onDrag;
    bool acceptsPointer()const override{return true;}
    bool acceptsFocus()const override{return true;}
    void cancelInteraction() { m_pressed=false; m_dragging=false; }
    void setKind(WidgetKind kind) {
        m_kind=kind;while(!children().empty())takeChild(children().back().get());m_text.clear();
        const auto spec=widgetSizeSpec(m_size);const double w=spec.width,h=spec.height;
        auto put=[&](std::string text,ui::RectF r,double font=14,int weight=400,bool accent=false) {
            auto* l=label(this,std::move(text),font,weight);m_text.push_back({l,r,font,accent});return l;
        };
        put(title(kind),{14,10,w-28,24},13,600,true);
        if(kind==WidgetKind::Calendar) {
            put(tr("April"),{14,33,w-28,28},20,600);
            const std::array<const char*,7> days{"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
            const double cell=(w-24)/7,step=(h-76)/6;
            for(int col=0;col<7;++col){auto* l=put(tr(days[col]),{12+col*cell,66,cell,18},10,500);l->setTextAlignment(ui::Label::TextAlignment::Center);}
            for(int d=1;d<=30;++d){int index=d;auto* l=put(std::to_string(d),{12+(index%7)*cell,86+(index/7)*step,cell,step},12,d==15?700:400,d==15);l->setTextAlignment(ui::Label::TextAlignment::Center);}
        } else if(kind==WidgetKind::WorldClock) {
            const int n=m_size==WidgetSize::Small?1:4;
            const std::array<std::string,4> cities{tr("New York"),tr("London"),tr("Shanghai"),tr("Tokyo")};
            for(int i=0;i<n;++i){const auto face=clockFace(i);const double textWidth=face.width+12;
                auto* l=put(cities[i],{face.x-6,face.bottom()+3,textWidth,18},n==1?14:11,500);l->setTextAlignment(ui::Label::TextAlignment::Center);
                auto* time=put(i<2?"09:41":"17:41",{face.x-6,face.bottom()+22,textWidth,20},n==1?18:13,600);time->setTextAlignment(ui::Label::TextAlignment::Center);}
        } else {
            std::string value,detail;
            switch(kind) {
            case WidgetKind::Clock:value="09:41";detail=tr("Tuesday, April 1");break;
            case WidgetKind::Weather:value="23°";detail=tr("Sunny");break;
            case WidgetKind::System:value="24%";detail=tr("CPU usage");break;
            case WidgetKind::Battery:value="86%";detail=tr("Battery");break;
            case WidgetKind::NowPlaying:value=tr("Your music");detail=tr("Now Playing");break;
            case WidgetKind::Notes:value=tr("A little inspiration");detail=tr("Capture a thought");break;
            case WidgetKind::Countdown:value="28";detail=tr("Days remaining");break;
            case WidgetKind::NetworkSpeed:value="1.2 MB/s";detail=tr("Download");break;
            case WidgetKind::Todo:value="3";detail=tr("Things to do");break;
            case WidgetKind::Launcher:value=tr("Favorite apps");detail=tr("One click away");break;
            default:break;
            }
            const bool sentence=kind==WidgetKind::NowPlaying||kind==WidgetKind::Notes||kind==WidgetKind::Launcher;
            auto* main=put(value,{14,h*.27,w-28,h*.29},sentence?20:42,600);main->setWrap(true);
            put(detail,{14,h*.61,w-28,30},13);
            if(m_size==WidgetSize::Large) {
                put(tr("Sample preview"),{14,h-64,w-28,22},12);
                put(tr("Content updates on your desktop"),{14,h-42,w-28,24},11);
            }
        }
        requestLayout();requestPaint();
    }
    void layout()override {
        const double s=geometry().width/widgetSizeSpec(m_size).width;const auto& t=ui::Theme::current();
        for(auto& item:m_text){const auto& r=item.rect;item.label->setGeometry({r.x*s,r.y*s,r.width*s,r.height*s});item.label->setFontSize(std::max(9.0,item.font*s));item.label->setColor(ui::ColorRecipe(item.accent?t.highlight:t.text));}
    }
    void paint(ui::Painter& p)override {
        const auto& t=ui::Theme::current();const auto r=localRect();
        p.fillRoundedRect(r,20,t.darkMode?ui::Color{59,61,80,255}:ui::Color{235,237,249,255});
        p.drawRoundedRect(r,{20,0,ui::Corners::Continuous},{0,0,0,0},m_hovered||hasFocus()?2:1,(m_hovered||hasFocus()?t.highlight:t.text.withAlpha(.10)));
        if(m_kind==WidgetKind::WorldClock) {
            const int n=m_size==WidgetSize::Small?1:4;const double scale=r.width/widgetSizeSpec(m_size).width;
            for(int i=0;i<n;++i){const auto f=clockFace(i);const double radius=f.width*scale/2;const ui::PointF c{(f.x+f.width/2)*scale,(f.y+f.height/2)*scale};p.fillEllipse({c.x-radius,c.y-radius,2*radius,2*radius},t.text.withAlpha(.08));
                for(int tick=0;tick<12;++tick){const double a=tick*3.141592653589793/6; p.drawLine({c.x+std::sin(a)*radius*.82,c.y-std::cos(a)*radius*.82},{c.x+std::sin(a)*radius*.93,c.y-std::cos(a)*radius*.93},t.text.withAlpha(.5),1);}
                p.drawLine(c,{c.x-radius*.48,c.y-radius*.18},t.text,2);p.drawLine(c,{c.x+radius*.48,c.y-radius*.5},t.text,2);p.fillEllipse({c.x-2,c.y-2,4,4},t.highlight);
            }
        }
        if(m_kind==WidgetKind::System||m_kind==WidgetKind::Battery) {
            const ui::RectF bar{r.width*.08,r.height*.86,r.width*.84,5};p.fillRoundedRect(bar,2,t.text.withAlpha(.1));auto fill=bar;fill.width*=m_kind==WidgetKind::Battery?.86:.24;p.fillRoundedRect(fill,2,t.highlight);
        }
    }
protected:
    void pointerEnter()override{m_hovered=true;requestPaint();}
    void pointerLeave()override{m_hovered=false;requestPaint();}
    void pointerPress(const ui::PointerEvent& e)override{m_pressed=e.button==ui::MouseButton::Left;m_dragging=false;m_start=mapToWindow(e.position);}
    void pointerMove(const ui::PointerEvent& e)override {
        if(!m_pressed)return;const auto p=mapToWindow(e.position);const auto delta=p-m_start;
        if(std::abs(delta.x)>6||std::abs(delta.y)>6)m_dragging=true;
        if(m_dragging&&onDrag)onDrag(p,false);
    }
    void pointerRelease(const ui::PointerEvent& e)override {
        if(!m_pressed||e.button!=ui::MouseButton::Left)return;m_pressed=false;
        const bool dragged=m_dragging;m_dragging=false;
        if(dragged){if(onDrag)onDrag(mapToWindow(e.position),true);}
        else if(localRect().contains(e.position)&&onClicked)onClicked();
    }
    bool keyPress(const ui::KeyEvent& e)override{if(e.key==ui::key::Return||e.key==ui::key::Space){if(onClicked)onClicked();return true;}return false;}
private:
    ui::RectF clockFace(int i) const {
        const auto spec=widgetSizeSpec(m_size);const bool large=m_size==WidgetSize::Large;
        const int columns=m_size==WidgetSize::Small?1:large?2:4,rows=large?2:1;
        const double cw=(spec.width-28-8*(columns-1))/columns,ch=(spec.height-48-8*(rows-1))/rows;
        const double face=std::max(0.0,std::min(cw,ch-42));
        return {14+(i%columns)*(cw+8)+(cw-face)/2,36+(i/columns)*(ch+8),face,face};
    }
    struct Item{ui::Label* label;ui::RectF rect;double font;bool accent;};
    WidgetKind m_kind;WidgetSize m_size;std::vector<Item> m_text;
    bool m_pressed=false,m_dragging=false,m_hovered=false;ui::PointF m_start;
};
class WidgetGallery::DragPreview final:public ui::View {
public:
    void paint(ui::Painter& p)override{const auto& t=ui::Theme::current();p.fillRoundedRect(localRect(),20,t.highlight.withAlpha(.25));p.drawRoundedRect(localRect(),{20,0,ui::Corners::Continuous},{0,0,0,0},2,t.highlight);}
};
WidgetGallery::WidgetGallery() {
    setName("widget-gallery");setVisible(false);setClipsChildren(false);
    m_panel=addChild<PanelView>();m_search=m_panel->addChild<ui::TextField>();m_search->setName("widget-gallery:search");m_search->setFontSize(12);
    m_categories=m_panel->addChild<ui::ScrollView>();m_categories->setElastic(false);m_list=m_categories->setContent<ui::Column>();m_list->setSpacing(3);
    for(auto kind:kinds){auto* row=m_list->addChild<Category>(kind);row->setLayoutHints({.fillWidth=true});row->onClicked=[this,kind]{selectKind(kind);};m_rows.push_back(row);}
    m_title=label(m_panel,"",23,650);m_description=label(m_panel,"",12);m_hint=label(m_panel,"",12);m_hint->setWrap(true);
    m_empty=label(m_panel,"",16,500);m_empty->setTextAlignment(ui::Label::TextAlignment::Center);m_empty->setWrap(true);
    for(auto size:{WidgetSize::Small,WidgetSize::Medium,WidgetSize::Large}) {
        auto* preview=m_panel->addChild<PreviewCard>(m_selected,size);preview->onClicked=[this,size]{add(m_selected,size);};
        preview->onDrag=[this,size](ui::PointF point,bool finished){
            if(m_notificationTarget)return;
            const auto local=mapFromWindow(point);const auto spec=widgetSizeSpec(size);
            if(!finished){m_drag->setGeometry({local.x-spec.width/2,local.y-spec.height/2,spec.width,spec.height});m_drag->setVisible(true);}
            else{m_drag->setVisible(false);if(localRect().contains(local)&&!m_panel->geometry().contains(local)&&onAddAt)onAddAt(m_selected,size,point);}
        };
        m_previews.push_back(preview);auto* caption=m_panel->addChild<ui::Button>();caption->onClicked=[this,size]{add(m_selected,size);};m_captions.push_back(caption);
    }
    m_desktopTarget=m_panel->addChild<ui::Button>();m_desktopTarget->setName("widget-gallery:desktop-target");m_desktopTarget->onClicked=[this]{setNotificationTarget(false);};
    m_notificationTargetButton=m_panel->addChild<ui::Button>();m_notificationTargetButton->setName("widget-gallery:notification-target");m_notificationTargetButton->onClicked=[this]{setNotificationTarget(true);};
    m_done=m_panel->addChild<ui::Button>();m_done->setName("widget-gallery:done");m_done->onClicked=[this]{m_drag->setVisible(false);if(onDone)onDone();};
    m_drag=addChild<DragPreview>();m_drag->setVisible(false);
    m_search->onTextChanged=[this]{filter();};retranslate();
}
WidgetGallery::~WidgetGallery()=default;
void WidgetGallery::add(WidgetKind kind,WidgetSize size){if(onAddSized)onAddSized(kind,size);else if(onAdd)onAdd(kind);}
void WidgetGallery::focusSearch(){for(auto* preview:m_previews)preview->cancelInteraction();m_drag->setVisible(false);m_search->setFocus(ui::FocusReason::Other);}
void WidgetGallery::selectKind(WidgetKind kind) {
    m_selected=kind;m_title->setText(title(kind));for(auto* row:m_rows){row->selected=row->kind==kind;row->requestLayout();row->requestPaint();}
    for(auto* preview:m_previews)preview->setKind(kind);requestLayout();
}
void WidgetGallery::filter() {
    const auto query=folded(m_search->text());Category* first=nullptr;bool selectedVisible=false;
    for(auto* row:m_rows){const auto text=folded(title(row->kind)+" "+widgetKindKey(row->kind));const bool visible=query.empty()||text.find(query)!=std::string::npos;
        row->setVisible(visible);if(visible){if(!first)first=row;if(row->kind==m_selected)selectedVisible=true;}}
    if(first&&!selectedVisible)selectKind(first->kind);
    for(auto* card:m_previews)card->setVisible(first!=nullptr);for(auto* caption:m_captions)caption->setVisible(first!=nullptr);
    m_title->setVisible(first!=nullptr);m_description->setVisible(first!=nullptr);m_empty->setVisible(first==nullptr);
    m_categories->setScrollPosition({0,0});m_list->requestLayout();m_categories->requestLayout();requestLayout();
}
void WidgetGallery::setNotificationTarget(bool notifications) {
    m_notificationTarget=notifications;m_drag->setVisible(false);
    for(auto* preview:m_previews)preview->cancelInteraction();
    if(onTargetChanged)onTargetChanged(notifications);
    m_desktopTarget->setText(notifications?tr("Desktop"):std::string("✓ ")+tr("Desktop"));
    m_notificationTargetButton->setText(notifications?std::string("✓ ")+tr("Notification Center"):tr("Notification Center"));
    m_hint->setText(notifications?tr("Click a size to add to Notification Center. Open it again to see changes."):tr("Click to add, or drag a preview onto the desktop."));
}
void WidgetGallery::retranslate() {
    m_search->setPlaceholder(tr("Search widgets"));m_description->setText(tr("Sample previews · choose a size to add"));m_hint->setText(tr("Click to add, or drag a preview onto the desktop."));
    m_empty->setText(tr("No widgets found. Try another search."));m_done->setText(tr("Done"));
    const std::array<std::string,3> sizes{tr("Small"),tr("Medium"),tr("Large")};for(size_t i=0;i<3;++i)m_captions[i]->setText(sizes[i]+"  +");
    for(auto* row:m_rows)row->retranslate();selectKind(m_selected);filter();setNotificationTarget(m_notificationTarget);
}
void WidgetGallery::paint(ui::Painter& p){p.fillRect(localRect(),ui::Color{0,0,0,48});}
void WidgetGallery::layout() {
    const auto area=localRect();const double width=std::min(1000.0,std::max(0.0,area.width-48));const double height=std::min(600.0,std::max(0.0,area.height-48));
    m_panel->setGeometry({(area.width-width)/2,(area.height-height)/2,width,height});
    const double sidebar=std::min(204.0,width*.28);m_panel->sidebar=sidebar;
    m_search->setGeometry({12,18,std::max(0.0,sidebar-24),32});m_categories->setGeometry({8,62,std::max(0.0,sidebar-16),std::max(0.0,height-82)});
    const double contentX=sidebar+24,contentWidth=std::max(0.0,width-sidebar-48);
    m_title->setGeometry({contentX,24,std::max(0.0,contentWidth-290),32});
    m_desktopTarget->setGeometry({width-298,24,108,30});m_notificationTargetButton->setGeometry({width-180,24,156,30});m_description->setGeometry({contentX,62,contentWidth,24});
    const double gap=16,unit=std::max(0.0,(contentWidth-gap*2)/(176.0+368.0+368.0));
    double x=contentX;for(size_t i=0;i<3;++i){const auto spec=widgetSizeSpec(static_cast<WidgetSize>(i));
        const double scale=std::min(unit,std::max(0.0,height-210)/368);const double w=spec.width*scale,h=spec.height*scale;
        m_previews[i]->setGeometry({x,116,w,h});const double buttonWidth=std::min(w,100.0);m_captions[i]->setGeometry({x+(w-buttonWidth)/2,116+h+12,buttonWidth,28});x+=w+gap;}
    m_empty->setGeometry({contentX,130,contentWidth,std::max(0.0,height-240)});
    m_hint->setGeometry({contentX,height-65,std::max(0.0,contentWidth-108),44});m_done->setGeometry({width-106,height-58,82,32});
    const auto& t=ui::Theme::current();m_description->setColor(ui::ColorRecipe(t.text.withAlpha(.55)));m_hint->setColor(ui::ColorRecipe(t.text.withAlpha(.6)));
    if(onSidebarGeometry && isVisible()){const auto pos=m_panel->mapToWindow({0,0});onSidebarGeometry({pos.x,pos.y,sidebar,height});}
}
bool WidgetGallery::keyPress(const ui::KeyEvent& event) {
    if(event.key==ui::key::Escape){for(auto* preview:m_previews)preview->cancelInteraction();m_drag->setVisible(false);if(onDone)onDone();return true;}return false;
}
} // namespace fm
