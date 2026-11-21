// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgets/colors.h"
#include "widgets/listrow.h"

#include <algorithm>
#include <chrono>
#include <cmath>

#include "ui/animation.h"
#include "ui/color_provider.h"
#include "ui/icon.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/list_view.h"
#include "ui/painter.h"
#include "ui/switch.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/metrics.h"

using namespace Lingmo::ui;

namespace settings {

// Holds the details panel and shows as much of it as the row has unfolded.
class DetailsClip : public View
{
public:
    DetailsClip() { setClipsChildren(true); }

protected:
    void layout() override
    {
        if (children().empty())
            return;
        View* content = children().front().get();
        content->setGeometry({0, 0, geometry().width, content->heightForWidth(geometry().width)});
    }
};

ListRow::ListRow(std::string text)
    : m_text(std::move(text))
    , m_minimumRowHeight(metrics::rowHeight)
{
    m_row = addChild<RowLayout>();
    m_row->setSpacing(metrics::rowSpacing);
    m_leadingBox = m_row->addChild<Row>();
    m_leadingBox->setVisible(false);
    m_label = m_row->addChild<Label>(m_text);
    // A window as narrow as half a screen wraps a long label beside its
    // control rather than cutting it.
    m_label->setWrap(true);
    m_label->setMaximumLineCount(2);
    m_label->setVisible(!m_text.empty());
    updateLabelHints();
    setLayoutHints({.fillWidth = true});
    setTracksHover(true);
}

ListRow::~ListRow() = default;

void ListRow::hoverChanged()
{
    if (onHoveredChanged)
        onHoveredChanged();
}

void ListRow::updateLabelHints()
{
    LayoutHints hints{.fillWidth = true};
    hints.margins.bottom = m_labelBottomMargin;
    m_label->setLayoutHints(hints);
}

void ListRow::setText(std::string text)
{
    if (text == m_text)
        return;
    m_text = std::move(text);
    m_label->setText(m_text);
    m_label->setVisible(!m_text.empty());
}

void ListRow::setContentSpacing(double spacing)
{
    m_row->setSpacing(spacing);
}

void ListRow::setLeadingView(std::unique_ptr<View> view)
{
    if (m_leading)
        m_leadingBox->takeChild(m_leading);
    m_leading = view.get();
    m_leadingBox->setVisible(m_leading != nullptr);
    if (m_leading)
        m_leadingBox->addChild(std::move(view));
}

ColumnLayout* ListRow::details()
{
    if (!m_details) {
        m_detailsClip = addChild<DetailsClip>();
        m_details = m_detailsClip->addChild<ColumnLayout>();
    }
    return m_details;
}

void ListRow::setExpanded(bool expanded)
{
    if (expanded == m_expanded)
        return;
    m_expanded = expanded;
    if (!window()) {
        m_unfolded = expanded ? 1 : 0;
        requestLayout();
        return;
    }
    if (!m_detailsAnimation) {
        m_detailsAnimation = std::make_unique<Animation>(window());
        m_detailsAnimation->setDuration(std::chrono::milliseconds(200));
        m_detailsAnimation->setEasing(Animation::Easing::OutSine);
        m_detailsAnimation->onUpdate = [this](double value) {
            m_unfolded = m_unfoldedFrom + ((m_expanded ? 1 : 0) - m_unfoldedFrom) * value;
            requestLayout();
        };
    }
    m_unfoldedFrom = m_unfolded;
    m_detailsAnimation->start();
}

void ListRow::setInteractive(bool interactive)
{
    m_interactive = interactive;
    if (!interactive)
        setPressed(false);
}

void ListRow::setSeparatorVisible(bool visible)
{
    m_separatorVisible = visible;
    requestPaint();
}

void ListRow::setMinimumRowHeight(double height)
{
    m_minimumRowHeight = height;
    requestLayout();
}

void ListRow::setLabelBottomMargin(double margin)
{
    m_labelBottomMargin = margin;
    updateLabelHints();
}

bool ListRow::isHighlighted() const
{
    return m_interactive && m_highlightOnPress && isEnabled() && m_pressed;
}

double ListRow::rowHeightForWidth(double width) const
{
    const double content = std::max(0.0, width - 2 * metrics::cardHorizontalPadding);
    return std::max(m_minimumRowHeight, m_row->heightForWidth(content) + metrics::cardVerticalPadding);
}

double ListRow::rowHeight() const
{
    return rowHeightForWidth(geometry().width);
}

double ListRow::detailsHeight(double width) const
{
    if (!m_details || m_unfolded <= 0)
        return 0;
    return m_details->heightForWidth(width - 2 * metrics::cardHorizontalPadding) * m_unfolded;
}

SizeF ListRow::sizeHint() const
{
    const SizeF row = m_row->sizeHint();
    return {row.width + 2 * metrics::cardHorizontalPadding,
            std::max(m_minimumRowHeight, row.height + metrics::cardVerticalPadding)};
}

double ListRow::heightForWidth(double width) const
{
    return rowHeightForWidth(width) + detailsHeight(width);
}

// The last item's Layout.rightMargin, when it reaches past the content edge
// so its drawing ends where a switch does.
void ListRow::layout()
{
    for (auto it = m_row->children().rbegin(); it != m_row->children().rend(); ++it) {
        if (!(*it)->isVisible())
            continue;
        if (auto* trailing = dynamic_cast<const TrailingOverhang*>(it->get())) {
            LayoutHints hints = (*it)->layoutHints();
            if (hints.margins.right != -trailing->trailingOverhang()) {
                hints.margins.right = -trailing->trailingOverhang();
                (*it)->setLayoutHints(hints);
            }
        }
        break;
    }
    // On top of the label's own fade while disabled, as the Qt version's row did.
    const Theme& theme = Theme::current();
    m_label->setColor(isEnabled() ? theme.text : theme.disabledText);
    const double width = geometry().width;
    const double rowHeight = rowHeightForWidth(width);
    m_row->setGeometry({metrics::cardHorizontalPadding, 0,
                        std::max(0.0, width - 2 * metrics::cardHorizontalPadding), rowHeight});
    if (m_detailsClip) {
        m_detailsClip->setVisible(m_unfolded > 0);
        m_detailsClip->setGeometry({metrics::cardHorizontalPadding, rowHeight,
                                    std::max(0.0, width - 2 * metrics::cardHorizontalPadding),
                                    detailsHeight(width)});
    }
}

void ListRow::setViewIndex(ListView* view, int index)
{
    m_view = view;
    m_viewIndex = index;
    requestPaint();
}

ListRow::Neighbours ListRow::neighbours() const
{
    Neighbours result;
    if (m_view && m_viewIndex >= 0) {
        const int count = int(m_view->rowCount());
        result.first = m_viewIndex == 0;
        result.last = m_viewIndex >= count - 1;
        if (!result.first)
            result.previous = dynamic_cast<ListRow*>(m_view->itemForRow(size_t(m_viewIndex - 1)));
        if (!result.last)
            result.next = dynamic_cast<ListRow*>(m_view->itemForRow(size_t(m_viewIndex + 1)));
        return result;
    }
    if (!parent())
        return result;
    bool seen = false;
    for (const auto& child : parent()->children()) {
        auto* row = dynamic_cast<ListRow*>(child.get());
        if (!row || !row->isVisible())
            continue;
        if (row == this) {
            seen = true;
            continue;
        }
        if (!seen) {
            result.first = false;
            result.previous = row;
        } else {
            result.last = false;
            result.next = row;
            break;
        }
    }
    return result;
}

void ListRow::paint(Painter& painter)
{
    const Theme& theme = Theme::current();
    const Neighbours around = neighbours();
    const double width = geometry().width;
    const double rowHeight = m_row->geometry().height;
    const double unfolded = m_detailsClip ? m_detailsClip->geometry().height : 0;

    // One shape only: overlapping translucent pieces would darken where they
    // meet. Corners that meet a neighbouring row, or the row's own unfolded
    // details, are pushed outside the clip.
    if (isHighlighted()) {
        const double radius = theme.bigRadius;
        const double top = around.first ? 0 : -radius;
        const double bottom = around.last && !m_expanded ? rowHeight : rowHeight + radius;
        painter.save();
        painter.clip({0, 0, width, rowHeight});
        painter.drawRoundedRect({0, top, width, bottom - top}, {radius, 0, themeCorners()},
                                color(SettingsColor::RowHighlight));
        painter.restore();
    }

    // A highlight hides the separators it touches, as the translucent fill
    // would otherwise show them through.
    if (m_separatorVisible && !around.last && !(isHighlighted() && unfolded == 0)
        && !(around.next && around.next->isHighlighted())) {
        const double height = geometry().height;
        // One device pixel at any scale, snapped as FishUI's RoundedRectangle.
        const double pixel = 1 / painter.scale();
        painter.drawRoundedRect({metrics::cardHorizontalPadding, height - pixel,
                                 width - 2 * metrics::cardHorizontalPadding, pixel},
                                {}, color(ColorId::Separator));
    }
}

void ListRow::enabledChanged()
{
    requestLayout();
}

void ListRow::setPressed(bool pressed)
{
    if (pressed == m_pressed)
        return;
    m_pressed = pressed;
    requestPaint();
    // The separator above belongs to the previous row.
    if (ListRow* previous = neighbours().previous)
        previous->requestPaint();
}

void ListRow::pointerPress(const PointerEvent& event)
{
    if (event.button != MouseButton::Left || event.position.y >= m_row->geometry().height)
        return;
    setPressed(true);
}

void ListRow::pointerRelease(const PointerEvent& event)
{
    if (!m_pressed)
        return;
    setPressed(false);
    if (RectF{0, 0, geometry().width, m_row->geometry().height}.contains(event.position)) {
        setFocus(FocusReason::Mouse);
        if (onClicked)
            onClicked();
    }
}

bool ListRow::keyPress(const KeyEvent& event)
{
    if (!m_interactive)
        return false;
    if (event.key == key::Space || event.key == key::Return || event.key == key::KeypadEnter) {
        if (onClicked)
            onClicked();
        return true;
    }
    return false;
}

void setTrailingSwitchPadding(Switch* toggle)
{
    toggle->setPadding({8, 0, 0, 0});
}

SwitchRow::SwitchRow(std::string text)
    : ListRow(std::move(text))
{
    setInteractive(false);
    m_switch = addContent<Switch>();
    setTrailingSwitchPadding(m_switch);
    m_switch->onToggled = [this](bool checked) {
        if (onToggled)
            onToggled(checked);
    };
}

void SwitchRow::setChecked(bool checked)
{
    m_switch->setChecked(checked);
}

bool SwitchRow::isChecked() const
{
    return m_switch->isChecked();
}

InfoRow::InfoRow(std::string text, std::string value)
    : ListRow(std::move(text))
{
    setInteractive(false);
    m_value = addContent<Label>();
    m_value->setColor(Theme::current().disabledText);
    m_value->setTextAlignment(Label::TextAlignment::Right);
    m_value->setWrap(true);
    m_value->setMaximumLineCount(2);
    setValue(std::move(value));
}

void InfoRow::setValue(std::string value)
{
    setVisible(!value.empty());
    m_value->setText(std::move(value));
}

void InfoRow::layout()
{
    // Long values such as a processor name would otherwise squeeze the label out.
    m_value->setColor(Theme::current().disabledText);
    const double cap = std::round(geometry().width * 0.65);
    if (m_value->layoutHints().maximumWidth != cap)
        m_value->setLayoutHints({.maximumWidth = cap});
    ListRow::layout();
}

TrailingIcon::TrailingIcon(std::string name, double size)
    : m_size(size)
    , m_overhang(-metrics::trailingGlyphMargin(name, size))
{
    m_icon = addChild<Icon>(std::move(name), size);
}

void TrailingIcon::layout()
{
    m_icon->setGeometry({0, (geometry().height - m_size) / 2, m_size, m_size});
}

NavigationItem::NavigationItem(std::string text)
    : ListRow(std::move(text))
{
    m_value = addContent<Label>();
    m_value->setVisible(false);
    TrailingIcon* chevron = addContent<TrailingIcon>("chevron-right", 16);
    chevron->setLayoutHints({.preferredWidth = 16, .preferredHeight = 16});
    chevron->icon()->setOpacity(0.6);
}

void NavigationItem::setValue(std::string value)
{
    m_value->setVisible(!value.empty());
    m_value->setText(std::move(value));
}

void NavigationItem::layout()
{
    m_value->setColor(Theme::current().disabledText);
    const double cap = geometry().width / 2;
    if (m_value->layoutHints().maximumWidth != cap)
        m_value->setLayoutHints({.maximumWidth = cap});
    ListRow::layout();
}

} // namespace settings
