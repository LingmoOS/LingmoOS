// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "components.h"

#include "ui/color_provider.h"
#include "ui/theme.h"

#include <algorithm>
#include <cmath>

using namespace Lingmo::ui;

static constexpr double kRowHeight = 44;
static constexpr double kInset = 16;
static constexpr double kSpacing = 12;

Color tint(Color base, Color overlay)
{
    const double alpha = overlay.a / 255.0;
    const auto mix = [alpha](uint8_t from, uint8_t to) { return uint8_t(std::lround(from + (to - from) * alpha)); };
    return {mix(base.r, overlay.r), mix(base.g, overlay.g), mix(base.b, overlay.b), base.a};
}

Label* addLabel(View* parent, std::string text, double pointSize, int weight, std::optional<Color> color)
{
    Label* label = parent->addChild<Label>(std::move(text));
    if (pointSize > 0)
        label->setFontSize(points(pointSize));
    label->setWeight(weight);
    label->setColor(color);
    return label;
}

Plate::Plate(double radius)
    : m_radius(radius)
{
}

void Plate::setRadius(double radius)
{
    m_radius = radius;
    requestPaint();
}

void Plate::setColor(std::optional<Color> color)
{
    m_color = color;
    requestPaint();
}

void Plate::setContinuousCorners(std::optional<bool> continuous)
{
    m_continuous = continuous;
    requestPaint();
}

Corners Plate::corners() const
{
    return m_continuous.value_or(Theme::current().continuousCorners) ? Corners::Continuous : Corners::Circular;
}

void Plate::paint(Painter& painter)
{
    painter.drawRoundedRect(localRect(), {m_radius, 0, corners()}, m_color.value_or(Theme::current().secondBackground));
}

void paintSeparator(Painter& painter, double x, double bottom, double width)
{
    // FishUI's Separator: a RoundedRectangle 1 / Dpi.ratio high on the bottom edge, snapped as PixelGrid does.
    const double height = 1 / painter.scale();
    painter.drawRoundedRect({x, bottom - height, width, height}, {0, 0, themeCorners()}, color(ColorId::Separator));
}

InfoRow::InfoRow(std::string text, std::string value)
    : m_row(addChild<RowLayout>())
    , m_text(m_row->addChild<Label>(std::move(text)))
    , m_value(m_row->addChild<Label>(std::move(value)))
{
    m_row->setSpacing(kSpacing);
    m_text->setLayoutHints({.fillWidth = true});
    m_value->setTextAlignment(Label::TextAlignment::Right);
    m_value->setColor(Theme::current().disabledText);
}

void InfoRow::setText(std::string text)
{
    if (text == m_text->text())
        return;
    m_text->setText(std::move(text));
    requestLayout();
}

void InfoRow::setValue(std::string value)
{
    if (value == m_value->text())
        return;
    m_value->setText(std::move(value));
    requestLayout();
}

void InfoRow::setSeparatorVisible(bool visible)
{
    if (m_separator == visible)
        return;
    m_separator = visible;
    requestPaint();
}

void InfoRow::layout()
{
    // Long values such as a hardware address would otherwise squeeze the label out.
    m_value->setLayoutHints({.maximumWidth = std::round(geometry().width * 0.65)});
    m_value->setColor(Theme::current().disabledText);
    m_row->setGeometry(localRect().adjusted(kInset, 0, -kInset, 0));
}

void InfoRow::paint(Painter& painter)
{
    if (m_separator)
        paintSeparator(painter, kInset, geometry().height, geometry().width - 2 * kInset);
}

ListCard::ListCard()
    : Plate(Theme::current().bigRadius)
{
}

InfoRow* ListCard::addRow(std::string text, std::string value)
{
    InfoRow* row = addChild<InfoRow>(std::move(text), std::move(value));
    updateSeparators();
    return row;
}

void ListCard::updateSeparators()
{
    InfoRow* last = nullptr;
    for (const auto& child : children()) {
        auto* row = dynamic_cast<InfoRow*>(child.get());
        if (!row || !row->isVisible())
            continue;
        row->setSeparatorVisible(true);
        last = row;
    }
    if (last)
        last->setSeparatorVisible(false);
    requestLayout();
}

SizeF ListCard::sizeHint() const
{
    return {0, heightForWidth(0)};
}

double ListCard::heightForWidth(double width) const
{
    double height = 0;
    for (const auto& child : children()) {
        if (child->isVisible())
            height += child->layoutHints().preferredHeight >= 0 ? child->layoutHints().preferredHeight
                                                                : child->heightForWidth(width);
    }
    return height;
}

void ListCard::layout()
{
    double y = 0;
    const double width = geometry().width;
    for (const auto& child : children()) {
        if (!child->isVisible())
            continue;
        const double height = child->layoutHints().preferredHeight >= 0 ? child->layoutHints().preferredHeight
                                                                        : child->heightForWidth(width);
        child->setGeometry({0, y, width, height});
        y += height;
    }
}

SectionLabel::SectionLabel(std::string text)
    : m_label(addChild<Label>(std::move(text)))
{
    m_label->setColor(Theme::current().disabledText);
}

void SectionLabel::setText(std::string text)
{
    if (text == m_label->text())
        return;
    m_label->setText(std::move(text));
    requestLayout();
}

void SectionLabel::setElide(Label::Elide elide)
{
    m_label->setElide(elide);
}

SizeF SectionLabel::sizeHint() const
{
    const SizeF label = m_label->sizeHint();
    return {label.width + 2 * kInset, label.height};
}

double SectionLabel::heightForWidth(double) const
{
    return m_label->sizeHint().height;
}

void SectionLabel::layout()
{
    m_label->setColor(Theme::current().disabledText);
    m_label->setGeometry(localRect().adjusted(kInset, 0, -kInset, 0));
}
