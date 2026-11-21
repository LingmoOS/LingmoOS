// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/color_provider.h"
#include "ui/segmented_control.h"

#include <algorithm>
#include <chrono>
#include <cmath>

#include "base/event_loop.h"
#include "ui/application.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/theme.h"

namespace Lingmo::ui {

// 32 high inside a padding of 2, the plate 2 inside its button. A tab bar: a
// tab button high, a medium icon and a small spacing, without padding.
static constexpr double kSegmentedHeight = 32;
static constexpr double kTabHeight = 38;
static constexpr double kSegmentedPadding = 2;
static constexpr double kPlateInset = 2;
static constexpr std::chrono::milliseconds kPressDelay{50};

class SegmentedControl::Segment : public View
{
public:
    Segment(SegmentedControl* control, std::string text)
        : m_control(control)
        , m_label(addChild<Label>(std::move(text)))
    {
        // The label is plain text, coloured rather than faded.
        m_label->setFadesWhenDisabled(false);
        m_label->setTextAlignment(Label::TextAlignment::Center);
        m_label->setVerticallyCentered(true);
    }

    Label* label() const { return m_label; }
    bool acceptsFocus() const override { return true; }

protected:
    void layout() override { m_label->setGeometry(localRect()); }

    bool keyPress(const KeyEvent& event) override
    {
        if (event.key != key::Space || event.modifiers != NoModifier)
            return false;
        m_control->setPressed(index());
        return true;
    }

    bool keyRelease(const KeyEvent& event) override
    {
        if (event.key != key::Space || m_control->m_pressed != index())
            return false;
        m_control->setPressed(-1);
        m_control->pick(index());
        return true;
    }

    void focusOut() override
    {
        if (m_control->m_pressed == index())
            m_control->setPressed(-1);
    }

private:
    int index() const
    {
        const auto& segments = m_control->m_segments;
        return int(std::find(segments.begin(), segments.end(), this) - segments.begin());
    }

    SegmentedControl* m_control;
    Label* m_label;
};

SegmentedControl::SegmentedControl(std::vector<std::string> items, Style style)
    : m_style(style)
    , m_contentHeight(style == Style::Segmented ? kSegmentedHeight : kTabHeight)
{
    setItems(std::move(items));
}

SegmentedControl::~SegmentedControl()
{
    if (m_pressTimer)
        Application::instance()->loop().stopTimer(m_pressTimer);
}

void SegmentedControl::setItems(std::vector<std::string> items)
{
    for (Segment* segment : m_segments)
        takeChild(segment);
    m_segments.clear();
    for (std::string& item : items) {
        Segment* segment = addChild<Segment>(this, std::move(item));
        segment->label()->setFontSize(m_fontSize);
        m_segments.push_back(segment);
    }
    m_current = std::clamp(m_current, 0, std::max(0, int(m_segments.size()) - 1));
    m_pressed = -1;
    updateColors();
    requestLayout();
}

void SegmentedControl::setCurrentIndex(int index)
{
    if (index < 0 || index >= int(m_segments.size()) || index == m_current)
        return;
    m_current = index;
    requestPaint();
}

void SegmentedControl::setContentHeight(double height)
{
    if (height == m_contentHeight)
        return;
    m_contentHeight = height;
    requestLayout();
}

void SegmentedControl::setFontSize(double size)
{
    if (size == m_fontSize)
        return;
    m_fontSize = size;
    for (Segment* segment : m_segments)
        segment->label()->setFontSize(size);
    requestLayout();
}

void SegmentedControl::setTrackColor(std::optional<ColorRecipe> color)
{
    m_trackColor = std::move(color);
    requestPaint();
}

double SegmentedControl::padding() const
{
    return m_style == Style::Segmented ? kSegmentedPadding : 0;
}

double SegmentedControl::implicitItemWidth(size_t index) const
{
    return index < m_segments.size() ? m_segments[index]->label()->sizeHint().width : 0;
}

SizeF SegmentedControl::sizeHint() const
{
    double width = 0;
    for (size_t i = 0; i < m_segments.size(); ++i)
        width += implicitItemWidth(i);
    return {width + 2 * padding() + Theme::current().smallSpacing, m_contentHeight + 2 * padding()};
}

// Buttons without a width of their own get equal shares.
RectF SegmentedControl::itemRect(size_t index) const
{
    const RectF inner = localRect().adjusted(padding(), padding(), -padding(), -padding());
    const double width = m_segments.empty() ? 0 : inner.width / double(m_segments.size());
    return {inner.x + width * double(index), inner.y, width, inner.height};
}

int SegmentedControl::itemAt(PointF point) const
{
    for (size_t i = 0; i < m_segments.size(); ++i) {
        if (itemRect(i).contains(point))
            return int(i);
    }
    return -1;
}

void SegmentedControl::layout()
{
    for (size_t i = 0; i < m_segments.size(); ++i)
        m_segments[i]->setGeometry(itemRect(i));
}

void SegmentedControl::paint(Painter& painter)
{
    const Theme& theme = Theme::current();
    const RectF bounds = localRect();
    const Color plate = color(ColorId::SegmentedControlPlate);
    const double tabRadius = theme.smallRadius + 2;
    if (m_style == Style::Segmented) {
        painter.drawRoundedRect(bounds, {bounds.height / 2},
                                m_trackColor ? m_trackColor->resolve() : color(ColorId::SegmentedControlTrack));
    } else {
        painter.drawRoundedRect(bounds, {tabRadius, 0, themeCorners()}, color(ColorId::SegmentedTabBackground));
    }
    if (m_current < 0 || m_current >= int(m_segments.size()))
        return;

    const RectF item = itemRect(size_t(m_current));
    if (m_style == Style::Segmented) {
        // A plain rectangle with a drop shadow: Gaussian deviation (radius + 1) / 3.33.
        const RectF highlight = item.adjusted(kPlateInset, kPlateInset, -kPlateInset, -kPlateInset);
        painter.drawShadow(highlight, highlight.height / 2, 1.2, color(ColorId::SegmentedControlPlateShadow),
                           {0, 1});
        painter.drawSmoothRectangle(highlight, highlight.height / 2, plate);
    } else {
        // Concentric with the bar through the inset.
        const Color shadow = color(ColorId::SegmentedTabPlateShadow);
        if (shadow.a > 0)
            painter.drawShadow(item.adjusted(kPlateInset, kPlateInset, -kPlateInset, -kPlateInset), theme.smallRadius,
                               0.9, shadow);
        painter.drawRoundedRect(item.adjusted(kPlateInset, kPlateInset, -kPlateInset, -kPlateInset),
                                {tabRadius, kPlateInset, themeCorners()}, plate);
    }
}

void SegmentedControl::enabledChanged()
{
    updateColors();
}

void SegmentedControl::updateColors()
{
    for (size_t i = 0; i < m_segments.size(); ++i) {
        // The disabled text colour, half the text colour while pressed.
        Label* label = m_segments[i]->label();
        if (!isEnabled())
            label->setColor(ColorId::DisabledText);
        else if (int(i) == m_pressed && m_pressShown)
            label->setColor(ColorId::SegmentedControlTextPressed);
        else
            label->setColor(std::nullopt);
    }
}

void SegmentedControl::setPressed(int index)
{
    if (index == m_pressed)
        return;
    m_pressed = index;
    // A quick click shows no pressed label; a held press does.
    m_pressShown = false;
    if (m_pressTimer) {
        Application::instance()->loop().stopTimer(m_pressTimer);
        m_pressTimer = 0;
    }
    if (index >= 0) {
        m_pressTimer = Application::instance()->loop().startTimer(kPressDelay, false, [this] {
            m_pressTimer = 0;
            m_pressShown = true;
            updateColors();
        });
    }
    updateColors();
}

void SegmentedControl::pick(int index)
{
    if (index < 0 || index >= int(m_segments.size()) || index == m_current)
        return;
    m_current = index;
    requestPaint();
    if (onCurrentChanged)
        onCurrentChanged(index);
}

void SegmentedControl::pointerPress(const PointerEvent& event)
{
    if (event.button != MouseButton::Left)
        return;
    const int index = itemAt(event.position);
    if (index >= 0)
        m_segments[size_t(index)]->setFocus(FocusReason::Mouse);
    setPressed(index);
}

void SegmentedControl::pointerRelease(const PointerEvent& event)
{
    const int pressed = m_pressed;
    setPressed(-1);
    if (pressed >= 0 && itemAt(event.position) == pressed)
        pick(pressed);
}

} // namespace Lingmo::ui
