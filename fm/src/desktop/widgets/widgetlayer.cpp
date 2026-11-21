// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktop/widgets/widgetlayer.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <random>
#include <string>

#include "base/event_loop.h"
#include "base/i18n.h"
#include "desktop/widgets/widget.h"
#include "ui/animation.h"
#include "ui/menu.h"
#include "ui/path.h"
#include "ui/popup_glass.h"
#include "ui/theme.h"
#include "ui/window.h"

static const char* tr(const char* text) { return Lingmo::i18n::dtr("lingmo-filemanager", text); }

namespace fm {

namespace {

constexpr double kSnap = 8;
constexpr double kMargin = 8;

void refreshAppearance(ui::View* view)
{
    view->requestLayout();view->requestPaint();
    for(const auto& child:view->children())refreshAppearance(child.get());
}

bool intersects(const ui::RectF& a, const ui::RectF& b)
{
    return a.x < b.right() && b.x < a.right() && a.y < b.bottom() && b.y < a.bottom();
}

ui::RectF expanded(const ui::RectF& rect, double amount)
{
    return {rect.x - amount, rect.y - amount, rect.width + amount * 2, rect.height + amount * 2};
}

std::string newWidgetId(WidgetKind kind)
{
    static const uint64_t seed = std::random_device {}();
    static std::atomic<uint64_t> counter {0};
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    return std::string(widgetKindKey(kind)) + "-" + std::to_string(seed) + "-"
            + std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(now).count()) + "-"
            + std::to_string(++counter);
}

} // namespace

WidgetLayer::WidgetLayer()
{
    setName("desktop-widgets");
    setClipsChildren(false);
}

WidgetLayer::~WidgetLayer()
{
    *m_alive = false;
    m_menu.reset();
}

void WidgetLayer::setDesktopWindow(ui::Window* window)
{
    m_window = window;
    if (m_window && m_workArea.width <= 0 && m_window->workArea().width > 0)
        m_workArea = m_window->workArea();
    updateBlur();
}

void WidgetLayer::setWorkArea(const ui::RectF& area)
{
    if (area == m_workArea)
        return;
    m_workArea = area;
    layout();
    updateBlur();
}

void WidgetLayer::setEditing(bool editing)
{
    if (m_editing == editing)
        return;
    m_editing = editing;
    for (DesktopWidget* widget : m_widgets) {
        widget->setEditing(editing);refreshAppearance(widget);
    }
}

ui::PointF WidgetLayer::clamped(ui::PointF position, ui::SizeF size) const
{
    if (m_workArea.width <= 0 || m_workArea.height <= 0)
        return position;
    const double maxX = std::max(m_workArea.x, m_workArea.right() - size.width);
    const double maxY = std::max(m_workArea.y, m_workArea.bottom() - size.height);
    position.x = std::clamp(position.x, m_workArea.x, maxX);
    position.y = std::clamp(position.y, m_workArea.y, maxY);
    return position;
}

ui::PointF WidgetLayer::snapped(ui::PointF position) const
{
    return {std::round(position.x / kSnap) * kSnap, std::round(position.y / kSnap) * kSnap};
}

bool WidgetLayer::overlaps(const ui::RectF& rect) const
{
    for (const DesktopWidget* widget : m_widgets) {
        if (intersects(expanded(widget->geometry(), kWidgetGap / 2), expanded(rect, 1)))
            return true;
    }
    return false;
}

ui::PointF WidgetLayer::freePosition(ui::SizeF size) const
{
    const ui::RectF area = m_workArea.width > 0 ? m_workArea : localRect();
    const double x = area.x + kMargin;
    const double y = area.y + kMargin;
    const double usableWidth = std::max(size.width, area.width - kMargin * 2);
    const int columns = std::max(1, int((usableWidth + kWidgetGap) / (size.width + kWidgetGap)));
    const int rows = std::max(1, int((std::max(size.height, area.height - kMargin * 2) + kWidgetGap)
                                     / (size.height + kWidgetGap)));
    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            const ui::RectF candidate {x + column * (size.width + kWidgetGap),
                                       y + row * (size.height + kWidgetGap), size.width, size.height};
            if (!overlaps(candidate))
                return candidate.topLeft();
        }
    }
    // Every slot is taken: let the cards overlap rather than not appear.
    return {x, y};
}

DesktopWidget* WidgetLayer::create(const WidgetPlacement& placement)
{
    std::unique_ptr<DesktopWidget> widget =
            createDesktopWidget(*this, placement.kind, placement.size);
    if (!widget)
        return nullptr;
    const ui::SizeF size = widgetSizeSpec(placement.size);
    const ui::PointF position = clamped({placement.x, placement.y}, size);
    widget->setInstanceId(placement.id.empty() ? newWidgetId(placement.kind) : placement.id);
    widget->setGeometry({position.x, position.y, size.width, size.height});
    DesktopWidget* raw = widget.get();
    addChild(std::move(widget));
    m_widgets.push_back(raw);
    return raw;
}

void WidgetLayer::setAppearance(bool visible, int colorMode, bool desktopActive)
{
    setVisible(visible);
    const bool mono = colorMode == 1 || (colorMode == 0 && !desktopActive);
    if (m_monochrome != mono) {
        m_monochrome = mono;
        for (auto* widget : m_widgets) refreshAppearance(widget);
    }
    updateBlur();
}

double WidgetLayer::flowHeight() const
{
    double bottom = 0;
    for (auto* widget : m_widgets) bottom = std::max(bottom, widget->geometry().bottom());
    return bottom;
}

void WidgetLayer::addWidget(WidgetKind kind, WidgetSize size)
{
    const ui::SizeF spec = widgetSizeSpec(size);
    const ui::PointF position = freePosition(spec);
    DesktopWidget* widget = create({kind, size, position.x, position.y});
    if (!widget)
        return;
    widget->setEditing(m_editing);

    updateBlur();
    if (onChanged)
        onChanged();
}

void WidgetLayer::addWidgetAt(WidgetKind kind, WidgetSize size, ui::PointF windowPosition)
{
    const auto spec = widgetSizeSpec(size);
    const auto centre = mapFromWindow(windowPosition);
    const auto position = clamped(snapped({centre.x - spec.width / 2, centre.y - spec.height / 2}), spec);
    auto* widget = create({kind, size, position.x, position.y});
    if (!widget)
        return;
    widget->setEditing(m_editing);
    updateBlur();
    if (onChanged)
        onChanged();
}

void WidgetLayer::setGalleryBlurRect(ui::RectF rect)
{
    if (m_galleryBlur == rect)
        return;
    m_galleryBlur = rect;
    updateBlur();
}

void WidgetLayer::raiseWidget(DesktopWidget* widget)
{
    if (m_flow) { raiseChild(widget); return; }
    const auto found = std::find(m_widgets.begin(), m_widgets.end(), widget);
    if (found == m_widgets.end() || found + 1 == m_widgets.end())
        return;
    m_widgets.erase(found);
    m_widgets.push_back(widget);
    raiseChild(widget);
    // The order is what the layout stores, so a card raised by hand is saved.
    if (onChanged)
        onChanged();
}

void WidgetLayer::removeWidget(DesktopWidget* widget)
{
    const auto found = std::find(m_widgets.begin(), m_widgets.end(), widget);
    if (found == m_widgets.end())
        return;

    // Remove from list immediately so it is not returned by placements() etc.
    m_widgets.erase(found);

    // Menu and edit-button callers defer removal until input dispatch ends.
    // Do not retain widget pointers in animations after a host is hidden.
    takeChild(widget);
    layout();
    updateBlur();
    if (onChanged) onChanged();

}

void WidgetLayer::removeWidgetDeferred(DesktopWidget* widget)
{
    auto* loop=base::EventLoop::current();
    if(!loop){removeWidget(widget);return;}
    auto alive=m_alive;
    loop->post([this,widget,alive]{if(*alive)removeWidget(widget);});
}

void WidgetLayer::setPlacements(const std::vector<WidgetPlacement>& placements)
{
    m_menu.reset();
    for(auto* widget:m_widgets)takeChild(widget);
    m_widgets.clear();bool assignedIds=false;
    for(const auto& placement:placements){create(placement);assignedIds=assignedIds||placement.id.empty();}
    for(auto* widget:m_widgets)widget->setEditing(m_editing);
    layout();updateBlur();
    if(assignedIds&&onChanged)onChanged();
}

std::vector<WidgetPlacement> WidgetLayer::placements() const
{
    std::vector<WidgetPlacement> result;
    for(auto* widget:m_widgets)result.push_back(widget->placement());
    return result;
}

void WidgetLayer::layout()
{
    double x=0,y=0,rowHeight=0;
    for(auto* widget:m_widgets){
        const auto card=widget->geometry();
        if(m_flow){
            const auto size=widgetSizeSpec(widget->size());
            if(x>0&&x+size.width>geometry().width+.5){x=0;y+=rowHeight+kWidgetGap;rowHeight=0;}
            widget->setGeometry({x,y,size.width,size.height});x+=size.width+kWidgetGap;rowHeight=std::max(rowHeight,size.height);
        }else{
            const auto position=clamped(card.topLeft(),card.size());
            if(position!=card.topLeft())widget->setGeometry({position.x,position.y,card.width,card.height});
        }
    }
    updateBlur();
}

void WidgetLayer::setWidgetSize(DesktopWidget* widget,WidgetSize size)
{
    const auto spec=widgetSizeSpec(size);const auto position=clamped(widget->geometry().topLeft(),spec);
    widget->setSize(size);widget->setGeometry({position.x,position.y,spec.width,spec.height});
    layout();updateBlur();if(onChanged)onChanged();
}

void WidgetLayer::moveWidget(DesktopWidget* widget,ui::PointF position,bool finished)
{
    const auto card=widget->geometry();
    if(m_flow){
        widget->setGeometry({position.x,position.y,card.width,card.height});
        if(!finished)return;
        std::stable_sort(m_widgets.begin(),m_widgets.end(),[](auto* a,auto* b){
            const double ay=a->geometry().y+a->geometry().height/2,by=b->geometry().y+b->geometry().height/2;
            return ay==by?a->geometry().x<b->geometry().x:ay<by;
        });
        layout();if(onChanged)onChanged();return;
    }
    auto where=clamped(position,card.size());if(finished)where=clamped(snapped(where),card.size());
    widget->setGeometry({where.x,where.y,card.width,card.height});updateBlur();if(finished&&onChanged)onChanged();
}

void WidgetLayer::showWidgetMenu(DesktopWidget* widget,ui::PointF windowPosition)
{
    if(!m_window)return;
    m_menu=std::make_unique<ui::Menu>();auto* menu=m_menu.get();
    menu->addItem(tr("Remove Widget"),[this,widget]{removeWidgetDeferred(widget);});
    if(widget->configurable())menu->addItem(tr("Widget Settings…"),[widget]{widget->configure();});
    auto* sizes=menu->addSubmenu(tr("Widget Size"));
    for(auto size:{WidgetSize::Small,WidgetSize::Medium,WidgetSize::Large}){
        const char* label=size==WidgetSize::Small?tr("Small"):size==WidgetSize::Medium?tr("Medium"):tr("Large");
        const auto index=sizes->addItem(label,[this,widget,size]{setWidgetSize(widget,size);});sizes->setItemChecked(index,widget->size()==size);
    }
    menu->addSeparator();menu->addItem(tr("Edit Widgets…"),[this]{if(onEditWidgets)onEditWidgets();});
    menu->popup(m_window,windowPosition);
}

void WidgetLayer::updateBlur()
{
    if(!m_window||m_flow)return;
    const bool gallery=m_galleryBlur.width>0&&m_galleryBlur.height>0;
    const bool cards=isVisible()&&!m_widgets.empty();
    auto clear=[this]{m_window->setBackgroundGlass(std::nullopt);m_window->setBackgroundBlur({},0);m_window->setBackgroundBlurShape(ui::Path());};
    if((!gallery&&!cards)||!m_window->backgroundGlassAvailable()){clear();return;}
    ui::Path shape;auto bounds=gallery?m_galleryBlur:m_widgets.front()->blurRect();
    if(gallery)shape.addRoundedRect(m_galleryBlur,24,24);
    if(cards)for(auto* widget:m_widgets){
        const auto card=widget->blurRect();shape.addRoundedRect(card,kWidgetRadius,kWidgetRadius);
        const double right=std::max(bounds.right(),card.right()),bottom=std::max(bounds.bottom(),card.bottom());
        bounds.x=std::min(bounds.x,card.x);bounds.y=std::min(bounds.y,card.y);bounds.width=right-bounds.x;bounds.height=bottom-bounds.y;
    }
    bounds=bounds.adjusted(-48,-48,48,48);const auto area=localRect();const auto origin=mapToWindow(area.topLeft());
    const ui::RectF windowArea{origin.x,origin.y,area.width,area.height};
    const double right=std::min(bounds.right(),windowArea.right()),bottom=std::min(bounds.bottom(),windowArea.bottom());
    bounds.x=std::max(bounds.x,windowArea.x);bounds.y=std::max(bounds.y,windowArea.y);bounds.width=std::max(0.0,right-bounds.x);bounds.height=std::max(0.0,bottom-bounds.y);
    if(bounds.width<=0||bounds.height<=0){clear();return;}
    auto glass=ui::popupGlass(ui::Theme::current().darkMode);glass.tintAlpha=.28;glass.blurSigma=12;glass.displacement=8;
    m_window->setBackgroundGlass(glass);m_window->setTranslucentSubpixelText(false);m_window->setBackgroundBlur(bounds,0);m_window->setBackgroundBlurShape(shape);
}

void WidgetLayer::retranslate()
{
    for(auto* widget:m_widgets)widget->retranslate();
}
} // namespace fm
