// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktop/widgets/widget.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <ctime>

#include <curl/curl.h>

#include "base/event_loop.h"
#include "base/dbus.h"
#include "base/settings.h"
#include "core/config.h"
#include "ui/dialog_window.h"
#include "ui/layout.h"
#include "ui/label.h"
#include "ui/text_field.h"
#include "base/i18n.h"
#include "desktop/widgets/calendarwidget.h"
#include "desktop/widgets/batterywidget.h"
#include "desktop/widgets/clockwidget.h"
#include "desktop/widgets/nowplayingwidget.h"
#include "desktop/widgets/noteswidget.h"
#include "desktop/widgets/weatherwidget.h"
#include "desktop/widgets/systemwidget.h"
#include "desktop/widgets/worldclockwidget.h"
#include "desktop/widgets/countdownwidget.h"
#include "desktop/widgets/networkspeedwidget.h"
#include "desktop/widgets/todowidget.h"
#include "desktop/widgets/launcherwidget.h"
#include "desktop/widgets/widgetlayer.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/window.h"

namespace fm {

bool widgetMonochrome(const ui::View* view)
{
    for (auto* p=view; p; p=p->parent())
        if (const auto* layer=dynamic_cast<const WidgetLayer*>(p)) return layer->monochrome();
    return false;
}
ui::Theme widgetTheme(const ui::View* view)
{
    auto theme=ui::Theme::current();
    if (widgetMonochrome(view)) {
        theme.highlight=theme.text.withAlpha(.72);
        theme.highlightedText=theme.background;
    }
    return theme;
}


namespace {

constexpr double kRemoveInset = 8;
constexpr double kRemoveSize = 18;
constexpr double kDragDistance = 4;
constexpr double kContentPaddingX = 14;
constexpr double kContentPaddingY = 14;

ui::PointF removeCentre(const ui::RectF& card)
{
    return {card.width - kRemoveInset - kRemoveSize / 2, kRemoveInset + kRemoveSize / 2};
}

} // namespace

DesktopWidget::DesktopWidget(WidgetLayer& layer, WidgetKind kind, WidgetSize size)
    : m_layer(layer)
    , m_kind(kind)
    , m_size(size)
{
    setTracksHover(true);
    setClipsChildren(true);
    setChildClipRadius(kWidgetRadius);
    setCursorShape(ui::CursorShape::Default);
    std::vector<std::string> command;
    switch (kind) {
    case WidgetKind::Clock: case WidgetKind::WorldClock: command={"lingmo-clock"}; break;
    case WidgetKind::Calendar: command={"lingmo-calendar"}; break;
    case WidgetKind::System: case WidgetKind::NetworkSpeed: command={"lingmo-system-monitor"}; break;
    case WidgetKind::Battery: command={"lingmo-settings","-m","battery"}; break;
    case WidgetKind::Weather: command={"lingmo-settings","-m","desktopwidgets"}; break;
    default: break;
    }
    if (!command.empty()) {
        setToolTip(Lingmo::I18n::dtr("lingmo-filemanager","Click to open · Drag to move · Right-click for options"));
        onClicked=[command] {
            auto* bus=base::dbus::Connection::session();
            if(bus)base::dbus::Proxy(bus,"org.lingmo.AppRuntime","/AppRuntime","org.lingmo.AppRuntime")
                .send("launchCommand",command.front(),command,std::string(),std::string());
        };
    }
}

DesktopWidget::~DesktopWidget() = default;

bool DesktopWidget::configurable() const
{
    return m_kind==WidgetKind::WorldClock || m_kind==WidgetKind::Countdown || m_kind==WidgetKind::Launcher;
}
void DesktopWidget::configure()
{
    auto tr=[](const char* text){return Lingmo::I18n::dtr("lingmo-filemanager",text);};
    if(!window() || !configurable())return;
    if(m_configDialog && m_configDialog->window()){m_configDialog->show();return;}
    m_configDialog=std::make_unique<ui::DialogWindow>(window(),tr("Widget Settings"),420);
    auto* column=m_configDialog->setContent<ui::Column>();column->setMargins({16,16,16,16});column->setSpacing(10);
    base::Settings settings(fm::settingsFile());
    std::vector<std::pair<std::string,ui::TextField*>> fields;
    auto field=[&](const std::string& key,const std::string& label,const std::string& fallback){
        column->addChild<ui::Label>(label);
        auto* entry=column->addChild<ui::TextField>(settings.stringValue(key,fallback));entry->setLayoutHints({.fillWidth=true});
        fields.emplace_back(key,entry);
    };
    if(m_kind==WidgetKind::WorldClock){
        const char* defaults[]={"America/New_York","Europe/London","Asia/Shanghai","Asia/Tokyo"};
        for(int i=0;i<4;++i)field("WorldClock/Zone"+std::to_string(i+1),std::string(tr("Time zone"))+" "+std::to_string(i+1),defaults[i]);
        auto* note=column->addChild<ui::Label>(tr("Use IANA names such as Asia/Shanghai. Leave a field empty to hide that city."));note->setWrap(true);note->setLayoutHints({.fillWidth=true});
    } else if(m_kind==WidgetKind::Countdown){
        field("Countdown/Name",tr("Event name"),tr("Event"));field("Countdown/TargetDate",tr("Target date (YYYY-MM-DD)"),"");
    } else field("Launcher/Pins",tr("Application IDs, separated by commas"),"lingmo-filemanager,lingmo-settings,lingmo-terminal");
    auto* error=column->addChild<ui::Label>();error->setWrap(true);error->setLayoutHints({.fillWidth=true});error->setColor(ui::accent::red);
    auto* save=column->addChild<ui::DialogButton>(tr("Save"),true);
    save->onClicked=[this,fields,error]{
        if(m_kind==WidgetKind::WorldClock)for(const auto& [key,field]:fields)if(!field->text().empty()&&!worldClockTime(field->text(),std::time(nullptr)).valid){error->setText(Lingmo::I18n::tr("Enter a valid IANA time zone."));return;}
        if(m_kind==WidgetKind::Countdown){
            const auto value=fields[1].second->text();int year=0,month=0,day=0;char extra;
            if(std::sscanf(value.c_str(),"%d-%d-%d%c",&year,&month,&day,&extra)!=3 || !std::chrono::year_month_day{std::chrono::year{year},std::chrono::month{unsigned(month)},std::chrono::day{unsigned(day)}}.ok()){
                error->setText(Lingmo::I18n::tr("Enter a valid date in YYYY-MM-DD format."));return;
            }
        }
        base::Settings settings(fm::settingsFile());for(const auto& [key,field]:fields)settings.setValue(key,field->text());settings.sync();
        m_layer.retranslate();m_configDialog->close();
    };
    m_configDialog->show();
}

void DesktopWidget::setSize(WidgetSize size)
{
    if (m_size == size) return;
    // The layer owns geometry and the matching glass mask; update together.
    m_size = size;
    requestLayout(); requestPaint();

}

void DesktopWidget::setInstanceId(std::string id)
{
    if (m_id == id)
        return;
    m_id = std::move(id);
    instanceIdChanged();
}

WidgetPlacement DesktopWidget::placement() const
{
    const ui::RectF card = geometry();
    return {m_kind, m_size, card.x, card.y, m_id};
}

void DesktopWidget::setEditing(bool editing)
{
    if (m_editing == editing)
        return;
    m_editing = editing;
    setTransform({});
    setLayered(false);
    requestPaint();
}

ui::RectF DesktopWidget::blurRect() const
{
    const ui::RectF card = localRect();
    const ui::PointF origin = mapToWindow(card.topLeft());
    return {origin.x, origin.y, card.width, card.height};
}

void DesktopWidget::retranslate()
{
    retranslateContent();
    requestLayout();
    requestPaint();
}

ui::RectF DesktopWidget::contentRect() const
{
    const ui::RectF card = localRect();
    return {kContentPaddingX, kContentPaddingY, std::max(0.0, card.width - kContentPaddingX * 2),
            std::max(0.0, card.height - kContentPaddingY * 2)};
}

bool DesktopWidget::frosted() const
{
    const ui::Window* window = this->window();
    return window && window->backgroundGlassAvailable();
}

ui::CursorShape DesktopWidget::cursorShape() const
{
    if (m_dragging)
        return ui::CursorShape::Grabbing;
    return m_hovered ? (m_editing ? ui::CursorShape::Grab : onClicked ? ui::CursorShape::Pointer : ui::CursorShape::Default) : ui::CursorShape::Default;
}

void DesktopWidget::layout()
{
    ui::View::layout();
    layoutContent();
}

void DesktopWidget::paint(ui::Painter& painter)
{
    const ui::Theme& theme = widgetTheme(this);
    const ui::RectF card = localRect();
    // The compositor blurs the card's silhouette behind the window; the fill is
    // a light tint over it. Without the blur the card is its own background.
    ui::Color fill;
    if (frosted()) fill = theme.darkMode ? ui::Color{24,24,30,80} : ui::Color{255,255,255,70};
    else fill = theme.darkMode ? ui::Color{38,38,45,255} : ui::Color{248,248,252,255};
    if (m_hovered && !m_dragging)
        fill = fill.lighter(theme.darkMode ? 1.15 : 1.02);

    // A card under the pointer, and more so while dragged, sits a little
    // further above the desktop than the resting ones.
    const double shadowBlur = m_dragging ? 20 : m_hovered ? 14 : 10;
    const int shadowAlpha = theme.darkMode ? (m_dragging ? 135 : m_hovered ? 110 : 90)
                                           : (m_dragging ? 85 : m_hovered ? 65 : 45);
    painter.drawShadow(card, kWidgetRadius, shadowBlur, ui::Color {0, 0, 0, uint8_t(shadowAlpha)},
                       {0, m_dragging ? 8.0 : 3.0});
    const ui::Color border = m_editing ? theme.highlight
            : (m_hovered || m_dragging) ? theme.text.withAlpha(0.22)
                                        : theme.text.withAlpha(0.10);
    painter.drawRoundedRect(card, {kWidgetRadius, 0, ui::Corners::Continuous}, fill,
                            m_editing ? 1.5 : 1.0, border);

    if (frosted() && !m_editing) {
        // A hairline where the light catches the top of the glass.
        painter.drawHorizontalLine(card.x + kWidgetRadius, card.y + 0.5,
                                   std::max(0.0, card.width - kWidgetRadius * 2),
                                   ui::Color {255, 255, 255, uint8_t(theme.darkMode ? 18 : 72)}, 1);
    }

    paintContent(painter);

    if (m_editing) {
        const ui::PointF centre = removeCentre(card);
        const ui::RectF button {centre.x - kRemoveSize / 2, centre.y - kRemoveSize / 2, kRemoveSize,
                                kRemoveSize};
        painter.fillEllipse(button, ui::Color {0, 0, 0, 145});
        painter.drawLine({centre.x - 3, centre.y - 3}, {centre.x + 3, centre.y + 3},
                         ui::Color {255, 255, 255, 235}, 1.5);
        painter.drawLine({centre.x + 3, centre.y - 3}, {centre.x - 3, centre.y + 3},
                         ui::Color {255, 255, 255, 235}, 1.5);
    }
}

void DesktopWidget::pointerEnter()
{
    m_hovered = true;
    requestPaint();
}

void DesktopWidget::pointerLeave()
{
    m_hovered = false;
    requestPaint();
}

void DesktopWidget::hoverChanged()
{
    m_hovered = isHovered();
    requestPaint();
}

void DesktopWidget::pointerPress(const ui::PointerEvent& event)
{
    m_layer.raiseWidget(this);
    if (event.button == ui::MouseButton::Right) {
        m_layer.showWidgetMenu(this, mapToWindow(event.position));
        return;
    }
    if (event.button != ui::MouseButton::Left)
        return;
    if (m_editing) {
        const ui::PointF centre = removeCentre(localRect());
        const ui::RectF button {centre.x - kRemoveSize / 2, centre.y - kRemoveSize / 2, kRemoveSize,
                                kRemoveSize};
        if (button.contains(event.position)) {
            // The card is this event's receiver: remove it once the event is
            // over, not under the pointer's own dispatch.
            m_layer.removeWidgetDeferred(this);
            return;
        }
    }
    m_pressed = true;
    m_dragging = false;
    m_pressWindow = mapToWindow(event.position);
    m_dragOrigin = geometry().topLeft();
}

void DesktopWidget::pointerMove(const ui::PointerEvent& event)
{
    if (!m_pressed)
        return;
    const ui::PointF current = mapToWindow(event.position);
    const ui::PointF delta = current - m_pressWindow;
    if (!m_dragging && (std::abs(delta.x) > kDragDistance || std::abs(delta.y) > kDragDistance))
        m_dragging = true;
    if (!m_dragging)
        return;
    m_layer.moveWidget(this, {m_dragOrigin.x + delta.x, m_dragOrigin.y + delta.y}, false);

}

void DesktopWidget::pointerRelease(const ui::PointerEvent& event)
{
    if (!m_pressed)
        return;
    const bool dragged = m_dragging;
    m_pressed = false;
    m_dragging = false;
    if (dragged) {
        m_layer.moveWidget(this, geometry().topLeft(), true);
    } else if (!m_editing && onClicked && localRect().contains(event.position)) {
        onClicked();
    }
    (void)event;
    requestPaint();
}

std::unique_ptr<DesktopWidget> createDesktopWidget(WidgetLayer& layer, WidgetKind kind, WidgetSize size)
{
    switch (kind) {
    case WidgetKind::Clock:
        return std::make_unique<ClockWidget>(layer, size);
    case WidgetKind::Calendar:
        return std::make_unique<CalendarWidget>(layer, size);
    case WidgetKind::System:
        return std::make_unique<SystemWidget>(layer, size);
    case WidgetKind::Battery:
        return std::make_unique<BatteryWidget>(layer, size);
    case WidgetKind::NowPlaying:
        return std::make_unique<NowPlayingWidget>(layer, size);
    case WidgetKind::Notes:
        return std::make_unique<NotesWidget>(layer, size);
    case WidgetKind::Weather:
        return std::make_unique<WeatherWidget>(layer, size);
    case WidgetKind::WorldClock:
        return std::make_unique<WorldClockWidget>(layer, size);
    case WidgetKind::Countdown:
        return std::make_unique<CountdownWidget>(layer, size);
    case WidgetKind::NetworkSpeed:
        return std::make_unique<NetworkSpeedWidget>(layer, size);
    case WidgetKind::Todo:
        return std::make_unique<TodoWidget>(layer, size);
    case WidgetKind::Launcher:
        return std::make_unique<LauncherWidget>(layer, size);
    }
    return nullptr;
}

} // namespace fm
